
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <iostream>
#include <cmath>
#include <array>
#include <vector>
#include <queue>
#include <set>
#include <sstream>
#include <map>
#include <cstring>
#include <complex>
#include <numeric>
#include <functional>
#include <unordered_map>
#include <cassert>
#include <limits>
#include <limits.h>
#include <cstdarg>
#include <iterator>
#include <valarray>
#include <thread>
#include <memory>
//using namespace std;
#define debug(x) cerr<<#x<<"=\""<<x<<"\""<<" at line#"<<__LINE__<<endl;
#define flag(x) FLAGS_##x

int A = 129;
int B = A + 1;
int a = B + 1;
int b = a + 1;
int d1 = b + 1;
int d2 = d1 + 1;

#include <array>
#include <map>

const std::array<int, 6> wilds = {
  A, B, a, b, d1, d2,  
};

const int INF = 1000000000;
struct St {
  bool lower = false;
  bool upper = false;
  bool dig = false;
  int l = 0;
  int next = 0;
  int prev = A;
  int cur = B;
};

size_t getKey(const St& st) {
  size_t ret = st.cur + 128;
  ret <<= 9; ret ^= st.prev + 128;
  ret <<=9; ret ^= st.next + 128;
  ret <<= 9; ret ^= st.l;
  ret <<= 1; ret ^= st.dig;
  ret <<= 1; ret ^= st.upper;
  ret <<= 1; ret ^= st.lower;
  return ret;
}

std::unordered_map<size_t, int> mem;

bool goodOne(const St& st) {
    return st.lower && st.upper && st.dig &&
        st.l >=6 and st.l <= 20;
}

St nextSt(const St& st, int ch) {
    auto nextSt = st;
    nextSt.lower = st.lower || ('a' <= ch && ch <= 'z') || ch == a || ch == b;
    nextSt.upper = st.upper || ('A' <= ch && ch <= 'Z') || ch == A || ch == B;
    nextSt.dig = st.dig || ('0' <= ch && ch <= '9') || ch == d1 || ch == d2;
    nextSt.l++;
    nextSt.next++;
    nextSt.prev = st.cur;
    nextSt.cur = ch;
    
    return nextSt;
}

using string = std::string;
class Solution {
public:
    int impl(const St& st, const string& s) {
        const auto key = getKey(st);
        auto itr = mem.find(key);
        if (itr != mem.end()) {
            return itr->second;
        }
        
        if (s.length() == st.next && goodOne(st)) {
            return 0;
        }
        int ret = INF;
        int op = -1;        
        if (st.next < s.length()) {
        // Remove the next cha
            auto next = st;
            next.next++;
            ret = std::min(ret, 1 + impl(next, s));
        }
        std::vector<int> chs(wilds.begin(), wilds.end());
        if (st.next < s.length()) {
            chs.push_back(s[st.next]);
        }
        for (int ch : chs) {
            if (st.prev == st.cur && st.cur == ch) {
                continue;
            }
            // Insert one
            if (st.l < 20 ) {
                auto  next = nextSt(st, ch);
                next.next--;
                ret = std::min(ret, 1 + impl(next, s));
                /*
              if (ret == impl(next, s) + 1) {
                op = -ch;
              }
              */
            }
            // change one
            if (st.next < s.length() && st.l < 20) {
                auto  next = nextSt(st, ch);
                ret = std::min(ret, ((ch == s[st.next])?0:1) + impl(next, s));
                /*
              if (ret == ((ch == s[st.next])?0:1) + impl(next, s)) {
                op = ch;
              }
              */
            }
        }
        mem[key] = ret;
        /*
        if (ret <= 6) {
          std::cout << st.l << " " << st.next << " " << st.prev << " " << st.cur  << " "
           << st.lower << " " << st.upper << " " << st.dig << " = " << ret 
           << " <== " << op << std::endl;
        }
        */
        return ret;

    }
    
    int strongPasswordChecker(string s) {
        return impl(St(), s);
    }
};

int main() {
  Solution s;
  std::cout << s.strongPasswordChecker("hoAISJDBVWD09232UHJEPODKNLADU1") << std::endl;
  std::cout << mem.size() << std::endl;
  return 0;
}
