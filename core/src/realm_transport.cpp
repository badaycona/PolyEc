#include "polyec/realm.hpp"
#include "polyec/managers/estate_manager.hpp"

#include <algorithm>
#include <limits>
#include <map>
#include <set>

namespace polyec {

bool Realm::transport_waiting_units_by_link(std::array<int64_t,kAtlasLinkCount>& waiting) const {
    waiting.fill(0);
    for (const auto& lot : transport_cargo_lots_) {
        if (lot.arrival_day != 0 || lot.quantity <= 0) continue;
        const auto shipment = std::lower_bound(transport_shipments_.begin(), transport_shipments_.end(), lot.shipment_id,
            [](const TransportShipment& value, uint64_t id) { return value.id < id; });
        if (shipment == transport_shipments_.end() || shipment->id != lot.shipment_id ||
            lot.route_position >= shipment->route_indices.size()) return false;
        const size_t route_index=shipment->route_indices[lot.route_position];
        if(route_index>=national_transport_routes_.size())return false;
        const auto link = national_transport_routes_[route_index].atlas_link;
        if(link>=kAtlasLinkCount)return false;
        if (lot.quantity > std::numeric_limits<int64_t>::max() - waiting[link]) return false;
        waiting[link] += lot.quantity;
    }
    size_t coastal_atlas_link=kAtlasLinkCount;
    std::array<size_t,2> coastal_routes{national_transport_routes_.size(),national_transport_routes_.size()};
    if(!foreign_trade_orders_.empty()){
        const auto link=std::find_if(kWorldAtlasLinks.begin(),kWorldAtlasLinks.end(),[](const AtlasLink& candidate){
            return candidate.from==8&&candidate.to==9&&candidate.corridor==AtlasCorridor::Coastal&&
                   candidate.mode==AtlasRouteMode::CoastalShipping&&
                   candidate.direction==AtlasLinkDirection::Bidirectional;
        });
        if(link==kWorldAtlasLinks.end())return false;
        coastal_atlas_link=size_t(link-kWorldAtlasLinks.begin());
        for(size_t direction=0;direction<2;++direction){
            const uint8_t from=direction==0?9:8,to=direction==0?8:9;
            const auto route=std::find_if(national_transport_routes_.begin(),national_transport_routes_.end(),
                [&](const NationalTransportRoute& candidate){
                    return candidate.atlas_link==coastal_atlas_link&&candidate.from==from&&candidate.to==to;
                });
            if(route==national_transport_routes_.end())return false;
            coastal_routes[direction]=size_t(route-national_transport_routes_.begin());
        }
    }
    for (const auto& order : foreign_trade_orders_) {
        if(foreign_trade_partner_id(order.operation)<0)return false;
        const size_t direction=foreign_trade_is_import(order.operation)?0:1;
        const auto& route=national_transport_routes_[coastal_routes[direction]];
        const int load_day = order.arrival_day - route.travel_days;
        if (load_day <= day_) continue;
        if (order.quantity > std::numeric_limits<int64_t>::max() - waiting[coastal_atlas_link]) return false;
        waiting[coastal_atlas_link] += order.quantity;
    }
    return true;
}

RealmResult Realm::execute(const TransportRouteCommand& command) {
    if (!command.actor || command.actor > people_.size()) return {false, "Unknown actor"};
    auto& actor = mutable_person(command.actor);
    if (!actor.alive) return {false, "Actor deceased"};
    if (actor.family != Family::Governance || !actor.sovereign)
        return {false, "National transport routes require the sovereign's authority"};
    if (regions_ != int(kAtlasTerritoryCount) - 1 ||
        command.route_index >= national_transport_routes_.size())
        return {false, "National route is unavailable in this Realm profile"};
    if (actor.decisions == UINT64_MAX || actor.sampling == UINT64_MAX || next_fact_ == UINT64_MAX)
        return {false, "Decision or fact counter overflow"};
    if (command.sequence != actor.decisions + 1)
        return {false, "Stale or duplicate decision ID"};

    auto& route = national_transport_routes_[command.route_index];
    if(region_military_occupied(route.from)||region_military_occupied(route.to))
        return {false,"The Sovereign cannot change a transport route while either endpoint is occupied"};
    if (route.open == command.open) return {false, "Route already has the requested open state"};
    route.open = command.open;
    atlas_distance_cache_valid_=false;
    actor.last_decision_day = day_;
    ++actor.decisions;
    ++actor.sampling;
    event(actor.id, actor.region, 224, command.open ? 1 : 0, command.route_index);
    return {true, command.open ? "National route opened" : "National route closed",
            next_fact_ - 1, command.route_index};
}

RealmResult Realm::execute(const TransportShipmentCommand& command) {
    if (!command.actor || command.actor > people_.size()) return {false, "Unknown actor"};
    auto& actor = mutable_person(command.actor);
    if (!actor.alive) return {false, "Actor deceased"};
    if (regions_ != int(kAtlasTerritoryCount) - 1)
        return {false, "National shipments require the 27-region Realm profile"};
    if (actor.decisions == UINT64_MAX || actor.sampling == UINT64_MAX || next_fact_ == UINT64_MAX)
        return {false, "Decision or fact counter overflow"};
    if (command.sequence != actor.decisions + 1)
        return {false, "Stale or duplicate decision ID"};

    if (command.operation == TransportShipmentOperation::Create) {
        if (command.shipment_id != 0 || command.quantity <= 0 || command.quantity > 1000000 ||
            command.destination < 0 || command.destination > regions_ ||
            command.destination == actor.region || !valid_good_id(command.good) ||
            static_cast<unsigned>(command.service_goal) > static_cast<unsigned>(TransportServiceGoal::LowestFee))
            return {false, "Invalid shipment request"};
        if(command.military_ration_depot&&
           (actor.family!=Family::Governance||!has_local_government_authority(actor)||!provinces_[size_t(actor.region)].charter||
            (command.good!=int(GoodId::DryFood)&&command.good!=int(GoodId::SupplementaryFood))||command.quantity>64))
            return {false,"Public ration shipment requires local Governance authority and at most 64 ration goods"};
        if(command.military_ration_depot&&regions_!=int(kAtlasTerritoryCount)-1)
            return {false,"Public ration shipments require the 27-region Realm profile"};
        const size_t good = static_cast<size_t>(command.good);
        const int source_region=actor.region;
        const int64_t source_goods=command.military_ration_depot?
            military_ration_depot_stock(source_region,command.good):actor.inventory[good];
        if (source_goods < command.quantity) return {false, "Insufficient goods"};
        std::array<int64_t,kAtlasLinkCount> waiting_units_by_link{};
        if(!transport_waiting_units_by_link(waiting_units_by_link))
            return {false,"Invalid or overflowing shared route queue"};
        const auto plan = NationalTransportManager::plan(day_, static_cast<uint8_t>(actor.region),
            static_cast<uint8_t>(command.destination), command.quantity, command.service_goal,
            national_transport_routes_,waiting_units_by_link);
        if (!plan.reachable || plan.route_indices.empty()) return {false, "No open directed route reaches the destination"};
        const int64_t fee = plan.total_fee;
        const int64_t payer_cash=command.military_ration_depot?
            provinces_[size_t(source_region)].treasury:actor.cash;
        if (payer_cash < fee) return {false, command.military_ration_depot?
            "Insufficient source-region treasury for route fees":"Insufficient cash for route fees"};
        if (next_id_ == 0 || next_id_ == UINT64_MAX || next_transport_lot_id_ == 0 ||
            next_transport_lot_id_ == UINT64_MAX || transport_shipments_.size() >= 1000000 ||
            transport_cargo_lots_.size() >= 1000000)
            return {false, "Shipment identity overflow"};
        if (fee > std::numeric_limits<int64_t>::max() - transport_escrow_)
            return {false, "Transport escrow overflow"};

        TransportShipment shipment;
        shipment.id = next_id_++;
        shipment.owner = actor.id;
        shipment.owner_kind=command.military_ration_depot?
            TransportShipmentOwnerKind::MilitaryRationDepot:TransportShipmentOwnerKind::Person;
        shipment.source_region=source_region;
        shipment.created_day = day_;
        shipment.destination = command.destination;
        shipment.good = command.good;
        shipment.service_goal = command.service_goal;
        shipment.quantity = command.quantity;
        shipment.fee_escrow = fee;
        shipment.route_indices = plan.route_indices;
        transport_shipments_.reserve(transport_shipments_.size() + 1);
        transport_cargo_lots_.reserve(transport_cargo_lots_.size() + 1);
        TransportCargoLot lot{next_transport_lot_id_++, shipment.id, 0, source_region, 0, command.quantity};
        if(command.military_ration_depot){
            military_ration_depot_stock(source_region,command.good)-=command.quantity;
            provinces_[size_t(source_region)].treasury-=fee;
        }else{
            actor.inventory[good] -= command.quantity;
            actor.cash -= fee;
        }
        transport_escrow_ += fee;
        transport_shipments_.push_back(std::move(shipment));
        transport_cargo_lots_.push_back(lot);
    } else if (command.operation == TransportShipmentOperation::Cancel) {
        if (command.shipment_id == 0 || command.quantity <= 0)
            return {false, "Invalid shipment cancellation"};
        auto shipment = std::lower_bound(transport_shipments_.begin(), transport_shipments_.end(), command.shipment_id,
            [](const TransportShipment& value, uint64_t id) { return value.id < id; });
        if (shipment == transport_shipments_.end() || shipment->id != command.shipment_id)
            return {false, "Shipment does not exist"};
        const bool public_ration=shipment->owner_kind==TransportShipmentOwnerKind::MilitaryRationDepot;
        if(public_ration){
            if(actor.family!=Family::Governance||!has_local_government_authority(actor)||actor.region!=shipment->source_region||
               !provinces_[size_t(actor.region)].charter)
                return {false,"Public ration cancellation requires Governance authority in the source region"};
        }else if(shipment->owner!=actor.id)return {false,"Shipment does not belong to this actor"};
        std::vector<size_t> waiting;
        int64_t available = 0;
        for (size_t i = 0; i < transport_cargo_lots_.size(); ++i) {
            const auto& lot = transport_cargo_lots_[i];
            if (lot.shipment_id == shipment->id && lot.arrival_day == 0 &&
                (!public_ration||lot.route_position==0)&&
                lot.current_region == (public_ration?shipment->source_region:actor.region)) {
                if (lot.quantity > std::numeric_limits<int64_t>::max() - available)
                    return {false, "Waiting shipment quantity overflow"};
                available += lot.quantity;
                waiting.push_back(i);
            }
        }
        if (available < command.quantity) return {false, "Not enough unshipped goods at the actor's current region"};
        const size_t good = static_cast<size_t>(shipment->good);
        if(public_ration){
            if(military_ration_depot_stock(shipment->source_region,shipment->good)>
               std::numeric_limits<int64_t>::max()-command.quantity)
                return {false,"Public ration depot overflow while canceling shipment"};
        }else if (actor.inventory[good] > std::numeric_limits<int64_t>::max() - command.quantity)
            return {false, "Inventory overflow while canceling shipment"};
        int64_t refund = 0;
        int64_t left = command.quantity;
        for (const size_t index : waiting) {
            const auto& lot = transport_cargo_lots_[index];
            const int64_t amount = std::min(left, lot.quantity);
            int64_t fee_per_unit = 0;
            for (size_t leg = lot.route_position; leg < shipment->route_indices.size(); ++leg) {
                const auto& route = national_transport_routes_[shipment->route_indices[leg]];
                if (route.fee_per_unit > std::numeric_limits<int64_t>::max() - fee_per_unit)
                    return {false, "Cancellation fee overflow"};
                fee_per_unit += route.fee_per_unit;
            }
            if (fee_per_unit > std::numeric_limits<int64_t>::max() / amount ||
                fee_per_unit * amount > std::numeric_limits<int64_t>::max() - refund)
                return {false, "Cancellation fee refund overflow"};
            refund += fee_per_unit * amount;
            left -= amount;
            if (!left) break;
        }
        const int64_t refund_balance=public_ration?
            provinces_[size_t(shipment->source_region)].treasury:actor.cash;
        if (refund > shipment->fee_escrow || refund > transport_escrow_ ||
            refund_balance > std::numeric_limits<int64_t>::max() - refund)
            return {false, "Cancellation escrow or cash overflow"};

        left = command.quantity;
        for (const size_t index : waiting) {
            auto& lot = transport_cargo_lots_[index];
            const int64_t amount = std::min(left, lot.quantity);
            lot.quantity -= amount;
            left -= amount;
            if (!left) break;
        }
        std::erase_if(transport_cargo_lots_, [](const TransportCargoLot& lot) { return lot.quantity == 0; });
        shipment->canceled += command.quantity;
        shipment->fee_escrow -= refund;
        transport_escrow_ -= refund;
        if(public_ration){
            provinces_[size_t(shipment->source_region)].treasury+=refund;
            military_ration_depot_stock(shipment->source_region,shipment->good)+=command.quantity;
        }else{
            actor.cash += refund;
            actor.inventory[good] += command.quantity;
        }
    } else {
        return {false, "Unknown shipment operation"};
    }

    actor.last_decision_day = day_;
    ++actor.decisions;
    ++actor.sampling;
    event(actor.id, actor.region, 225, command.quantity, command.shipment_id);
    return {true, command.operation == TransportShipmentOperation::Create ? "Shipment accepted" : "Waiting cargo canceled",
            next_fact_ - 1, command.operation == TransportShipmentOperation::Create ? transport_shipments_.back().id : command.shipment_id};
}

void Realm::process_transport_day() {
    if (transport_shipments_.empty()) return;
    std::set<uint64_t> possibly_completed_owners;
    for (auto& lot : transport_cargo_lots_) {
        if (lot.arrival_day == 0 || lot.arrival_day >= day_) continue;
        auto shipment = std::lower_bound(transport_shipments_.begin(), transport_shipments_.end(), lot.shipment_id,
            [](const TransportShipment& value, uint64_t id) { return value.id < id; });
        if (shipment == transport_shipments_.end() || shipment->id != lot.shipment_id)
            throw std::logic_error("transport arrival refers to a missing shipment");
        const auto& route = national_transport_routes_.at(shipment->route_indices.at(lot.route_position));
        if (lot.arrival_day != day_ - 1 && lot.arrival_day < day_ - 1)
            throw std::logic_error("transport arrival day was skipped");
        lot.current_region = route.to;
        lot.arrival_day = 0;
        if (lot.route_position + 1 == shipment->route_indices.size()) {
            const size_t good = static_cast<size_t>(shipment->good);
            if(shipment->owner_kind==TransportShipmentOwnerKind::MilitaryRationDepot){
                auto& destination_stock=military_ration_depot_stock(shipment->destination,shipment->good);
                if(destination_stock>std::numeric_limits<int64_t>::max()-lot.quantity)
                    throw std::overflow_error("public ration depot delivery overflow");
                destination_stock+=lot.quantity;
            } else {
                const uint64_t owner = shipment->owner;
                if (person(owner).alive) {
                    if (person(owner).inventory[good] > std::numeric_limits<int64_t>::max() - lot.quantity)
                        throw std::overflow_error("transport delivery inventory overflow");
                    mutable_person(owner).inventory[good] += lot.quantity;
                } else {
                    auto estate = std::lower_bound(estate_cases_.begin(), estate_cases_.end(), owner,
                        [](const EstateCase& value, uint64_t id) { return value.decedent < id; });
                    if (estate == estate_cases_.end() || estate->decedent != owner ||
                        estate->inventory[good] > std::numeric_limits<int64_t>::max() - lot.quantity)
                        throw std::logic_error("deceased shipment has no available estate inventory");
                    estate->inventory[good] += lot.quantity;
                }
                possibly_completed_owners.insert(owner);
            }
            shipment->delivered += lot.quantity;
            lot.quantity = 0;
        } else {
            ++lot.route_position;
        }
    }
    std::erase_if(transport_cargo_lots_, [](const TransportCargoLot& lot) { return lot.quantity == 0; });

    std::map<uint8_t, int> used_by_link;
    const auto coastal_link=std::find_if(kWorldAtlasLinks.begin(),kWorldAtlasLinks.end(),[](const AtlasLink& candidate){
        return candidate.from==8&&candidate.to==9&&candidate.corridor==AtlasCorridor::Coastal&&
               candidate.mode==AtlasRouteMode::CoastalShipping&&
               candidate.direction==AtlasLinkDirection::Bidirectional;
    });
    const size_t coastal_atlas_link=coastal_link==kWorldAtlasLinks.end()?kAtlasLinkCount:
        size_t(coastal_link-kWorldAtlasLinks.begin());
    std::array<size_t,2> coastal_routes{national_transport_routes_.size(),national_transport_routes_.size()};
    if(coastal_atlas_link<kAtlasLinkCount){
        for(size_t direction=0;direction<2;++direction){
            const uint8_t from=direction==0?9:8,to=direction==0?8:9;
            const auto route=std::find_if(national_transport_routes_.begin(),national_transport_routes_.end(),
                [&](const NationalTransportRoute& candidate){
                    return candidate.atlas_link==coastal_atlas_link&&candidate.from==from&&candidate.to==to;
                });
            if(route!=national_transport_routes_.end())coastal_routes[direction]=size_t(route-national_transport_routes_.begin());
        }
    }
    for (const auto& order : foreign_trade_orders_) {
        if(foreign_trade_partner_id(order.operation)<0)
            throw std::logic_error("foreign trade direction is invalid while loading shared route capacity");
        const size_t direction=foreign_trade_is_import(order.operation)?0:1;
        const size_t route_index=coastal_routes[direction];
        if(coastal_atlas_link>=kAtlasLinkCount||route_index>=national_transport_routes_.size())
            throw std::logic_error("foreign trade coastal route record is missing");
        const auto& route=national_transport_routes_[route_index];
        if (order.arrival_day - route.travel_days == day_) {
            const auto atlas_link=uint8_t(coastal_atlas_link);
            if (order.quantity > std::numeric_limits<int>::max() - used_by_link[atlas_link])
                throw std::overflow_error("shared transport capacity accounting overflow");
            used_by_link[atlas_link] += int(order.quantity);
        }
    }
    for (const auto& lot : transport_cargo_lots_) {
        if (lot.arrival_day <= 0) continue;
        const auto shipment = std::lower_bound(transport_shipments_.begin(), transport_shipments_.end(), lot.shipment_id,
            [](const TransportShipment& value, uint64_t id) { return value.id < id; });
        if (shipment == transport_shipments_.end() || shipment->id != lot.shipment_id)
            throw std::logic_error("transport capacity record refers to a missing shipment");
        const auto& route = national_transport_routes_.at(shipment->route_indices.at(lot.route_position));
        if (lot.arrival_day - route.travel_days == day_) {
            if (lot.quantity > std::numeric_limits<int>::max() - used_by_link[route.atlas_link])
                throw std::overflow_error("transport daily capacity accounting overflow");
            used_by_link[route.atlas_link] += int(lot.quantity);
        }
    }
    const size_t lots_at_start_of_loading = transport_cargo_lots_.size();
    transport_cargo_lots_.reserve(lots_at_start_of_loading + kAtlasLinkCount);
    for (auto& shipment : transport_shipments_) {
        for (size_t lot_index = 0; lot_index < lots_at_start_of_loading; ++lot_index) {
            auto& lot = transport_cargo_lots_[lot_index];
            if (lot.shipment_id != shipment.id || lot.arrival_day != 0 ||
                lot.route_position >= shipment.route_indices.size()) continue;
            const size_t route_index = shipment.route_indices[lot.route_position];
            const auto& route = national_transport_routes_.at(route_index);
            if (!route.open || route.from != lot.current_region) continue;
            if (day_ > 365000 - route.travel_days) continue;
            int& used = used_by_link[route.atlas_link];
            const int free_capacity = std::max(0, route.capacity_per_day - used);
            if (!free_capacity) continue;
            const int64_t treasury = provinces_.at(size_t(route.fee_recipient_region)).treasury;
            const int64_t affordable_by_recipient = route.fee_per_unit == 0 ? lot.quantity :
                (std::numeric_limits<int64_t>::max() - treasury) / route.fee_per_unit;
            const int64_t affordable_by_escrow = route.fee_per_unit == 0 ? lot.quantity : shipment.fee_escrow / route.fee_per_unit;
            const int64_t loading = std::min<int64_t>({lot.quantity, free_capacity, affordable_by_recipient, affordable_by_escrow});
            if (loading <= 0 || next_transport_lot_id_ == UINT64_MAX) continue;
            if (loading < lot.quantity && transport_cargo_lots_.size() >= 1000000) continue;
            const int64_t fee = route.fee_per_unit * loading;
            auto& recipient = provinces_.at(size_t(route.fee_recipient_region)).treasury;
            if (recipient > std::numeric_limits<int64_t>::max() - fee)
                throw std::overflow_error("transport fee recipient overflow after checked loading");
            lot.quantity -= loading;
            if (lot.quantity > 0) {
                transport_cargo_lots_.push_back({next_transport_lot_id_++, shipment.id, lot.route_position,
                    lot.current_region, day_ + route.travel_days, loading});
            } else {
                lot.quantity = loading;
                lot.arrival_day = day_ + route.travel_days;
            }
            recipient += fee;
            shipment.fee_escrow -= fee;
            transport_escrow_ -= fee;
            used += int(loading);
        }
    }
    for (const auto owner : possibly_completed_owners) settle_estate_if_transport_complete(owner);
}

void Realm::settle_estate_if_transport_complete(uint64_t decedent) {
    if (person(decedent).alive) return;
    for (const auto& shipment : transport_shipments_)
        if (shipment.owner_kind==TransportShipmentOwnerKind::Person&&shipment.owner == decedent &&
            shipment.delivered + shipment.canceled < shipment.quantity) return;
    const auto estate = std::lower_bound(estate_cases_.begin(), estate_cases_.end(), decedent,
        [](const EstateCase& value, uint64_t id) { return value.decedent < id; });
    if (estate == estate_cases_.end() || estate->decedent != decedent) return;
    const auto claims = EstateManager::prepare_cash_claims(decedent, estate_, legacy_estate_unassigned_,
        credit_pool_, estate_cases_, loans_, provinces_);
    EstateManager::apply_cash_claims(claims, estate_, legacy_estate_unassigned_, credit_pool_,
        estate_cases_, loans_, provinces_);
    const auto distribution = EstateManager::prepare_distribution(decedent, estate_,
        legacy_estate_unassigned_, estate_cases_, people_, edges_, provinces_, loans_,
        establishments_, land_holdings_, land_use_rights_);
    EstateManager::apply_distribution(distribution, estate_, legacy_estate_unassigned_,
        estate_cases_, people_, provinces_, loans_, establishments_, land_holdings_, land_use_rights_);
    const int death_event_code=person(decedent).health>0?112:100;
    event(decedent, person(decedent).region, death_event_code, person(decedent).age_days);
}

} // namespace polyec
