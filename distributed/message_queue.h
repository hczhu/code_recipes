#include <queue>
#include <condition_variable>
#include <mutex>
#include <unordered_map>


template<typename M>
class MessageQueue {
 public:
  using Qtype = std::queue<M>;
  MessageQueue() = default;

  void sendTo(size_t destId, M&& msg) {
    std::unique_lock<std::mutex> lk(m_);
    id2q_[destId].push(std::move(msg));
    cv_.notify_all();
  }

  // This is a blocking API.
  M receive(size_t srcId) {
    std::unique_lock<std::mutex> lk(m_);
    cv_.wait(lk, [this, srcId]() { return !id2q_[srcId].empty(); });
    auto msg = std::move(id2q_[srcId].front());
    id2q_[srcId].pop();
    return msg;
  }

 private:
  std::mutex m_;
  std::condition_variable cv_;
  std::unordered_map<size_t, std::queue<M>> id2q_;

};
