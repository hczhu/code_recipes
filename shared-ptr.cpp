#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <map>
#include <cassert>
using namespace std;


int main() {
  vector<thread> threads;
  map<int, FileAndLock> fileMap;
  fileMap.insert(make_pair(0, FileAndLock("/tmp/test-0.txt")));
  fileMap.insert(make_pair(1, FileAndLock("/tmp/test-1.txt")));
  fileMap.insert(make_pair(2, FileAndLock("/tmp/test-2.txt")));
  for (int i = 0; i < 6; ++i) {
    threads.emplace_back([i, &fileMap]() {
      for (int j = 0; j < 5; ++j) {
        lock_guard<mutex> lock(*fileMap[i % 3].fileLock);
        *fileMap[i % 3].fileToAppend << "Thread #" << i << " couter: " << j << endl;
      }
    });
  }
  for (auto& thr : threads) {
    thr.join();
  }
  return 0;
}
