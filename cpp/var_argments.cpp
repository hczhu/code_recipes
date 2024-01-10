#include "../cpp/header.h"
using namespace std;
#define debug(x)                   \
  cerr << #x << "=\"" << x << "\"" \
       << " at line#" << __LINE__ << endl;

int AddAll(int a...) {
  va_list ap;
  va_start(ap, a);
  int re = a;
  while ((a = va_arg(ap, int)) != -1) {
    re += a;
  }
  return re;
}

int main() {
  debug(AddAll(1, 2, 3, 4, -1));
  debug(AddAll(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, -1));
  return 0;
}
