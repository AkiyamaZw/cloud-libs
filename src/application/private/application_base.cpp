#include "application_base.h"
#include <iostream>
#include <chrono>
#include <thread>
#include "utility.h"
namespace cloud
{
float UpdateTimer::Update()
{
    auto ts = std::chrono::high_resolution_clock::now();
    last_update_ts_ = std::chrono::duration_cast<std::chrono::milliseconds>(ts - start_ts_).count();
    start_ts_ = ts;
    return last_update_ts_;
}

Module::Module()
    : id_("Unknown")
{
}

Module::Module(const std::string &module_id)
    : id_(module_id)
{
}

Module::~Module() = default;

void Module::OnDestroy() {}

ApplicationBase::ApplicationBase() {}

ApplicationBase::~ApplicationBase() {}

void ApplicationBase::Setup() { state_ = ApplicationState::SETUP; }

void ApplicationBase::Run()
{
    if (state_ != ApplicationState::SETUP)
        return;
    state_ = ApplicationState::RUNNING;
    while (state_ != ApplicationState::CLOSING)
    {
        float dt = update_timer_.Update();
        INFO("good is now in update with dt: {}", dt);
        for (auto &update_func : updaters_)
        {
            update_func(dt);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        OnTick();
    }
    Exit();
}

void ApplicationBase::RequestEndApplication()
{
    if (int(state_) < int(ApplicationState::CLOSING))
    {
        state_ = ApplicationState::CLOSING;
    }
}

void ApplicationBase::Exit()
{
    if (state_ != ApplicationState::CLOSING)
        return;
    ReleaseAllModule();
    updaters_.clear();
    state_ = ApplicationState::CLOSED;
}

void ApplicationBase::RegisterSlotUpdate(UpdateFunc func) { updaters_.push_back(func); }

void ApplicationBase::RegisterModule(const std::string &id, Module *module_)
{
    if (auto iter = modules_.find(id); iter == modules_.end())
    {
        modules_.emplace(id, module_);
    }
    else
    {
        ERROR("module with id has registered");
    }
}

void ApplicationBase::UnregisterModule(const std::string &id)
{
    auto iter = modules_.find(id);
    if (iter == modules_.end())
        return;
    auto module = iter->second;
    modules_.erase(iter);
    module->OnDestroy();
}

void ApplicationBase::ReleaseAllModule()
{
    for (auto [key, module] : modules_)
    {
        module->OnDestroy();
    }
    modules_.clear();
}

} // namespace cloud