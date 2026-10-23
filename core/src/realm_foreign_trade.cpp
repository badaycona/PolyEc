#include "polyec/realm.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace polyec {

RealmResult Realm::execute(const ForeignTradeSuspensionCommand& command){
    if(!command.actor||command.actor>people_.size())return {false,"Unknown actor"};
    auto& actor=mutable_person(command.actor);
    if(!actor.alive)return {false,"Actor deceased"};
    if(actor.decisions==UINT64_MAX||actor.sampling==UINT64_MAX||next_fact_==UINT64_MAX)
        return {false,"Decision or fact counter overflow"};
    if(command.sequence!=actor.decisions+1)return {false,"Stale or duplicate decision ID"};
    if(!actor.sovereign||actor.family!=Family::Governance)
        return {false,"Only the Sovereign may change foreign trade policy"};
    if(regions_!=int(kAtlasTerritoryCount)-1||command.partner_id<0||command.partner_id>=kForeignTradePartnerCount)
        return {false,"Unknown foreign trade partner"};
    auto& partner=foreign_trade_partners_[size_t(command.partner_id)];
    if(partner.trade_suspended==command.suspended)
        return {false,"Foreign trade policy is already in the requested state"};
    partner.trade_suspended=command.suspended;
    actor.last_decision_day=day_;
    ++actor.decisions;
    ++actor.sampling;
    event(actor.id,8,command.suspended?242:243,command.partner_id,0);
    return {true,command.suspended?"New trade with this partner is suspended":"Trade with this partner is reopened",next_fact_-1,uint64_t(command.partner_id)};
}

NationalTransportPathPlan Realm::foreign_trade_route_plan(ForeignTradeOperation operation,int64_t quantity) const {
    NationalTransportPathPlan unavailable;
    if(regions_!=int(kAtlasTerritoryCount)-1||foreign_trade_partner_id(operation)<0)return unavailable;
    const auto atlasLink=std::find_if(kWorldAtlasLinks.begin(),kWorldAtlasLinks.end(),[](const AtlasLink& link){
        return link.from==8&&link.to==9&&link.corridor==AtlasCorridor::Coastal&&
               link.mode==AtlasRouteMode::CoastalShipping&&link.direction==AtlasLinkDirection::Bidirectional;
    });
    if(atlasLink==kWorldAtlasLinks.end())return unavailable;
    const auto linkIndex=size_t(atlasLink-kWorldAtlasLinks.begin());
    const bool importing=foreign_trade_is_import(operation);
    const uint8_t from=importing?9:8;
    const uint8_t to=importing?8:9;
    const auto route=std::find_if(national_transport_routes_.begin(),national_transport_routes_.end(),
        [&](const NationalTransportRoute& candidate){
            return candidate.atlas_link==linkIndex&&candidate.from==from&&candidate.to==to;
        });
    if(route==national_transport_routes_.end())return unavailable;
    auto directCoastalService=national_transport_routes_;
    for(auto& candidate:directCoastalService)candidate.open=false;
    directCoastalService[size_t(route-national_transport_routes_.begin())].open=route->open;
    std::array<int64_t,kAtlasLinkCount> waiting_units_by_link{};
    if(!transport_waiting_units_by_link(waiting_units_by_link))return unavailable;
    return NationalTransportManager::plan(day_,from,to,quantity,TransportServiceGoal::Fastest,
                                          directCoastalService,waiting_units_by_link);
}

RealmResult Realm::execute(const ForeignTradeCommand& command) {
    if (!command.actor || command.actor > people_.size()) return {false, "Unknown actor"};
    auto& actor = mutable_person(command.actor);
    if (!actor.alive) return {false, "Actor deceased"};
    if (actor.decisions == UINT64_MAX || actor.sampling == UINT64_MAX || next_fact_ == UINT64_MAX)
        return {false, "Decision or fact counter overflow"};
    if (command.sequence != actor.decisions + 1) return {false, "Stale or duplicate decision ID"};
    const int partner_id=foreign_trade_partner_id(command.operation);
    if(partner_id<0||partner_id>=kForeignTradePartnerCount)return {false,"Unknown foreign trade partner"};
    const bool importing=foreign_trade_is_import(command.operation);
    auto& partner=foreign_trade_partners_[size_t(partner_id)];
    const int good = importing ? int(GoodId::DryFood) : int(GoodId::Salt);
    const Candidate candidate{Intent::ForeignTrade, 8, good, command.quantity, int(command.operation)};
    std::string reason;
    if (!legal(command.actor, candidate, reason)) return {false, "Foreign trade rejected: " + reason};
    ForeignTradeOrder order;
    order.id = next_id_;
    order.owner = actor.id;
    order.partner_id=partner_id;
    order.operation = command.operation;
    order.good = good;
    order.created_day = day_;
    const auto routePlan=foreign_trade_route_plan(command.operation,command.quantity);
    if(!routePlan.reachable||routePlan.total_days<1||routePlan.total_days>365000-day_)
        return {false,"Foreign trade rejected: coastal route is closed or unavailable"};
    order.arrival_day = day_ + routePlan.total_days;
    order.quantity = command.quantity;
    order.gross = (importing ? 24 : 28) * command.quantity;
    order.duty = order.gross / 20;
    const int64_t port_fee = 2 * command.quantity;
    foreign_trade_orders_.reserve(foreign_trade_orders_.size() + 1);
    if (importing) {
        order.escrow = order.gross + order.duty;
        transfer(actor.cash, partner.escrow, order.escrow);
        partner.goods[size_t(GoodId::DryFood)] -= order.quantity;
    } else {
        order.escrow = order.gross;
        transfer(partner.money, partner.escrow, order.gross);
        actor.inventory[size_t(GoodId::Salt)] -= order.quantity;
        partner.salt_capacity -= order.quantity;
    }
    transfer(actor.cash, provinces_[8].treasury, port_fee);
    for(size_t routeIndex:routePlan.route_indices){
        const auto& route=national_transport_routes_[routeIndex];
        transfer(actor.cash,provinces_[size_t(route.fee_recipient_region)].treasury,
                 route.fee_per_unit*command.quantity);
    }
    ++next_id_;
    foreign_trade_orders_.push_back(order);
    actor.last_decision_day = day_;
    ++actor.decisions;
    ++actor.sampling;
    event(actor.id, 8, 230, order.quantity, order.id);
    return {true, "Foreign trade order accepted; cargo arrives after the coastal crossing", next_fact_ - 1, order.id};
}

RealmResult Realm::execute(const ForeignTradeOfferCommand& command){
    if(!command.actor||command.actor>people_.size())return {false,"Unknown actor"};
    auto& actor=mutable_person(command.actor);
    if(!actor.alive)return {false,"Actor deceased"};
    if(actor.decisions==UINT64_MAX||actor.sampling==UINT64_MAX||next_id_==UINT64_MAX||next_fact_==UINT64_MAX)
        return {false,"Trade offer counter overflow"};
    if(command.sequence!=actor.decisions+1)return {false,"Stale or duplicate decision ID"};
    if(command.partner_id<0||command.partner_id>=kForeignTradePartnerCount)
        return {false,"Unknown foreign trade partner"};
    if(day_>365000-7)return {false,"Trade offer exceeds the game calendar"};
    if(std::any_of(foreign_trade_offers_.begin(),foreign_trade_offers_.end(),[&](const ForeignTradeOffer& offer){
        return offer.partner_id==command.partner_id&&offer.status==ForeignTradeOfferStatus::Pending;
    }))return {false,"This partner already has a pending trade offer"};
    const bool importing=foreign_trade_is_import(command.operation);
    const int good=importing?int(GoodId::DryFood):int(GoodId::Salt);
    const Candidate candidate{Intent::ForeignTrade,8,good,command.quantity,int(command.operation)};
    std::string reason;
    if(!legal(command.actor,candidate,reason))return {false,"Foreign trade offer rejected: "+reason};
    const auto routePlan=foreign_trade_route_plan(command.operation,command.quantity);
    if(!routePlan.reachable||routePlan.total_days<1||routePlan.total_days>365000-day_-7)
        return {false,"Foreign trade offer rejected: coastal route or response window is unavailable"};
    const int64_t gross=(importing?24:28)*command.quantity;
    const int64_t duty=gross/20;
    const int64_t portFee=2*command.quantity;
    const int64_t localFees=portFee+routePlan.total_fee;
    if(localFees<portFee||localFees>std::numeric_limits<int64_t>::max()-gross-duty)
        return {false,"Foreign trade offer accounting overflow"};
    auto& partner=foreign_trade_partners_[size_t(command.partner_id)];
    if(importing&&partner.goods[size_t(GoodId::DryFood)]<command.quantity)
        return {false,"Partner no longer has the offered DryFood"};
    if(!importing&&(partner.salt_capacity<command.quantity||partner.money<gross||
       partner.offer_cash_escrow>std::numeric_limits<int64_t>::max()-gross))
        return {false,"Partner cannot reserve the offered Salt purchase"};
    const int64_t buyerHold=importing?gross+duty+localFees:localFees;
    if(buyerHold<0||actor.cash<buyerHold||buyerHold>std::numeric_limits<int64_t>::max())
        return {false,"Trader cannot reserve the complete trade cost"};
    if(foreign_trade_offers_.size()>=1000000)return {false,"Foreign trade offer limit reached"};
    foreign_trade_offers_.reserve(foreign_trade_offers_.size()+1);
    ForeignTradeOffer offer;
    offer.id=next_id_;offer.owner=actor.id;offer.partner_id=command.partner_id;
    offer.operation=command.operation;offer.good=good;offer.created_day=day_;offer.due_day=day_+7;
    offer.quantity=command.quantity;offer.gross=gross;offer.duty=duty;
    offer.held_good=command.quantity;offer.port_fee=portFee;offer.route_fee=routePlan.total_fee;
    transfer(actor.cash,offer.held_cash, buyerHold);
    if(importing)partner.goods[size_t(GoodId::DryFood)]-=command.quantity;
    else{
        actor.inventory[size_t(GoodId::Salt)]-=command.quantity;
        transfer(partner.money,partner.offer_cash_escrow,gross);
        partner.salt_capacity-=command.quantity;
    }
    ++next_id_;foreign_trade_offers_.push_back(offer);
    actor.last_decision_day=day_;++actor.decisions;++actor.sampling;
    event(actor.id,8,244,command.quantity,offer.id);
    return {true,"Trade offer sent; the partner has seven game days to respond",next_fact_-1,offer.id};
}

void Realm::resolve_foreign_trade_offer(ForeignTradeOffer& offer,ForeignTradeOfferStatus outcome){
    if(offer.status!=ForeignTradeOfferStatus::Pending||outcome==ForeignTradeOfferStatus::Pending||
       outcome==ForeignTradeOfferStatus::Accepted)throw std::logic_error("invalid foreign trade offer resolution");
    auto& partner=foreign_trade_partners_[size_t(offer.partner_id)];
    const bool importing=foreign_trade_is_import(offer.operation);
    if(importing){
        if(offer.held_good>std::numeric_limits<int64_t>::max()-partner.goods[size_t(GoodId::DryFood)])
            throw std::logic_error("foreign trade offer goods refund overflow");
        partner.goods[size_t(GoodId::DryFood)]+=offer.held_good;
    }else{
        if(offer.held_good>std::numeric_limits<int64_t>::max()-partner.salt_capacity)
            throw std::logic_error("foreign trade offer capacity refund overflow");
        partner.salt_capacity+=offer.held_good;
        if(partner.offer_cash_escrow<offer.gross)throw std::logic_error("foreign trade offer partner escrow is inconsistent");
        transfer(partner.offer_cash_escrow,partner.money,offer.gross);
    }
    if(offer.owner==0||offer.owner>people_.size())throw std::logic_error("foreign trade offer owner is invalid");
    auto& owner=mutable_person(offer.owner);
    if(owner.alive){
        transfer(offer.held_cash,owner.cash,offer.held_cash);
        if(!importing){
            auto& amount=owner.inventory[size_t(GoodId::Salt)];
            if(offer.held_good>std::numeric_limits<int64_t>::max()-amount)
                throw std::logic_error("foreign trade offer Salt refund overflow");
            amount+=offer.held_good;
        }
    }else{
        const auto estate=std::lower_bound(estate_cases_.begin(),estate_cases_.end(),offer.owner,
            [](const EstateCase& item,uint64_t id){return item.decedent<id;});
        if(estate==estate_cases_.end()||estate->decedent!=offer.owner)
            throw std::logic_error("deceased foreign trader has no estate ledger");
        if(offer.held_cash>std::numeric_limits<int64_t>::max()-estate->cash||
           offer.held_cash>std::numeric_limits<int64_t>::max()-estate_)
            throw std::logic_error("foreign trade offer estate refund overflow");
        transfer(offer.held_cash,estate_,offer.held_cash);estate->cash+=offer.held_cash;
        if(!importing){
            auto& amount=estate->inventory[size_t(GoodId::Salt)];
            if(offer.held_good>std::numeric_limits<int64_t>::max()-amount)
                throw std::logic_error("foreign trade offer estate Salt refund overflow");
            amount+=offer.held_good;
        }
    }
    offer.held_cash=0;offer.held_good=0;offer.status=outcome;
}

RealmResult Realm::execute(const ForeignTradeResponseCommand& command){
    if(command.partner_id<0||command.partner_id>=kForeignTradePartnerCount)
        return {false,"Unknown foreign trade partner"};
    auto& partner=foreign_trade_partners_[size_t(command.partner_id)];
    if(partner.decision_sequence==UINT64_MAX||next_fact_==UINT64_MAX)
        return {false,"Partner response sequence overflow"};
    if(command.sequence!=partner.decision_sequence+1)return {false,"Stale or duplicate partner response sequence"};
    auto found=std::find_if(foreign_trade_offers_.begin(),foreign_trade_offers_.end(),[&](const ForeignTradeOffer& offer){
        return offer.id==command.offer_id&&offer.partner_id==command.partner_id&&offer.status==ForeignTradeOfferStatus::Pending;
    });
    if(found==foreign_trade_offers_.end())return {false,"No pending offer for this partner"};
    auto& offer=*found;
    if(day_>offer.due_day)return {false,"Trade offer has expired"};
    if(!command.accept){
        resolve_foreign_trade_offer(offer,ForeignTradeOfferStatus::Rejected);
        ++partner.decision_sequence;
        event(offer.owner,8,246,offer.quantity,offer.id);
        return {true,"Trade offer rejected and reserved resources returned",next_fact_-1,offer.id};
    }
    if(partner.trade_suspended)return {false,"The Sovereign has suspended new trade with this partner"};
    const auto routePlan=foreign_trade_route_plan(offer.operation,offer.quantity);
    if(!routePlan.reachable||routePlan.route_indices.empty()||routePlan.total_days<1||routePlan.total_fee!=offer.route_fee||
       routePlan.total_days>365000-day_||
       std::any_of(foreign_trade_orders_.begin(),foreign_trade_orders_.end(),[&](const ForeignTradeOrder& order){return order.created_day==day_;}))
        return {false,"The current coastal route or port schedule cannot accept this order today"};
    if(next_id_==UINT64_MAX)return {false,"Foreign trade order ID overflow"};
    const bool importing=foreign_trade_is_import(offer.operation);
    auto& owner=mutable_person(offer.owner);
    const int64_t expectedHeld=importing?offer.gross+offer.duty+offer.port_fee+offer.route_fee:
        offer.port_fee+offer.route_fee;
    if(offer.held_good!=offer.quantity||offer.held_cash!=expectedHeld||
       (importing&&partner.escrow>std::numeric_limits<int64_t>::max()-offer.gross-offer.duty)||
       (!importing&&(partner.offer_cash_escrow<offer.gross||partner.escrow>std::numeric_limits<int64_t>::max()-offer.gross)))
        return {false,"Reserved offer resources no longer match its original terms"};
    if(provinces_[8].treasury>std::numeric_limits<int64_t>::max()-offer.port_fee||
       provinces_[8].tax_receipts>std::numeric_limits<int64_t>::max()-offer.duty)
        return {false,"Port or customs account cannot accept the settlement"};
    for(size_t routeIndex:routePlan.route_indices){
        const auto& route=national_transport_routes_[routeIndex];
        const int64_t fee=route.fee_per_unit*offer.quantity;
        if(provinces_[size_t(route.fee_recipient_region)].treasury>std::numeric_limits<int64_t>::max()-fee)
            return {false,"Route account cannot accept the settlement"};
    }
    ForeignTradeOrder order;
    order.id=next_id_;order.owner=offer.owner;order.partner_id=offer.partner_id;
    order.operation=offer.operation;order.good=offer.good;order.created_day=day_;
    order.arrival_day=day_+routePlan.total_days;order.quantity=offer.quantity;
    order.gross=offer.gross;order.duty=offer.duty;order.escrow=importing?offer.gross+offer.duty:offer.gross;
    foreign_trade_orders_.reserve(foreign_trade_orders_.size()+1);
    if(importing){
        transfer(offer.held_cash,partner.escrow,order.escrow);
        transfer(offer.held_cash,provinces_[8].treasury,offer.port_fee);
        for(size_t routeIndex:routePlan.route_indices){const auto& route=national_transport_routes_[routeIndex];transfer(offer.held_cash,provinces_[size_t(route.fee_recipient_region)].treasury,route.fee_per_unit*offer.quantity);}
    }else{
        transfer(partner.offer_cash_escrow,partner.escrow,offer.gross);
        transfer(offer.held_cash,provinces_[8].treasury,offer.port_fee);
        for(size_t routeIndex:routePlan.route_indices){const auto& route=national_transport_routes_[routeIndex];transfer(offer.held_cash,provinces_[size_t(route.fee_recipient_region)].treasury,route.fee_per_unit*offer.quantity);}
    }
    offer.held_cash=0;offer.held_good=0;offer.status=ForeignTradeOfferStatus::Accepted;offer.order_id=order.id;
    ++next_id_;++partner.decision_sequence;foreign_trade_orders_.push_back(order);
    event(offer.owner,8,245,offer.quantity,offer.id);
    (void)owner;
    return {true,"Partner accepted the offer; the trade order is booked for delivery",next_fact_-1,order.id};
}

void Realm::process_foreign_trade_day() {
    for (size_t index = 0; index < foreign_trade_orders_.size();) {
        const auto order = foreign_trade_orders_[index];
        if (order.arrival_day > day_) {
            ++index;
            continue;
        }
        if (order.arrival_day != day_ || order.owner == 0 || order.owner > people_.size() ||
            order.quantity <= 0 || order.escrow < 0)
            throw std::logic_error("foreign trade arrival record is invalid");
        const bool deceased = !person(order.owner).alive;
        if(order.partner_id<0||order.partner_id>=kForeignTradePartnerCount)
            throw std::logic_error("foreign trade partner is invalid at arrival");
        auto& partner=foreign_trade_partners_[size_t(order.partner_id)];

        auto& completed_orders=foreign_trade_is_import(order.operation)?partner.completed_import_orders:partner.completed_export_orders;
        auto& completed_units=foreign_trade_is_import(order.operation)?partner.imported_units:partner.exported_units;
        auto& completed_gross=foreign_trade_is_import(order.operation)?partner.import_gross:partner.export_gross;
        if(completed_orders==std::numeric_limits<int64_t>::max()||
           order.quantity>std::numeric_limits<int64_t>::max()-completed_units||
           order.gross>std::numeric_limits<int64_t>::max()-completed_gross)
            throw std::logic_error("foreign trade history overflow");

        if (foreign_trade_is_import(order.operation)) {
            if (order.escrow != order.gross + order.duty || partner.escrow < order.escrow ||
                provinces_[8].treasury > std::numeric_limits<int64_t>::max() - order.duty ||
                provinces_[8].tax_receipts > std::numeric_limits<int64_t>::max() - order.duty ||
                provinces_[8].stock[size_t(GoodId::DryFood)]>std::numeric_limits<int64_t>::max()-order.quantity)
                throw std::logic_error("foreign import escrow or customs ledger is inconsistent");
            transfer(partner.escrow, partner.money, order.gross);
            transfer(partner.escrow, provinces_[8].treasury, order.duty);
            provinces_[8].tax_receipts += order.duty;
            provinces_[8].stock[size_t(GoodId::DryFood)] += order.quantity;
        } else {
            const auto estate = std::lower_bound(estate_cases_.begin(), estate_cases_.end(), order.owner,
                [](const EstateCase& item, uint64_t id) { return item.decedent < id; });
            if (deceased && (estate == estate_cases_.end() || estate->decedent != order.owner))
                throw std::logic_error("deceased foreign trader has no estate ledger");
            if (order.escrow != order.gross || partner.escrow < order.escrow ||
                order.duty > order.gross || provinces_[8].treasury > std::numeric_limits<int64_t>::max() - order.duty ||
                provinces_[8].tax_receipts > std::numeric_limits<int64_t>::max() - order.duty ||
                partner.goods[size_t(GoodId::Salt)] > std::numeric_limits<int64_t>::max() - order.quantity)
                throw std::logic_error("foreign export escrow or customs ledger is inconsistent");
            const int64_t proceeds = order.gross - order.duty;
            if (deceased) {
                if (proceeds > std::numeric_limits<int64_t>::max() - estate->cash ||
                    proceeds > std::numeric_limits<int64_t>::max() - estate_)
                    throw std::logic_error("foreign export estate proceeds overflow");
                transfer(partner.escrow, estate_, proceeds);
                estate->cash += proceeds;
            } else {
                if (proceeds > std::numeric_limits<int64_t>::max() - person(order.owner).cash)
                    throw std::logic_error("foreign export proceeds overflow");
                transfer(partner.escrow, mutable_person(order.owner).cash, proceeds);
            }
            transfer(partner.escrow, provinces_[8].treasury, order.duty);
            provinces_[8].tax_receipts += order.duty;
            partner.goods[size_t(GoodId::Salt)] += order.quantity;
        }
        event(order.owner, 8, 231, order.quantity, order.id);
        ++completed_orders;
        completed_units+=order.quantity;
        completed_gross+=order.gross;
        foreign_trade_orders_.erase(foreign_trade_orders_.begin() + std::ptrdiff_t(index));
    }
}

void Realm::process_foreign_trade_offer_expirations(){
    for(auto& offer:foreign_trade_offers_)if(offer.status==ForeignTradeOfferStatus::Pending&&offer.due_day<day_){
        resolve_foreign_trade_offer(offer,ForeignTradeOfferStatus::Expired);
        event(offer.owner,8,247,offer.quantity,offer.id);
    }
}

} // namespace polyec
