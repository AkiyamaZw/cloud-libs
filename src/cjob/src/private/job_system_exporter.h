#pragma once
#include "job_system_extension.h"

namespace cloud::js
{
class JobCounterEntry;
class JobWaitEntry;
class JobSystemExporter : public JobSystemProxy
{
  public:
    using Super::JobSystemProxy;
    void export_grapviz(const std::string &path);

  private:
    std::string counter_grapviz(const JobCounterEntry *counter);
    std::string job_graphviz(const JobWaitEntry &job) const;
};
} // namespace cloud::js
