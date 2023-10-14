#include <header.h>

void relaxedMagic1() {
  std::atomic<int> x(0), y(0);
  std::atomic<bool> loaded{false};
  bool stop = false;
  std::mutex mt;
  std::condition_variable cv;
  std::thread writer([&]{
    while (!stop) {
      x.store(1,std::memory_order_relaxed);
      y.store(1,std::memory_order_relaxed);
      std::unique_lock<std::mutex> lk(mt);
      cv.wait(lk, [&]{
        return stop || loaded;
      });
      loaded = false;
    }
  });

  std::thread reader([&]{
    int round = 0;
    while (!stop) {
      int a = y.load(std::memory_order_relaxed);
      int b = x.load(std::memory_order_relaxed);
      std::cout << "Round #" << ++round << std::endl;
      if (a==1 && b!=1) {
        std::cout << "Magic happend: a == 1 && b != 1." << std::endl;
        stop = true;
      }
      loaded = true;
      cv.notify_one();
    }
  });

  reader.join();
  writer.join();
}

int main() {
  relaxedMagic1();
  return 0;
}
