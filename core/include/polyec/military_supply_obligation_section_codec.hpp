#pragma once

#include "polyec/realm.hpp"

#include <algorithm>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <unordered_set>

namespace polyec::detail {

inline void write_military_supply_obligation_section(std::ostream& out,
    const std::vector<MilitarySupplyObligation>& obligations){
    out << " MILITARY_SUPPLY_OBLIGATION_1 " << obligations.size() << ' ';
    for(const auto& item:obligations)
        out << item.id << ' ' << item.personnel_obligation_id << ' ' << item.obligor << ' ' << item.recipient << ' '
            << item.first_eligible_shipment_id << ' ' << item.source_region << ' ' << item.destination_region << ' '
            << item.created_day << ' ' << item.first_due_day << ' ' << item.due_day << ' ' << item.completed_day << ' '
            << item.extensions << ' ' << item.required << ' ' << item.delivered << ' ' << item.delivered_at_first_deadline << ' '
            << item.shortage_at_first_deadline << ' ' << item.delivered_at_last_deadline << ' '
            << item.shortage_at_last_deadline << ' ' << item.first_deadline_checked << ' ' << item.due_checked << ' ';
}

inline void read_military_supply_obligation_section(std::istream& in,
    std::vector<MilitarySupplyObligation>& obligations,const std::vector<MilitaryObligation>& personnel,
    size_t people_count,int regions,int current_day,uint64_t next_id){
    std::string tag;size_t count=0;in>>tag>>count;
    if(!in||tag!="MILITARY_SUPPLY_OBLIGATION_1"||count>1000000)
        throw std::runtime_error("Save military supply obligation header");
    std::vector<MilitarySupplyObligation> staged;staged.reserve(count);
    std::unordered_set<uint64_t> ids,personnel_ids;
    for(size_t row=0;row<count;++row){
        MilitarySupplyObligation item;
        in>>item.id>>item.personnel_obligation_id>>item.obligor>>item.recipient>>item.first_eligible_shipment_id
          >>item.source_region>>item.destination_region>>item.created_day>>item.first_due_day>>item.due_day
          >>item.completed_day>>item.extensions>>item.required>>item.delivered>>item.delivered_at_first_deadline
          >>item.shortage_at_first_deadline>>item.delivered_at_last_deadline>>item.shortage_at_last_deadline
          >>item.first_deadline_checked>>item.due_checked;
        const auto source=std::find_if(personnel.begin(),personnel.end(),[&](const auto& value){
            return value.id==item.personnel_obligation_id&&value.obligor==item.obligor&&value.recipient==item.recipient&&
                value.source_region==item.source_region&&value.destination_region==item.destination_region;
        });
        if(!in||item.id==0||item.id>=next_id||!ids.insert(item.id).second||source==personnel.end()||
           !personnel_ids.insert(item.personnel_obligation_id).second||item.obligor==0||item.obligor>people_count||
           item.recipient==0||item.recipient>people_count||item.source_region<0||item.source_region>regions||
           item.destination_region<0||item.destination_region>regions||item.created_day<0||item.created_day>current_day||
           item.first_due_day!=item.created_day+14||item.due_day<item.first_due_day||item.due_day>current_day+14||
           item.completed_day>current_day||item.completed_day<-1||item.extensions<0||item.extensions>1||
           item.required!=source->required_members||item.delivered<0||item.delivered>item.required||
           item.first_eligible_shipment_id==0||item.first_eligible_shipment_id>next_id||
           item.delivered_at_first_deadline<0||item.delivered_at_first_deadline>item.required||
           item.shortage_at_first_deadline<0||
           (item.first_deadline_checked&&item.delivered_at_first_deadline+item.shortage_at_first_deadline!=item.required)||
           item.delivered_at_last_deadline<0||item.delivered_at_last_deadline>item.required||item.shortage_at_last_deadline<0||
           (item.due_checked&&item.delivered_at_last_deadline+item.shortage_at_last_deadline!=item.required)||
           (!item.first_deadline_checked&&(item.delivered_at_first_deadline||item.shortage_at_first_deadline))||
           (!item.due_checked&&(item.delivered_at_last_deadline||item.shortage_at_last_deadline))||
           (item.due_checked&&item.due_day>current_day)||(item.extensions==1&&!item.first_deadline_checked))
            throw std::runtime_error("Save military supply obligation fields");
        staged.push_back(item);
    }
    obligations=std::move(staged);
}

} // namespace polyec::detail
