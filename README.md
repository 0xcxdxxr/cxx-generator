<h1 align="center">CXX Generator</h1>

<p align="center">
Naive CXX17 generator implementation with std::thread<>
</p>

## Usage

```cpp
#include <iostream>
#include "generator.hpp"

int main() {
  generator::Generate<int, int, int>
      s([](gen::Ctx<int, int> &_) -> int {
    const auto a = _.yield(0);
    const auto b = _.yield(a);
    const auto c = _.yield(b);
    const auto d = _.yield(c);
    return d;
  });
  int i = 1;
  while (!s.done()) {
    std::cout << s.resume(i++) << std::endl;
  }
}
```

Yields `0 2 3 4 5`
