#inlcude "../header.h"


size_t ceilingTwo(size_t n) {
  size_t res = 1;
  while (res < n) {
    res <<= 1;
  }
  return res;
}

template<typename Segment>
class SegmentTree {
 public:
  // The root represent [0, n_).
  SegmentTree(size_t n) : n_(ceilingTwo(n)), segs_(n_ << 1) {
  }

  using Callback = std::function<bool(Segment &, size_t, size_t)>;
  using UpdateCallback =
      std::function<void(Segment &, const Segment &, const Segment &)>;

  // Traverse a disjoint set of segments exactly covering [l, r). The order is
  // from the leftmost to the rightmost.
  void traverseSegmentsFor(size_t l, size_t r, Callback& cb) {
    Callback overlapCheck = [&](Segment& seg, size_t sl, size_t sr) {
      if (sr <= l || r <= sl)  {
        return false;
      }
      if (l <= sl && sr <= r) {
        if (!cb(seg, sl, sr)) {
          return false;
        }
      }
      return true;
    };

    UpdateCallback null = [](Segment &, const Segment &, const Segment &) {

    };

    dfsIf(overlapCheck, null);
  }

  void dfsIf(Callback& cb, UpdateCallback& updateCb) {
    if (!cb(segs_[0], 0, n_)) {
      return;
    }
    dfsIf_(0, 0, n_, cb, updateCb);
  }

  void dfsIf_(size_t segIdx, size_t l, size_t r, Callback &cb,
              UpdateCallback &updateCb) {
    // std::cout << "seg @" << segIdx << ": [" << l << ", " << r << ")" << std::endl;
    if (l + 1 == r) {
      return;
    }

    size_t mid = (l + r) >> 1;

    const size_t lc = (segIdx << 1) + 1;
    const size_t rc = lc + 1;
    if (cb(segs_[lc], l, mid)) {
      dfsIf_(lc, l, mid, cb, updateCb);
    }

    if (cb(segs_[rc], mid, r)) {
      dfsIf_(rc, mid, r, cb, updateCb);
    }

    updateCb(segs_[segIdx], segs_[lc], segs_[rc]);
  }

  size_t n_;
  std::vector<Segment> segs_;
};

class SegmentTreeTest : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};


struct Seats {
  size_t minOccupied = 0;
  size_t sumOccupied = 0;
};

class BookMyShow {
public:
  BookMyShow(int n, int m) : m_(m), tree_(n) {}

  bool filterOutSeg(const Seats &seg, size_t l, size_t r, size_t maxR) {
    if (seg.minOccupied == m_) {
      return true;
    }
    if (maxR <= l) {
      // Not overlap
      return true;
    }
    return false;
  }

  std::vector<int> gather(int k, int maxRow) {
    int row = -1, col = -1;
    size_t maxR = maxRow + 1;
    SegmentTree<Seats>::Callback canGather = [&, this](Seats &seg, size_t l,
                                                       size_t r) -> bool {
      // std::cout << "[" << l << ", " << r << ") = " << seg.minOccupied << " @"
      // << (&seg - &tree_.segs_[0]) << std::endl;

      if (row >= 0) {
        // Already found a row
        return false;
      }
      if (filterOutSeg(seg, l, r, maxR)) {
        return false;
      }
      if (seg.minOccupied + k > m_) {
        // No enough capacity in the whole segment.
        return false;
      }
      if (l + 1 == r) {
        // std::cout << " Picked row " << l << std::endl;
        row = l;
        col = seg.minOccupied;
        seg.minOccupied += k;
        seg.sumOccupied += k;
      }
      return true;
    };

    // std::cout << " ====================== " << std::endl;
    tree_.dfsIf(canGather, updateCb_);
    if (row >= 0) {
      return {row, col};
    }
    return {};
  }

  bool scatter(int k, int maxRow) {
    bool updateNode = false;
    auto neededSeats = k;
    Tree::Callback scatterSeg = [&updateNode, &neededSeats, maxr = maxRow + 1,
                                 this](Seats &seg, size_t l, size_t r) -> bool {
      // std::cout << "[" << l << ", " << r << ") = " << seg.sumOccupied <<
      // std::endl;

      if (neededSeats == 0) {
        return false;
      }
      if (filterOutSeg(seg, l, r, maxr)) {
        return false;
      }

      if (maxr >= r) {
        // The rows in this segment are all usable.
        auto seats = (r - l) * m_ - seg.sumOccupied;
        if (seats <= neededSeats) {
          if (updateNode) {
            // The whole segment is occupied now.
            seg.minOccupied = m_;
            seg.sumOccupied = m_ * (r - l);
          }
          // std::cout << "    occupied " << seats << " seats." << std::endl;
          neededSeats -= seats;
          return false;
        } else if (l + 1 == r) {
          // Leaf node
          if (updateNode) {
            seg.minOccupied += neededSeats;
            seg.sumOccupied = seg.minOccupied;
          }
          // std::cout << "    occupied at a leaf " << neededSeats << " seats: "
          // << seg.minOccupied << " " << seg.sumOccupied << " @" << (&seg -
          // &tree_.segs_[0]) << std::endl;
          neededSeats = 0;
          return false;
        }
      }
      return true;
    };

    Tree::UpdateCallback nullCb = [](Seats &, const Seats &, const Seats &) {};
    // std::cout << " ====================== " << std::endl;
    tree_.dfsIf(scatterSeg, nullCb);
    if (neededSeats > 0) {
      return false;
    }

    neededSeats = k;
    updateNode = true;
    // std::cout << " ====================== " << std::endl;
    tree_.dfsIf(scatterSeg, updateCb_);
    return true;
  }

  void updateSegment(Seats &parent, const Seats &lc, const Seats &rc) {
    parent.minOccupied = std::min(lc.minOccupied, rc.minOccupied);
    parent.sumOccupied = lc.sumOccupied + rc.sumOccupied;
  }

  size_t m_;
  using Tree = SegmentTree<Seats>;
  Tree tree_;
  SegmentTree<Seats>::UpdateCallback updateCb_ =
      std::bind(&BookMyShow::updateSegment, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3);
};

/**
 * Your BookMyShow object will be instantiated and called as such:
 * BookMyShow* obj = new BookMyShow(n, m);
 * vector<int> param_1 = obj->gather(k,maxRow);
 * bool param_2 = obj->scatter(k,maxRow);
 */

TEST_F(SegmentTreeTest, LeetCodeBookMyShow) {
  {
    BookMyShow bms(2, 5); // There are 2 rows with 5 seats each
    EXPECT_EQ(
        bms.gather(4, 0),
        std::vector<int>({0, 0})); // return [0, 0]
                                   // The group books seats [0, 3] of row 0.
    EXPECT_TRUE(
        bms.gather(2, 0)
            .empty()); // return []
                       // There is only 1 seat left in row 0,
                       // so it is not possible to book 2 consecutive seats.
    EXPECT_TRUE(bms.scatter(
        5, 1)); // return True
                // The group books seat 4 of row 0 and seats [0, 3] of row 1.

    EXPECT_FALSE(bms.scatter(5, 1)); // return False
  }
  {
    BookMyShow show(4, 5);
    EXPECT_TRUE(show.scatter(6, 2));
    EXPECT_TRUE(show.gather(6, 3).empty());
    EXPECT_FALSE(show.scatter(9, 1));
  }

  {

    BookMyShow show(5, 9);
    EXPECT_TRUE(show.gather(10, 1).empty());
    EXPECT_TRUE(show.scatter(3, 3));
    EXPECT_EQ(show.gather(9, 1), std::vector<int>({ 1, 0 }));
    EXPECT_EQ(show.gather(10, 2), std::vector<int>({ }));
    EXPECT_EQ(show.gather(2, 0), std::vector<int>({ 0, 3}));

  }
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

