#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

struct Meeting {
  int b;
  int e;
};

std::vector<std::pair<Meeting, int>> scheduleMeetings(std::vector<Meeting> ms) {
  std::sort(ms.begin(), ms.end(), [](const auto& l, const auto& r) {
    return l.b < r.b;
  });
  // The pair is <current meeting's room ending time, room idx>.
  std::vector<std::pair<int, int>> roomsHeap;

  std::vector<std::pair<Meeting, int>> ret;
  auto greater = std::greater<std::pair<int, int>>();

  for (const auto& m : ms) {
    if (roomsHeap.empty() || roomsHeap[0].first > m.b) {
      auto idx = roomsHeap.size();
      roomsHeap.push_back(std::make_pair(m.e, idx));
      std::push_heap(roomsHeap.begin(), roomsHeap.end(), greater);

      ret.push_back(std::make_pair(m, idx));
    } else {
      ret.push_back(std::make_pair(m, roomsHeap[0].second));
      std::pop_heap(roomsHeap.begin(), roomsHeap.end(), greater);

      roomsHeap.back().first = m.e;
      std::push_heap(roomsHeap.begin(), roomsHeap.end(), greater);
    }

    std::cerr << "[" << m.b << ", " << m.e << ") ==> " << ret.back().second
              << std::endl;
  }
  return ret;
};

int main() {
  scheduleMeetings({
      Meeting{.b = 4, .e = 5},

      Meeting{.b = 0, .e = 3},
      Meeting{.b = 1, .e = 3},
      Meeting{.b = 1, .e = 4},

  });

  std::cerr << "--------" << std::endl;

  scheduleMeetings({
      Meeting{.b = 0, .e = 2},
      Meeting{.b = 1, .e = 3},
      Meeting{.b = 2, .e = 4},
      Meeting{.b = 3, .e = 5},
      Meeting{.b = 4, .e = 6},
  });

  std::cerr << "--------" << std::endl;

  scheduleMeetings({
      Meeting{.b = 0, .e = 1},
      Meeting{.b = 1, .e = 2},
      Meeting{.b = 2, .e = 3},
      Meeting{.b = 3, .e = 4},
      Meeting{.b = 4, .e = 5},
  });

  std::cerr << "--------" << std::endl;

  scheduleMeetings({
      Meeting{.b = 0, .e = 10},
      Meeting{.b = 1, .e = 2},
      Meeting{.b = 2, .e = 3},
      Meeting{.b = 3, .e = 4},
      Meeting{.b = 4, .e = 5},
  });

  std::cerr << "--------" << std::endl;

  scheduleMeetings({
      Meeting{.b = 0, .e = 10},
      Meeting{.b = 0, .e = 2},
      Meeting{.b = 0, .e = 3},
      Meeting{.b = 0, .e = 4},
      Meeting{.b = 0, .e = 5},
  });

  std::cerr << "--------" << std::endl;

  scheduleMeetings({
      Meeting{.b = 0, .e = 6},
      Meeting{.b = 1, .e = 6},
      Meeting{.b = 2, .e = 6},
      Meeting{.b = 3, .e = 6},
      Meeting{.b = 4, .e = 6},
  });
  return 0;
}

