#pragma once

#include "polyec/jobs/job.hpp"

namespace polyec {

class TeacherJob : public Job {
public:
    JobType type() const noexcept override { return JobType::Teacher; }
    std::string_view name() const noexcept override { return "Teacher"; }
    bool is_work_job() const noexcept override { return true; }
};

inline const TeacherJob kTeacherJob;

} // namespace polyec
