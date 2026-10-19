#pragma once

#include "polyec/jobs/job.hpp"

namespace polyec {

class AppointedOfficeJob : public Job {
public:
    JobType type() const noexcept override { return JobType::AppointedOffice; }
    std::string_view name() const noexcept override { return "AppointedOffice"; }
    bool is_work_job() const noexcept override { return false; }
};

inline const AppointedOfficeJob kAppointedOfficeJob;

} // namespace polyec
