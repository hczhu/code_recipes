#include "../header.h"

using namespace std;

typedef long long int llint;

template<int Index, llint FbN, llint FbN1>
class Fibonacci {
  public:
   static const llint fb = Fibonacci<Index - 1, FbN + FbN1, FbN>::fb;
};

template<llint FbN, llint FbN1>
class Fibonacci<0, FbN, FbN1> {
  public:
    static const llint fb = FbN;
};

int main() {
  cout << "Fibonacci[1] = << " << Fibonacci<1, 0 ,1>::fb << endl;
  cout << "Fibonacci[10] = << " << Fibonacci<10, 0 ,1>::fb << endl;
  cout << "Fibonacci[100] = << " << Fibonacci<20, 0 ,1>::fb << endl;
  return 0;
}

