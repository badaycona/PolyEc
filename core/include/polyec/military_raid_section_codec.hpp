#pragma once

#include "polyec/realm.hpp"

#include <algorithm>
#include <istream>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <vector>

namespace polyec::detail {

inline void write_military_raid_section(std::ostream& out,const std::vector<MilitaryRaid>& raids){
    out<<" MILITARY_RAID_1 "<<raids.size()<<' ';
    for(const auto& raid:raids){
        out<<raid.id<<' '<<raid.partner_id<<' '<<raid.target_region<<' '<<raid.created_day<<' '
           <<raid.resolved_day<<' '<<raid.force_units<<' '<<raid.defense_score<<' '
           <<raid.attacker_losses<<' '<<raid.defender_casualties<<' '<<raid.salt_looted<<' '
           <<static_cast<unsigned>(raid.status)<<' '<<raid.defenders.size()<<' ';
        for(const auto member:raid.defenders)out<<member<<' ';
    }
}

inline void read_military_raid_section(std::istream& in,std::vector<MilitaryRaid>& out,
                                       size_t people_count,int regions,int current_day,uint64_t next_id){
    std::string tag;in>>tag;
    if(tag!="MILITARY_RAID_1")throw std::runtime_error("Save military raid section version");
    size_t count=0;in>>count;
    if(!in||count>1000000)throw std::runtime_error("Save military raid count");
    std::vector<MilitaryRaid> staged;staged.reserve(count);
    uint64_t previous_id=0;unsigned pending=0;
    for(size_t i=0;i<count;++i){
        MilitaryRaid raid;unsigned status=0;size_t defenders=0;
        in>>raid.id>>raid.partner_id>>raid.target_region>>raid.created_day>>raid.resolved_day
          >>raid.force_units>>raid.defense_score>>raid.attacker_losses>>raid.defender_casualties
          >>raid.salt_looted>>status>>defenders;
        if(!in||raid.id<=previous_id||raid.id>=next_id||raid.partner_id<0||
           raid.partner_id>=kForeignTradePartnerCount||raid.target_region!=kMilitaryRaidTargetRegion||
           raid.created_day<0||raid.created_day>current_day||raid.force_units<1||
           raid.force_units>kMilitaryRaidUnitMaximum||status>static_cast<unsigned>(MilitaryRaidStatus::Withdrawn)||
           defenders>16||raid.defense_score<0||raid.defense_score>64||raid.attacker_losses<0||
           raid.attacker_losses>raid.force_units||raid.defender_casualties<0||
           raid.defender_casualties>int(defenders)||raid.salt_looted<0||
           raid.salt_looted>kForeignTradePartnerSaltCapacity)
            throw std::runtime_error("Save military raid fields");
        raid.status=static_cast<MilitaryRaidStatus>(status);
        raid.defenders.resize(defenders);
        for(auto& member:raid.defenders)in>>member;
        if(!in||!std::is_sorted(raid.defenders.begin(),raid.defenders.end())||
           std::adjacent_find(raid.defenders.begin(),raid.defenders.end())!=raid.defenders.end()||
           std::any_of(raid.defenders.begin(),raid.defenders.end(),[&](uint64_t member){return member==0||member>people_count;}))
            throw std::runtime_error("Save military raid defenders");
        if(raid.status==MilitaryRaidStatus::Pending){
            if(++pending>1||raid.resolved_day!=-1||raid.defense_score||raid.attacker_losses||
               raid.defender_casualties||raid.salt_looted||!raid.defenders.empty())
                throw std::runtime_error("Save pending military raid");
        }else{
            if(raid.resolved_day<raid.created_day||raid.resolved_day>current_day)
                throw std::runtime_error("Save resolved military raid day");
            if(raid.status==MilitaryRaidStatus::Repelled){
                if(raid.defense_score<=raid.force_units||raid.attacker_losses!=raid.force_units||
                   raid.defender_casualties||raid.salt_looted||raid.defenders.empty())
                    throw std::runtime_error("Save repelled raid outcome");
            }else if(raid.status==MilitaryRaidStatus::Successful){
                const int expected_losses=std::min(raid.force_units,raid.defense_score/2);
                const int expected_casualties=std::min<int>(int(defenders),std::max(1,(raid.force_units+1)/2));
                if(raid.defense_score>raid.force_units||raid.defenders.empty()||
                   raid.attacker_losses!=expected_losses||raid.defender_casualties!=expected_casualties||
                   raid.salt_looted>raid.force_units-raid.attacker_losses)
                    throw std::runtime_error("Save successful raid outcome");
            }else if(raid.status==MilitaryRaidStatus::Withdrawn){
                if(raid.defense_score||raid.attacker_losses||raid.defender_casualties||!raid.defenders.empty()||
                   raid.salt_looted>raid.force_units)
                    throw std::runtime_error("Save withdrawn raid outcome");
            }
        }
        previous_id=raid.id;staged.push_back(std::move(raid));
    }
    if(regions!=int(kAtlasTerritoryCount)-1&&!staged.empty())
        throw std::runtime_error("Save military raid outside Atlas profile");
    out=std::move(staged);
}

} // namespace polyec::detail
