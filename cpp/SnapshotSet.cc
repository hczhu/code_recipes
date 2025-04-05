
#include <algorithm>
#include <cassert>
#include <iostream>
#include <unordered_set>
#include <vector>
using namespace std;

template <typename K>
class SnapshotSet {
 public:
  SnapshotSet() : segments_(1) {}

  void add(K k) {
    auto& segment = segments_.back();
    if (segment.removed.count(k)) {
      segment.removed.erase(k);
    }
    segment.added.insert(std::move(k));
  }

  void remove(const K& k) {
    auto& segment = segments_.back();
    if (segment.added.count(k)) {
      segment.added.erase(k);
    }
    segment.removed.insert(k);
  }

  bool get(const K& k) const {
    for (int i = segments_.size() - 1; i >= 0; --i) {
      if (segments_[i].added.count(k)) {
        return true;
      }
      if (segments_[i].removed.count(k)) {
        return false;
      }
    }
    return false;
  }

 private:
  struct Segment;
  using Set = unordered_set<K>;

 public:
  class Itr {
   public:
    bool operator!=(const Itr& rhs) const {
      return segIdx_ != rhs.segIdx_ || itr_ != rhs.itr_;
    }
    Itr& operator++() {
      ++itr_;
      while (!findNextItr() && segIdx_ > 0) {
        moveToNextSeg();
      }
      return *this;
    }
    const K& operator*() { return *itr_; }
    struct EndTag {};

   private:
    friend class SnapshotSet;
    Itr(const vector<Segment>& segs) : segs_(&segs) {
      segIdx_ = segs_->size();
      do {
        moveToNextSeg();
      } while (!findNextItr() && segIdx_ > 0);
    }
    Itr(const vector<Segment>& segs, EndTag)
        : segIdx_(0), itr_(segs[0].added.end()) {}
    void moveToNextSeg() {
      --segIdx_;
      itr_ = (*segs_)[segIdx_].added.begin();
      removed_.insert((*segs_)[segIdx_].removed.begin(),
                      (*segs_)[segIdx_].removed.end());
    }
    bool findNextItr() {
      auto& seg = (*segs_)[segIdx_];
      while (itr_ != seg.added.end()) {
        if (removed_.count(*itr_) == 0) {
          return true;
        }
        ++itr_;
      }
      return false;
    }
    const vector<Segment>* segs_;
    size_t segIdx_;
    typename Set::const_iterator itr_;
    Set removed_;
  };

  Itr begin() {
    Itr it(segments_);
    if (!segments_.back().added.empty() || !segments_.back().removed.empty()) {
      segments_.emplace_back();
    }
    return it;
  }

  const Itr end() const { return Itr(segments_, typename Itr::EndTag()); }

 private:
  void compact(size_t releasedSegmentIdx) {
    // Trigger potential compactions of segments_.
  }
  struct Segment {
    size_t outstandingIterators = 0;
    Set added;
    Set removed;
  };
  // Order from the oldest one to the neweset one.
  // A newer one overwrites any older ones.
  // NB: this is wrong. vector resizing will move all elements (Segment).
  vector<Segment> segments_;
};

// To execute C++, please define "int main()"
int main() {
  using Set = SnapshotSet<int>;
  auto expectSet = [](const Set& s, Set::Itr& it, vector<int> v) {
    vector<int> actual;
    while (it != s.end()) {
      actual.push_back(*it);
      ++it;
    }
    sort(actual.begin(), actual.end());
    sort(v.begin(), v.end());
    if (actual != v) {
      cout << "Expect: ";
      // Print expected vs. actual.
      for (auto i : v) {
        cout << i << " ";
      }
      cout << endl;
      cout << "Actual: ";
      for (auto i : actual) {
        cout << i << " ";
      }
      cout << endl;
    }
    assert(actual == v);
  };
  {
    SnapshotSet<int> s;
    s.remove(1);
    s.add(1);
    assert(s.get(1));
    s.remove(1);
    s.remove(2);
    s.add(2);
    s.remove(1);
    assert(!s.get(1));
    assert(s.get(2));
  }
  {
    SnapshotSet<int> s;
    auto itr1 = s.begin();
    s.add(1);
    s.add(2);
    auto itr2 = s.begin();
    s.remove(2);
    s.add(1);
    auto itr3 = s.begin();
    s.remove(2);

    expectSet(s, itr1, {});
    expectSet(s, itr2, {});
  }

  cout << "All done." << endl;

  return 0;
}
