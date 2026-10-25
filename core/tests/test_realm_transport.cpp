#include "polyec/realm.hpp"
#include "work_authorization_fixture.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <vector>

using namespace polyec;

namespace polyec {
struct RealmTransportTestAccess {
    static void clear_stock(Realm& world,int region,int good){
        auto& province=world.provinces_.at(size_t(region));
        auto& consumed=world.consumed_.at(size_t(good));
        if(province.stock.at(size_t(good))>std::numeric_limits<int64_t>::max()-consumed)
            throw std::overflow_error("transport market fixture stock overflow");
        consumed+=province.stock.at(size_t(good));
        province.stock.at(size_t(good))=0;
    }
    static void ensure_public_stock(Realm& world,int region,int good,int64_t amount){
        auto& province=world.provinces_.at(size_t(region));
        auto& stock=province.stock.at(size_t(good));
        if(stock>=amount)return;
        const int64_t added=amount-stock;
        if(world.opening_.at(size_t(good))>std::numeric_limits<int64_t>::max()-added)
            throw std::overflow_error("public ration fixture opening stock overflow");
        stock=amount;world.opening_[size_t(good)]+=added;
    }
    static uint64_t actor_at(Realm& world, int region, uint64_t exclude=0) {
        for (const auto& person : world.people_)
            if (person.id!=exclude&&person.alive && person.family == Family::Economic && person.region == region) return person.id;
        throw std::runtime_error("test fixture has no economic actor in requested region");
    }
    static uint64_t any_actor_at(Realm& world,int region,uint64_t exclude=0){
        for(const auto& person:world.people_)if(person.id!=exclude&&person.alive&&person.region==region)return person.id;
        throw std::runtime_error("test fixture has no living actor in requested region");
    }
    static uint64_t governance_at(Realm& world,int region){
        for(const auto& person:world.people_)if(person.alive&&person.family==Family::Governance&&person.region==region)
            return person.id;
        return 0;
    }
    static void give_food_from_source(Realm& world,uint64_t actor,int64_t amount){
        auto& person=world.people_.at(size_t(actor-1));
        auto& stock=world.provinces_.at(size_t(person.region)).stock[size_t(GoodId::DryFood)];
        if(stock<amount)throw std::runtime_error("volunteer route fixture lacks source DryFood");
        stock-=amount;person.inventory[size_t(GoodId::DryFood)]+=amount;
    }
    static void clear_food_stock(Realm& world){
        for(auto& province:world.provinces_){
            auto& stock=province.stock[size_t(GoodId::DryFood)];
            world.consumed_[size_t(GoodId::DryFood)]+=stock;stock=0;
        }
    }
    static void clear_non_depot_food(Realm& world){
        int64_t removed=0;const size_t food=size_t(GoodId::DryFood);
        auto clear=[&](int64_t& stock){if(stock<0||stock>std::numeric_limits<int64_t>::max()-removed)
                throw std::overflow_error("food clearing fixture overflow");removed+=stock;stock=0;};
        for(auto& person:world.people_)clear(person.inventory[food]);
        for(auto& province:world.provinces_)clear(province.stock[food]);
        for(auto& site:world.establishments_)clear(site.stock[food]);
        for(auto& estate:world.estate_cases_)clear(estate.inventory[food]);
        if(removed>std::numeric_limits<int64_t>::max()-world.consumed_[food])
            throw std::overflow_error("food clearing consumed counter overflow");
        world.consumed_[food]+=removed;
    }
    static void set_person_needs(Realm& world,uint64_t actor,int nutrition,int health){
        auto& person=world.people_.at(size_t(actor-1));person.nutrition=nutrition;person.health=health;
    }
    static void fund(Realm& world, uint64_t actor, int good_id, int64_t goods, int64_t cash) {
        auto& person = world.people_.at(size_t(actor - 1));
        const size_t good = size_t(good_id);
        auto& province = world.provinces_.at(size_t(person.region));
        if (province.stock[good] < goods || province.firm < cash) throw std::runtime_error("fixture funding unavailable");
        province.stock[good] -= goods;
        person.inventory[good] += goods;
        province.firm -= cash;
        person.cash += cash;
    }
    static void open_only(Realm& world, size_t forward, size_t reverse) {
        for (size_t i = 0; i < world.national_transport_routes_.size(); ++i)
            world.national_transport_routes_[i].open = i == forward || i == reverse;
    }
    static void open_one_direction_only(Realm& world,size_t route_index){
        for(size_t i=0;i<world.national_transport_routes_.size();++i)
            world.national_transport_routes_[i].open=i==route_index;
    }
    static void set_day_for_fixture(Realm& world,int day){
        world.day_=day;
        world.schedule_book_.cursor=int64_t(day)*kSubdayUnitsPerDay;
        world.market_settled_day_=day;
        world.market_processed_month_=day/30;
    }
    static void open_path_only(Realm& world, const std::vector<size_t>& path) {
        for(size_t i=0;i<world.national_transport_routes_.size();++i)
            world.national_transport_routes_[i].open=std::find(path.begin(),path.end(),i)!=path.end();
    }
    static void configure_public_route(Realm& world,size_t index,int travel_days,int capacity,int64_t fee){
        auto& route=world.national_transport_routes_.at(index);
        route.travel_days=travel_days;route.capacity_per_day=capacity;route.fee_per_unit=fee;
        route.fee_recipient_region=route.to;
    }
    static void configure_pair(Realm& world,size_t forward,size_t reverse,int travel_days,int capacity) {
        for(const size_t index:{forward,reverse}){
            world.national_transport_routes_[index].travel_days=travel_days;
            world.national_transport_routes_[index].capacity_per_day=capacity;
        }
    }
    static void configure_queue_fixture(Realm& world,size_t direct,size_t first_detour,size_t second_detour) {
        const auto direct_link=world.national_transport_routes_[direct].atlas_link;
        const auto first_link=world.national_transport_routes_[first_detour].atlas_link;
        const auto second_link=world.national_transport_routes_[second_detour].atlas_link;
        for(size_t i=0;i<world.national_transport_routes_.size();++i)
            world.national_transport_routes_[i].open=i==direct||i==first_detour||i==second_detour;
        for(auto& route:world.national_transport_routes_){
            if(route.atlas_link==direct_link){route.travel_days=1;route.capacity_per_day=64;}
            if(route.atlas_link==first_link){route.travel_days=2;route.fee_per_unit=0;}
            if(route.atlas_link==second_link){route.travel_days=2;route.fee_per_unit=0;}
        }
    }
    static void make_ill(Realm& world, uint64_t actor) { world.people_.at(size_t(actor - 1)).health = -10; }
    static void suppress_random_death(Realm& world) { world.reference_scenario_=true; }
    static int distance(Realm& world,int from,int to){return world.distance(from,to);}
};
}

namespace {
int checks=0;
void check(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
std::vector<std::string> tokenize(const std::string& text){std::istringstream input(text);std::vector<std::string> result;std::string token;while(input>>token)result.push_back(token);return result;}
std::string detokenize(const std::vector<std::string>& tokens){std::ostringstream output;for(const auto&token:tokens)output<<token<<' ';return output.str();}
std::string as_schema46_person_shipments(const std::string& current){
    std::string schema47=current;
    test_fixtures::downgrade_save_48_to_47(schema47);
    auto tokens=tokenize(schema47);
    const auto section=std::find(tokens.begin(),tokens.end(),"TRANSPORT_SHIPMENT_2");
    if(section==tokens.end())throw std::runtime_error("schema46 fixture lacks shipment section version2");
    const size_t at=size_t(section-tokens.begin()),count=size_t(std::stoull(tokens.at(at+3)));
    std::vector<std::string> legacy(tokens.begin(),tokens.begin()+std::ptrdiff_t(at));
    legacy.push_back("TRANSPORT_SHIPMENT_1");
    legacy.insert(legacy.end(),tokens.begin()+std::ptrdiff_t(at+1),tokens.begin()+std::ptrdiff_t(at+5));
    size_t row=at+5;
    for(size_t shipment=0;shipment<count;++shipment){
        legacy.push_back(tokens.at(row));legacy.push_back(tokens.at(row+1));
        const size_t route_count=size_t(std::stoull(tokens.at(row+12)));
        legacy.insert(legacy.end(),tokens.begin()+std::ptrdiff_t(row+4),tokens.begin()+std::ptrdiff_t(row+13+route_count));
        row+=13+route_count;
    }
    legacy.insert(legacy.end(),tokens.begin()+std::ptrdiff_t(row),tokens.end());
    legacy.front()="POLYEC_REALM_46";
    return detokenize(legacy);
}
std::pair<size_t,size_t> opposing_routes(const Realm& world){
    const auto& routes=world.national_transport_routes();
    for(size_t i=0;i<routes.size();++i)for(size_t j=i+1;j<routes.size();++j)
        if(routes[i].atlas_link==routes[j].atlas_link&&routes[i].from==routes[j].to&&routes[i].to==routes[j].from)
            return {i,j};
    throw std::runtime_error("atlas has no bidirectional test link");
}
RealmResult create(Realm& world,uint64_t actor,int destination,int good,int64_t quantity){
    return world.execute(TransportShipmentCommand{actor,world.person(actor).decisions+1,0,
        TransportShipmentOperation::Create,destination,good,quantity,TransportServiceGoal::Fastest});
}
int run_binding_replay(){
    Realm world(804,26,128);
    const auto [route,reverse]=opposing_routes(world);
    (void)reverse;
    const int source=world.national_transport_routes()[route].from;
    const int destination=world.national_transport_routes()[route].to;
    const int good=canonical_from_legacy_good(3);
    const uint64_t actor=RealmTransportTestAccess::actor_at(world,source);
    const auto purchase=world.execute(RealmCommand{actor,world.person(actor).decisions+1,
        {Intent::Buy,source,good,5}});
    if(!purchase.success)return 2;
    const auto shipment=create(world,actor,destination,good,5);
    if(!shipment.success)return 3;
    world.advance(10);
    const auto& completed=world.transport_shipments().back();
    std::cout<<world.fingerprint()<<' '<<world.person(actor).cash<<' '
             <<world.person(actor).inventory[size_t(good)]<<' '
             <<world.view().provinces[size_t(world.national_transport_routes()[route].fee_recipient_region)].treasury<<' '
             <<world.total_money()<<' '<<world.transport_cargo_lots().size()<<' '
             <<completed.delivered<<' '<<completed.canceled<<' '<<completed.fee_escrow<<' '
             <<int(world.invariant())<<'\n';
    return world.invariant()?0:4;
}
}

int main(int argc,char** argv){
    try{
        if(argc==2&&std::string_view(argv[1])=="--binding-replay")return run_binding_replay();
        {
            Realm atlas_supply(819,26,128),small_supply(819,6,128);
            check(atlas_supply.view().provinces[2].stock[size_t(GoodId::SupplementaryFood)]==
                      kMilitarySupplementaryRationOpeningStock&&
                  small_supply.view().provinces[2].stock[size_t(GoodId::SupplementaryFood)]==0&&
                  atlas_supply.invariant()&&small_supply.invariant(),
                  "the Atlas specialty market has its versioned opening SupplementaryFood stock while smaller profiles receive none");
        }
        {
            Realm shared_capacity(813,26,128);
            size_t import_route=shared_capacity.national_transport_routes().size();
            size_t public_route=shared_capacity.national_transport_routes().size();
            const auto routes=shared_capacity.national_transport_routes();
            for(size_t index=0;index<routes.size();++index){
                const auto& route=routes[index];const auto& link=kWorldAtlasLinks[route.atlas_link];
                if(link.corridor!=AtlasCorridor::Coastal||link.from!=8||link.to!=9)continue;
                if(route.from==9&&route.to==8)import_route=index;
                if(route.from==8&&route.to==9)public_route=index;
            }
            uint64_t port_trader=0,port_governor=0;
            for(uint64_t id=1;id<=uint64_t(shared_capacity.count());++id){
                const auto& person=shared_capacity.person(id);
                if(person.alive&&person.family==Family::Economic&&person.region==8)port_trader=id;
                if(person.alive&&person.family==Family::Governance&&person.taxing_authority&&person.region==8&&
                   shared_capacity.view().provinces[8].charter&&
                   shared_capacity.view().provinces[8].stock[size_t(GoodId::DryFood)]>=4)port_governor=id;
            }
            check(import_route<routes.size()&&public_route<routes.size()&&port_trader&&port_governor,
                  "shared-capacity fixture has both coastal directions and authorized port actors");
            RealmTransportTestAccess::open_only(shared_capacity,import_route,public_route);
            RealmTransportTestAccess::configure_pair(shared_capacity,import_route,public_route,2,4);
            const auto purchase=shared_capacity.execute(MilitaryRationPurchaseCommand{port_governor,
                shared_capacity.person(port_governor).decisions+1,8,4});
            check(purchase.success&&shared_capacity.military_ration_stock(8)==4,
                  "local government places four real DryFood units in the public depot");
            const auto foreign=shared_capacity.execute(ForeignTradeCommand{port_trader,
                shared_capacity.person(port_trader).decisions+1,ForeignTradeOperation::Import,int(GoodId::DryFood),4});
            check(foreign.success&&shared_capacity.foreign_trade_orders().size()==1&&
                  shared_capacity.foreign_trade_orders()[0].arrival_day==3,
                  "the earlier foreign order reserves all four units of the shared coastal link on its first loading day");
            const auto shipment=shared_capacity.execute(TransportShipmentCommand{port_governor,
                shared_capacity.person(port_governor).decisions+1,0,TransportShipmentOperation::Create,
                9,int(GoodId::DryFood),4,TransportServiceGoal::Fastest,true});
            check(shipment.success&&shared_capacity.transport_shipments().size()==1&&
                  shared_capacity.transport_cargo_lots().size()==1&&
                  shared_capacity.transport_cargo_lots()[0].arrival_day==0&&shared_capacity.invariant(),
                  "a public ration shipment in the reverse direction queues behind the already booked foreign cargo");
            const auto queued_save=shared_capacity.serialize("shared-route-capacity");
            Realm queued_restore(1,1,128);std::string queued_error;
            check(queued_restore.deserialize(queued_save,"shared-route-capacity",queued_error)&&
                  queued_restore.fingerprint()==shared_capacity.fingerprint()&&queued_restore.invariant(),
                  queued_error.empty()?"save/load keeps the foreign reservation and domestic queue in the same order":queued_error.c_str());
            shared_capacity.advance(1);queued_restore.advance(1);
            check(shared_capacity.day()==1&&shared_capacity.transport_cargo_lots().size()==1&&
                  shared_capacity.transport_cargo_lots()[0].arrival_day==0&&shared_capacity.invariant(),
                  "the foreign order consumes the route's four-unit daily capacity before domestic cargo loads");
            const auto mid_route_save=shared_capacity.serialize("shared-route-capacity-mid-route");
            Realm mid_route_restore(1,1,128);
            check(mid_route_restore.deserialize(mid_route_save,"shared-route-capacity-mid-route",queued_error)&&
                  mid_route_restore.fingerprint()==shared_capacity.fingerprint()&&mid_route_restore.invariant(),
                  queued_error.empty()?"save/load during the booked foreign crossing preserves the still-waiting public cargo":queued_error.c_str());
            shared_capacity.advance(1);mid_route_restore.advance(1);
            check(shared_capacity.day()==2&&shared_capacity.transport_cargo_lots().size()==1&&
                  shared_capacity.transport_cargo_lots()[0].arrival_day==4&&shared_capacity.invariant()&&
                  shared_capacity.fingerprint()==mid_route_restore.fingerprint(),
                  "the public cargo loads on the next day and receives the matching two-day travel arrival");
            shared_capacity.advance(3);mid_route_restore.advance(3);
            check(shared_capacity.day()==5&&shared_capacity.foreign_trade_orders().empty()&&
                  shared_capacity.military_ration_stock(9)==4&&shared_capacity.invariant()&&
                  shared_capacity.fingerprint()==mid_route_restore.fingerprint(),
                  "both transactions settle on their scheduled days and save/load continues identically");
        }
        {
            Realm public_world(810,26,128);
            size_t public_route=public_world.national_transport_routes().size();
            uint64_t public_governor=0;int public_source=-1,public_destination=-1;
            for(size_t route=0;route<public_world.national_transport_routes().size()&&!public_governor;++route){
                const auto& edge=public_world.national_transport_routes()[route];
                const auto& province=public_world.view().provinces[size_t(edge.from)];
                for(uint64_t id=1;id<=uint64_t(public_world.count());++id){
                    const auto& person=public_world.person(id);
                    if(person.alive&&person.family==Family::Governance&&person.taxing_authority&&person.region==edge.from&&
                       province.charter){
                        public_route=route;public_governor=id;public_source=edge.from;public_destination=edge.to;break;
                    }
                }
            }
            check(public_governor&&public_route<public_world.national_transport_routes().size(),
                  "public ration route fixture has a local authorized Governor, source food and an atlas edge");
            RealmTransportTestAccess::ensure_public_stock(public_world,public_source,int(GoodId::DryFood),4);
            RealmTransportTestAccess::ensure_public_stock(public_world,public_source,int(GoodId::SupplementaryFood),4);
            RealmTransportTestAccess::open_path_only(public_world,{public_route});
            RealmTransportTestAccess::configure_public_route(public_world,public_route,2,2,2);
            const auto purchase=public_world.execute(MilitaryRationPurchaseCommand{public_governor,
                public_world.person(public_governor).decisions+1,public_source,4});
            check(purchase.success&&public_world.military_ration_stock(public_source)==4,
                  "local Government purchases four DryFood before dispatching public cargo");
            const auto treasury_before=public_world.view().provinces[size_t(public_source)].treasury;
            const auto candidates=public_world.candidates(public_governor);
            const auto shipment=candidates[size_t(realm_military_ration_shipment_candidate_begin+public_destination)];
            const auto shipment_mask=public_world.mask(public_governor,candidates);
            check(shipment.kind==Intent::ShipMilitaryRations&&shipment.target==public_destination&&
                  shipment.good==int(GoodId::DryFood)&&shipment.amount==4&&
                  shipment_mask[size_t(realm_military_ration_shipment_candidate_begin+public_destination)]==1.f,
                  "the destination candidate offers all four stored rations on the only open route");
            const auto sent=public_world.execute({public_governor,
                public_world.person(public_governor).decisions+1,shipment});
            const auto after_dispatch_observation=public_world.observation(public_governor);
            const size_t shipment_feature=size_t(realm_state+
                (realm_military_ration_shipment_candidate_begin+public_destination)*realm_features+10);
            check(sent.success&&public_world.military_ration_stock(public_source)==0&&
                  public_world.view().provinces[size_t(public_source)].treasury==treasury_before-8&&
                  public_world.transport_escrow()==8&&
                  std::abs(after_dispatch_observation[shipment_feature]-0.004f)<1e-7f&&public_world.invariant(),
                  "public dispatch reserves four rations and eight route-fee coins from the source treasury");
            const auto saved_public=public_world.serialize("public-ration-transport");
            Realm public_restore(1,1,128);std::string public_error;
            check(public_restore.deserialize(saved_public,"public-ration-transport",public_error)&&
                  public_restore.fingerprint()==public_world.fingerprint()&&public_restore.invariant(),
                  "schema47 save/load preserves public shipment ownership, source region and held fees");
            auto corrupt_public_source=tokenize(saved_public);
            const auto public_section=std::find(corrupt_public_source.begin(),corrupt_public_source.end(),"TRANSPORT_SHIPMENT_2");
            const size_t public_section_at=size_t(public_section-corrupt_public_source.begin());
            const auto public_fingerprint=public_restore.fingerprint();
            corrupt_public_source.at(public_section_at+8)=std::to_string((public_source+1)%int(kAtlasTerritoryCount));
            check(!public_restore.deserialize(detokenize(corrupt_public_source),"public-ration-transport",public_error)&&
                  public_restore.fingerprint()==public_fingerprint,
                  "a save that changes the public shipment source away from the first route region is rejected atomically");
            public_world.advance(1);
            const auto waiting=std::count_if(public_world.transport_cargo_lots().begin(),public_world.transport_cargo_lots().end(),
                [&](const TransportCargoLot& lot){return lot.arrival_day==0&&lot.current_region==public_source&&lot.quantity==2;});
            const auto in_transit=std::count_if(public_world.transport_cargo_lots().begin(),public_world.transport_cargo_lots().end(),
                [](const TransportCargoLot& lot){return lot.arrival_day>0&&lot.quantity==2;});
            check(waiting==1&&in_transit==1&&public_world.transport_escrow()==4&&public_world.invariant(),
                  "daily route capacity sends two rations and leaves two rations waiting with only their unused fee held");
            const uint64_t shipment_id=sent.session;
            const auto treasury_before_cancel=public_world.view().provinces[size_t(public_source)].treasury;
            const auto canceled=public_world.execute(TransportShipmentCommand{public_governor,
                public_world.person(public_governor).decisions+1,shipment_id,
                TransportShipmentOperation::Cancel,-1,-1,2,TransportServiceGoal::Fastest,true});
            if(!canceled.success)std::cerr<<"public ration cancellation rejected: "<<canceled.reason<<'\n';
            check(canceled.success&&public_world.military_ration_stock(public_source)==2&&
                  public_world.transport_shipments().front().canceled==2&&public_world.transport_escrow()==0&&
                  public_world.view().provinces[size_t(public_source)].treasury==treasury_before_cancel+4&&public_world.invariant(),
                  "Governance cancels only the two rations still at the source depot and returns their unused fee to its treasury");
            const auto governor_food_before_death=public_world.person(public_governor).inventory[size_t(GoodId::DryFood)];
            RealmTransportTestAccess::make_ill(public_world,public_governor);
            public_world.advance(10);
            const auto public_shipments=public_world.transport_shipments();
            const auto public_record=std::find_if(public_shipments.begin(),public_shipments.end(),
                [shipment_id](const TransportShipment& row){return row.id==shipment_id;});
            const auto public_estates=public_world.estate_cases();
            const auto public_estate=std::find_if(public_estates.begin(),public_estates.end(),
                [public_governor](const EstateCase& estate){return estate.decedent==public_governor;});
            check(!public_world.person(public_governor).alive&&public_record!=public_shipments.end()&&
                  public_record->owner_kind==TransportShipmentOwnerKind::MilitaryRationDepot&&public_record->delivered==2&&
                  public_world.military_ration_stock(public_destination)==2&&
                  public_estate!=public_estates.end()&&
                  public_estate->inventory[size_t(GoodId::DryFood)]<=governor_food_before_death&&
                  public_world.invariant(),
                  "public cargo arrives after its initiating Governor dies and does not enter that person's estate");
        }
        {
            Realm world(821,26,128);
            size_t route_index=world.national_transport_routes().size();
            uint64_t governor=0;int source=-1,destination=-1;
            for(size_t route=0;route<world.national_transport_routes().size()&&!governor;++route){
                const auto& edge=world.national_transport_routes()[route];
                for(uint64_t id=1;id<=uint64_t(world.count());++id){
                    const auto& person=world.person(id);
                    if(person.alive&&person.family==Family::Governance&&person.taxing_authority&&
                       person.region==edge.from&&world.view().provinces[size_t(edge.from)].charter){
                        route_index=route;governor=id;source=edge.from;destination=edge.to;break;
                    }
                }
            }
            check(governor&&route_index<world.national_transport_routes().size(),
                  "supplementary ration route fixture has a local authorized Governor and an atlas edge");
            RealmTransportTestAccess::ensure_public_stock(world,source,int(GoodId::SupplementaryFood),1);
            RealmTransportTestAccess::open_path_only(world,{route_index});
            RealmTransportTestAccess::configure_public_route(world,route_index,2,1,1);
            const auto purchase=world.execute(MilitaryRationPurchaseCommand{governor,
                world.person(governor).decisions+1,source,1,int(GoodId::SupplementaryFood)});
            check(purchase.success&&world.military_ration_stock(source,int(GoodId::SupplementaryFood))==1&&
                  world.military_ration_stock(source,int(GoodId::DryFood))==0&&
                  std::abs(world.observation(governor)[size_t(realm_supplementary_ration_observation_begin+source)]-0.001f)<1e-7f&&
                  world.invariant(),
                  "authorized Governance purchases one SupplementaryFood into its separate public depot");
            const int shipment_slot=realm_supplementary_ration_shipment_candidate_begin+destination;
            const auto shipment=world.candidates(governor)[size_t(shipment_slot)];
            check(shipment.kind==Intent::ShipMilitaryRations&&shipment.good==int(GoodId::SupplementaryFood)&&
                  shipment.amount==1&&world.mask(governor)[size_t(shipment_slot)]==1.f,
                  "the destination action names SupplementaryFood and exposes only its own depot quantity");
            const auto sent=world.execute({governor,world.person(governor).decisions+1,shipment});
            const auto during_trip=world.serialize("supplementary-ration-trip");
            Realm restored(1,1,128);std::string error;
            const bool loaded=restored.deserialize(during_trip,"supplementary-ration-trip",error);
            if(!sent.success||!loaded)std::cerr<<"supplementary shipment diagnostic: sent="<<sent.success
                <<" reason="<<sent.reason<<" loaded="<<loaded<<" error="<<error<<'\n';
            check(sent.success&&world.military_ration_stock(source,int(GoodId::SupplementaryFood))==0&&
                  world.military_ration_stock(source,int(GoodId::DryFood))==0&&
                  during_trip.find("MILITARY_RATION_DEPOT_2")!=std::string::npos&&
                  loaded&&
                  restored.fingerprint()==world.fingerprint()&&restored.invariant(),
                  "schema53 roundtrip during transit preserves shipment good, source, depot balances and fees");
            auto malformed_depot=during_trip;
            const auto depot_marker=malformed_depot.find(" MILITARY_RATION_DEPOT_2 ");
            const auto tag_end=malformed_depot.find(' ',depot_marker+1);
            const auto count_begin=malformed_depot.find_first_not_of(' ',tag_end);
            const auto count_end=malformed_depot.find_first_of(" \r\n\t",count_begin);
            const auto stock_begin=malformed_depot.find_first_not_of(" \r\n\t",count_end);
            const auto stock_end=malformed_depot.find_first_of(" \r\n\t",stock_begin);
            malformed_depot.replace(stock_begin,stock_end-stock_begin,"-1");
            const auto restored_fingerprint=restored.fingerprint();
            check(!restored.deserialize(malformed_depot,"supplementary-ration-trip",error)&&
                  restored.fingerprint()==restored_fingerprint,
                  "a negative stock in either v2 public depot is rejected without replacing the loaded world");
            auto legacy52=during_trip;test_fixtures::downgrade_save_53_to_52(legacy52);
            Realm migrated(1,1,128);std::string migration_error;
            check(migrated.deserialize(legacy52,"supplementary-ration-trip",migration_error)&&
                  migrated.military_ration_stock(source,int(GoodId::SupplementaryFood))==0&&
                  migrated.military_ration_stock(source,int(GoodId::DryFood))==0&&migrated.invariant(),
                  "schema52 migration retains the old one-good depot format and initializes the new SupplementaryFood depot to zero");
            restored.advance(10);
            check(restored.military_ration_stock(destination,int(GoodId::SupplementaryFood))==1&&
                  restored.military_ration_stock(destination,int(GoodId::DryFood))==0&&restored.invariant(),
                  "arriving SupplementaryFood is credited only to the matching destination depot");
        }
        {
            Realm routed(820,26,128),blocked(820,26,128);
            size_t route_index=routed.national_transport_routes().size();
            uint64_t source_governor=0,destination_governor=0;
            int source_region=-1,destination_region=-1;
            for(size_t route=0;route<routed.national_transport_routes().size()&&!source_governor;++route){
                const auto& edge=routed.national_transport_routes()[route];
                const auto& province=routed.view().provinces[size_t(edge.from)];
                uint64_t local_source=0,local_destination=0;
                for(uint64_t id=1;id<=uint64_t(routed.count());++id){
                    const auto& person=routed.person(id);
                    if(!person.alive||person.family!=Family::Governance||!person.taxing_authority)continue;
                    if(person.region==edge.from&&province.charter)local_source=id;
                    if(person.region==edge.to)local_destination=id;
                }
                if(local_source&&local_destination){route_index=route;source_governor=local_source;
                    destination_governor=local_destination;source_region=edge.from;destination_region=edge.to;}
            }
            check(source_governor&&destination_governor&&route_index<routed.national_transport_routes().size(),
                  "paired ration fixture has authorized source and destination Governors on a route with sixteen DryFood available");
            RealmTransportTestAccess::ensure_public_stock(routed,source_region,int(GoodId::DryFood),16);
            RealmTransportTestAccess::ensure_public_stock(blocked,source_region,int(GoodId::DryFood),16);
            RealmTransportTestAccess::open_path_only(routed,{route_index});
            RealmTransportTestAccess::open_path_only(blocked,{});
            RealmTransportTestAccess::configure_public_route(routed,route_index,2,64,1);
            RealmTransportTestAccess::configure_public_route(blocked,route_index,2,64,1);
            const auto buy_rations=[&](Realm& world,uint64_t governor,int region){
                return world.execute(MilitaryRationPurchaseCommand{governor,world.person(governor).decisions+1,region,16});
            };
            const auto routed_purchase=buy_rations(routed,source_governor,source_region);
            const auto blocked_purchase=buy_rations(blocked,source_governor,source_region);
            check(routed_purchase.success&&blocked_purchase.success&&
                  routed.military_ration_stock(source_region)==16&&blocked.military_ration_stock(source_region)==16,
                  "paired worlds buy the same sixteen DryFood into the source-region public depot");
            const auto destination_slot=size_t(realm_military_ration_shipment_candidate_begin+destination_region);
            const auto send_public=[&](Realm& world){
                const auto action=world.candidates(source_governor)[destination_slot];
                return world.execute({source_governor,world.person(source_governor).decisions+1,action});
            };
            const auto routed_send=send_public(routed);
            const auto blocked_before=blocked.fingerprint();
            const auto blocked_send=send_public(blocked);
            check(routed_send.success&&!blocked_send.success&&blocked.fingerprint()==blocked_before,
                  "the same public shipment command commits with an open route and rejects atomically with all routes closed");
            routed.advance(10);blocked.advance(10);
            check(routed.military_ration_stock(destination_region)==16&&
                  blocked.military_ration_stock(destination_region)==0&&routed.invariant()&&blocked.invariant(),
                  "only the open-route world receives all sixteen rations at the destination depot");
            const auto offer_for=[&](Realm& world){
                const auto choices=world.candidates(destination_governor);
                for(int index=realm_military_service_candidate_begin;
                    index<realm_military_service_candidate_begin+realm_military_service_candidate_count;++index)
                    if(choices[size_t(index)].kind==Intent::Offer&&choices[size_t(index)].good==3)return choices[size_t(index)];
                return Candidate{};
            };
            const auto routed_offer=offer_for(routed),blocked_offer=offer_for(blocked);
            check(routed_offer.kind==Intent::Offer&&routed_offer.target==blocked_offer.target&&
                  routed.execute({destination_governor,routed.person(destination_governor).decisions+1,routed_offer}).success&&
                  blocked.execute({destination_governor,blocked.person(destination_governor).decisions+1,blocked_offer}).success,
                  "both worlds offer service to the same eligible civilian in the destination region");
            const uint64_t volunteer=uint64_t(routed_offer.target);
            routed.advance(10);blocked.advance(10);
            const auto routed_accept=routed.candidates(volunteer)[28];
            const auto blocked_accept=blocked.candidates(volunteer)[28];
            check(routed_accept.kind==Intent::Accept&&blocked_accept.kind==Intent::Accept&&
                  routed.execute({volunteer,routed.person(volunteer).decisions+1,routed_accept}).success&&
                  blocked.execute({volunteer,blocked.person(volunteer).decisions+1,blocked_accept}).success&&
                  routed.person(volunteer).military_service.source_region==destination_region&&
                  blocked.person(volunteer).military_service.source_region==destination_region,
                  "the same volunteer accepts service in both paired worlds at the shipment destination");
            const auto mobilize_and_check=[&](Realm& world){
                const auto action=world.candidates(destination_governor)[16];
                return action.kind==Intent::Mobilize&&action.target==destination_region&&
                    world.execute({destination_governor,world.person(destination_governor).decisions+1,action}).success;
            };
            check(mobilize_and_check(routed)&&mobilize_and_check(blocked)&&
                  routed.military_unit_rosters().size()==1&&blocked.military_unit_rosters().size()==1&&
                  routed.military_unit_rosters()[0].members==std::vector<uint64_t>{volunteer}&&
                  blocked.military_unit_rosters()[0].members==std::vector<uint64_t>{volunteer}&&
                  routed.military_unit_rosters()[0].mobilized_members==std::vector<uint64_t>{volunteer}&&
                  blocked.military_unit_rosters()[0].mobilized_members==std::vector<uint64_t>{volunteer}&&
                  routed.invariant()&&blocked.invariant(),
                  "both accepted volunteers are explicitly mustered under the same local commander without creating extra people");
            const auto buy_supplementary=[&](Realm& world){
                RealmTransportTestAccess::ensure_public_stock(world,destination_region,int(GoodId::SupplementaryFood),1);
                return world.execute(MilitaryRationPurchaseCommand{destination_governor,
                    world.person(destination_governor).decisions+1,destination_region,1,int(GoodId::SupplementaryFood)});
            };
            const auto routed_supplementary=buy_supplementary(routed);
            const auto blocked_supplementary=buy_supplementary(blocked);
            check(routed_supplementary.success&&blocked_supplementary.success&&
                  routed.military_ration_stock(destination_region,int(GoodId::SupplementaryFood))==1&&
                  blocked.military_ration_stock(destination_region,int(GoodId::SupplementaryFood))==1&&
                  routed.invariant()&&blocked.invariant(),
                  "both paired worlds keep one separately purchased SupplementaryFood before their meal check");
            RealmTransportTestAccess::clear_non_depot_food(routed);
            RealmTransportTestAccess::clear_non_depot_food(blocked);
            RealmTransportTestAccess::set_person_needs(routed,volunteer,60,80);
            RealmTransportTestAccess::set_person_needs(blocked,volunteer,60,80);
            const auto mustered_save=routed.serialize("mustered-ration-transport");
            Realm mustered_restore(1,1,128);std::string mustered_error;
            check(mustered_restore.deserialize(mustered_save,"mustered-ration-transport",mustered_error)&&
                  mustered_restore.fingerprint()==routed.fingerprint()&&
                  mustered_restore.military_unit_rosters().size()==1&&
                  mustered_restore.military_unit_rosters()[0].mobilized_members==std::vector<uint64_t>{volunteer}&&
                  mustered_restore.military_ration_stock(destination_region)==16&&mustered_restore.invariant(),
                  "save/load preserves the accepted volunteer, muster window, sixteen delivered rations and world accounting");
            const int days_to_meal=(int(volunteer%7)-routed.view().day%7+7)%7;
            const int meal_wait=days_to_meal==0?7:days_to_meal;
            routed.advance(meal_wait);blocked.advance(meal_wait);mustered_restore.advance(meal_wait);
            check(routed.person(volunteer).nutrition==90&&blocked.person(volunteer).nutrition==40&&
                  routed.person(volunteer).health==81&&blocked.person(volunteer).health==78&&
                  routed.person(volunteer).inventory[size_t(GoodId::DryFood)]==0&&
                  blocked.person(volunteer).inventory[size_t(GoodId::DryFood)]==0&&
                  routed.military_ration_stock(destination_region)==15&&
                  routed.military_ration_stock(destination_region,int(GoodId::SupplementaryFood))==0&&
                  blocked.military_ration_stock(destination_region,int(GoodId::SupplementaryFood))==1&&
                  blocked.military_ration_stock(destination_region)==0&&
                  mustered_restore.fingerprint()==routed.fingerprint()&&routed.invariant()&&blocked.invariant()&&
                  mustered_restore.invariant(),
                  "the base DryFood meal adds twenty nutrition and one health, then the separate supplement adds ten nutrition only; without a base meal the supplement stays in the depot");
        }
        Realm world(801,26,128);
        const auto [forward,reverse]=opposing_routes(world);
        const int left_region=world.national_transport_routes()[forward].from;
        const int right_region=world.national_transport_routes()[forward].to;
        check(world.national_transport_routes()[forward].atlas_link==world.national_transport_routes()[reverse].atlas_link,
              "opposing route records refer to one shared physical link");
        RealmTransportTestAccess::open_only(world,forward,reverse);
        const uint64_t left=RealmTransportTestAccess::actor_at(world,left_region);
        const uint64_t right=RealmTransportTestAccess::actor_at(world,right_region);
        const int good=canonical_from_legacy_good(3);
        RealmTransportTestAccess::fund(world,left,good,100,500);
        RealmTransportTestAccess::fund(world,right,good,100,500);
        const auto money_before=world.total_money();
        const auto left_order=create(world,left,right_region,good,40);
        const auto right_order=create(world,right,left_region,good,40);
        if(!left_order.success)std::cerr<<"left shipment rejected: "<<left_order.reason<<'\n';
        if(!right_order.success)std::cerr<<"right shipment rejected: "<<right_order.reason<<'\n';
        check(left_order.success&&right_order.success,
              "actors on opposite ends can reserve goods and the route fee through Realm commands");
        check(world.person(left).inventory[size_t(good)]==60&&world.person(right).inventory[size_t(good)]==60&&
              world.total_money()==money_before&&world.invariant(),
              "creating shipments removes goods from private inventory and holds fees without creating money");
        const auto reject_before=world.fingerprint();
        check(!create(world,left,right_region,good,61).success&&world.fingerprint()==reject_before,
              "an order larger than current inventory is rejected without changing money, goods, or decision sequence");
        world.advance();
        int64_t loaded_day_one=0;
        bool exact_arrival_day=true;
        for(const auto& lot:world.transport_cargo_lots())if(lot.arrival_day>0){
            const auto shipment=std::find_if(world.transport_shipments().begin(),world.transport_shipments().end(),
                [&](const TransportShipment& item){return item.id==lot.shipment_id;});
            const auto& edge=world.national_transport_routes().at(shipment->route_indices.at(lot.route_position));
            if(lot.arrival_day-edge.travel_days==world.day()){
                loaded_day_one+=lot.quantity;
                exact_arrival_day=exact_arrival_day&&lot.arrival_day==world.day()+edge.travel_days;
            }
        }
        if(loaded_day_one!=64||!world.invariant())std::cerr<<"loaded="<<loaded_day_one<<" invariant="<<world.invariant()<<" day="<<world.day()<<" escrow="<<world.transport_escrow()<<'\n';
        if(!world.invariant())for(const auto&lot:world.transport_cargo_lots())std::cerr<<"lot id="<<lot.id<<" ship="<<lot.shipment_id<<" pos="<<lot.route_position<<" region="<<lot.current_region<<" arrive="<<lot.arrival_day<<" qty="<<lot.quantity<<'\n';
        check(loaded_day_one==64&&exact_arrival_day&&world.invariant(),
              "opposite shipments share one 64-unit capacity and each lot records arrival at load day plus edge days");
        const auto halfway=world.serialize("transport-roundtrip");
        Realm restored;
        std::string error;
        check(halfway.rfind("POLYEC_REALM_76 ",0)==0&&restored.deserialize(halfway,"transport-roundtrip",error)&&
              restored.fingerprint()==world.fingerprint()&&restored.invariant(),
              "current schema47 preserves in-flight and waiting cargo, remaining fee escrow, and next-day capacity state");
        Realm restored_schema46;
        const auto earlier_transport=as_schema46_person_shipments(halfway);
        const bool schema46_loaded=restored_schema46.deserialize(earlier_transport,"transport-roundtrip",error);
        const auto migrated_person_shipments=restored_schema46.transport_shipments();
        check(schema46_loaded&&
              restored_schema46.transport_shipments().size()==world.transport_shipments().size()&&
              std::all_of(migrated_person_shipments.begin(),migrated_person_shipments.end(),
                  [](const TransportShipment& row){return row.owner_kind==TransportShipmentOwnerKind::Person;})&&
              restored_schema46.transport_cargo_lots()==world.transport_cargo_lots()&&restored_schema46.invariant(),
              "schema46 migration preserves personal in-flight shipments and cargo while initializing public ownership fields");
        const uint64_t restored_fingerprint=restored.fingerprint();
        auto corrupt_route=tokenize(halfway);
        const auto shipment_tag=std::find(corrupt_route.begin(),corrupt_route.end(),"TRANSPORT_SHIPMENT_2");
        size_t shipment_row=size_t(shipment_tag-corrupt_route.begin())+5;
        const size_t shipment_count=size_t(std::stoull(corrupt_route[size_t(shipment_tag-corrupt_route.begin())+3]));
        size_t first_route_token=0;
        for(size_t i=0;i<shipment_count;++i){
            const size_t route_count_at=shipment_row+12;
            const size_t route_count=size_t(std::stoull(corrupt_route[route_count_at]));
            if(i==0&&route_count)first_route_token=route_count_at+1;
            shipment_row+=13+route_count;
        }
        check(first_route_token!=0,"transport save fixture includes a route path token");
        corrupt_route[first_route_token]="255";
        check(!restored.deserialize(detokenize(corrupt_route),"transport-roundtrip",error)&&
              restored.fingerprint()==restored_fingerprint,
              "save with a route reference outside the atlas is rejected before replacing the open Realm");
        auto corrupt_escrow=tokenize(halfway);
        const size_t escrow_header=size_t(std::find(corrupt_escrow.begin(),corrupt_escrow.end(),"TRANSPORT_SHIPMENT_2")-corrupt_escrow.begin());
        corrupt_escrow[escrow_header+2]=std::to_string(std::stoll(corrupt_escrow[escrow_header+2])+1);
        check(!restored.deserialize(detokenize(corrupt_escrow),"transport-roundtrip",error)&&
              restored.fingerprint()==restored_fingerprint,
              "save whose global fee escrow disagrees with shipment liabilities is rejected atomically");
        auto corrupt_lot_position=tokenize(halfway);
        const size_t lot_position_header=size_t(std::find(corrupt_lot_position.begin(),corrupt_lot_position.end(),"TRANSPORT_SHIPMENT_2")-
            corrupt_lot_position.begin());
        size_t lot_row=lot_position_header+5;
        size_t shipment_record=lot_position_header+5;
        for(size_t i=0;i<shipment_count;++i){
            const size_t route_count_at=shipment_record+12;
            const size_t route_count=size_t(std::stoull(corrupt_lot_position[route_count_at]));
            shipment_record+=13+route_count;
        }
        lot_row=shipment_record;
        check(std::stoull(corrupt_lot_position[lot_row+5])>0,
              "transport save fixture includes a cargo lot");
        corrupt_lot_position[lot_row+2]="255";
        check(!restored.deserialize(detokenize(corrupt_lot_position),"transport-roundtrip",error)&&
              restored.fingerprint()==restored_fingerprint,
              "save with a cargo lot beyond its shipment route is rejected before replacing the open Realm");
        auto corrupt_arrival=tokenize(halfway);
        const size_t arrival_header=size_t(std::find(corrupt_arrival.begin(),corrupt_arrival.end(),"TRANSPORT_SHIPMENT_2")-
            corrupt_arrival.begin());
        size_t arrival_record=arrival_header+5;
        for(size_t i=0;i<shipment_count;++i){
            const size_t route_count=size_t(std::stoull(corrupt_arrival[arrival_record+12]));
            arrival_record+=13+route_count;
        }
        const auto arrival_day=std::stoll(corrupt_arrival[arrival_record+4]);
        corrupt_arrival[arrival_record+4]=std::to_string(arrival_day+1000000);
        check(!restored.deserialize(detokenize(corrupt_arrival),"transport-roundtrip",error)&&
              restored.fingerprint()==restored_fingerprint,
              "save with a cargo arrival farther away than its current route leg permits is rejected atomically");
        world.advance();
        restored.advance();
        int64_t loaded_day_two=0;
        for(const auto& lot:world.transport_cargo_lots())if(lot.arrival_day>0){
            const auto shipment=std::find_if(world.transport_shipments().begin(),world.transport_shipments().end(),
                [&](const TransportShipment& item){return item.id==lot.shipment_id;});
            const auto& edge=world.national_transport_routes().at(shipment->route_indices.at(lot.route_position));
            if(lot.arrival_day-edge.travel_days==world.day())loaded_day_two+=lot.quantity;
        }
        check(loaded_day_two==16&&world.invariant()&&restored.fingerprint()==world.fingerprint(),
              "next-day loading uses the reset capacity and finishes only the 16 units left after the first day's 64");
        world.advance(7);
        restored.advance(7);
        check(world.fingerprint()==restored.fingerprint()&&world.person(left).inventory[size_t(good)]==100&&
              world.person(right).inventory[size_t(good)]==100&&world.transport_escrow()==0&&world.invariant(),
              "both directions deliver every reserved unit once and save/load continues the same simulation");

        {
            Realm queued(811,26,128);
            const auto& configured=queued.national_transport_routes();
            size_t direct=configured.size(),first_detour=configured.size(),second_detour=configured.size();int destination=-1;
            for(size_t d=0;d<configured.size()&&direct==configured.size();++d)
                for(size_t i=0;i<configured.size()&&direct==configured.size();++i){
                    if(configured[i].from!=configured[d].from||configured[i].to==configured[d].to)continue;
                    for(size_t j=0;j<configured.size();++j)
                        if(configured[j].from==configured[i].to&&configured[j].to==configured[d].to){
                            direct=d;first_detour=i;second_detour=j;destination=configured[d].to;break;
                        }
                }
            check(direct<configured.size()&&first_detour<configured.size()&&second_detour<configured.size(),
                  "queue fixture finds a direct edge and a two-edge detour");
            RealmTransportTestAccess::configure_queue_fixture(queued,direct,first_detour,second_detour);
            const int source=configured[direct].from;
            const uint64_t earlier=RealmTransportTestAccess::any_actor_at(queued,source);
            const uint64_t later=RealmTransportTestAccess::any_actor_at(queued,source,earlier);
            const int queue_good=canonical_from_legacy_good(3);
            RealmTransportTestAccess::fund(queued,earlier,queue_good,256,1000);
            RealmTransportTestAccess::fund(queued,later,queue_good,1,100);
            const auto first_order=create(queued,earlier,destination,queue_good,256);
            if(!first_order.success)std::cerr<<"queue fixture first order rejected: "<<first_order.reason<<" source="<<source<<" destination="<<destination<<"\n";
            check(first_order.success,
                  "first shipment reserves the direct route before any goods have loaded");
            const auto next_order=create(queued,later,destination,queue_good,1);
            check(next_order.success&&queued.transport_shipments().back().route_indices.front()==first_detour&&
                  queued.transport_shipments().back().route_indices.back()==second_detour&&queued.invariant(),
                  "later shipment chooses the faster detour when earlier waiting cargo fills the direct edge");
        }

        {
            Realm seasonal(901,26,128);
            check(seasonal.model_contract()==
                  "realm-household-fund-goods30-config-v54-atlas-north-pass-v1"&&
                  std::string_view(realm_schema)=="realm-v45",
                  "seasonal route semantics version the model compatibility string without changing save/model layout tag");
            RealmTransportTestAccess::set_day_for_fixture(seasonal,272);
            size_t north_route=seasonal.national_transport_routes().size();
            for(size_t i=0;i<seasonal.national_transport_routes().size();++i){
                const auto& route=seasonal.national_transport_routes()[i];
                if(route.atlas_link==NationalTransportManager::north_pass_atlas_link&&route.from==16&&route.to==17){
                    north_route=i;break;
                }
            }
            check(north_route<seasonal.national_transport_routes().size(),
                  "season fixture locates the directed Bắc Mã to Phong Ải pass route");
            RealmTransportTestAccess::open_one_direction_only(seasonal,north_route);
            const uint64_t traveler=RealmTransportTestAccess::any_actor_at(seasonal,16);
            const int seasonal_good=canonical_from_legacy_good(3);
            RealmTransportTestAccess::fund(seasonal,traveler,seasonal_good,65,1000);
            const auto seasonal_order=create(seasonal,traveler,17,seasonal_good,65);
            check(seasonal_order.success&&seasonal.transport_shipments().back().route_indices==
                  std::vector<size_t>{north_route},
                  "a shipment placed at the autumn boundary keeps the only available pass route and waits for opening");
            seasonal.advance();
            check(seasonal.day()==273&&seasonal.transport_cargo_lots().size()==1&&
                  seasonal.transport_cargo_lots().front().arrival_day==0&&
                  seasonal.transport_cargo_lots().front().current_region==16&&
                  seasonal.transport_shipments().front().fee_escrow==65,
                  "goods that cannot load in winter remain at the origin and keep their unpaid route fee in escrow");
            const std::string waiting_save=seasonal.serialize("north-pass-wait-save");
            check(waiting_save.rfind("POLYEC_REALM_76 ",0)==0,
                  "seasonal waiting uses the existing schema76 save format");
            Realm restored(1,26,128);std::string restore_error;
            check(restored.deserialize(waiting_save,"north-pass-wait-save",restore_error)&&
                  restored.invariant()&&restored.fingerprint()==seasonal.fingerprint(),
                  "save/load preserves cargo waiting at a seasonally closed pass without adding save data");
            seasonal.advance(91);restored.advance(91);
            size_t fee_region=size_t(seasonal.national_transport_routes()[north_route].fee_recipient_region);
            const int64_t treasury_before_spring=seasonal.view().provinces[fee_region].treasury;
            seasonal.advance();restored.advance();
            check(seasonal.day()==365&&restored.fingerprint()==seasonal.fingerprint()&&
                  seasonal.transport_cargo_lots().size()==2&&
                  std::count_if(seasonal.transport_cargo_lots().begin(),seasonal.transport_cargo_lots().end(),
                      [&](const TransportCargoLot& lot){return lot.arrival_day==368&&lot.quantity==64;})==1&&
                  std::count_if(seasonal.transport_cargo_lots().begin(),seasonal.transport_cargo_lots().end(),
                      [](const TransportCargoLot& lot){return lot.arrival_day==0&&lot.quantity==1;})==1&&
                  seasonal.view().provinces[fee_region].treasury==treasury_before_spring+64,
                  "the first spring service day loads only the 64-unit daily capacity after restored winter waiting");
            seasonal.advance(5);restored.advance(5);
            check(seasonal.fingerprint()==restored.fingerprint()&&seasonal.transport_cargo_lots().empty()&&
                  seasonal.person(traveler).inventory[size_t(seasonal_good)]==65&&seasonal.invariant(),
                  "the split shipment arrives once and save/load resumes identically after the seasonal delay");
        }

        {
            Realm in_flight(902,26,128);
            RealmTransportTestAccess::set_day_for_fixture(in_flight,270);
            size_t north_route=in_flight.national_transport_routes().size();
            for(size_t i=0;i<in_flight.national_transport_routes().size();++i){
                const auto& route=in_flight.national_transport_routes()[i];
                if(route.atlas_link==NationalTransportManager::north_pass_atlas_link&&route.from==16&&route.to==17){
                    north_route=i;break;
                }
            }
            check(north_route<in_flight.national_transport_routes().size(),
                  "in-flight fixture locates the same directed northern pass");
            RealmTransportTestAccess::open_one_direction_only(in_flight,north_route);
            const uint64_t traveler=RealmTransportTestAccess::any_actor_at(in_flight,16);
            const int good_id=canonical_from_legacy_good(3);
            RealmTransportTestAccess::fund(in_flight,traveler,good_id,1,1000);
            check(create(in_flight,traveler,17,good_id,1).success,
                  "cargo can be dispatched before the winter boundary");
            in_flight.advance();
            const int saved_arrival=in_flight.transport_cargo_lots().front().arrival_day;
            in_flight.advance(2);
            check(saved_arrival==274&&in_flight.day()==273&&
                  in_flight.transport_cargo_lots().front().arrival_day==saved_arrival,
                  "winter closing does not delay cargo already moving on the pass");
        }

        Realm market_world(804,26,128), market_control(804,26,128);
        const auto [market_route,market_reverse]=opposing_routes(market_world);
        const int market_from=market_world.national_transport_routes()[market_route].from;
        const int market_to=market_world.national_transport_routes()[market_route].to;
        RealmTransportTestAccess::open_only(market_world,market_route,market_reverse);
        const uint64_t buyer=RealmTransportTestAccess::actor_at(market_world,market_from);
        const uint64_t control_buyer=RealmTransportTestAccess::actor_at(market_control,market_from);
        const auto purchase=market_world.execute(RealmCommand{buyer,market_world.person(buyer).decisions+1,
            {Intent::Buy,market_from,good,5}});
        const auto control_purchase=market_control.execute(RealmCommand{control_buyer,
            market_control.person(control_buyer).decisions+1,{Intent::Buy,market_from,good,5}});
        check(purchase.success&&control_purchase.success&&market_world.person(buyer).inventory[size_t(good)]==5,
              "a real market purchase supplies inventory used by the transport command");
        const int fee_region=market_world.national_transport_routes()[market_route].fee_recipient_region;
        check(create(market_world,buyer,market_to,good,5).success,
              "purchased goods move into the shipment ledger through a sequenced Realm command");
        market_world.advance();market_control.advance();
        const auto treasury_after=market_world.view().provinces[size_t(fee_region)].treasury;
        const auto control_treasury=market_control.view().provinces[size_t(fee_region)].treasury;
        check(treasury_after-control_treasury==5&&
              market_world.transport_escrow()==0&&market_world.invariant(),
              "the five-unit edge fee leaves escrow and reaches the configured departure treasury exactly once");
        market_world.advance(8);market_control.advance(8);
        check(market_world.person(buyer).inventory[size_t(good)]==5&&
              market_world.transport_cargo_lots().empty()&&market_world.invariant(),
              "purchased cargo is absent while in transit and returns to the buyer only after arrival");

        Realm blocked_market(811,26,128), reopened_market(811,26,128);
        const auto [chain_route,chain_reverse]=opposing_routes(blocked_market);
        const int chain_source=blocked_market.national_transport_routes()[chain_route].from;
        const int chain_destination=blocked_market.national_transport_routes()[chain_route].to;
        RealmTransportTestAccess::open_only(blocked_market,chain_route,chain_reverse);
        RealmTransportTestAccess::open_only(reopened_market,chain_route,chain_reverse);
        const auto chain_supplier=RealmTransportTestAccess::actor_at(blocked_market,chain_source);
        const auto reopened_supplier=RealmTransportTestAccess::actor_at(reopened_market,chain_source);
        const auto chain_buyer=RealmTransportTestAccess::actor_at(blocked_market,chain_destination);
        const auto reopened_buyer=RealmTransportTestAccess::actor_at(reopened_market,chain_destination);
        const int chain_good=int(GoodId::Tools);
        RealmTransportTestAccess::clear_stock(blocked_market,chain_destination,chain_good);
        RealmTransportTestAccess::clear_stock(reopened_market,chain_destination,chain_good);
        RealmTransportTestAccess::fund(blocked_market,chain_supplier,chain_good,10,1000);
        RealmTransportTestAccess::fund(reopened_market,reopened_supplier,chain_good,10,1000);
        check(create(blocked_market,chain_supplier,chain_destination,chain_good,10).success&&
              create(reopened_market,reopened_supplier,chain_destination,chain_good,10).success,
              "paired market worlds create the same real ten-unit shipment");
        for(Realm* candidate:{&blocked_market,&reopened_market})
            check(candidate->execute(TransportRouteCommand{1,candidate->person(1).decisions+1,
                  uint8_t(chain_route),false}).success,
                  "sovereign closes the same route after both shipments are ordered");
        blocked_market.advance();reopened_market.advance();
        check(blocked_market.transport_cargo_lots().size()==1&&reopened_market.transport_cargo_lots().size()==1&&
              blocked_market.transport_cargo_lots()[0].arrival_day==0&&
              reopened_market.transport_cargo_lots()[0].arrival_day==0,
              "closed route leaves both shipments waiting without charging an unused route fee");
        check(reopened_market.execute(TransportRouteCommand{1,reopened_market.person(1).decisions+1,
              uint8_t(chain_route),true}).success,
              "sovereign intervenes by reopening the route in one paired world");
        blocked_market.advance();reopened_market.advance();
        check(blocked_market.transport_cargo_lots()[0].arrival_day==0&&
              reopened_market.transport_cargo_lots()[0].arrival_day>0,
              "opening the route loads cargo while the untreated shipment remains blocked");
        for(int day=0;day<12&&reopened_market.transport_shipments().back().delivered<10;++day){
            blocked_market.advance();reopened_market.advance();
        }
        check(reopened_market.transport_shipments().back().delivered==10&&
              reopened_market.person(reopened_supplier).inventory[size_t(chain_good)]==10&&
              blocked_market.transport_shipments().back().delivered==0,
              "only the reopened route delivers the goods to their owner");
        const auto travel=reopened_market.execute(RealmCommand{reopened_supplier,
            reopened_market.person(reopened_supplier).decisions+1,
            {Intent::Travel,chain_destination,0,0}});
        check(travel.success,"supplier can travel over the reopened route to the destination market");
        check(!blocked_market.execute(RealmCommand{chain_supplier,blocked_market.person(chain_supplier).decisions+1,
              {Intent::Travel,chain_destination,0,0}}).success,
              "supplier cannot travel to the market while the only route remains closed");
        for(int day=0;day<20&&reopened_market.person(reopened_supplier).region!=chain_destination;++day){
            blocked_market.advance();reopened_market.advance();
        }
        check(reopened_market.person(reopened_supplier).region==chain_destination,
              "reopened-route supplier reaches the market with delivered goods");
        check(reopened_market.execute(RealmCommand{reopened_supplier,
              reopened_market.person(reopened_supplier).decisions+1,
              {Intent::Sell,chain_destination,chain_good,10}}).success,
              "supplier sells the delivered goods into the destination's real market stock");
        const int64_t chain_price=blocked_market.view().provinces[size_t(chain_destination)].price[size_t(chain_good)];
        bool orders_accepted=true;
        for(int order=0;order<4;++order){
            orders_accepted=orders_accepted&&blocked_market.execute(MarketOrderCommand{chain_buyer,
                blocked_market.person(chain_buyer).decisions+1,0,MarketOrderOperation::Place,
                chain_destination,chain_good,1,chain_price}).success;
            orders_accepted=orders_accepted&&reopened_market.execute(MarketOrderCommand{reopened_buyer,
                reopened_market.person(reopened_buyer).decisions+1,0,MarketOrderOperation::Place,
                chain_destination,chain_good,1,chain_price}).success;
        }
        check(orders_accepted,"buyers place the same four-unit purchase after the paired worlds diverge");
        blocked_market.advance();reopened_market.advance();
        check(blocked_market.market_monthly_totals(chain_destination,chain_good).demand==4&&
              blocked_market.market_monthly_totals(chain_destination,chain_good).unmet>0&&
              reopened_market.market_monthly_totals(chain_destination,chain_good)==MarketMonthlyTotals{4,0},
              "market records unmet purchases behind the closed route and fulfills all four after intervention");
        const auto blocked_month=blocked_market.market_monthly_totals(chain_destination,chain_good);
        const auto reopened_month=reopened_market.market_monthly_totals(chain_destination,chain_good);
        check(blocked_market.person(chain_buyer).inventory[size_t(chain_good)]<4&&
              reopened_market.person(reopened_buyer).inventory[size_t(chain_good)]==4&&
              blocked_market.invariant()&&reopened_market.invariant(),
              "goods reach the buyer only in the world where the route was reopened");
        const int blocked_price_before=blocked_market.view().provinces[size_t(chain_destination)].price[size_t(chain_good)];
        const int reopened_price_before=reopened_market.view().provinces[size_t(chain_destination)].price[size_t(chain_good)];
        const int until_month_end=30-blocked_market.view().day;
        check(until_month_end>0&&reopened_market.view().day==blocked_market.view().day,
              "paired worlds reach the same first month-end from the same game day");
        blocked_market.advance(until_month_end);reopened_market.advance(until_month_end);
        check(blocked_market.view().provinces[size_t(chain_destination)].price[size_t(chain_good)]>blocked_price_before&&
              reopened_market.view().provinces[size_t(chain_destination)].price[size_t(chain_good)]<=reopened_price_before,
              "month-end price reflects unmet demand behind the closed route and available supply after intervention");
        check(blocked_market.invariant()&&reopened_market.invariant(),
              "both route-intervention outcomes preserve Realm accounting invariants");
        std::cout<<"M5.4_ROUTE_MARKET,seed,811,source,"<<chain_source<<",destination,"<<chain_destination
                 <<",good,"<<chain_good<<",shipment_units,10,blocked_demand,"<<blocked_month.demand
                 <<",blocked_unmet,"<<blocked_month.unmet<<",reopened_demand,"<<reopened_month.demand
                 <<",reopened_unmet,"<<reopened_month.unmet<<",blocked_price,"<<blocked_price_before
                 <<"->"<<blocked_market.view().provinces[size_t(chain_destination)].price[size_t(chain_good)]
                 <<",reopened_price,"<<reopened_price_before<<"->"
                 <<reopened_market.view().provinces[size_t(chain_destination)].price[size_t(chain_good)]
                 <<",blocked_stock,"<<blocked_market.view().provinces[size_t(chain_destination)].stock[size_t(chain_good)]
                 <<",reopened_stock,"<<reopened_market.view().provinces[size_t(chain_destination)].stock[size_t(chain_good)]<<'\n';

        Realm blocked_food(812,26,128), reopened_food(812,26,128);
        RealmTransportTestAccess::suppress_random_death(blocked_food);
        RealmTransportTestAccess::suppress_random_death(reopened_food);
        const auto [food_route,food_reverse]=opposing_routes(blocked_food);
        const int food_source=blocked_food.national_transport_routes()[food_route].from;
        const int food_destination=blocked_food.national_transport_routes()[food_route].to;
        RealmTransportTestAccess::open_only(blocked_food,food_route,food_reverse);
        RealmTransportTestAccess::open_only(reopened_food,food_route,food_reverse);
        const auto food_supplier=RealmTransportTestAccess::actor_at(blocked_food,food_source);
        const auto reopened_food_supplier=RealmTransportTestAccess::actor_at(reopened_food,food_source);
        const int food_good=int(GoodId::DryFood);
        RealmTransportTestAccess::clear_stock(blocked_food,food_destination,food_good);
        RealmTransportTestAccess::clear_stock(reopened_food,food_destination,food_good);
        RealmTransportTestAccess::fund(blocked_food,food_supplier,food_good,200,1000);
        RealmTransportTestAccess::fund(reopened_food,reopened_food_supplier,food_good,200,1000);
        check(create(blocked_food,food_supplier,food_destination,food_good,200).success&&
              create(reopened_food,reopened_food_supplier,food_destination,food_good,200).success,
              "paired food worlds send the same 200 real DryFood units from source stock");
        for(Realm* candidate:{&blocked_food,&reopened_food})
            check(candidate->execute(TransportRouteCommand{1,candidate->person(1).decisions+1,
                  uint8_t(food_route),false}).success,
                  "sovereign closes the food route in both paired worlds");
        blocked_food.advance();reopened_food.advance();
        check(reopened_food.execute(TransportRouteCommand{1,reopened_food.person(1).decisions+1,
              uint8_t(food_route),true}).success,
              "sovereign reopens the food route in the intervention world");
        int food_delivery_day=-1;
        for(int day=0;day<20&&reopened_food.transport_shipments().back().delivered<200;++day){
            blocked_food.advance();reopened_food.advance();
            if(reopened_food.transport_shipments().back().delivered==200)
                food_delivery_day=reopened_food.view().day;
        }
        check(reopened_food.transport_shipments().back().delivered==200&&
              food_delivery_day>0&&
              reopened_food.person(reopened_food_supplier).inventory[size_t(food_good)]>0&&
              blocked_food.transport_shipments().back().delivered==0,
              "only reopened route delivers the full food shipment");
        check(reopened_food.execute(RealmCommand{reopened_food_supplier,
              reopened_food.person(reopened_food_supplier).decisions+1,
              {Intent::Travel,food_destination,0,0}}).success,
              "food seller can travel to the destination over the reopened route");
        check(!blocked_food.execute(RealmCommand{food_supplier,blocked_food.person(food_supplier).decisions+1,
              {Intent::Travel,food_destination,0,0}}).success,
              "food seller cannot reach the destination while the route remains closed");
        for(int day=0;day<20&&reopened_food.person(reopened_food_supplier).region!=food_destination;++day){
            blocked_food.advance();reopened_food.advance();
        }
        check(reopened_food.person(reopened_food_supplier).region==food_destination,
              "intervention seller reaches the destination with DryFood");
        const int food_sale_day=reopened_food.view().day;
        const auto food_sale_quantity=reopened_food.person(reopened_food_supplier).inventory[size_t(food_good)];
        check(reopened_food.execute(RealmCommand{reopened_food_supplier,
              reopened_food.person(reopened_food_supplier).decisions+1,
              {Intent::Sell,food_destination,food_good,food_sale_quantity}}).success,
              "seller places the remaining delivered DryFood in the destination market stock");
        const int food_days=30-reopened_food.view().day;
        check(food_days>0&&blocked_food.view().day==reopened_food.view().day,
              "paired food worlds have equal time remaining to the first month-end");
        blocked_food.advance(food_days);reopened_food.advance(food_days);
        int64_t blocked_nutrition=0,reopened_nutrition=0,blocked_health=0,reopened_health=0;
        int blocked_residents=0,reopened_residents=0,blocked_alive=0,reopened_alive=0;
        for(uint64_t id=1;id<=uint64_t(blocked_food.count());++id){
            const auto& left=blocked_food.person(id);const auto& right=reopened_food.person(id);
            if(left.region!=food_destination||id==food_supplier)continue;
            ++blocked_residents;++reopened_residents;
            if(left.alive){++blocked_alive;blocked_nutrition+=left.nutrition;blocked_health+=left.health;}
            if(right.alive){++reopened_alive;reopened_nutrition+=right.nutrition;reopened_health+=right.health;}
        }
        check(blocked_residents==reopened_residents&&blocked_residents>0,
              "paired food comparison includes the same residents and excludes the traveling seller");
        check(reopened_alive>=blocked_alive&&reopened_nutrition>blocked_nutrition&&
              reopened_health>blocked_health,
              "food intervention improves aggregate nutrition and health without reducing survival");
        check(blocked_food.invariant()&&reopened_food.invariant(),
              "food shortage and intervention preserve both Realm accounting invariants");
        const auto blocked_stock_after_month=blocked_food.view().provinces[size_t(food_destination)].stock[size_t(food_good)];
        const auto reopened_stock_after_month=reopened_food.view().provinces[size_t(food_destination)].stock[size_t(food_good)];
        uint64_t shortage_witness=0;
        for(uint64_t id=1;id<=uint64_t(blocked_food.count());++id){
            const auto& person=blocked_food.person(id);
            if(person.alive&&person.family==Family::Civilian&&person.region==food_destination){
                shortage_witness=id;break;
            }
        }
        check(shortage_witness!=0,"food report fixture has a living civilian witness in the affected region");
        const auto local_shortage_facts=blocked_food.view(shortage_witness).facts;
        const auto sovereign_shortage_facts=blocked_food.view(1).facts;
        const auto count_shortage_fact=[&](const std::vector<Fact>& facts,uint64_t actor,int region){
            return std::count_if(facts.begin(),facts.end(),[&](const Fact& fact){
                return fact.kind==113&&fact.actor==actor&&fact.region==region&&fact.amount<=60;
            });
        };
        check(count_shortage_fact(local_shortage_facts,shortage_witness,food_destination)==1,
              "local resident sees one report when their nutrition first crosses the shortage threshold");
        check(count_shortage_fact(sovereign_shortage_facts,shortage_witness,food_destination)==1,
              "sovereign receives the same shortage fact after the existing reporting delay");
        uint64_t outside_witness=0;
        for(uint64_t id=1;id<=uint64_t(blocked_food.count());++id){
            const auto& person=blocked_food.person(id);
            if(id!=1&&person.alive&&person.region!=food_destination){outside_witness=id;break;}
        }
        check(outside_witness!=0,"food report fixture has a non-sovereign resident outside the affected region");
        check(count_shortage_fact(blocked_food.view(outside_witness).facts,shortage_witness,food_destination)==0,
              "a resident outside the affected region does not see the local shortage fact");
        uint64_t recovered_witness=0;
        for(uint64_t id=1;id<=uint64_t(reopened_food.count());++id){
            const auto& person=reopened_food.person(id);
            if(person.alive&&person.family==Family::Civilian&&person.region==food_destination&&person.nutrition>60){
                recovered_witness=id;break;
            }
        }
        check(recovered_witness!=0,"intervention world has a nourished resident for a repeat-shortage check");
        const auto reports_before_recurrence=count_shortage_fact(
            reopened_food.view(recovered_witness).facts,recovered_witness,food_destination);
        RealmTransportTestAccess::clear_stock(reopened_food,food_destination,food_good);
        reopened_food.advance(14);
        check(reopened_food.person(recovered_witness).nutrition<=60&&
              count_shortage_fact(reopened_food.view(recovered_witness).facts,recovered_witness,food_destination)==
                  reports_before_recurrence+1,
              "a new shortage is reported again only after nutrition recovered above the threshold");
        Realm historical_food(813,6,128,false,GameplayConfig::historical());
        RealmTransportTestAccess::suppress_random_death(historical_food);
        uint64_t historical_witness=0;
        for(uint64_t id=1;id<=uint64_t(historical_food.count());++id){
            const auto& person=historical_food.person(id);
            if(person.alive&&person.family==Family::Civilian&&person.region==0){historical_witness=id;break;}
        }
        check(historical_witness!=0,"non-market fixture has a living civilian in the test region");
        RealmTransportTestAccess::clear_stock(historical_food,0,food_good);
        historical_food.advance(14);
        historical_food.advance(1);
        const auto historical_facts=historical_food.view(historical_witness).facts;
        check(!historical_food.gameplay_config().market_enabled()&&
              count_shortage_fact(historical_facts,historical_witness,0)==1,
              "the direct no-market missed-meal path records the same threshold fact once");

        // A real active volunteer uses the same personal-food need and transport
        // ledger as any other Person. Only route availability differs in this pair.
        Realm blocked_volunteer(814,26,128),routed_volunteer(814,26,128);
        RealmTransportTestAccess::suppress_random_death(blocked_volunteer);
        RealmTransportTestAccess::suppress_random_death(routed_volunteer);
        const auto [volunteer_route,volunteer_reverse]=opposing_routes(blocked_volunteer);
        const int volunteer_source=blocked_volunteer.national_transport_routes()[volunteer_route].from;
        const int volunteer_destination=blocked_volunteer.national_transport_routes()[volunteer_route].to;
        RealmTransportTestAccess::open_only(blocked_volunteer,volunteer_route,volunteer_reverse);
        RealmTransportTestAccess::open_only(routed_volunteer,volunteer_route,volunteer_reverse);
        const uint64_t volunteer_governor=RealmTransportTestAccess::governance_at(blocked_volunteer,volunteer_source);
        const uint64_t routed_governor=RealmTransportTestAccess::governance_at(routed_volunteer,volunteer_source);
        check(volunteer_governor==routed_governor,"paired volunteer supply worlds use the same local Governance actor");
        auto offer=blocked_volunteer.candidates(volunteer_governor)[realm_military_service_candidate_begin];
        uint64_t volunteer=0;
        for(int slot=realm_military_service_candidate_begin;slot<realm_military_service_candidate_begin+realm_military_service_candidate_count;++slot){
            const auto candidate=blocked_volunteer.candidates(volunteer_governor)[size_t(slot)];
            if(candidate.kind!=Intent::Offer)continue;
            const int days_to_meal=(candidate.target%7-int(blocked_volunteer.view().day%7)+7)%7;
            if(days_to_meal>=3){offer=candidate;volunteer=uint64_t(candidate.target);break;}
        }
        check(volunteer!=0,"route fixture finds an eligible volunteer with time before the next scheduled meal");
        const auto routed_offer=offer;
        check(blocked_volunteer.execute({volunteer_governor,1,offer}).success&&
              routed_volunteer.execute({routed_governor,1,routed_offer}).success,
              "both paired worlds create the same real service offer");
        blocked_volunteer.advance(10);routed_volunteer.advance(10);
        const auto accept=blocked_volunteer.candidates(volunteer)[28];
        check(accept.kind==Intent::Accept&&
              blocked_volunteer.execute({volunteer,blocked_volunteer.person(volunteer).decisions+1,accept}).success&&
              routed_volunteer.execute({volunteer,routed_volunteer.person(volunteer).decisions+1,accept}).success&&
              blocked_volunteer.person(volunteer).military_service.active&&routed_volunteer.person(volunteer).military_service.active,
              "the same civilian explicitly accepts service in both paired worlds");
        RealmTransportTestAccess::give_food_from_source(blocked_volunteer,volunteer,10);
        RealmTransportTestAccess::give_food_from_source(routed_volunteer,volunteer,10);
        RealmTransportTestAccess::clear_food_stock(blocked_volunteer);
        RealmTransportTestAccess::clear_food_stock(routed_volunteer);
        check(create(blocked_volunteer,volunteer,volunteer_destination,int(GoodId::DryFood),10).success&&
              create(routed_volunteer,volunteer,volunteer_destination,int(GoodId::DryFood),10).success,
              "active volunteer places identical ten-unit DryFood shipments from their personal inventory");
        for(Realm* candidate:{&blocked_volunteer,&routed_volunteer}){
            const auto move=candidate->execute(RealmCommand{volunteer,candidate->person(volunteer).decisions+1,
                  {Intent::Migrate,volunteer_destination,0,10,5+std::abs(volunteer_destination-volunteer_source)}});
            if(!move.success)throw std::runtime_error("active volunteer movement failed: "+move.reason+" age_days="+
                std::to_string(candidate->person(volunteer).age_days)+" cash="+std::to_string(candidate->person(volunteer).cash)+
                " distance="+std::to_string(RealmTransportTestAccess::distance(*candidate,volunteer_source,volunteer_destination))+
                " mask="+std::to_string(candidate->mask(volunteer)[realm_extended_travel_candidate_begin+volunteer_destination-realm_extended_travel_region_begin]));
            check(move.success,"active volunteer travels toward the region where their food shipment is addressed");
        }
        for(Realm* candidate:{&blocked_volunteer,&routed_volunteer})
            check(candidate->execute(TransportRouteCommand{1,candidate->person(1).decisions+1,
                  uint8_t(volunteer_route),false}).success,
                  "sovereign closes the same route after the volunteer begins moving");
        check(routed_volunteer.execute(TransportRouteCommand{1,routed_volunteer.person(1).decisions+1,
              uint8_t(volunteer_route),true}).success,
              "the paired treatment opens exactly one route while the comparison remains closed");
        const auto volunteer_in_transit=routed_volunteer.serialize("volunteer-food-route-roundtrip");
        Realm service_restored;std::string service_save_error;
        check(service_restored.deserialize(volunteer_in_transit,"volunteer-food-route-roundtrip",service_save_error)&&
              service_restored.fingerprint()==routed_volunteer.fingerprint(),
              "save/load during active service and a food shipment preserves the full world");
        int volunteer_delivery_day=-1;
        for(int elapsed=0;elapsed<20&&routed_volunteer.transport_shipments().back().delivered<10;++elapsed){
            blocked_volunteer.advance();routed_volunteer.advance();service_restored.advance();
            check(blocked_volunteer.invariant()&&routed_volunteer.invariant()&&service_restored.invariant(),
                  "both volunteer supply branches preserve accounting while the shipment moves");
            if(routed_volunteer.transport_shipments().back().delivered==10)volunteer_delivery_day=routed_volunteer.view().day;
            check(service_restored.fingerprint()==routed_volunteer.fingerprint(),
                  "resumed active-service shipment replay matches uninterrupted simulation");
        }
        check(volunteer_delivery_day>0&&routed_volunteer.transport_shipments().back().delivered==10&&
              routed_volunteer.person(volunteer).inventory[size_t(GoodId::DryFood)]==10&&
              blocked_volunteer.transport_shipments().back().delivered==0&&
              blocked_volunteer.person(volunteer).inventory[size_t(GoodId::DryFood)]==0,
              "only the opened route delivers all ten units to the active volunteer's inventory");
        const int nutrition_before_ration=blocked_volunteer.person(volunteer).nutrition;
        const int volunteer_days_begin=blocked_volunteer.view().day;
        for(int day=0;day<8;++day){blocked_volunteer.advance();routed_volunteer.advance();service_restored.advance();}
        check(routed_volunteer.person(volunteer).nutrition>blocked_volunteer.person(volunteer).nutrition&&
              routed_volunteer.person(volunteer).inventory[size_t(GoodId::DryFood)]<10&&
              blocked_volunteer.person(volunteer).nutrition<=nutrition_before_ration,
              "delivered DryFood is consumed by the active volunteer's existing weekly need; no extra military ration is invented");
        const auto volunteer_days=blocked_volunteer.view().day-volunteer_days_begin;
        check(volunteer_days==8&&blocked_volunteer.person(volunteer).military_service.active&&
              routed_volunteer.person(volunteer).military_service.active&&
              blocked_volunteer.person(volunteer).military_service.paid_total==routed_volunteer.person(volunteer).military_service.paid_total&&
              blocked_volunteer.person(volunteer).military_service.paid_total>=8*kMilitaryServiceTrialWagePerDay&&
              blocked_volunteer.mask(volunteer)[1+volunteer_destination]==0.f&&
              routed_volunteer.mask(volunteer)[1+volunteer_destination]==0.f,
              "eight volunteer-person-days retain payroll and reject new Work; this measures exposure, not hypothetical output");
        check(blocked_volunteer.total_money()==routed_volunteer.total_money()&&
              blocked_volunteer.invariant()&&routed_volunteer.invariant()&&service_restored.invariant(),
              "the paired service and transport chain conserves money and remains invariant after food use");
        std::cout<<"VOLUNTEER_FOOD_ROUTE,seed,814,source,"<<volunteer_source<<",destination,"<<volunteer_destination
                 <<",volunteer,"<<volunteer<<",shipment_units,10,delivery_day,"<<volunteer_delivery_day
                 <<",volunteer_person_days,"<<volunteer_days<<",nutrition_blocked,"<<blocked_volunteer.person(volunteer).nutrition
                 <<",nutrition_routed,"<<routed_volunteer.person(volunteer).nutrition<<'\n';
        std::cout<<"M5.4_FOOD_LIVELIHOOD,seed,812,source,"<<food_source<<",destination,"<<food_destination
                 <<",shipment_units,200,delivery_day,"<<food_delivery_day<<",sale_day,"<<food_sale_day
                 <<",days,"<<blocked_food.view().day<<",residents,"<<blocked_residents
                 <<",blocked_alive,"<<blocked_alive<<",reopened_alive,"<<reopened_alive
                 <<",blocked_nutrition,"<<blocked_nutrition<<",reopened_nutrition,"<<reopened_nutrition
                 <<",blocked_health,"<<blocked_health<<",reopened_health,"<<reopened_health
                 <<",blocked_stock,"<<blocked_stock_after_month
                 <<",reopened_stock,"<<reopened_stock_after_month<<'\n';

        Realm cancel_world(802,26,128);
        const auto [cancel_route,cancel_reverse]=opposing_routes(cancel_world);
        const int cancel_from=cancel_world.national_transport_routes()[cancel_route].from;
        const int cancel_to=cancel_world.national_transport_routes()[cancel_route].to;
        RealmTransportTestAccess::open_only(cancel_world,cancel_route,cancel_reverse);
        const uint64_t sender=RealmTransportTestAccess::actor_at(cancel_world,cancel_from);
        const int cancel_good=canonical_from_legacy_good(3);
        RealmTransportTestAccess::fund(cancel_world,sender,cancel_good,80,100);
        const int64_t cash_before=cancel_world.person(sender).cash;
        check(create(cancel_world,sender,cancel_to,cancel_good,80).success,"one-route shipment creates for closure and cancellation fixture");
        const auto shipment_id=cancel_world.transport_shipments().back().id;
        check(cancel_world.execute(TransportRouteCommand{1,cancel_world.person(1).decisions+1,
              uint8_t(cancel_route),false}).success,"sovereign can close the direct route while cargo is still waiting");
        cancel_world.advance();
        check(cancel_world.transport_cargo_lots().size()==1&&cancel_world.transport_cargo_lots()[0].arrival_day==0&&
              cancel_world.transport_cargo_lots()[0].quantity==80&&cancel_world.transport_escrow()==80&&cancel_world.invariant(),
              "a closed route leaves all waiting goods and reserved fee untouched");
        check(cancel_world.execute(TransportRouteCommand{1,cancel_world.person(1).decisions+1,
              uint8_t(cancel_route),true}).success,"sovereign can reopen a route with waiting cargo");
        cancel_world.advance();
        int64_t loaded=0,waiting=0;
        for(const auto&lot:cancel_world.transport_cargo_lots()){
            if(lot.arrival_day>0)loaded+=lot.quantity;
            else waiting+=lot.quantity;
        }
        check(loaded==64&&waiting==16&&cancel_world.transport_escrow()==16&&cancel_world.invariant(),
              "reopening resumes the shipment and loads 64 while leaving 16 for the next day");
        check(cancel_world.execute(TransportShipmentCommand{sender,cancel_world.person(sender).decisions+1,shipment_id,
              TransportShipmentOperation::Cancel,-1,-1,16,TransportServiceGoal::Fastest}).success&&
              cancel_world.person(sender).inventory[size_t(cancel_good)]==16&&cancel_world.person(sender).cash==cash_before-64&&
              cancel_world.transport_shipments().back().canceled==16&&cancel_world.invariant(),
              "after part of a shipment departs, cancel returns only the 16 waiting units and their unspent fees");
        cancel_world.advance(5);
        check(cancel_world.person(sender).inventory[size_t(cancel_good)]==80&&cancel_world.invariant(),
              "the 64 units already traveling still arrive once after partial cancellation");

        Realm sensitivity_probe(805,26,128);
        size_t road_forward=sensitivity_probe.national_transport_routes().size();
        size_t road_reverse=sensitivity_probe.national_transport_routes().size();
        for(size_t i=0;i<sensitivity_probe.national_transport_routes().size();++i)
            for(size_t j=i+1;j<sensitivity_probe.national_transport_routes().size();++j){
                const auto& a=sensitivity_probe.national_transport_routes()[i];
                const auto& b=sensitivity_probe.national_transport_routes()[j];
                if(a.mode==AtlasRouteMode::Road&&b.mode==AtlasRouteMode::Road&&a.atlas_link==b.atlas_link&&
                   a.from==b.to&&a.to==b.from){road_forward=i;road_reverse=j;break;}
            }
        check(road_forward<sensitivity_probe.national_transport_routes().size(),
              "sensitivity fixture finds one bidirectional road link");
        const int sensitivity_from=sensitivity_probe.national_transport_routes()[road_forward].from;
        const int sensitivity_to=sensitivity_probe.national_transport_routes()[road_forward].to;
        std::cout<<"SENSITIVITY_TRIAL_1,seed,805,quantity,80,fee_per_unit,1\n"
                 <<"duration_days,capacity_per_day,loaded_days,delivery_observed_day,delivered,invariant\n";
        for(const int travel_days:{1,2,4})for(const int capacity:{32,64,128}){
            Realm scenario(805,26,128);
            RealmTransportTestAccess::open_only(scenario,road_forward,road_reverse);
            RealmTransportTestAccess::configure_pair(scenario,road_forward,road_reverse,travel_days,capacity);
            const auto actor=RealmTransportTestAccess::actor_at(scenario,sensitivity_from);
            const int sensitivity_good=canonical_from_legacy_good(3);
            RealmTransportTestAccess::fund(scenario,actor,sensitivity_good,80,1000);
            check(create(scenario,actor,sensitivity_to,sensitivity_good,80).success,
                  "each paired sensitivity fixture accepts the same 80-unit shipment");
            int delivery_day=-1;
            std::vector<int> load_days;
            for(int day=0;day<12;++day){
                scenario.advance();
                for(const auto& lot:scenario.transport_cargo_lots())if(lot.arrival_day>0)
                    load_days.push_back(int(lot.arrival_day)-travel_days);
                const auto& shipment=scenario.transport_shipments().back();
                if(shipment.delivered==80){delivery_day=scenario.day();break;}
                if(!scenario.invariant())std::cerr<<"sensitivity invariant failure days="<<travel_days<<" capacity="<<capacity
                    <<" world_day="<<scenario.day()<<" shipment_escrow="<<shipment.fee_escrow
                    <<" transport_escrow="<<scenario.transport_escrow()<<" lots="<<scenario.transport_cargo_lots().size()<<'\n';
                check(scenario.invariant(),"sensitivity shipment preserves Realm invariants each simulated day");
            }
            std::sort(load_days.begin(),load_days.end());
            load_days.erase(std::unique(load_days.begin(),load_days.end()),load_days.end());
            check(delivery_day>0&&scenario.transport_shipments().back().delivered==80&&
                  scenario.person(actor).inventory[size_t(sensitivity_good)]==80&&scenario.invariant(),
                  "each sensitivity case delivers exactly 80 units without breaking invariants");
            std::cout<<travel_days<<','<<capacity<<','<<load_days.size()<<','<<delivery_day<<",80,true\n";
        }

        Realm estate_world(803,26,128);
        RealmTransportTestAccess::suppress_random_death(estate_world);
        const auto default_routes=estate_world.national_transport_routes();
        std::vector<size_t> multi_leg_path;
        int source=-1,destination=-1;
        for(int from=0;from<=estate_world.regions()&&multi_leg_path.empty();++from)
            for(int to=0;to<=estate_world.regions()&&multi_leg_path.empty();++to){
                if(from==to)continue;
                const auto candidate=NationalTransportManager::plan(0,uint8_t(from),uint8_t(to),5,
                    TransportServiceGoal::Fastest,default_routes);
                if(candidate.reachable&&candidate.route_indices.size()>=2){
                    source=from;destination=to;multi_leg_path=candidate.route_indices;
                }
            }
        check(multi_leg_path.size()>=2,"atlas oracle supplies a reachable multi-leg route");
        RealmTransportTestAccess::open_path_only(estate_world,multi_leg_path);
        const uint64_t deceased_sender=RealmTransportTestAccess::actor_at(estate_world,source);
        const int estate_good=canonical_from_legacy_good(3);
        RealmTransportTestAccess::fund(estate_world,deceased_sender,estate_good,5,1000);
        RealmTransportTestAccess::make_ill(estate_world,deceased_sender);
        check(create(estate_world,deceased_sender,destination,estate_good,5).success,
              "multi-leg shipment is accepted before the sender dies");
        const uint64_t estate_shipment_id=estate_world.transport_shipments().back().id;
        estate_world.advance();
        check(!estate_world.person(deceased_sender).alive&&
              estate_world.transport_shipments().back().id==estate_shipment_id&&
              estate_world.transport_shipments().back().delivered==0&&estate_world.invariant(),
              "death after departure keeps the undelivered goods and unpaid fees attached to the estate");
        const auto in_transit=estate_world.serialize("transport-estate-roundtrip");
        Realm estate_restored;
        check(estate_restored.deserialize(in_transit,"transport-estate-roundtrip",error)&&
              estate_restored.fingerprint()==estate_world.fingerprint(),
              "save/load preserves a deceased owner and cargo waiting between route legs");
        estate_world.advance(100);
        estate_restored.advance(100);
        const auto finished=std::find_if(estate_world.transport_shipments().begin(),estate_world.transport_shipments().end(),
            [estate_shipment_id](const TransportShipment& shipment){return shipment.id==estate_shipment_id;});
        check(finished!=estate_world.transport_shipments().end()&&finished->delivered==5&&finished->canceled==0&&
              estate_world.transport_cargo_lots().empty()&&estate_world.transport_escrow()==0&&
              estate_world.invariant()&&estate_restored.fingerprint()==estate_world.fingerprint(),
              "multi-leg cargo reaches the estate after death, settles inheritance once, and matches uninterrupted simulation");

        std::cout<<"REALM TRANSPORT: "<<checks<<" checks passed\n";
        return 0;
    }catch(const std::exception& error){
        std::cerr<<"FAIL after "<<checks<<": "<<error.what()<<'\n';
        return 1;
    }
}
