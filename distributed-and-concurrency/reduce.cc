#include <header.h>


class FooTest : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};


struct Msg {
  int rec, sent, r;
  bool operator==(const Msg &rhs) const {
    return rec == rhs.rec && sent == rhs.sent && r == rhs.r;
  }
};

struct MsgHash {
  std::size_t operator()(const Msg& msg) const {
    std::size_t h1 = std::hash<decltype(msg.rec)>()(msg.rec);
    std::size_t h2 = std::hash<int>()(msg.sent);
    std::size_t h3 = std::hash<int>()(msg.r);

    return (h1 << 16) ^ (h2 << 8) ^ h3;
  }
};
using MsgSet = std::unordered_set<Msg, MsgHash>;

struct Fence {
  std::unordered_map<int, int> round2count;
  std::mutex m;
  std::condition_variable cv;

  void waitFor(int round, int count) {
    std::unique_lock<std::mutex> lk(m);
    ++round2count[round];
    if (round2count[round] == count) {
      lk.unlock();
      cv.notify_all();
    } else {
      cv.wait(lk,
              [this, round, count]() { return round2count[round] == count; });
    }
  }
};

void binaryTreeTreadRun(size_t threadId, std::vector<int> &values,
                        Fence &fence) {
  // LOG(INFO) << "Running thread #" << threadId;
  const auto n = values.size();
  bool shouldEnd = false;
  for (size_t r = 1; r < n && !shouldEnd; r <<= 1) {
    const auto participants = (n + r - 1) / r;
    const auto rr = r << 1;
    if (threadId % rr) {
      size_t destThreadId = threadId - (threadId % rr);
      // LOG(INFO) << "Node #" << threadId << " sent a msg to node #" << destThreadId << " at round #" << r;
      values[destThreadId] += values[threadId];
      shouldEnd = true;
    }
    // LOG(INFO) << "Node #" << threadId << " is waiting for round #" << r
              // << " to finish expecting " << participants << " participants.";
    fence.waitFor(r, participants);
  }
  // LOG(INFO) << "Node #" << threadId << " is done.";
}

struct Message {
  int v;
  size_t from;
};

void binaryTreeTreadRunWithMsg(size_t threadId, size_t n, int &value,
                               MessageQueue<Message> &mq) {
  for (size_t r = 1; r < n; r <<= 1) {
    const auto rr = r << 1;
    if (threadId % rr) {
      size_t destThreadId = threadId - (threadId % rr);
      mq.sendTo(destThreadId, Message{
                                  .v = value,
                                  .from = threadId,
                              });
      break;
    }
    if (threadId + r < n) {
      auto msg = mq.receive(threadId);
      value += msg.v;
    }
  }
  // LOG(INFO) << "Node #" << threadId << " is done.";
}

int binaryTreeReduce(std::vector<int> &values, bool withMq = false) {
  LOG(INFO) << "----------- Running " << values.size() << " theads -----------";
  std::vector<std::thread> threads;
  Fence fence;
  MessageQueue<Message> mq;
  for (size_t i = 0; i < values.size(); ++i) {
    threads.emplace_back([&, i]() { 
      if (withMq) {
        binaryTreeTreadRunWithMsg(i, values.size(), values[i], mq);
      } else {
        binaryTreeTreadRun(i, values, fence);
      }
    });
  }
  for (auto& thread : threads) {
    thread.join();
  }
  return values[0];
}

int simulateBinaryTreeReduce(std::vector<int>& values) {
  const auto n = values.size();
  for (size_t r = 1; r < n; r <<= 1) {
    // LOG(INFO) << "---- Round #" << r << " ----------"; 
    const auto rr = r << 1;
    for (size_t i = 0; i + r < n; i += rr) {
      values[i] += values[i + r];
      // LOG(INFO) << "Node #" << (i + r) << " sent a msg to node #" << i;
    }
  }
  return values[0];
}

void ringAllReduceThreadRun(size_t threadId, std::vector<int> &values,
                            MessageQueue<Message> &mq, Fence& fence) {
  const size_t n = values.size();
  size_t prevId = (threadId + n - 1) % n;
  size_t nextId = (threadId + 1) % n;
  // scatter-reduce phase
  for (size_t r = 0; r + 1 < n; ++r) {
    const size_t chunkIdToSend = (threadId + n - r) % n;
    const size_t chunkIdToRecv = (chunkIdToSend + n - 1) % n;
    mq.sendTo(nextId, Message{
                          .v = values[chunkIdToSend],
                          .from = chunkIdToSend,
                      });
    // LOG(INFO) << "Node #" << threadId << " sent value @" << chunkIdToSend << " " << values[chunkIdToSend]
              // << " to node #" << nextId;
    auto msg = mq.receive(threadId);
    values[chunkIdToRecv] += msg.v;
    CHECK_EQ(chunkIdToRecv, msg.from);
    // LOG(INFO) << "Node #" << threadId << " received value @" << chunkIdToRecv << " " << msg.v;
    fence.waitFor(r, n);
  }

  // reduce sharing phase
  for (size_t r = 0; r + 1 < n; ++r) {
    const size_t chunkIdToSend = (threadId + 1 + n - r) % n;
    const size_t chunkIdToRecv = (chunkIdToSend + n - 1) % n;
    mq.sendTo(nextId, Message{
                          .v = values[chunkIdToSend],
                          .from = chunkIdToSend,
                      });
    auto msg = mq.receive(threadId);
    values[chunkIdToRecv] = msg.v;
    CHECK_EQ(chunkIdToRecv, msg.from);
    fence.waitFor(r + n, n);
  }
}

std::vector<int> ringAllReduce(std::vector<std::vector<int>>& values) {
  const auto n = values.size();
  MessageQueue<Message> mq;
  LOG(INFO) << "----------- Running " << n << " theads -----------";
  std::vector<std::thread> threads;
  Fence fence;
  for (size_t i = 0; i < n; ++i) {
    threads.emplace_back([&, i]() { 
      ringAllReduceThreadRun(i, values[i], mq, fence);
    });
  }
  for (auto& thread : threads) {
    thread.join();
  }
  for (const auto &v : values) {
    EXPECT_EQ(v, values[0]);
  }
  return values[0];
}

TEST(Reduce, bianryTreeReduceSimulationTest) {
  auto testN = [](size_t n) {
    std::vector<int> values(n);
    for (size_t i = 0; i < n; ++i) {
      values[i] = i;
    }
    const auto ans = std::accumulate(values.begin(), values.end(), 0);

    EXPECT_EQ(simulateBinaryTreeReduce(values), ans);
  };

  SCOPED_TRACE("test 2"); testN(2);
  SCOPED_TRACE("test 9"); testN(9);
  SCOPED_TRACE("test 1"); testN(1);
  SCOPED_TRACE("test 101"); testN(101);
}

TEST(Reduce, bianryTreeReduceTest) {
  auto testN = [](size_t n) {
    std::vector<int> values(n);
    for (size_t i = 0; i < n; ++i) {
      values[i] = i;
    }
    const auto ans = std::accumulate(values.begin(), values.end(), 0);

    EXPECT_EQ(binaryTreeReduce(values), ans);
  };

  SCOPED_TRACE("test 1"); testN(1);
  SCOPED_TRACE("test 2"); testN(2);
  SCOPED_TRACE("test 9"); testN(9);
  SCOPED_TRACE("test 1"); testN(1);
  SCOPED_TRACE("test 101"); testN(101);
  SCOPED_TRACE("test 1017"); testN(1017);
}

TEST(Reduce, bianryTreeReduceWithMsgTest) {
  auto testN = [](size_t n) {
    std::vector<int> values(n);
    for (size_t i = 0; i < n; ++i) {
      values[i] = i;
    }
    const auto ans = std::accumulate(values.begin(), values.end(), 0);

    EXPECT_EQ(binaryTreeReduce(values, true), ans);
  };

  SCOPED_TRACE("test 1"); testN(1);
  SCOPED_TRACE("test 2"); testN(2);
  SCOPED_TRACE("test 9"); testN(9);
  SCOPED_TRACE("test 1"); testN(1);
  SCOPED_TRACE("test 101"); testN(101);
  SCOPED_TRACE("test 1017"); testN(1017);
}

TEST(Reduce, ringAllReduceTest) {
  auto testN = [](size_t n) {
    std::vector<std::vector<int>> values(n);
    int s = 0;
    for (size_t i = 0; i < n; ++i) {
      values[i] = std::vector<int>(n, i);
      s += i;
    }

    const auto ans = std::vector<int>(n, s);

    EXPECT_EQ(ringAllReduce(values), ans);
  };

  SCOPED_TRACE("test 1"); testN(1);
  SCOPED_TRACE("test 2"); testN(2);
  SCOPED_TRACE("test 9"); testN(9);
  SCOPED_TRACE("test 1"); testN(1);
  SCOPED_TRACE("test 101"); testN(101);
  SCOPED_TRACE("test 102"); testN(102);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

