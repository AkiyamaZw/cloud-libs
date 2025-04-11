#pragma once
#include "registry.h"
#include "temp/selector.h"
#include <chrono>
#include <iostream>

namespace cloud::world::ecs::vsgo {

struct Transform {
  float x{1.f};
  float y{1.f};
  float z{1.f};
};

struct Movement {
  float speed{1.f};
  float accelerate{1.f};
};

struct Render {
  float model{1.f};
  float view{1.f};
  float project{1.f};
};

struct Animation {
  float pose1{1.f};
  float joint1{1.f};
  float rotation{1.f};
};

#define SystemMacro(name)                                                      \
  class name##System {                                                         \
  public:                                                                      \
    void on_update(Selector &select);                                          \
  }

SystemMacro(Controller);
SystemMacro(Render);
SystemMacro(Animator);

class ControlComponent {
public:
  void on_update();

  float x;
  float y;
  float z;
  float speed;
  float accelerate;
};

class GameObject;
struct RenderComponent {
  RenderComponent(GameObject *_go);

  void on_update();
  GameObject *go;
  float model;
  float view;
  float project;
};

struct AnimatorComponent {
  AnimatorComponent(GameObject *obj);
  void on_update();
  GameObject *go;
  float pose1;
  float joint1;
  float rotation;
};

class GameObject {
public:
  GameObject();

  ~GameObject();

  void update();

  ControlComponent *c_comp{nullptr};
  RenderComponent *r_comp{nullptr};
  AnimatorComponent *a_comp{nullptr};
};

class Timer {
  using hrc = std::chrono::high_resolution_clock;

public:
  Timer() { start = hrc::now(); }

  ~Timer() {
    // std::cout << std::chrono::duration_cast<std::chrono::microseconds>(
    //                  hrc::now() - start)
    //           << std::endl;
  }

  size_t get_us() {
    return std::chrono::duration_cast<std::chrono::microseconds>(hrc::now() -
                                                                 start)
        .count();
  }

private:
  std::chrono::high_resolution_clock::time_point start;
};

size_t test_ecs(size_t entity_count, size_t epoch);

size_t test_go(size_t entity_count, size_t epoch);
void test_main();
} // namespace cloud::world::ecs::vsgo