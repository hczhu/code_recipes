#include "LogisticReg.h"

#include <armadillo>
#include <folly/gen/Base.h>
#include <folly/gen/String.h>
#include <glog/logging.h>

DEFINE_int32(log_every_n, 1000, "Log logloss every N epoch.");
DEFINE_int32(num_epoch, 0, "Number of epochs");

namespace mlight {

std::vector<double> fitLR(const std::vector<std::vector<double>>& X,
                          const std::vector<int>& Y, Options options,
                          const std::vector<double>& W) {
  CHECK(std::all_of(Y.begin(), Y.end(), [](auto y) {
    return y == -1 || y == 1;
  })) << "All Ys must be 1 or -1";
  if (options.learningRate * 2 * options.L2 > 1) {
    auto newLr = 0.5 / options.L2 * 0.95;
    LOG(WARNING) << "The learning rate " << options.learningRate
                 << " is too big because of a big L2 " << options.L2
                 << ". Adjusted it to " << newLr;
    options.learningRate = newLr;
  }
  auto sigmoid = [](double z) {
    constexpr int kCutoff = 100;
    constexpr double eps = 1e-50;
    if (abs(z) > kCutoff) {
      return z > 0 ? (1.0 - eps) : eps;
    }
    return 1.0 / (1 + exp(-z));
  };
  const int n = options.miniBatchSize;
  const int m = X[0].size();
  arma::vec theta(m + 1, arma::fill::zeros);
  if (options.randomInit) {
    theta.randn();
  }
  arma::Mat<double> X1(n, m + 1, arma::fill::ones);
  arma::Col<int> vY(options.miniBatchSize);
  auto predProb = [&] {
    // Return P{ y = vY[i] | X1[i] }
    // The distance between the origin and the projected point of 'X1[i]'
    // on the vector of 'theta'.
    arma::vec margin = (X1 * theta) % vY;
    margin.for_each([&](double& val) { val = sigmoid(val); });
    return margin;
  };
  auto loglossAndError = [&]() -> std::pair<double, int> {
    double res = 0;
    const arma::vec theta1 = theta(arma::span(0, m - 1));
    int error = 0;
    for (int i = 0; i < X.size(); ++i) {
      auto margin = Y[i] * (arma::dot(arma::vec(X[i]), theta1) + theta(m));
      if (margin < 0) {
        ++error;
      }
      res += log(sigmoid(margin));
    }
    return {-res / X.size() +
                0.5 * options.L2 * arma::norm(theta1) * arma::norm(theta1),
            error};
  };
  auto checkGrad = [&](const arma::vec& dtheta) {
    constexpr double eps = 1e-8;
    auto ll = loglossAndError().first;
    for (int idx = 0; idx < m + 1; ++idx) {
      theta(idx) += eps;
      auto di = (loglossAndError().first - ll) / eps;
      theta(idx) -= eps;
      CHECK(abs(di - dtheta(idx)) < 1e-6 || abs(di - dtheta(idx)) / di < 1e-6)
          << di << " v.s. " << dtheta(idx);
    }
    return true;
  };
  int numBatches =
      (X.size() + options.miniBatchSize - 1) / options.miniBatchSize;
  decltype(theta) momentum = theta * 0;
  double best = 1e20;
  decltype(theta) bestTheta = theta;
  int bestEpoch = 0;
  CHECK(1 == numBatches || !options.useNewton)
      << "Newton optimizatio doesn't work with mini batching.";
  if (FLAGS_num_epoch > 0) {
    options.numEpoch = FLAGS_num_epoch;
  }
  ResCode resCode = ResCode::NOT_CONVERGED;
  const arma::vec l2Diag = [&] {
    arma::vec l2Diag(m + 1);
    l2Diag.fill(options.L2);
    l2Diag(m) = 0;
    return l2Diag;
  }();
  for (int epoch = 0;
       epoch < options.numEpoch && resCode == ResCode::NOT_CONVERGED; ++epoch) {
    auto prevTheta = theta;
    for (int batch = 0; batch < numBatches; ++batch) {
      // Prepare data for this batch
      if (epoch == 0 || numBatches > 1) {
        for (int i = 0; i < n; ++i) {
          int instanceId = (n * batch + i) % Y.size();
          vY(i) = Y[instanceId];
          for (int j = 0; j < m; ++j) {
            X1(i, j) = X[instanceId][j];
          }
        }
      }
      const auto probs = predProb();
      // Minimize log-loss:
      //  -Sigma(log P{ vY[i] | X1[i] }) / n
      //   + L2 / 2 * norm(theta) * norm(theta) (excluding the intercep)
      double intercept = 0;
      std::swap(intercept, theta(m));
      arma::vec dtheta =
          ((((probs - 1.0) % vY).t() * X1) / n).t() + (options.L2 * theta);
      std::swap(intercept, theta(m));
      using namespace folly::gen;
      VLOG_EVERY_N(1, FLAGS_log_every_n)
          << (from(arma::conv_to<std::vector<double>>::from(dtheta)) |
              unsplit(','))
          << (n == X.size() && checkGrad(dtheta));
      if (options.useNewton) {
        dtheta =
            arma::inv((X1.t() * arma::diagmat(probs % (1 - probs) / n) * X1) +
                      arma::diagmat(l2Diag)) *
            dtheta;
      } else {
        momentum = momentum * options.momentumMultiplier +
                   dtheta * (1 - options.momentumMultiplier);
        dtheta = momentum * options.learningRate;
      }
      theta -= dtheta;
    }
    double ll;
    int er;
    std::tie(ll, er) = loglossAndError();
    if (options.chooseBestErrorRateTheta) {
      if (er <= best) {
        best = er;
        bestTheta = theta;
        bestEpoch = epoch;
      }
    } else if (ll <= best) {
      best = ll;
      bestTheta = theta;
      bestEpoch = epoch;
    }
    if (arma::norm(prevTheta - theta) < options.minThetaDiffNorm) {
      LOG(INFO) << "The model converged at epoch #" << epoch;
      resCode = ResCode::CONVERGED;
    }
    if (er == 0 && (options.stopIfZeroError || options.L2 == 0)) {
      LOG(INFO) << "Training data got classified perfectly. Exiting...";
      resCode = ResCode::EARLY_TERM;
    }
    LOG_IF(INFO,
           (0 == (epoch % FLAGS_log_every_n)) ||
               epoch + 1 == options.numEpoch ||
               resCode != ResCode::NOT_CONVERGED)
        << "Epoch #" << epoch << " learning rate = " << options.learningRate
        << " theta diff norm = " << arma::norm(prevTheta - theta)
        << " theta norm = " << arma::norm(theta)
        << " logloss (with L2=" << options.L2 << "): " << ll
        << " error rate = " << er;
    prevTheta = theta;
    options.learningRate *= options.lrDecay;
  }
  LOG(INFO) << "Best "
            << (options.chooseBestErrorRateTheta ? "#error = " : "logloss = ")
            << best << " at epoch #" << bestEpoch;
  return arma::conv_to<std::vector<double>>::from(
      (options.chooseBestLoglossTheta || options.chooseBestErrorRateTheta)
          ? bestTheta
          : theta);
}

}  // namespace mlight
