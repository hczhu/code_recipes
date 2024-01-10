#include "../cpp/header.h"

using namespace std;

template<int S, typename T>
class FixedSizeVectorBase {
 public:
  FixedSizeVectorBase() = default;
  FixedSizeVectorBase(initializer_list<T> args) {
    int pos = 0;
    for (const auto& arg : args) {
      value_[pos++] = arg;
    }
  }
  void Print() const {
    cout << "{";
    for (int i = 0; i < S; ++i) {
      cout << value_[i] << ", ";
    }
    cout << "}" <<endl;
  }
 protected:
  T value_[S];
};

template<int S, typename T>
class FixedSizeVector : public FixedSizeVectorBase<S, T>  {
 public:
  FixedSizeVector(initializer_list<T> args)
    : FixedSizeVectorBase<S, T>(args) { }
};

template<int S>
class FixedSizeVector<S, char> : public FixedSizeVectorBase<S, char> {
 public:
  FixedSizeVector(const char value[]) {
    memcpy(FixedSizeVectorBase<S, char>::value_, (const void*)value, sizeof(char) * S);
  }
  FixedSizeVector(initializer_list<char> args)
    : FixedSizeVectorBase<S, char>(args) { }
  void Set(const char value[]) {
    memcpy(FixedSizeVectorBase<S, char>::value_, (const void*)value, sizeof(char) * S);
  }
};

template<int S>
using FixedString = FixedSizeVector<S, char>;

class Base {
protected:
  int data_;
};

class Derived : public Base {
 public:
  Derived(int data) {
    data_ = data;
  }
};

struct Simple {
  int a;
  char b;
  string c;
};

int main() {
  auto sp = Simple{
    .a = 10,
    .b = -1,
    .c = "dfadsfa",
  };
  FixedString<7> fixChar{"abcdefg"};
  FixedSizeVector<5, int> fixInt{1, 2, 3, 4, 5};
  fixInt.Print();
  fixChar.Print();
  Simple simple{1, 'a', "adfasfa"};
  simple = Simple({2, 'b', "afasfas"});
  int ivalue = 10;
  Simple another{ivalue, 'c', "afads"};
  map<string, set<string>> complicated{
    { "aa", { "1", "2"}},
    { "bb", set<string>{}},
  };
  cout << complicated.size();
  return 0;
}
