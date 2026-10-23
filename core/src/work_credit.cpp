#include "polyec/work_credit.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace polyec {
namespace {
int64_t add(int64_t a,int64_t b){
    if(a<0||b<0||b>std::numeric_limits<int64_t>::max()-a)throw std::overflow_error("work credit sum overflow");
    return a+b;
}
int64_t multiply(int64_t a,int64_t b){
    if(a<0||b<0||(a&&b>std::numeric_limits<int64_t>::max()/a))throw std::overflow_error("work credit product overflow");
    return a*b;
}
void validate_sources(std::span<const WorkCreditSource> sites){
    if(sites.size()>1000000)throw std::logic_error("work credit source limit");
    uint64_t previous=0;
    for(const auto& site:sites){
        if(site.establishment_id<=previous||site.eligible_units<0||site.credited_units<0||site.credited_units>site.eligible_units)
            throw std::logic_error("invalid work credit source");
        previous=site.establishment_id;
    }
}
}
WorkCreditAllocation initialize_work_credit(int64_t legacy_units,int64_t unassigned_eligible,
                                            std::span<const WorkCreditSource> sites){
    if(legacy_units<0||unassigned_eligible<0)throw std::logic_error("invalid initial work credit");
    validate_sources(sites);
    WorkCreditAllocation result;result.sites.assign(sites.begin(),sites.end());
    int64_t credited=unassigned_eligible;
    for(auto& site:result.sites){
        if(site.credited_units!=0)throw std::logic_error("work credit already initialized");
        site.credited_units=site.eligible_units;credited=add(credited,site.credited_units);
    }
    if(credited>legacy_units)throw std::logic_error("initial credit exceeds regional entitlement");
    result.region={legacy_units,unassigned_eligible,legacy_units-credited};
    result.regional_entitlement=legacy_units;
    return result;
}
WorkCreditAllocation allocate_work_credit(const WorkRegionCredit& region,int64_t unassigned_eligible,
                                          std::span<const WorkCreditSource> sites,int64_t day){
    if(day<0||region.legacy_units<0||region.unassigned_units<0||region.bridge_units<0||
       unassigned_eligible<region.unassigned_units)throw std::logic_error("invalid regional work credit");
    validate_sources(sites);
    int64_t credited=add(region.unassigned_units,region.bridge_units);
    for(const auto& site:sites)credited=add(credited,site.credited_units);
    if(credited!=region.legacy_units)throw std::logic_error("work credit conservation mismatch");
    WorkCreditAllocation result{region,{sites.begin(),sites.end()},0};
    struct Available {std::size_t slot;int64_t units,allocated=0;};
    std::vector<Available> available;available.reserve(sites.size()+1);
    int64_t total=0;
    if(unassigned_eligible>region.unassigned_units){
        const int64_t units=unassigned_eligible-region.unassigned_units;
        available.push_back({0,units});total=add(total,units);
    }
    for(std::size_t index=0;index<sites.size();++index){
        const int64_t units=sites[index].eligible_units-sites[index].credited_units;
        if(units){available.push_back({index+1,units});total=add(total,units);}
    }
    const int64_t budget=std::min(total,region.bridge_units);
    int64_t left=budget;
    if(budget){
        for(auto& claim:available){
            claim.allocated=budget==total?claim.units:multiply(budget,claim.units)/total;
            left-=claim.allocated;
        }
        const std::size_t start=std::size_t(uint64_t(day)%available.size());
        for(std::size_t offset=0;left&&offset<available.size();++offset){
            auto& claim=available[(start+offset)%available.size()];
            if(claim.allocated<claim.units){++claim.allocated;--left;}
        }
        if(left)throw std::logic_error("incomplete work credit allocation");
        for(const auto& claim:available){
            if(claim.slot==0)result.region.unassigned_units=add(result.region.unassigned_units,claim.allocated);
            else result.sites[claim.slot-1].credited_units=add(result.sites[claim.slot-1].credited_units,claim.allocated);
        }
        result.region.bridge_units-=budget;
    }
    result.regional_entitlement=add(region.legacy_units,unassigned_eligible-result.region.unassigned_units);
    return result;
}
}
