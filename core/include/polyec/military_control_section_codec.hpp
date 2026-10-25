#pragma once

#include "polyec/realm.hpp"

#include <array>
#include <algorithm>
#include <istream>
#include <ostream>
#include <stdexcept>

namespace polyec::detail {

inline void write_military_control_section(std::ostream& out,
        const std::array<MilitaryOccupation,kAtlasTerritoryCount>& occupations){
    out<<" MILITARY_CONTROL_1 "<<occupations.size()<<' ';
    for(const auto& occupation:occupations)
        out<<occupation.partner_id<<' '<<occupation.garrison_units<<' ';
}

inline void read_military_control_section(std::istream& in,
        std::array<MilitaryOccupation,kAtlasTerritoryCount>& occupations,int regions){
    std::string tag;size_t count=0;in>>tag>>count;
    if(!in||tag!="MILITARY_CONTROL_1"||count!=kAtlasTerritoryCount)
        throw std::runtime_error("Save military control section header");
    std::array<MilitaryOccupation,kAtlasTerritoryCount> staged{};
    for(auto& occupation:staged){
        in>>occupation.partner_id>>occupation.garrison_units;
        if(!in||occupation.partner_id< -1||occupation.partner_id>=kForeignTradePartnerCount||
           occupation.garrison_units<0||occupation.garrison_units>kMilitaryRaidUnitMaximum||
           ((occupation.partner_id<0)!=(occupation.garrison_units==0)))
            throw std::runtime_error("Save military control record");
    }
    if(regions!=int(kAtlasTerritoryCount)-1&&
       std::any_of(staged.begin(),staged.end(),[](const auto& occupation){return occupation.occupied();}))
        throw std::runtime_error("Save military occupation outside Atlas profile");
    occupations=staged;
}

} // namespace polyec::detail
