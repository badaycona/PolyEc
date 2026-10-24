#include "polyec/realm.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace polyec {
struct RealmPrivateHousingTestAccess {
    static std::pair<uint64_t,uint64_t> prepare(Realm& world) {
        uint64_t owner=0,holding=0,tenant=0;
        for(auto& person:world.people_)
            if(person.id>64&&person.alive&&person.age_days>=18*365){owner=person.id;break;}
        if(!owner)throw std::runtime_error("no owner fixture");
        auto& landlord=world.people_[size_t(owner-1)];
        landlord.family=Family::Civilian;landlord.sovereign=false;landlord.taxing_authority=false;
        landlord.work_assignment={};landlord.office_appointment={};
        landlord.region=8;landlord.residence_kind=ResidenceKind::Urban;landlord.urban_site_id=8;
        landlord.destination=-1;landlord.busy_until=0;landlord.cash=200;
        landlord.inventory[size_t(GoodId::ConstructionMaterial)]=2;
        landlord.inventory[size_t(GoodId::Tools)]=1;
        for(auto& province:world.provinces_)province.firm=std::max<int64_t>(province.firm,1000);
        world.land_holdings_[0].title_holder=owner;world.land_holdings_[0].region=8;
        world.land_holdings_[0].area_units=1;
        // Model these test-only goods as an explicit opening endowment.
        world.opening_[size_t(GoodId::ConstructionMaterial)]+=2;
        world.opening_[size_t(GoodId::Tools)]+=1;
        const auto household=landlord.household_membership.household_id;
        for(auto& person:world.people_)if(person.household_membership.household_id==household){
            person.region=8;person.residence_kind=ResidenceKind::Urban;person.urban_site_id=8;
        }
        for(auto& person:world.people_){
            if(person.alive&&person.age_days>=18*365&&person.household_membership.household_id!=household){
                const auto candidateHouse=person.household_membership.household_id;
                const auto authority=world.household_authority(candidateHouse);
                if(authority){
                    tenant=authority;
                    for(auto& member:world.people_)if(member.household_membership.household_id==candidateHouse){
                        member.region=8;member.residence_kind=ResidenceKind::Urban;member.urban_site_id=8;
                    }
                    break;
                }
            }
        }
        if(!tenant)throw std::runtime_error("no tenant fixture");
        auto& renter=world.people_[size_t(tenant-1)];
        renter.family=Family::Civilian;renter.cash=100;renter.sovereign=false;renter.taxing_authority=false;
        world.opening_money_=world.total_money();
        world.refresh_demography();
        if(!world.invariant())throw std::runtime_error("invalid prepared private-housing fixture");
        return {owner,tenant};
    }
    static void make_renter_poor(Realm& world,uint64_t tenant){
        auto& renter=world.people_[size_t(tenant-1)];
        const auto household=renter.household_membership.household_id;
        auto found=world.household_funds_.find(household);
        if(found!=world.household_funds_.end()){world.provinces_[8].firm+=found->second;world.household_funds_.erase(found);}
        world.provinces_[8].firm+=renter.cash;renter.cash=0;
    }
    static void make_rent_due_now(Realm& world){
        world.private_residences_.front().next_rent_day=world.day_;
        world.process_private_housing();
    }
    static void move_renter_home_region(Realm& world,uint64_t tenant,int region){
        const auto household=world.household_of(tenant);
        for(auto& member:world.people_)if(member.household_membership.household_id==household){
            member.region=region;member.destination=-1;member.residence_kind=ResidenceKind::Urban;
            member.urban_site_id=uint8_t(region);
        }
        world.urban_housing_capacity_[8]=0;
        world.refresh_demography();
    }
};
}

namespace {
int checks=0;
void check(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
}

int main(){
    using namespace polyec;
    try{
        Realm world(20261024,26,256);
        const auto [owner,tenant]=RealmPrivateHousingTestAccess::prepare(world);
        const auto holding=world.land_holdings().front().id;
        const int64_t landlordBefore=world.person(owner).cash;
        const auto build=world.execute(PrivateResidenceBuildCommand{owner,world.person(owner).decisions+1,holding});
        check(build.success&&world.private_residences().size()==1,"owner can build a private home on their urban title");
        check(world.person(owner).cash==landlordBefore-kPrivateResidenceBuildCost,"construction charges the title holder's personal cash");
        check(world.person(owner).inventory[size_t(GoodId::ConstructionMaterial)]==0&&world.person(owner).inventory[size_t(GoodId::Tools)]==0,"construction consumes the exact personal materials");
        check(!world.private_residences().front().completed&&world.invariant(),"home remains under construction and world accounting is valid");

        const auto blockedBuild=world.execute(PrivateResidenceBuildCommand{owner,world.person(owner).decisions+1,holding});
        check(!blockedBuild.success,"duplicate home on one holding is rejected");
        world.advance(kPrivateResidenceBuildDays);
        check(world.private_residences().front().completed&&world.invariant(),"home completes after thirty game days");
        const auto home=world.private_residences().front();

        const auto offer=world.execute(PrivateHousingCommand{owner,world.person(owner).decisions+1,home.id,tenant,PrivateHousingOperation::Offer,kPrivateResidenceRentPerPeriod});
        check(offer.success&&world.private_residences().front().offered_household==tenant,"landlord makes a voluntary offer to a local household");
        const auto accept=world.execute(PrivateHousingCommand{tenant,world.person(tenant).decisions+1,home.id,tenant,PrivateHousingOperation::Accept,0});
        check(accept.success&&world.private_residences().front().tenant_household==tenant,"tenant household authority accepts the offer");
        check(world.private_residences().front().next_rent_day==world.day()+kPrivateResidenceRentPeriodDays,"first rent is due thirty game days after acceptance");

        RealmPrivateHousingTestAccess::move_renter_home_region(world,tenant,0);
        const int moveAction=realm_extended_travel_candidate_begin+8-realm_extended_travel_region_begin;
        check(world.candidates(tenant)[size_t(moveAction)].kind==Intent::Migrate&&
              world.mask(tenant)[size_t(moveAction)]>0,"active lease gives its household a reserved place when public beds are full");
        const auto travel=world.execute({tenant,world.person(tenant).decisions+1,world.candidates(tenant)[size_t(moveAction)]});
        check(travel.success,"tenant can start the ordinary migration command toward the rented home");
        world.advance(kPrivateResidenceBuildDays);
        check(world.person(tenant).region==8&&world.invariant(),"arrival retains the rented household's urban residence");

        std::stringstream save(world.serialize("private-housing-test"));
        Realm restored(1,1,128);std::string error;
        const bool loaded=restored.deserialize(save.str(),"private-housing-test",error);
        if(!loaded)std::cerr<<"load error: "<<error<<'\n';
        check(loaded,"schema 67 roundtrip loads the active lease");
        check(restored.private_residences()==world.private_residences()&&restored.invariant(),"save/load preserves construction and lease terms");
        const int64_t ownerCash=restored.person(owner).cash;
        RealmPrivateHousingTestAccess::make_rent_due_now(restored);
        check(restored.person(owner).cash==ownerCash+kPrivateResidenceRentPerPeriod,"rent transfers to the current title holder when due");
        check(restored.private_residences().front().next_rent_day==restored.day()+kPrivateResidenceRentPeriodDays,"rent schedule advances by one period");

        RealmPrivateHousingTestAccess::make_renter_poor(restored,tenant);
        RealmPrivateHousingTestAccess::make_rent_due_now(restored);
        check(restored.private_residences().front().tenant_household==0,"lease ends when household cannot pay rent");
        check(restored.invariant(),"ending a lease leaves a valid world");
        std::cout<<"PRIVATE HOUSING TESTS: "<<checks<<" checks passed\n";
        return 0;
    }catch(const std::exception&error){std::cerr<<"FAIL after "<<checks<<": "<<error.what()<<'\n';return 1;}
}
