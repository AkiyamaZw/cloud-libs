#pragma once
#include "job_define.h"

namespace cloud::js
{
class JobCounterEntry;
class JobSystem;
class Counter
{
  public:
    Counter(JobSystem &js);
    ~Counter();
    Counter(const Counter &rhs);
    Counter(Counter &&rhs) noexcept;
    Counter &operator=(const Counter &rhs);
    Counter &operator=(Counter &&rhs);
    Counter &operator+=(const Counter &rhs);
    bool operator==(const Counter &rhs) const;
    void finish_submit_job();
    bool finished() const;
    JobCounterEntry *get_entry() const { return entry_; };
    void set_entry(JobCounterEntry *entry) { entry_ = entry; }
    uint32_t get_cnt() const;

  private:
    JobCounterEntry *entry_{nullptr};
};

} // namespace cloud::js