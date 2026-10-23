#pragma once

#include "polyec/realm.hpp"

#include <algorithm>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <unordered_set>

namespace polyec::detail {

inline void write_military_obligation_section(std::ostream& out,const std::vector<MilitaryObligation>& obligations,
    const std::vector<Proposal>& proposals){
    out << " MILITARY_OBLIGATION_1 " << obligations.size() << ' ';
    for(const auto& item:obligations){
        out << item.id << ' ' << item.obligor << ' ' << item.recipient << ' '
            << item.source_region << ' ' << item.destination_region << ' ' << item.created_day << ' '
            << item.first_due_day << ' ' << item.due_day << ' ' << item.completed_day << ' '
            << item.extensions << ' ' << item.required_members << ' '
            << item.delivered_at_first_deadline << ' ' << item.shortage_at_first_deadline << ' '
            << item.delivered_at_last_deadline << ' ' << item.shortage_at_last_deadline << ' '
            << item.first_deadline_checked << ' ' << item.due_checked << ' '
            << item.member_ids.size() << ' ';
        for(const auto id:item.member_ids)out << id << ' ';
        out << item.delivered_member_ids.size() << ' ';
        for(const auto id:item.delivered_member_ids)out << id << ' ';
    }
    size_t pending_rosters=0;for(const auto& proposal:proposals)pending_rosters+=proposal.kind==5;
    out << " MILITARY_OBLIGATION_PROPOSALS_1 " << pending_rosters << ' ';
    for(const auto& proposal:proposals)if(proposal.kind==5){
        out << proposal.id << ' ' << proposal.roster_members.size() << ' ';
        for(const auto id:proposal.roster_members)out << id << ' ';
    }
}

inline void read_military_obligation_section(std::istream& in,std::vector<MilitaryObligation>& obligations,
    std::vector<Proposal>& proposals,size_t people_count,int regions,int current_day,uint64_t next_id){
    std::string tag;size_t count=0;in>>tag>>count;
    if(!in||tag!="MILITARY_OBLIGATION_1"||count>1000000)throw std::runtime_error("Save military obligation header");
    std::vector<MilitaryObligation> staged;staged.reserve(count);uint64_t previous=0;
    for(size_t row=0;row<count;++row){
        MilitaryObligation item;size_t members=0,delivered=0;
        in>>item.id>>item.obligor>>item.recipient>>item.source_region>>item.destination_region>>item.created_day
          >>item.first_due_day>>item.due_day>>item.completed_day>>item.extensions>>item.required_members
          >>item.delivered_at_first_deadline>>item.shortage_at_first_deadline
          >>item.delivered_at_last_deadline>>item.shortage_at_last_deadline
          >>item.first_deadline_checked>>item.due_checked>>members;
        if(!in||members<1||members>4)throw std::runtime_error("Save military obligation members");
        item.member_ids.resize(members);for(auto& id:item.member_ids)in>>id;
        in>>delivered;if(!in||delivered>members)throw std::runtime_error("Save military obligation delivery count");
        item.delivered_member_ids.resize(delivered);for(auto& id:item.delivered_member_ids)in>>id;
        if(!in||item.id<=previous||item.id>=next_id||item.obligor==0||item.obligor>people_count||
           item.recipient==0||item.recipient>people_count||item.source_region<0||item.source_region>regions||
           item.destination_region<0||item.destination_region>regions||item.created_day<0||item.created_day>current_day||
           item.first_due_day!=item.created_day+14||item.due_day<item.first_due_day||item.due_day>current_day+14||
           item.completed_day>current_day||item.completed_day<-1||item.extensions<0||item.extensions>1||
           item.required_members!=int(members)||item.delivered_at_first_deadline<0||
           item.delivered_at_first_deadline>item.required_members||item.shortage_at_first_deadline<0||
           (item.first_deadline_checked&&item.delivered_at_first_deadline+item.shortage_at_first_deadline!=item.required_members)||
           item.delivered_at_last_deadline<0||item.delivered_at_last_deadline>item.required_members||
           item.shortage_at_last_deadline<0||(item.due_checked&&item.delivered_at_last_deadline+item.shortage_at_last_deadline!=item.required_members)||
           (!item.first_deadline_checked&&(item.delivered_at_first_deadline||item.shortage_at_first_deadline))||
           (!item.due_checked&&(item.delivered_at_last_deadline||item.shortage_at_last_deadline))||
           (item.due_checked&&item.due_day>current_day)||
           (item.completed_day>=0&&item.delivered_member_ids.size()!=members)||
           (item.extensions==1&&!item.first_deadline_checked))
            throw std::runtime_error("Save military obligation fields");
        std::unordered_set<uint64_t> unique;
        for(const auto id:item.member_ids)if(id==0||id>people_count||!unique.insert(id).second)
            throw std::runtime_error("Save military obligation roster");
        unique.clear();
        for(const auto id:item.delivered_member_ids)if(!std::binary_search(item.member_ids.begin(),item.member_ids.end(),id)||!unique.insert(id).second)
            throw std::runtime_error("Save military obligation delivery record");
        if(item.completed_day<0&&item.delivered_member_ids.size()>=members)
            throw std::runtime_error("Save completed military obligation status");
        previous=item.id;staged.push_back(std::move(item));
    }
    std::string proposal_tag;size_t proposal_count=0;in>>proposal_tag>>proposal_count;
    if(!in||proposal_tag!="MILITARY_OBLIGATION_PROPOSALS_1"||proposal_count>1000000)
        throw std::runtime_error("Save military obligation proposal header");
    for(auto& proposal:proposals)if(proposal.kind==5)proposal.roster_members.clear();
    uint64_t previous_proposal=0;
    for(size_t row=0;row<proposal_count;++row){
        uint64_t proposal_id=0;size_t member_count=0;in>>proposal_id>>member_count;
        auto found=std::find_if(proposals.begin(),proposals.end(),[&](const Proposal& proposal){return proposal.id==proposal_id&&proposal.kind==5;});
        if(!in||proposal_id<=previous_proposal||found==proposals.end()||member_count<1||member_count>4)
            throw std::runtime_error("Save military obligation proposal fields");
        found->roster_members.resize(member_count);uint64_t previous_member=0;
        for(auto& id:found->roster_members){in>>id;if(!in||id==0||id>people_count||id<=previous_member)throw std::runtime_error("Save military obligation proposal roster");previous_member=id;}
        if(member_count!=size_t(found->amount))throw std::runtime_error("Save military obligation proposal quantity");
        previous_proposal=proposal_id;
    }
    for(const auto& proposal:proposals)if(proposal.kind==5&&proposal.roster_members.empty())
        throw std::runtime_error("Save military obligation proposal roster missing");
    obligations=std::move(staged);
}

} // namespace polyec::detail
