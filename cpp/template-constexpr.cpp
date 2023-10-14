#include <header.h>


template<typename T>
class is_vector : public std::false_type {};

template<typename T>
class is_vector<std::vector<T>> : public std::true_type {};


template<typename T>
constexpr auto is_vector_v = is_vector<T>::value;

TEST(Foo, Bar) {
  static_assert(!is_vector_v<std::string>, "std::string is not std::vector.");
  static_assert(is_vector_v<std::vector<int>>, "std::vector is std::vector.");
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}
