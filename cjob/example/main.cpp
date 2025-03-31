
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

const int array_size = 100;
const long target_num = 100000000;

void task1(long *result)
{
    Timer timer("for loop");
    for (long i = 0; i < array_size; i++)
    {
        for (long j = 0; j < target_num; j++)
        {
            result[i] += 1;
        }
    }
}

void task2(long *result, cloud::JobSystem &job_sys, int job_count) {}

void TestInit(long *data)
{
    for (int i = 0; i < array_size; i++)
    {
        data[i] = 0;
    }
}

bool CheckResult(long *data)
{
    bool result = true;
    for (int i = 0; i < array_size; ++i)
    {
        if (data[i] != target_num)
        {
            result = false;
            std::cout << i << " " << data[i] << std::endl;
            break;
        }
    }
    return result;
}

void BenchMark()
{
    long data[array_size];
    TestInit(data);
    task1(data);
    std::cout << "benchmark result right:" << CheckResult(data) << std::endl;
    std::cout << "-------------------------------------" << std::endl;
}

void TaskJob(cloud::JobSystem &job_sys, int split_job)
{
    long data[array_size];
    TestInit(data);
    task2(data, job_sys, split_job);
    std::cout << "result right:" << CheckResult(data) << std::endl;
    std::cout << "-------------------------------------" << std::endl;
}

void SimpleAddSample()
{
    {
        BenchMark();
    }
    {
        cloud::JobSystem job_sys(std::thread::hardware_concurrency());
        std::cout << "core count:" << std::thread::hardware_concurrency()
                  << std::endl;

        int split_config[] = {2, 4, 10, 20, 50};
        for (int i = 0; i < 5; ++i)
        {
            TaskJob(job_sys, split_config[i]);
        }
    }
}

void print_job(std::string name)
{
    uint64_t k = 0;
    uint64_t max_num = 1000000000;
    for (uint64_t i = 0; i < max_num; ++i)
    {
        k += 1;
    }
    std::cout << name << " " << (max_num == k) << std::endl;
}

int test_system_2()
{
    cloud::JobSystem job_sys(std::thread::hardware_concurrency());
    job_sys.adopt();
    std::cout << "core count:" << std::thread::hardware_concurrency()
              << std::endl;

    // auto simulate_job = job_sys.create(nullptr);
    // auto sub_job1 = job_sys.create(
    //     simulate_job, [](cloud::JobArgs &args) { print_job("sub_job1"); });
    // auto sub_job2 = job_sys.create(
    //     sub_job1, [](cloud::JobArgs &args) { print_job("sub_job2"); });
    // auto sub_job3 = job_sys.create(
    //     sub_job2, [](cloud::JobArgs &args) { print_job("sub_job3"); });

    // auto vis_job = job_sys.create(nullptr);
    // auto vis_job1 = job_sys.create(
    //     vis_job, [](cloud::JobArgs &args) { print_job("vis_job1"); });
    // auto vis_job2 = job_sys.create(
    //     vis_job, [](cloud::JobArgs &args) { print_job("vis_job2"); });

    // auto extract_job = job_sys.create(nullptr);
    // auto extract_job1 = job_sys.create(
    //     extract_j/ob, [](cloud::JobArgs &args) { print_job("extract_job1");
    //     });
    // auto extract_job2 = job_sys.create(
    //     extract_job, [](cloud::JobArgs &args) { print_job("extract_job2");
    //     });

    // auto render_job = job_sys.create(nullptr);
    // auto render_job1 = job_sys.create(
    //     render_job, [](cloud::JobArgs &args) { print_job("render_job1"); });
    // auto render_job2 = job_sys.create(
    //     render_job, [](cloud::JobArgs &args) { print_job("render_job2"); });

    // job_sys.run(sub_job1);
    // job_sys.run(sub_job2);
    // job_sys.run(sub_job3);
    // job_sys.run_and_wait(simulate_job);
    // job_sys.run(vis_job1);
    // job_sys.run(vis_job2);
    // job_sys.run_and_wait(vis_job);
    // job_sys.run(extract_job1);
    // job_sys.run(extract_job2);
    // job_sys.run_and_wait(extract_job);

    // job_sys.run(render_job1);
    // job_sys.run(render_job2);
    // job_sys.run_and_wait(render_job);

    job_sys.emancipate();
    return 0;
}
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

    cloud::Counter counter = cloud::Counter::create(js);

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
    cloud::Counter counter = cloud::Counter::create(js);
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

    js.spin_wait(builder4.extract_wait_counter().get_entry());
    js.spin_wait(builder3.extract_wait_counter().get_entry());
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