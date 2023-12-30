#include "../header.h"
using namespace std;
#define debug(x) cerr<<#x<<"=\""<<x<<"\""<<" at line#"<<__LINE__<<endl;
// Remove comments from C++ code.
int main() {
  static int a = 10; /* afds */
  const string b = "//dfadsf/*dfafsd*/"; /* nonsense
    haha */
  const char c = '"';
  const string d = "\\\\\\\\\r\t\"\\\\ aa\\\"\
        ";
  const char e = '\"';
  const char f = '\\';

  const string cppCode = R"c++(
  // comments /*
  static int a = 10; /* afds */
  const string b = "'\"'//dfadsf/*dfafsd*/"; /* nonsense
    // haha */
  /*aabb */
  const char c = '"';
  const string d = "\\\\\\\\\r\t\"\\\\ aa\\\"\
        "; /***aaa
************* "' '" *
*/
  const char e = '\"';
  const char f = '\\';
   /* //// */
    )c++";

  const string doubleQuote = R"(("([^"\\]*(\\(.|\n))*)*"))";
  const string singleQuote = R"('([^\\]|(\\.))')";
  const string singleLineComment = R"(//.*\n)";
  const string multipleLineComment = R"(/\*([^*]*(\*+[^/])*)*\*/)";
  regex commentRegex("(" + doubleQuote + ")|(" + singleQuote + ")|(" + singleLineComment + ")|(" +
    multipleLineComment + ")");
  sregex_iterator itr(cppCode.begin(), cppCode.end(), commentRegex);
  sregex_iterator end;
  while (itr != end) {
    cout << "[" << itr->position() << ", " << itr->position() + itr->length() << "): " << itr->str() << endl;
    ++itr; 
  }
  return 0;
}
