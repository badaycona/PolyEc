#pragma once

#include "polyec/jobs/job.hpp"

namespace polyec {

class WorkerJob : public Job {
public:
    JobType type() const noexcept override { return JobType::Worker; }
    std::string_view name() const noexcept override { return "Worker"; }
    bool is_work_job() const noexcept override { return true; }
};

inline const WorkerJob kWorkerJob;

} // namespace polyec
