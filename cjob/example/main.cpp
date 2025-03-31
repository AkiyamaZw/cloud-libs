
#include "job_system.h"
#include <iostream>
#include <format>
#include <thread>
#include "job_builder.h"

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
    cloud::JobSystem js(1);
    js.adopt();

    cloud::Counter counter = js.create_counter();

    cloud::JobBuilder builder(js);
    builder.dispatch("first_job1", [](cloud::JobArgs &) {
        std::cout << "first_job1" << std::endl;
    });
    builder.dispatch("first_job2", [](cloud::JobArgs &) {
        std::cout << "first_job2" << std::endl;
    });
    builder.dispatch("first_job3", [](cloud::JobArgs &) {
        std::cout << "first_job3" << std::endl;
    });
    counter += builder.extract_wait_counter();

    cloud::JobBuilder builder2(js);
    builder2.dispatch("second_job", [](cloud::JobArgs &) {
        std::cout << "second_job" << std::endl;
    });
    counter += builder2.extract_wait_counter();

    cloud::JobBuilder builder3(js);
    builder3.dispatch_wait(counter);
    builder3.dispatch("third_job", [](cloud::JobArgs &) {
        std::cout << "third_job" << std::endl;
    });
    builder3.dispatch_fence_explicitly();
    builder3.dispatch("third_job2", [](cloud::JobArgs &) {
        std::cout << "third_job2" << std::endl;
    });
    builder3.dispatch_fence_explicitly();
    builder3.dispatch("third_job3", [](cloud::JobArgs &) {
        std::cout << "third_job3" << std::endl;
    });
    // cloud::RunContext::get_context()->export_grapviz("./graph.dot");

    js.spin_wait(counter.get_entry());
    js.spin_wait(builder3.extract_wait_counter().get_entry());
    js.emancipate();
}

void test_counter_2()
{
    cloud::JobSystem js(5);
    js.adopt();
    cloud::Counter counter = js.create_counter();
    cloud::JobBuilder builder(js);
    builder.dispatch(
        "job_a", [](cloud::JobArgs &) { std::cout << "job_a" << std::endl; });
    builder.dispatch(
        "job_b", [](cloud::JobArgs &) { std::cout << "job_b" << std::endl; });
    counter += builder.extract_wait_counter();

    cloud::JobBuilder builder3(js);
    builder3.dispatch(
        "job_e", [](cloud::JobArgs &) { std::cout << "job_e" << std::endl; });
    counter += builder3.extract_wait_counter();

    cloud::JobBuilder builder2(js);
    builder2.dispatch_wait(counter);
    builder2.dispatch(
        "job_c", [](cloud::JobArgs &) { std::cout << "job_c" << std::endl; });

    cloud::JobBuilder builder4(js);
    builder4.dispatch_wait(counter);
    builder4.dispatch(
        "job_d", [](cloud::JobArgs &) { std::cout << "job_d" << std::endl; });

    js.spin_wait(builder4.extract_wait_counter());
    js.spin_wait(builder3.extract_wait_counter());
    js.emancipate();
}

int main()
{
    {
        Timer t("job system");
        test_counter();
    }
    {
        Timer t("job system counter test 2");
        test_counter_2();
    }
    return 0;
}