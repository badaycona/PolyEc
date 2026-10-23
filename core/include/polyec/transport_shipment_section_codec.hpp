#pragma once

#include "polyec/realm.hpp"

#include <istream>
#include <limits>
#include <ostream>
#include <stdexcept>

namespace polyec::detail {

inline void write_transport_shipment_section(std::ostream& out, uint64_t next_lot_id,
    int64_t escrow, const std::vector<TransportShipment>& shipments,
    const std::vector<TransportCargoLot>& lots) {
    out << " TRANSPORT_SHIPMENT_2 " << next_lot_id << ' ' << escrow << ' '
        << shipments.size() << ' ' << lots.size() << ' ';
    for (const auto& shipment : shipments) {
        out << shipment.id << ' ' << shipment.owner << ' ' << unsigned(shipment.owner_kind) << ' '
            << shipment.source_region << ' ' << shipment.created_day << ' '
            << shipment.destination << ' ' << shipment.good << ' ' << unsigned(shipment.service_goal) << ' '
            << shipment.quantity << ' ' << shipment.delivered << ' ' << shipment.canceled << ' '
            << shipment.fee_escrow << ' ' << shipment.route_indices.size() << ' ';
        for (const auto route : shipment.route_indices) out << route << ' ';
    }
    for (const auto& lot : lots)
        out << lot.id << ' ' << lot.shipment_id << ' ' << lot.route_position << ' '
            << lot.current_region << ' ' << lot.arrival_day << ' ' << lot.quantity << ' ';
}

inline void read_transport_shipment_section(std::istream& in, uint64_t& next_lot_id,
    int64_t& escrow, std::vector<TransportShipment>& shipments,
    std::vector<TransportCargoLot>& lots,int realm_schema) {
    std::string tag;
    size_t shipment_count = 0, lot_count = 0;
    in >> tag >> next_lot_id >> escrow >> shipment_count >> lot_count;
    const bool legacy_section=realm_schema<47&&tag=="TRANSPORT_SHIPMENT_1";
    const bool public_section=realm_schema>=47&&tag=="TRANSPORT_SHIPMENT_2";
    if (!in || (!legacy_section&&!public_section) || next_lot_id == 0 || escrow < 0 ||
        shipment_count > 1000000 || lot_count > 1000000)
        throw std::runtime_error("Save transport shipment header");
    shipments.clear(); shipments.reserve(shipment_count);
    uint64_t previous_shipment = 0;
    for (size_t i = 0; i < shipment_count; ++i) {
        TransportShipment shipment;
        unsigned goal = 0,owner_kind=0;
        size_t route_count = 0;
        in >> shipment.id >> shipment.owner;
        if(public_section)in>>owner_kind>>shipment.source_region;
        in >> shipment.created_day >> shipment.destination >> shipment.good >> goal
           >> shipment.quantity >> shipment.delivered >> shipment.canceled >> shipment.fee_escrow >> route_count;
        if (!in || shipment.id <= previous_shipment || owner_kind>unsigned(TransportShipmentOwnerKind::MilitaryRationDepot)||
            goal > unsigned(TransportServiceGoal::LowestFee) ||
            route_count > kAtlasTerritoryCount - 1)
            throw std::runtime_error("Save transport shipment fields");
        shipment.owner_kind=static_cast<TransportShipmentOwnerKind>(owner_kind);
        shipment.service_goal = static_cast<TransportServiceGoal>(goal);
        shipment.route_indices.resize(route_count);
        for (auto& route : shipment.route_indices) in >> route;
        if (!in) throw std::runtime_error("Save transport shipment route path");
        previous_shipment = shipment.id;
        shipments.push_back(std::move(shipment));
    }
    lots.clear(); lots.reserve(lot_count);
    uint64_t previous_lot = 0;
    for (size_t i = 0; i < lot_count; ++i) {
        TransportCargoLot lot;
        in >> lot.id >> lot.shipment_id >> lot.route_position >> lot.current_region >> lot.arrival_day >> lot.quantity;
        if (!in || lot.id <= previous_lot) throw std::runtime_error("Save transport cargo lot fields");
        previous_lot = lot.id;
        lots.push_back(lot);
    }
}

} // namespace polyec::detail
