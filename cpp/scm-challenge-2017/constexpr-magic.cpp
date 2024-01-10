#include "../cpp/header.h"

constexpr const auto &lorem_ipsum = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur eu lorem sed odio varius vestibulum et eu ante. Quisque rutrum, sem vitae accumsan finibus, enim elit mattis urna, gravida rhoncus erat sem quis lectus. Donec ultrices pretium arcu, rhoncus facilisis eros lobortis sit amet. Quisque vitae lorem at ante ultricies pulvinar. Sed suscipit faucibus tempus. Donec ut sem felis. Ut porttitor libero justo, ultrices egestas purus cursus cursus. Fusce et sapien felis. Phasellus ut ornare arcu. Vestibulum eget finibus dui. Sed quam sem, efficitur vitae risus egestas, vehicula vestibulum est. Nulla rutrum tempus mollis. Nunc a elementum felis";

struct StringPiece {
 public:
  template<typename T>
  constexpr StringPiece(T& str, size_t n) : b_(str), s_(n) {
  }
  constexpr size_t size() const {
    return s_;
  }

  const char* b_;
  const size_t s_;
};

std::ostream& operator<<(std::ostream& os, const StringPiece& sp) {
  auto p = sp.b_;
  for (int i = 0; i < sp.size(); ++i) {
    if (!(*p)) {
      p = sp.b_;
    }
    os << *p++;
  }
  return os;
}

constexpr StringPiece operator "" _lorem(unsigned long long int size) {
  return StringPiece(lorem_ipsum, size);
}

struct FSP {
  constexpr FSP(const StringPiece _sp) : sp(_sp) {}
  const StringPiece sp;
  const size_t s = 597;
  constexpr size_t size() const {
    return s;
  }
};
std::ostream& operator<<(std::ostream& os, const FSP& fsp) {
  const auto& sp = fsp.sp;
  auto p = sp.b_;
  for (int i = 0; i < sp.size(); ++i) {
    if (!(*p)) {
      p = sp.b_;
    }
    if (*p != ' ') {
      os << *p;
    }
    ++p;
  }
  return os;
}

constexpr FSP filt(const StringPiece sp) {
  return FSP(sp);
}

TEST(Foo, Bar) {
  constexpr auto filtered = filt(700_lorem);
  static_assert(filtered.size() == 597);

  std::cout << filtered << std::endl;
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  // _displayType(lorem_ipsum);
  return RUN_ALL_TESTS();
}

