#include "polyec/realm.hpp"

#include <algorithm>
#include <limits>
#include <unordered_set>

namespace polyec {

std::vector<uint64_t> Realm::eligible_muster_members(uint64_t commander,int region,int required_until)const{
    std::vector<uint64_t> result;
    for(const auto& candidate:people_){
        const auto& service=candidate.military_service;
        if(!candidate.alive||candidate.family!=Family::Civilian||candidate.region!=region||candidate.destination!=-1||
           !service.active||service.commander!=commander||service.source_region!=region||
           service.mobilized_until_day<required_until||service.end_day<required_until)continue;
        bool assigned=false;
        for(const auto& obligation:military_obligations_)
            if(obligation.completed_day<0&&std::find(obligation.member_ids.begin(),obligation.member_ids.end(),candidate.id)!=obligation.member_ids.end()){
                assigned=true;break;
            }
        if(!assigned)result.push_back(candidate.id);
    }
    return result;
}

void Realm::process_military_obligations_day(){
    for(auto& obligation:military_obligations_){
        if(obligation.completed_day>=0)continue;
        for(const auto member_id:obligation.member_ids){
            if(std::find(obligation.delivered_member_ids.begin(),obligation.delivered_member_ids.end(),member_id)!=obligation.delivered_member_ids.end())continue;
            const auto& member=person(member_id);
            const auto& service=member.military_service;
            if(member.alive&&service.active&&service.commander==obligation.obligor&&
               service.source_region==obligation.source_region&&service.mobilized_until_day>day_&&
               service.end_day>day_&&member.destination==-1&&member.region==obligation.destination_region)
                obligation.delivered_member_ids.push_back(member_id);
        }
        const int delivered=int(obligation.delivered_member_ids.size());
        if(delivered>=obligation.required_members){
            obligation.completed_day=day_;
            // The food offer is tied to this volunteer assignment. If the
            // assignment finishes before its pending offer is accepted, close
            // that offer instead of leaving an impossible stale proposal.
            for(auto& proposal:proposals_)
                if(proposal.kind==7&&!proposal.resolved&&proposal.cause==obligation.id)
                    refund_proposal(proposal);
            continue;
        }
        if(day_>=obligation.due_day&&!obligation.due_checked){
            const int shortage=obligation.required_members-delivered;
            if(!obligation.first_deadline_checked){
                obligation.first_deadline_checked=true;
                obligation.delivered_at_first_deadline=delivered;
                obligation.shortage_at_first_deadline=shortage;
            }
            obligation.due_checked=true;
            obligation.delivered_at_last_deadline=delivered;
            obligation.shortage_at_last_deadline=shortage;
            if(shortage>0&&next_fact_!=UINT64_MAX)
                event(obligation.obligor,obligation.source_region,230,shortage,obligation.recipient,false,obligation.id);
        }
    }
}

void Realm::process_military_supply_obligations_day(){
    for(auto& obligation:military_supply_obligations_){
        if(obligation.completed_day>=0)continue;
        int64_t delivered=0;
        for(const auto& shipment:transport_shipments_){
            if(shipment.id<obligation.first_eligible_shipment_id||
               shipment.owner_kind!=TransportShipmentOwnerKind::MilitaryRationDepot||
               shipment.owner!=obligation.obligor||shipment.source_region!=obligation.source_region||
               shipment.destination!=obligation.destination_region||shipment.good!=int(GoodId::DryFood))continue;
            if(shipment.delivered>std::numeric_limits<int64_t>::max()-delivered)
                throw std::overflow_error("military supply obligation delivery overflow");
            delivered+=shipment.delivered;
        }
        delivered=std::min(delivered,obligation.required);
        obligation.delivered=delivered;
        if(delivered>=obligation.required){
            obligation.completed_day=day_;
            // There is no shortage to extend after all promised food arrives.
            for(auto& proposal:proposals_)
                if(proposal.kind==8&&!proposal.resolved&&proposal.cause==obligation.id)
                    refund_proposal(proposal);
            continue;
        }
        if(day_>=obligation.due_day&&!obligation.due_checked){
            const int64_t shortage=obligation.required-delivered;
            if(!obligation.first_deadline_checked){
                obligation.first_deadline_checked=true;
                obligation.delivered_at_first_deadline=delivered;
                obligation.shortage_at_first_deadline=shortage;
            }
            obligation.due_checked=true;
            obligation.delivered_at_last_deadline=delivered;
            obligation.shortage_at_last_deadline=shortage;
            if(shortage>0&&next_fact_!=UINT64_MAX)
                event(obligation.obligor,obligation.source_region,234,shortage,obligation.recipient,false,obligation.id);
        }
    }
}

} // namespace polyec
