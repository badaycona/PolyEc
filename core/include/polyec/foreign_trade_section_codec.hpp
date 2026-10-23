#pragma once

#include "polyec/realm.hpp"

#include <algorithm>
#include <istream>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace polyec::detail {

inline void write_foreign_trade_section(std::ostream& out,
    const std::array<ForeignTradePartner,kForeignTradePartnerCount>& partners,
    const std::vector<ForeignTradeOrder>& orders,
    const std::vector<ForeignTradeOffer>& offers) {
    out << " FOREIGN_TRADE_6 ";
    for (const auto& partner : partners) {
        out << partner.money << ' ' << partner.escrow << ' ' << partner.salt_capacity << ' ';
        for (const auto amount : partner.goods) out << amount << ' ';
        out << partner.trade_suspended << ' ';
        out << partner.completed_import_orders << ' ' << partner.completed_export_orders << ' '
            << partner.imported_units << ' ' << partner.exported_units << ' '
            << partner.import_gross << ' ' << partner.export_gross << ' '
            << partner.offer_cash_escrow << ' ' << partner.decision_sequence << ' ';
    }
    out << orders.size() << ' ';
    for (const auto& order : orders)
        out << order.id << ' ' << order.owner << ' ' << order.partner_id << ' ' << unsigned(order.operation) << ' '
            << order.good << ' ' << order.created_day << ' ' << order.arrival_day << ' ' << order.quantity << ' '
            << order.gross << ' ' << order.duty << ' ' << order.escrow << ' ';
    out << offers.size() << ' ';
    for (const auto& offer : offers)
        out << offer.id << ' ' << offer.owner << ' ' << offer.partner_id << ' '
            << unsigned(offer.operation) << ' ' << offer.good << ' ' << offer.created_day << ' '
            << offer.due_day << ' ' << offer.quantity << ' ' << offer.gross << ' ' << offer.duty << ' '
            << offer.held_cash << ' ' << offer.held_good << ' ' << offer.port_fee << ' '
            << offer.route_fee << ' ' << unsigned(offer.status) << ' ' << offer.order_id << ' ';
}

inline void read_foreign_trade_section_v1(std::istream& in, ForeignTradePartner& partner,
    std::vector<ForeignTradeOrder>& orders, size_t people_count, int regions, int current_day) {
    std::string tag;
    int64_t staged_money=0, staged_escrow=0, staged_capacity=0;
    in >> tag >> staged_money >> staged_escrow >> staged_capacity;
    if (!in || tag != "FOREIGN_TRADE_1" || staged_money < 0 || staged_escrow < 0 ||
        staged_capacity < 0 || staged_capacity > kForeignTradeSaltCapacity)
        throw std::runtime_error("Save foreign trade header");
    std::array<int64_t,kGoodIdCount> staged_goods{};
    for (int good=0;good<27;++good) {
        auto& amount=staged_goods[static_cast<size_t>(good)];
        in >> amount;
        if (!in || amount < 0) throw std::runtime_error("Save foreign trade inventory");
    }
    if (staged_goods[size_t(GoodId::DryFood)] > kForeignTradeOpeningDryFood ||
        staged_goods[size_t(GoodId::Salt)] + staged_capacity > kForeignTradeSaltCapacity)
        throw std::runtime_error("Save foreign trade stock bounds");
    size_t count=0;
    in >> count;
    if (!in || count > 1000000) throw std::runtime_error("Save foreign trade order count");
    std::vector<ForeignTradeOrder> staged_orders;
    staged_orders.reserve(count);
    uint64_t previous_id=0;
    int previous_created_day=-1;
    int64_t calculated_escrow=0, pending_exports=0;
    for (size_t i=0;i<count;++i) {
        ForeignTradeOrder order;
        unsigned operation=0;
        in >> order.id >> order.owner >> operation >> order.good >> order.created_day >> order.arrival_day
           >> order.quantity >> order.gross >> order.duty >> order.escrow;
        if (!in || order.id<=previous_id || order.owner==0 || order.owner>people_count || operation>1 ||
            order.created_day<0 || order.created_day>current_day ||
            (order.arrival_day!=order.created_day+3&&order.arrival_day!=order.created_day+4) ||
            order.arrival_day<=current_day || order.created_day==previous_created_day ||
            (order.quantity!=1&&order.quantity!=4&&order.quantity!=8))
            throw std::runtime_error("Save foreign trade order fields");
        order.operation=static_cast<ForeignTradeOperation>(operation);
        const bool importing=order.operation==ForeignTradeOperation::Import;
        const int expected_good=importing?int(GoodId::DryFood):int(GoodId::Salt);
        const int64_t expected_gross=(importing?24:28)*order.quantity;
        const int64_t expected_duty=expected_gross/20;
        const int64_t expected_escrow=importing?expected_gross+expected_duty:expected_gross;
        if (order.good!=expected_good || order.gross!=expected_gross || order.duty!=expected_duty ||
            order.escrow!=expected_escrow || order.escrow>std::numeric_limits<int64_t>::max()-calculated_escrow)
            throw std::runtime_error("Save foreign trade order accounting");
        calculated_escrow+=order.escrow;
        if (!importing) {
            if (order.quantity>std::numeric_limits<int64_t>::max()-pending_exports)
                throw std::runtime_error("Save foreign trade reserved storage overflow");
            pending_exports+=order.quantity;
        }
        previous_id=order.id;
        previous_created_day=order.created_day;
        staged_orders.push_back(order);
    }
    if (calculated_escrow!=staged_escrow ||
        (regions==int(kAtlasTerritoryCount)-1&&
         pending_exports!=kForeignTradeSaltCapacity-staged_capacity-staged_goods[size_t(GoodId::Salt)]) ||
        (regions!=int(kAtlasTerritoryCount)-1&&
         (staged_money||staged_escrow||staged_capacity||count||
          std::any_of(staged_goods.begin(),staged_goods.end(),[](int64_t amount){return amount!=0;}))))
        throw std::runtime_error("Save foreign trade liabilities");
    partner.money=staged_money;
    partner.escrow=staged_escrow;
    partner.salt_capacity=staged_capacity;
    partner.goods=staged_goods;
    orders=std::move(staged_orders);
}

inline void read_foreign_trade_section(std::istream& in,
    std::array<ForeignTradePartner,kForeignTradePartnerCount>& partners,
    std::vector<ForeignTradeOrder>& orders, std::vector<ForeignTradeOffer>& offers,
    size_t people_count, int regions, int current_day, uint64_t next_id) {
    std::string tag;
    in >> tag;
    if (!in || (tag != "FOREIGN_TRADE_2" && tag != "FOREIGN_TRADE_3" && tag != "FOREIGN_TRADE_4" && tag != "FOREIGN_TRADE_5" && tag != "FOREIGN_TRADE_6")) throw std::runtime_error("Save foreign trade version");
    const int good_count = tag == "FOREIGN_TRADE_2" ? 27 : kGoodIdCount;
    std::array<ForeignTradePartner,kForeignTradePartnerCount> staged_partners{};
    for (auto& partner : staged_partners) {
        in >> partner.money >> partner.escrow >> partner.salt_capacity;
        if (!in || partner.money < 0 || partner.escrow < 0 || partner.salt_capacity < 0 ||
            partner.salt_capacity > kForeignTradePartnerSaltCapacity)
            throw std::runtime_error("Save foreign trade partner header");
        for (int good=0;good<good_count;++good) {
            auto& amount=partner.goods[static_cast<size_t>(good)];
            in >> amount;
            if (!in || amount < 0) throw std::runtime_error("Save foreign trade partner inventory");
        }
        if(tag=="FOREIGN_TRADE_4")in>>partner.trade_suspended;
        if(tag=="FOREIGN_TRADE_5")in>>partner.trade_suspended>>partner.completed_import_orders>>partner.completed_export_orders
            >>partner.imported_units>>partner.exported_units>>partner.import_gross>>partner.export_gross;
        if(tag=="FOREIGN_TRADE_6")in>>partner.trade_suspended>>partner.completed_import_orders>>partner.completed_export_orders
            >>partner.imported_units>>partner.exported_units>>partner.import_gross>>partner.export_gross
            >>partner.offer_cash_escrow>>partner.decision_sequence;
        if(!in)throw std::runtime_error("Save foreign trade policy");
        if(partner.completed_import_orders<0||partner.completed_export_orders<0||partner.imported_units<partner.completed_import_orders||
           partner.exported_units<partner.completed_export_orders||partner.import_gross<0||partner.export_gross<0)
            throw std::runtime_error("Save foreign trade history");
        for (size_t good=0;good<partner.goods.size();++good)
            if ((good!=size_t(GoodId::DryFood)&&good!=size_t(GoodId::Salt)&&partner.goods[good]!=0) ||
                partner.goods[size_t(GoodId::DryFood)]>kForeignTradePartnerOpeningDryFood ||
                partner.goods[size_t(GoodId::Salt)]>kForeignTradePartnerSaltCapacity)
                throw std::runtime_error("Save foreign trade partner stock bounds");
    }
    size_t count=0;
    in >> count;
    if (!in || count > 1000000) throw std::runtime_error("Save foreign trade order count");
    std::vector<ForeignTradeOrder> staged_orders;
    staged_orders.reserve(count);
    uint64_t previous_id=0;
    int previous_created_day=-1;
    std::array<int64_t,kForeignTradePartnerCount> calculated_escrow{},pending_exports{};
    for (size_t i=0;i<count;++i) {
        ForeignTradeOrder order;
        unsigned operation=0;
        in >> order.id >> order.owner >> order.partner_id >> operation >> order.good >> order.created_day >> order.arrival_day
           >> order.quantity >> order.gross >> order.duty >> order.escrow;
        if (!in || order.id<=previous_id || order.owner==0 || order.owner>people_count ||
            order.partner_id<0 || order.partner_id>=kForeignTradePartnerCount || operation>=2*kForeignTradePartnerCount ||
            foreign_trade_partner_id(static_cast<ForeignTradeOperation>(operation))!=order.partner_id ||
            order.created_day<0 || order.created_day>current_day ||
            (order.arrival_day!=order.created_day+3&&order.arrival_day!=order.created_day+4) ||
            order.arrival_day<=current_day || order.created_day==previous_created_day ||
            (order.quantity!=1&&order.quantity!=4&&order.quantity!=8))
            throw std::runtime_error("Save foreign trade order fields");
        order.operation=static_cast<ForeignTradeOperation>(operation);
        const bool importing=foreign_trade_is_import(order.operation);
        const int expected_good=importing?int(GoodId::DryFood):int(GoodId::Salt);
        const int64_t expected_gross=(importing?24:28)*order.quantity;
        const int64_t expected_duty=expected_gross/20;
        const int64_t expected_escrow=importing?expected_gross+expected_duty:expected_gross;
        auto& partner_escrow=calculated_escrow[size_t(order.partner_id)];
        if (order.good!=expected_good || order.gross!=expected_gross || order.duty!=expected_duty ||
            order.escrow!=expected_escrow || order.escrow>std::numeric_limits<int64_t>::max()-partner_escrow)
            throw std::runtime_error("Save foreign trade order accounting");
        partner_escrow+=order.escrow;
        if (!importing) {
            auto& pending=pending_exports[size_t(order.partner_id)];
            if (order.quantity>std::numeric_limits<int64_t>::max()-pending)
                throw std::runtime_error("Save foreign trade reserved storage overflow");
            pending+=order.quantity;
        }
        previous_id=order.id;
        previous_created_day=order.created_day;
        staged_orders.push_back(order);
    }
    std::vector<ForeignTradeOffer> staged_offers;
    std::array<int64_t,kForeignTradePartnerCount> calculated_offer_escrow{},pending_offer_exports{};
    if(tag=="FOREIGN_TRADE_6"){
        size_t offer_count=0;in>>offer_count;
        if(!in||offer_count>1000000)throw std::runtime_error("Save foreign trade offer count");
        staged_offers.reserve(offer_count);
        uint64_t previous_offer_id=0;
        std::array<unsigned,kForeignTradePartnerCount> pending_offer_count{};
        for(size_t i=0;i<offer_count;++i){
            ForeignTradeOffer offer;unsigned operation=0,status=0;
            in>>offer.id>>offer.owner>>offer.partner_id>>operation>>offer.good>>offer.created_day
              >>offer.due_day>>offer.quantity>>offer.gross>>offer.duty>>offer.held_cash>>offer.held_good
              >>offer.port_fee>>offer.route_fee>>status>>offer.order_id;
            if(!in||offer.id<=previous_offer_id||offer.id>=next_id||offer.owner==0||offer.owner>people_count||
               offer.partner_id<0||offer.partner_id>=kForeignTradePartnerCount||operation>=2*kForeignTradePartnerCount||
               foreign_trade_partner_id(static_cast<ForeignTradeOperation>(operation))!=offer.partner_id||status>3||
               offer.created_day<0||offer.created_day>current_day||offer.due_day!=offer.created_day+7||offer.due_day>365000||
               (offer.quantity!=1&&offer.quantity!=4&&offer.quantity!=8)||offer.gross<=0||offer.duty!=offer.gross/20||
               offer.port_fee!=2*offer.quantity||offer.route_fee<0||offer.held_cash<0||offer.held_good<0)
                throw std::runtime_error("Save foreign trade offer fields");
            offer.operation=static_cast<ForeignTradeOperation>(operation);
            offer.status=static_cast<ForeignTradeOfferStatus>(status);
            const bool importing=foreign_trade_is_import(offer.operation);
            const int expected_good=importing?int(GoodId::DryFood):int(GoodId::Salt);
            const int64_t expected_gross=(importing?24:28)*offer.quantity;
            if(offer.good!=expected_good||offer.gross!=expected_gross)throw std::runtime_error("Save foreign trade offer terms");
            if(offer.status==ForeignTradeOfferStatus::Pending){
                if(offer.due_day<current_day||offer.held_good!=offer.quantity||offer.order_id!=0||
                   ++pending_offer_count[size_t(offer.partner_id)]>1)
                    throw std::runtime_error("Save pending foreign trade offer");
                const int64_t expected_hold=importing?offer.gross+offer.duty+offer.port_fee+offer.route_fee:offer.port_fee+offer.route_fee;
                if(offer.held_cash!=expected_hold)throw std::runtime_error("Save foreign trade offer escrow");
                if(!importing){
                    auto& escrow=calculated_offer_escrow[size_t(offer.partner_id)];
                    auto& exports=pending_offer_exports[size_t(offer.partner_id)];
                    if(offer.gross>std::numeric_limits<int64_t>::max()-escrow||offer.quantity>std::numeric_limits<int64_t>::max()-exports)
                        throw std::runtime_error("Save foreign trade offer overflow");
                    escrow+=offer.gross;exports+=offer.quantity;
                }
            }else{
                if(offer.held_cash||offer.held_good)throw std::runtime_error("Save resolved foreign trade offer escrow");
                if(offer.status==ForeignTradeOfferStatus::Accepted){
                    if(offer.order_id<=offer.id||offer.order_id>=next_id)throw std::runtime_error("Save accepted foreign trade offer order");
                }else if(offer.order_id!=0)throw std::runtime_error("Save rejected foreign trade offer order");
            }
            previous_offer_id=offer.id;staged_offers.push_back(offer);
        }
    }
    for (size_t id=0;id<staged_partners.size();++id) {
        const auto& partner=staged_partners[id];
        if (regions==int(kAtlasTerritoryCount)-1) {
            if (partner.escrow!=calculated_escrow[id] || partner.offer_cash_escrow!=calculated_offer_escrow[id] ||
                partner.goods[size_t(GoodId::Salt)]+partner.salt_capacity+pending_exports[id]+pending_offer_exports[id] != kForeignTradePartnerSaltCapacity)
                throw std::runtime_error("Save foreign trade partner liabilities");
        } else if (count!=0 || !staged_offers.empty() || partner.money || partner.escrow || partner.offer_cash_escrow || partner.decision_sequence || partner.salt_capacity ||
                   std::any_of(partner.goods.begin(),partner.goods.end(),[](int64_t amount){return amount!=0;})) {
            throw std::runtime_error("Save foreign trade outside profile");
        }
    }
    partners=staged_partners;
    orders=std::move(staged_orders);
    offers=std::move(staged_offers);
}

} // namespace polyec::detail
