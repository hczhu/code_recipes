#include "../header.h"

using namespace std;

#define debug(x) cerr<<#x<<"=\""<<x<<"\""<<" at line#"<<__LINE__<<endl;

template<typename Policy>
void print(Policy& outputStream) {
  outputStream << std::endl;
}

template<typename... Args>
using VT = int;

template<typename Policy, typename Arg1, typename... Args>
void print(Policy& outputStream, const Arg1& arg1, const Args&... args) {
  outputStream << arg1 << " ";
  print(outputStream, args...);
}

template<typename... Args>
void confirm(const Args&... msg) {
  int _[] = {
    (std::cout << msg << " ", 0)...
  };
  std::cout << " N/y?" << std::endl;
  (void)_;
}

struct Vec {
  template<typename... Args>
  static std::shared_ptr<Vec> make(Args&&... eles) {
    auto res = std::make_shared<Vec>();
    res->add(std::forward<Args>(eles)...);
    return res;
  }
  template<typename Arg1, typename... Args>
  void add(Arg1 ele, Args... eles) {
    add(ele);
    add(eles...);
  }
  
  template<typename Arg>
  void add(Arg ele) {
    vec_.push_back(int(ele));
  }

  std::vector<int> vec_;
};

int foo(int) {
  return -1;
}

template<class... Container>
bool anyEmptyContainer(const Container&... args) {
  return (...|| args.empty());
}

TEST(Main, VariadicUsing) {
  static_assert(std::is_same_v<VT<int, double, std::string>, int>);
}

TEST(Main, Main) {
  auto& t = foo;
  print(cout, 1, 2, "sfsfd", 1.34243, 'a', 3241234);
  string ss;
  stringstream os(ss);
  print(os, 1, 2, "sfsfd", 1.34243, 'a', 3241234);
  cout << os.str() << std::endl;

  auto vec = Vec::make(1, 2, 4, 5);
  print(cout,
    vec->vec_[0],
    vec->vec_[1],
    vec->vec_[2],
    vec->vec_[3]
  );
  confirm(1, 2, "aaa", 1.2, 1.5, -1);
}

TEST(Variadic, IfExpansion) {
  EXPECT_TRUE(anyEmptyContainer(std::string(), std::string("dfa")));
  EXPECT_FALSE(anyEmptyContainer(std::vector<int>({
                                     1, 2,
                                 }),
                                 std::vector<int>({
                                     3,
                                 })));

  EXPECT_TRUE(anyEmptyContainer(std::vector<int>({}),
                                std::vector<int>({
                                    3,
                                })));
}


template <typename R>
struct remove_container {
  using type = R;
};

template <typename R>
struct remove_container<std::vector<R>> {
  using type = R;
};

template <typename R>
using remove_container_t = typename remove_container<R>::type;

TEST(Variadic, removeContainer) {
  static_assert(std::is_same_v<remove_container_t<std::string>, std::string>);
  static_assert(std::is_same_v<remove_container_t<std::vector<std::string>>,
                               std::string>);
}

template<typename T>
struct add_const_ref {
  using type = const T&;
};

template<>
struct add_const_ref<int> {
  using type = int;
};

template<typename T>
using add_const_ref_t = typename add_const_ref<T>::type;

int foo(std::string&, int, int) {
  return 1;
}

template <typename... Args>
void forwardToFoo(const Args&... args) {
  // _DisplayType<add_const_ref_t<Args>...> dd;
  static_assert(std::is_same_v<std::tuple<add_const_ref_t<Args>...>,
                               std::tuple<const std::string&, int, int>>);
}

TEST(Main, transformArgs) {
  std::string a;
  // EXPECT_EQ(forwardToFoo(a), 1);
  static_assert(std::is_same_v<int, add_const_ref_t<int>>);
  static_assert(
      std::is_same_v<const std::string&, add_const_ref_t<std::string>>);
  forwardToFoo(a, 1, 2);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}
