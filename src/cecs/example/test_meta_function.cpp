#include "test_meta_function.h"
#include "temp/meta_funtion.h"
#include "temp/view.h"
#include <iostream>

namespace cloud::world::ecs::test {
class A {
public:
  void get() {};
  void get_const() const {};
};

void foo(int u, double &x, double &y, double &z) {};

template <typename Func> void callback_wrapper(Func &&func) {
  std::cout << typeid(Func).name() << std::endl;
  std::cout << typeid(decltype(&Func::operator())).name() << std::endl;
  using traits = internal::FunctionTraits<std::decay_t<Func>>;
  std::cout << traits::is_const << std::endl;
  std::cout << typeid(typename traits::args_list).name() << std::endl;
}

void test_meta_function() {
  callback_wrapper(
      [](int x, int y) { std::cout << x << " " << y << std::endl; });
}
} // namespace cloud::world::ecs::test