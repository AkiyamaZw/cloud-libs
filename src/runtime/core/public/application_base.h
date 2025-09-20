#pragma once
#include <functional>
#include <unordered_map>
#include <string>
#include "job_system.h"

namespace cloud
{
enum class ApplicationState
{
    None,
    SETUP,
    RUNNING,
    CLOSING,
    CLOSED
};

class Module
{
  public:
    Module();
    Module(const std::string &module_id);
    virtual ~Module();
    virtual void OnDestroy();

  private:
    std::string id_;
};

using UpdateFunc = std::function<void(float)>;
class UpdateTimer
{
  public:
    UpdateTimer() { start_ts_ = std::chrono::high_resolution_clock::now(); }
    ~UpdateTimer() = default;
    float Update();
    float GetDeltaTime() const { return last_update_ts_; }

  private:
    float last_update_ts_{0.0}; // milisecond
    std::chrono::high_resolution_clock::time_point start_ts_;
};

struct ApplicationData
{
    void *window_handle;
};

class ApplicationBase
{
  public:
    ApplicationBase();
    virtual ~ApplicationBase();

    virtual void Setup();
    virtual void Run();
    virtual void RequestEndApplication();
    virtual void Exit();
    virtual void OnTick() = 0;

    void RegisterSlotUpdate(UpdateFunc func);

  protected:
    void RegisterModule(const std::string &id, Module *module_);
    void UnregisterModule(const std::string &id);
    void ReleaseAllModule();

  private:
    std::unordered_map<std::string, Module *> modules_;
    ApplicationState state_{ApplicationState::None};
    std::vector<UpdateFunc> updaters_;
    UpdateTimer update_timer_;
    js::JobSystem js_;
};
} // namespace cloud