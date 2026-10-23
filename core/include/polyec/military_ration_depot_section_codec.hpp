#pragma once

#include <cstdint>
#include <istream>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace polyec::detail {

inline void write_military_ration_depot_section(std::ostream& out,const std::vector<int64_t>& depots){
    out<<" MILITARY_RATION_DEPOT_1 "<<depots.size()<<' ';
    for(const int64_t stock:depots)out<<stock<<' ';
}

inline void write_military_ration_depot_section_v2(std::ostream& out,const std::vector<int64_t>& dry_food,
                                                    const std::vector<int64_t>& supplementary_food){
    if(dry_food.size()!=supplementary_food.size())throw std::runtime_error("Save military ration depot dimensions");
    out<<" MILITARY_RATION_DEPOT_2 "<<dry_food.size()<<' ';
    for(size_t region=0;region<dry_food.size();++region){
        if(dry_food[region]<0||dry_food[region]>std::numeric_limits<int64_t>::max()/2||
           supplementary_food[region]<0||supplementary_food[region]>std::numeric_limits<int64_t>::max()/2)
            throw std::runtime_error("Save military ration depot stock");
        out<<dry_food[region]<<' '<<supplementary_food[region]<<' ';
    }
}

inline void read_military_ration_depot_section(std::istream& in,std::vector<int64_t>& depots,size_t expected_regions){
    std::string tag;size_t count=0;
    in>>tag>>count;
    if(!in||tag!="MILITARY_RATION_DEPOT_1"||count!=expected_regions)
        throw std::runtime_error("Save military ration depot header");
    std::vector<int64_t> staged(count);
    for(auto& stock:staged){
        in>>stock;
        if(!in||stock<0||stock>std::numeric_limits<int64_t>::max()/2)
            throw std::runtime_error("Save military ration depot stock");
    }
    depots=std::move(staged);
}

inline void read_military_ration_depot_section_v2(std::istream& in,std::vector<int64_t>& dry_food,
                                                  std::vector<int64_t>& supplementary_food,size_t expected_regions){
    std::string tag;size_t count=0;
    in>>tag>>count;
    if(!in||tag!="MILITARY_RATION_DEPOT_2"||count!=expected_regions)
        throw std::runtime_error("Save military ration depot header");
    std::vector<int64_t> staged_dry(count),staged_supplementary(count);
    for(size_t region=0;region<count;++region){
        in>>staged_dry[region]>>staged_supplementary[region];
        if(!in||staged_dry[region]<0||staged_dry[region]>std::numeric_limits<int64_t>::max()/2||
           staged_supplementary[region]<0||staged_supplementary[region]>std::numeric_limits<int64_t>::max()/2)
            throw std::runtime_error("Save military ration depot stock");
    }
    dry_food=std::move(staged_dry);
    supplementary_food=std::move(staged_supplementary);
}

} // namespace polyec::detail
