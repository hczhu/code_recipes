#pragma once

#include "../header.h"

class Base {
 public:
  Base() {
    ++CtorCnt;
    ++LiveCnt;
    std::cout << "ctor in " << __FUNCTION__ << " @" << this << std::endl;
  }
  Base(const Base&) {
    ++CopyCnt;
    ++LiveCnt;
    std::cout << "Copy ctor in " << __FUNCTION__ << " @" << this << std::endl;
  }
  Base(Base&&) {
    ++MoveCnt;
    ++LiveCnt;
    std::cout << "Move ctor in " << __FUNCTION__ << " @" << this << std::endl;
  }
  const Base& operator=(const Base&) {
    ++CopyCnt;
    std::cout << "Copy = in " << __FUNCTION__ << " @" << this << std::endl;
    return *this;
  }
  const Base& operator=(Base&&) {
    ++MoveCnt;
    std::cout << "Move = in " << __FUNCTION__ << " @" << this << std::endl;
    return *this;
  }
  virtual ~Base() {
    ++DtorCnt;
    --LiveCnt;
    std::cout << "Dtor in " << __FUNCTION__ << " @" << this << std::endl;
  }
  void print() const {
    PEEK(a_);
  }
  void set(int a) {
    a_ = a;
  }

  static size_t MoveCnt;
  static size_t CopyCnt;
  static size_t DtorCnt;
  static size_t CtorCnt;
  static size_t LiveCnt;
 protected:
  int a_ = 10;
};
size_t Base::MoveCnt = 0;
size_t Base::CopyCnt = 0;
size_t Base::DtorCnt = 0;
size_t Base::CtorCnt = 0;
size_t Base::LiveCnt = 0;

class Derived : public Base {
 public:
  Derived() {
    std::cout << "ctor in " << __FUNCTION__ << " @" << this << std::endl;
  }
  Derived(const Base&) {
    std::cout << "Copy ctor in " << __FUNCTION__ << " @" << this << std::endl;
  }
  Derived(Base&&) {
    std::cout << "Move ctor in " << __FUNCTION__ << " @" << this << std::endl;
  }
  ~Derived() override {
    std::cout << "Dtor in " << __FUNCTION__ << " @" << this << std::endl;
  }
};

class Container {
 public:
  Container() {
    std::cout << "ctor in " << __FUNCTION__ << " @" << this << std::endl;
  }
  Container(const Base&) {
    std::cout << "Copy ctor in " << __FUNCTION__ << " @" << this << std::endl;
  }
  Container(Base&&) {
    std::cout << "Move ctor in " << __FUNCTION__ << " @" << this << std::endl;
  }
  ~Container() {
    std::cout << "Dtor in " << __FUNCTION__ << " @" << this << std::endl;
  }
 private:
  Base b_;
  Derived d_;
};
