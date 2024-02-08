#include "../cpp/header.h"


class ClassNameTrait {
  public:
   ClassNameTrait()
       : getName_([this] { return typeid(*this).name(); }),
         name_(typeid(*this).name()){
             // PEEK(name_);
         };

   std::string getName() const { return getName_(); }
   // virtual is important. If not used, the types are not polymorphic.
   virtual ~ClassNameTrait() = default;

  private:
   std::function<std::string()> getName_;
   std::string name_;
};

class MyClass : public ClassNameTrait {
  
};

template<typename T>
class TClass : public ClassNameTrait {

};

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  PEEK(MyClass().getName());
  MyClass my;
  const ClassNameTrait& cnt = my;
  PEEK(cnt.getName());
  PEEK(my.getName());
  PEEK(TClass<int>().getName());
  return 0;
}

