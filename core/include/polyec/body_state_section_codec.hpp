#pragma once

#include "polyec/realm.hpp"
#include "polyec/world_atlas.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace polyec::detail {

inline void write_body_state_section(std::ostream& out,const std::vector<Person>& people){
    std::size_t count=0;
    for(const auto& person:people){
        const auto& body=person.body_state;
        if(body.sleep_debt!=0||body.exertion!=0||body.sleep_balance_remainder!=0||
           body.exertion_effort_attention_processed!=0||body.exertion_effort_attention_remainder!=0||
           body.exertion_rest_tick_remainder!=0||
           body.rest_begin_tick!=0||body.rest_until_tick!=0||body.rest_region!=-1||body.rest_health_pending)++count;
    }
    out<<" BODY_3 "<<count<<' ';
    for(const auto& person:people){
        const auto& body=person.body_state;
        if(body.sleep_debt==0&&body.exertion==0&&body.sleep_balance_remainder==0&&
           body.exertion_effort_attention_processed==0&&body.exertion_effort_attention_remainder==0&&
           body.exertion_rest_tick_remainder==0&&
           body.rest_begin_tick==0&&body.rest_until_tick==0&&body.rest_region==-1&&!body.rest_health_pending)continue;
        out<<person.id<<' '<<body.sleep_debt<<' '<<body.exertion<<' '<<body.sleep_balance_remainder<<' '
           <<body.exertion_effort_attention_processed<<' '<<body.exertion_effort_attention_remainder<<' '
           <<body.exertion_rest_tick_remainder<<' '<<body.rest_begin_tick<<' '<<body.rest_until_tick<<' '
           <<body.rest_region<<' '<<body.rest_health_pending<<' ';
    }
}

inline void read_body_state_section(std::istream& in,std::vector<Person>& people,int regions,int realm_schema){
    std::string tag;std::size_t count=0;in>>tag>>count;
    const bool v1=realm_schema==18;
    const bool v3=realm_schema>=30;
    if(!in||tag!=(v1?"BODY_1":(v3?"BODY_3":"BODY_2"))||count>people.size())
        throw std::runtime_error("Save body state header");
    std::vector<std::pair<uint64_t,BodyState>> staged;staged.reserve(count);
    uint64_t previous=0;
    for(std::size_t row=0;row<count;++row){
        uint64_t actor=0;BodyState body;
        in>>actor>>body.sleep_debt>>body.exertion;
        if(v1)body.rest_region=-1;
        else if(v3)in>>body.sleep_balance_remainder>>body.exertion_effort_attention_processed>>
            body.exertion_effort_attention_remainder>>body.exertion_rest_tick_remainder>>
            body.rest_begin_tick>>body.rest_until_tick>>body.rest_region>>body.rest_health_pending;
        else in>>body.sleep_balance_remainder>>body.rest_begin_tick>>body.rest_until_tick>>body.rest_region>>body.rest_health_pending;
        if(!in||actor==0||actor>people.size()||(row&&actor<=previous)||
           (body.sleep_debt==0&&body.exertion==0&&body.sleep_balance_remainder==0&&
            body.exertion_effort_attention_processed==0&&body.exertion_effort_attention_remainder==0&&
            body.exertion_rest_tick_remainder==0&&body.rest_begin_tick==0&&
            body.rest_until_tick==0&&body.rest_region==-1&&!body.rest_health_pending)||
           body.sleep_debt<0||body.sleep_debt>100||body.exertion<0||body.exertion>100||
           body.sleep_balance_remainder<-63||body.sleep_balance_remainder>63||
           body.exertion_effort_attention_processed<0||body.exertion_effort_attention_remainder<0||
           body.exertion_effort_attention_remainder>=16000||body.exertion_rest_tick_remainder<0||
           body.exertion_rest_tick_remainder>=16||
           body.rest_begin_tick<0||body.rest_until_tick<0||regions<0||regions>int(kAtlasTerritoryCount)-1||
           (body.rest_until_tick==0?(body.rest_begin_tick!=0||body.rest_region!=-1||body.rest_health_pending):
             (body.rest_begin_tick>=body.rest_until_tick||body.rest_region<0||body.rest_region>regions||!body.rest_health_pending)))
            throw std::runtime_error("Save body state fields");
        staged.emplace_back(actor,body);previous=actor;
    }
    for(const auto& [actor,body]:staged)people[std::size_t(actor-1)].body_state=body;
}

} // namespace polyec::detail
