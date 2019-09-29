#include <iostream>

#define CXX_GENERATOR__GENERATOR_HPP
#include "generator.hpp"

int main() {
  auto s = generator(int, int, [&_] {
    auto a = yield
    0;
    auto b = yield
    a;
    auto c = yield
    b;
    auto d = yield
    c;
    return d;
  });
  int i = 1;
  while (!s.done()) {
    std::cout << s.resume(i++) << std::endl;
  }
}
