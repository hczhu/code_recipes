#include "../cpp/header.h"
using namespace std;

#define debug(x) cerr<<#x<<"=\""<<x<<"\""<<" at line#"<<__LINE__<<endl;

template<typename TP, size_t N>
struct TuplePrinter {
  static void print(const TP& tp) {
    TuplePrinter<TP, N - 1>::print(tp);
    cout << ", " << get<N-1>(tp);
  }
};

template<typename TP>
struct TuplePrinter<TP, 1> {
  static void print(const TP& tp) {
    cout << get<0>(tp);
  }
};

template<typename... Args>
void print(const tuple<Args...>& tp) {
  cout << "(";
  TuplePrinter<decltype(tp), sizeof...(Args)>::print(tp);
  cout << ")" << endl;
}

int main() {
  auto tp = make_tuple(1, 'a', "aaaaa", 1.22222);
  cout << get<0>(tp) << endl;
  cout << get<1>(tp) << endl;
  cout << get<2>(tp) << endl;
  cout << get<3>(tp) << endl;
  print(tp);
  return 0;
}
