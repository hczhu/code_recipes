

#include <iostream>
template<int N>
struct Fib {
    static constexpr long long value = Fib<N-1>::value + Fib<N-2>::value;
};
template<>
struct Fib<0> {
    static constexpr long long value = 0;
};
template<>
struct Fib<1> {
    static constexpr long long value = 1;
};

template<int N>
void printFib() {
    if constexpr (N > 0) {
        printFib<N-1>();
    }
    std::cout << Fib<N>::value << " ";
}

int main() {
    printFib<42>();
    std::cout << "\n" << Fib<42>::value << std::endl;
    return 0;
}
