#pragma once
#include "polyec/work_credit.hpp"
#include "polyec/world_atlas.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>

namespace polyec::detail {
inline void write_work_credit_section(std::ostream& out,const WorkCreditRegions& credits){
    out<<" WORKCREDIT_1 "<<credits.size()<<' ';
    for(const auto& [key,value]:credits)out<<key.first<<' '<<key.second<<' '<<value.legacy_units<<' '
        <<value.unassigned_units<<' '<<value.bridge_units<<' ';
}
inline WorkCreditRegions read_work_credit_section(std::istream& in,std::size_t people,int regions){
    std::string tag;std::size_t count=0;in>>tag>>count;
    if(!in||tag!="WORKCREDIT_1"||regions<0||regions>int(kAtlasTerritoryCount)-1||people>1000000||count>people*std::size_t(regions+1))
        throw std::runtime_error("Save work credit header");
    WorkCreditRegions result;std::pair<uint64_t,int> previous{};
    for(std::size_t row=0;row<count;++row){
        uint64_t actor=0;int region=-1;WorkRegionCredit value;
        in>>actor>>region>>value.legacy_units>>value.unassigned_units>>value.bridge_units;
        const std::pair<uint64_t,int> key{actor,region};
        if(!in||actor==0||actor>people||region<0||region>regions||(row&&key<=previous)||value.legacy_units<=0||
           value.unassigned_units<0||value.bridge_units<0||value.unassigned_units>value.legacy_units||
           value.bridge_units>value.legacy_units-value.unassigned_units)throw std::runtime_error("Save work credit fields");
        result.emplace(key,value);previous=key;
    }
    return result;
}
}
