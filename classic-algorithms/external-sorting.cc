#include <cstddef>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
using namespace std;


struct SplitFiles {
  vector<string> filenames;
  std::size_t max_kv_bytes;
};

SplitFiles streamToFiles(istream& is, int memoryBudget) {
  int file_idx = 0;
  SplitFiles sf;
  std::size_t max_kv_bytes = 0;
  while (!is.eof()) {
    string k, v;
    map<string, vector<string> > k2vs;
    cout << "-------------" << endl;
    int mb = memoryBudget;
    while (mb > 0 && !is.eof() && (is >> k >> v)) {
        auto kv_size = k.length() + v.length();
        max_kv_bytes = std::max(max_kv_bytes, kv_size);
        mb -= kv_size;
        cout << k << " ==> " << v << " " << mb << endl;
        k2vs[k].push_back(std::move(v));
    }
    if (k2vs.empty()) {
      break;
    }
    string fname("split_file_" + to_string(file_idx) + ".txt");
    cout << k2vs.size() << " keys in file: " << fname << std::endl;
    ofstream of(fname);
    ++file_idx;
    for (const auto& [k, vs] : k2vs) {
      of << k;
      for (const auto& v : vs) {
        of << " " << v;
      }
      of << std::endl;
    }
    sf.filenames.push_back(fname);
  }
  sf.max_kv_bytes = max_kv_bytes;
  return sf;
}

void merge_files(const vector<string>& filenames, ostream& os) {
  // kvs[i] is from filenames[i].
  vector<pair<string, string> > kvs;
  vector<ifstream> ifs;
  vector<int> heap;
  string k, v;
  for (const auto& f : filenames) {
    heap.push_back(kvs.size());
    ifstream ifile(f);
    ifile >> k >> v;
    kvs.emplace_back(std::move(k), std::move(v));
    ifs.push_back(std::move(ifile));
  }
  auto heap_cmp = [&kvs](int a, int b) {
    return kvs[a] > kvs[b];
  };
  std::make_heap(heap.begin(), heap.end(), heap_cmp);
  string prev_k;
  int loop = 10;
  while (!heap.empty() && (--loop) >= 0) {
    int min_idx = heap.front();
    std::pop_heap(heap.begin(), heap.end());
    heap.pop_back();
    std::tie(k, v) = kvs[min_idx];
    if (k == prev_k) {
      os << " " << v;
    } else {
      if (prev_k.size()) {
        os << std::endl;
      }
      os << k << " " << v;
      prev_k = k;
    }
    // cout << k << " " << v << " from "<< min_idx << std::endl;
    auto& input_file = ifs[min_idx];
    bool eof = false;
    // cout << "Peek: " << input_file.peek() << endl;
    if (input_file.peek() == '\n') {
      eof = eof || !(input_file >> k);
      eof = eof || !(input_file >> v);
    } else {
      eof = eof || !(input_file >> v);
    }
    if (!eof) {
      // cout << "New k v " << k << " " << v << endl;
      kvs[min_idx].first = k;
      kvs[min_idx].second = v;
      heap.push_back(min_idx);
      std::push_heap(heap.begin(), heap.end(), heap_cmp);
    }
  }
}

void sort_kvs(istream& is, ostream& os, int memory_budget) {
  os << "=====================" << endl;
  auto sf = streamToFiles(is, memory_budget);
  auto files = sf.filenames;
  std::cout << "Got " << files.size() << " split files.\n";
  const size_t file_batch_size = memory_budget / sf.max_kv_bytes;
  int round = 0;
  while (files.size() > file_batch_size) {
    vector<string> new_files;
    ++round;
    for (size_t b = 0; b < sf.filenames.size(); b += file_batch_size) {
      string new_file = "merged_" + to_string(round) + "_" + to_string(b);
      new_files.push_back(new_file);
      ofstream of(new_file);
      merge_files(
        vector<string>(files.begin() + b, 
          std::min(files.begin() + b + file_batch_size, files.end())),
          of
      );
    }
    files = new_files;
  }
  merge_files(files, os);
  os << endl;
}

int main() {
  {
    istringstream input("a b\nc d\na e\nb f\n");
    sort_kvs(input, std::cout, 4);
  }
  {
    istringstream input("a b\nc d\na e\nb f\n");
    sort_kvs(input, std::cout, 40);
  }
  {
    istringstream input("a 0\na 1\na 2\na 3\n");
    sort_kvs(input, std::cout, 4);
  }
  return 0;
}
