#include "../cpp/header.h"

using namespace mlight;

template <typename T>
arma::vec generateLinearData(
    std::vector<std::vector<double>>& X, std::vector<T>& Y,
    std::function<T(double, std::default_random_engine&)> transformer) {
  std::default_random_engine generator(FLAGS_seed);
  std::uniform_real_distribution<double> uniform(-1, 1);
  const int n = X.size();
  const int m = X[0].size();
  arma::vec theta(m + 1);
  for (auto& th : theta) {
    th = uniform(generator);
  }
  for (int i = 0; i < n;) {
    for (auto& x : X[i]) {
      x = uniform(generator);
    }
    try {
      Y[i] = transformer(
          arma::dot(theta(arma::span(0, m - 1)), arma::vec(X[i])) + theta(m),
          generator);
      ++i;
    } catch (...) {
    }
  }
  return theta;
}

TEST(PrototypesTest, LinearReg) {
  std::default_random_engine generator;
  std::uniform_real_distribution<double> uniform(-1, 1);
  std::normal_distribution<double> guass(0, 0.01);
  const int n = 100;
  const int m = 10;
  std::vector<std::vector<double>> X(n, std::vector<double>(m));
  std::vector<double> Y(n);
  arma::vec theta = generateLinearData<double>(
      X, Y, [&](double y, std::default_random_engine& generator) {
        return y; // + guass(generator);
      });
  auto estimate0 = fitLSM(X, Y);
  EXPECT_NEAR(0, arma::norm(theta - arma::vec(estimate0)), 1e-10);

  auto estimate1 = fitLSM(X, Y, 1);
  EXPECT_NEAR(0, arma::norm(theta - arma::vec(estimate1)), 1e-1);

  EXPECT_LT(arma::norm(arma::vec(estimate1)), arma::norm(arma::vec(estimate0)));
  auto prevEstimate = estimate1;
  for (double L2 = 2; L2 < 16; L2 += 4) {
    auto estimate = fitLSM(X, Y, L2);
    EXPECT_LE(arma::norm(arma::vec(estimate)),
              arma::norm(arma::vec(prevEstimate)));
    prevEstimate = std::move(estimate);
  }

  std::uniform_int_distribution<> intUniform(1, 100);
  std::vector<double> W(n);
  std::generate(W.begin(), W.end(), std::bind(intUniform, generator));
  std::vector<std::vector<double>> estimateWithW(5);
  for (int L2 = 0; L2 < estimateWithW.size(); ++L2) {
    estimateWithW[L2] = fitLSM(X, Y, L2, W);
  }
  for (int i = 0; i < n; ++i) {
    if (W[i] > 1) {
      auto x = X[i];
      X.push_back(std::move(x));
      Y.push_back(Y[i]);
      W[i] -= 1;
      while (W[i] > 0) {
        X.push_back(X.back());
        Y.push_back(Y[i]);
        W[i] -= 1;
      }
    }
  }
  EXPECT_NEAR(0, arma::norm(theta - arma::vec(estimateWithW[0])), 1e-10);
  for (int L2 = 0; L2 < estimateWithW.size(); ++L2) {
    auto estimateWithoutW = fitLSM(X, Y, L2);
    if (L2 == 0) {
      EXPECT_NEAR(0, arma::norm(theta - arma::vec(estimateWithoutW)), 1e-10);
    }
    EXPECT_NEAR(0, arma::norm(arma::vec(estimateWithoutW) -
                              arma::vec(estimateWithW[L2])),
                1e-4);
  }
}

TEST(PrototypesTest, LogisticReg) {
  const int n = 1000;
  const int m = 100;
  std::vector<std::vector<double>> X(n, std::vector<double>(m));
  std::vector<int> Y(n);
  const arma::vec theta = generateLinearData<int>(
      X, Y, [&](double y, std::default_random_engine& generator) {
        if (abs(y) < 1e-2) {
          throw std::runtime_error("too small");
        }
        return y > 0 ? 1 : -1;
      });
  for (int i = 0; i < n; ++i) {
    auto z = Y[i] * (arma::dot(arma::vec(X[i]), theta(arma::span(0, m - 1))) +
                     theta(m));
    EXPECT_GE(z, 0.0);
  }
  LOG(INFO) << "Testing SGD logistic reg.";
  {
    Options options;
    options.minThetaDiffNorm = 1e-8;
    options.stopIfZeroError = true;
    auto thetaHat = fitLR(X, Y, options);
    const arma::vec theta1(
        std::vector<double>(thetaHat.begin(), thetaHat.end() - 1));
    // Should be able to classify the training data perfectly.
    for (int i = 0; i < n; ++i) {
      auto z = Y[i] * (arma::dot(arma::vec(X[i]), theta1) + thetaHat[m]);
      EXPECT_GE(z, 0.0);
    }
  }
  LOG(INFO) << "Testing Newton logistic reg.";
  {
    Options options;
    options.useNewton = true;
    options.miniBatchSize = n;
    options.minThetaDiffNorm = 1e-8;
    options.stopIfZeroError = true;
    auto thetaHat = fitLR(X, Y, options);
    const arma::vec theta1(
        std::vector<double>(thetaHat.begin(), thetaHat.end() - 1));
    // Should be able to classify the training data perfectly.
    for (int i = 0; i < n; ++i) {
      auto z = Y[i] * (arma::dot(arma::vec(X[i]), theta1) + thetaHat[m]);
      EXPECT_GE(z, 0.0);
    }
  }
  LOG(INFO) << "Testing L2 SGD logistic reg.";
  {
    Options options;
    double prevNorm = 1e40;
    options.minThetaDiffNorm = 1e-6;
    for (int L2 = 1; L2 < 15; L2 += 4) {
      options.L2 = L2;
      auto norm = arma::norm(arma::vec(fitLR(X, Y, options)));
      EXPECT_LE(norm, prevNorm);
      prevNorm = norm;
    }
  }
  LOG(INFO) << "Testing L2 Newton logistic reg.";
  {
    Options options;
    options.useNewton = true;
    options.miniBatchSize = n;
    options.minThetaDiffNorm = 1e-6;
    double prevNorm = 1e40;
    for (int L2 = 1; L2 < 15; L2 += 4) {
      options.L2 = L2;
      auto norm = arma::norm(arma::vec(fitLR(X, Y, options)));
      EXPECT_LE(norm, prevNorm);
      prevNorm = norm;
    }
  }
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

