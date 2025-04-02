
#include "job_system.h"
#include <iostream>
#include <format>
#include <thread>
#include "job_builder.h"
#include <csignal>

using namespace cloud::js;
class Timer
{
  public:
    Timer(std::string &&in_name)
        : name(std::move(in_name))
    {
        start = std::chrono::steady_clock::now();
    }
    ~Timer()
    {
        double duration = std::chrono::duration_cast<std::chrono::microseconds>(
                              std::chrono::steady_clock::now() - start)
                              .count() /
                          1000;

        std::cout << name << ": " << duration << " (ms)" << std::endl;
    }

  private:
    std::chrono::steady_clock::time_point start;
    std::string name;
};

void massive_job()
{
    uint64_t c = 0;
    for (uint64_t i = 0; i < 1000000000; ++i)
    {
        c = i / 2;
    }
}
void test_counter()
{
    JobSystem js(1);
    js.adopt();

    {
        Counter counter(js);
        JobBuilder builder(js);
        builder.dispatch("first_job1", [](JobArgs &) {
            std::cout << "first_job1" << std::endl;
        });
        builder.dispatch("first_job2", [](JobArgs &) {
            std::cout << "first_job2" << std::endl;
        });
        builder.dispatch("first_job3", [](JobArgs &) {
            std::cout << "first_job3" << std::endl;
        });
        counter += builder.extract_wait_counter();

        JobBuilder builder2(js);
        builder2.dispatch("second_job", [](JobArgs &) {
            std::cout << "second_job" << std::endl;
        });
        counter += builder2.extract_wait_counter();

        JobBuilder builder3(js);
        builder3.dispatch_wait(counter);
        builder3.dispatch("third_job", [](JobArgs &) {
            std::cout << "third_job" << std::endl;
        });
        builder3.dispatch_fence_explicitly();
        builder3.dispatch("third_job2", [](JobArgs &) {
            std::cout << "third_job2" << std::endl;
        });
        builder3.dispatch_fence_explicitly();
        builder3.dispatch("third_job3", [](JobArgs &) {
            std::cout << "third_job3" << std::endl;
        });
        // cloud::RunContext::get_context()->export_grapviz("./graph.dot");

        js.spin_wait(counter.get_entry());
        js.spin_wait(builder3.extract_wait_counter().get_entry());
    }
    js.emancipate();
}

void test_counter_2()
{
    JobSystem js(5);
    js.adopt();
    Counter counter(js);
    {

        JobBuilder builder(js);
        builder.dispatch("job_a",
                         [](JobArgs &) { std::cout << "job_a" << std::endl; });
        builder.dispatch("job_b",
                         [](JobArgs &) { std::cout << "job_b" << std::endl; });
        counter += builder.extract_wait_counter();

        JobBuilder builder3(js);
        builder3.dispatch("job_e",
                          [](JobArgs &) { std::cout << "job_e" << std::endl; });
        counter += builder3.extract_wait_counter();

        JobBuilder builder2(js);
        builder2.dispatch_wait(counter);
        builder2.dispatch("job_c",
                          [](JobArgs &) { std::cout << "job_c" << std::endl; });

        JobBuilder builder4(js);
        builder4.dispatch_wait(counter);
        builder4.dispatch("job_d",
                          [](JobArgs &) { std::cout << "job_d" << std::endl; });

        js.spin_wait(builder4.extract_wait_counter());
        js.spin_wait(builder3.extract_wait_counter());
    }
    js.emancipate();
}

struct EngineGlobal
{
    EngineGlobal(JobSystem &in_js)
        : js(in_js)
    {
    }
    JobSystem &js;
    bool finished = false;
    std::atomic<uint64_t> frame_index{0};
    std::atomic<uint64_t> render_index{0};
    std::mutex render_mutex;
    std::condition_variable render_signal;
    std::mutex logic_mutex;
    std::condition_variable logic_signal;

    void wait_render()
    {
        if (frame_index.load() <= render_index.load() + 1)
            return;
        std::unique_lock lock(render_mutex);
        render_signal.wait(lock);
    }
    void dispatch_logic(uint64_t index)
    {
        frame_index.fetch_add(1);
        logic_signal.notify_all();
    }
    void wait_logic()
    {
        if (frame_index.load() > render_index.load())
            return;
        std::unique_lock lock(logic_mutex);
        logic_signal.wait(lock);
    }
    void disptach_render()
    {
        render_index.fetch_add(1);
        render_signal.notify_all();
    }
};

void dummy_logic(JobSystem &js, EngineGlobal &global)
{
    js.adopt();
    while (!global.finished)
    {
        global.wait_render();
        auto str = std::format("logic index:{} \n logic start:==>",
                               global.frame_index.load());
        std::cout << str << std::endl;
        JobBuilder builder(js);
        builder.dispatch("ecs_update", [](JobArgs &) {
            std::cout << "ecs_update" << std::endl;
        });
        builder.dispatch_fence_explicitly();
        builder.dispatch("logic_1", [](JobArgs &) {
            std::cout << "scene_update_start" << std::endl;
        });
        builder.dispatch("physcal_update", [](JobArgs &) {
            std::cout << "physcal_update" << std::endl;
        });
        builder.dispatch_fence_explicitly();
        builder.dispatch("visible_udpate", [](JobArgs &) {
            std::cout << "visible_udpate" << std::endl;
        });
        builder.dispatch_fence_explicitly();
        builder.dispatch("exract",
                         [](JobArgs &) { std::cout << "exract" << std::endl; });
        js.spin_wait(builder.extract_wait_counter());
        global.dispatch_logic(global.frame_index.load());
        std::cout << "logic end!" << std::endl;
    }
    js.emancipate();
}

void dummy_render(JobSystem &js, EngineGlobal &global)
{
    js.adopt();

    while (!global.finished)
    {
        global.wait_logic();
        auto str = std::format(" render_index{}, render start ==>",
                               global.render_index.load());
        std::cout << str << std::endl;
        JobBuilder builder(js);
        builder.dispatch("parse exract", [](JobArgs &) {
            std::cout << "parse exract" << std::endl;
        });
        builder.dispatch_fence_explicitly();
        builder.dispatch("prepare rhi", [](JobArgs &) {
            std::cout << "prepare_rhi" << std::endl;
        });
        builder.dispatch_fence_explicitly();
        builder.dispatch("render",
                         [](JobArgs &) { std::cout << "render" << std::endl; });

        js.spin_wait(builder.extract_wait_counter());
        std::cout << "render end==> " << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        global.disptach_render();
    }
    js.emancipate();
}

std::function<void(int)> signal_exit_handler;
void friendly_exit_multithread(int sig)
{
    if (signal_exit_handler)
    {
        signal_exit_handler(sig);
    }
}

void dummy_multithread()
{
    JobSystem js(5);
    EngineGlobal global(js);

    signal_exit_handler = [&](int) {
        std::cout << "try to stop program" << std::endl;
        global.finished = true;
    };
    std::signal(SIGINT, friendly_exit_multithread);
    std::thread logic_thread(dummy_logic, std::ref(js), std::ref(global));
    std::thread render_thread(dummy_render, std::ref(js), std::ref(global));
    while (!global.finished)
    {
        // std::this_thread::sleep_for(std::chrono::seconds(1000));
    }
    logic_thread.join();
    render_thread.join();
}

int main()
{
    //{
    //    Timer t("job system");
    //    test_counter();
    //}
    //{
    //    Timer t("job system counter test 2");
    //    test_counter_2();
    //}
    {
        Timer t("dummy_multithread");
        dummy_multithread();
    }
    return 0;
}