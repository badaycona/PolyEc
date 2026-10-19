#pragma once

#include <string_view>

namespace polyec {

enum class EmploymentState { Idle = 0, Employed = 1 };
enum class JobType { None = 0, Worker = 1, Teacher = 2, AppointedOffice = 3 };

class Job {
public:
    virtual ~Job() = default;
    virtual JobType type() const noexcept = 0;
    virtual std::string_view name() const noexcept = 0;
    virtual bool is_work_job() const noexcept = 0; // Worker/Teacher true; AppointedOffice false
};

} // namespace polyec
