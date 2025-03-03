
#include "job_system.h"
#include <iostream>
#include <format>
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

void task2(long *result, cloud::JobSystem &job_sys, int job_count)
{
    // cloud::JobContext ctx;
    // Timer timer(std::format("job System with split count: {}", job_count));
    // job_sys.Dispatch(
    //     ctx,
    //     job_count,
    //     1,
    //     [&result, job_count](cloud::JobArgs args) {
    //         int group_size = array_size / job_count;
    //         for (int i = 0; i < group_size; ++i)
    //         {
    //             for (long j = 0; j < target_num; j++)
    //             {
    //                 result[args.group_id * (group_size) + i] += 1;
    //             }
    //         }
    //     },
    //     0);
    // job_sys.Wait(ctx);
}

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

void print_job(std::string name) { std::cout << name << std::endl; }

int test_system_2()
{
    cloud::JobSystem job_sys(std::thread::hardware_concurrency());
    std::cout << "core count:" << std::thread::hardware_concurrency()
              << std::endl;
    cloud::Job *root_job = job_sys.create_parent_job(nullptr);
    cloud::Job *sub_job1 = job_sys.create(
        root_job, [](cloud::JobArgs &args) { print_job("sub_job1"); });
    cloud::Job *sub_job2 = job_sys.create(
        root_job, [](cloud::JobArgs &args) { print_job("sub_job2"); });

    job_sys.run(sub_job1);
    job_sys.run(sub_job2);
    job_sys.run_and_wait(root_job);
}

int main()
{
    // SimpleAddSample();
    return 0;
}