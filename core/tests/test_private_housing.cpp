#include "polyec/realm.hpp"

#include <algorithm>
#include <iostream>
#include <limits>
#include <set>
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
        landlord.inventory[size_t(GoodId::PlainGlass)]=1;
        for(auto& province:world.provinces_)province.firm=std::max<int64_t>(province.firm,1000);
        world.land_holdings_[0].title_holder=owner;world.land_holdings_[0].region=8;
        world.land_holdings_[0].area_units=1;
        // Model these test-only goods as an explicit opening endowment.
        world.opening_[size_t(GoodId::ConstructionMaterial)]+=2;
        world.opening_[size_t(GoodId::Tools)]+=1;
        world.opening_[size_t(GoodId::PlainGlass)]+=1;
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
    static void add_extra_urban_titles(Realm&world,uint64_t owner,int count){
        for(int index=0;index<count;++index)
            world.land_holdings_.push_back({world.next_id_++,8,1,owner});
    }
    static int64_t consumed_plain_glass(const Realm&world){return world.consumed_[size_t(GoodId::PlainGlass)];}
    static void remove_plain_glass(Realm&world,uint64_t owner){
        --world.people_[size_t(owner-1)].inventory[size_t(GoodId::PlainGlass)];
        --world.opening_[size_t(GoodId::PlainGlass)];
    }
    static void set_consumed_plain_glass(Realm&world,int64_t amount){
        world.consumed_[size_t(GoodId::PlainGlass)]=amount;
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
        Realm noGlass(20261026,26,256);
        const auto [noGlassOwner,noGlassTenant]=RealmPrivateHousingTestAccess::prepare(noGlass);
        (void)noGlassTenant;
        const auto noGlassHolding=noGlass.land_holdings().front().id;
        RealmPrivateHousingTestAccess::remove_plain_glass(noGlass,noGlassOwner);
        const auto noGlassBefore=noGlass.fingerprint();
        const auto noGlassResult=noGlass.execute(PrivateResidenceBuildCommand{
            noGlassOwner,noGlass.person(noGlassOwner).decisions+1,noGlassHolding});
        check(!noGlassResult.success&&noGlass.fingerprint()==noGlassBefore,
              "building without personally owned PlainGlass is rejected without changing any ledger");

        Realm glassOverflow(20261027,26,256);
        const auto [overflowOwner,overflowTenant]=RealmPrivateHousingTestAccess::prepare(glassOverflow);
        (void)overflowTenant;
        const auto overflowHolding=glassOverflow.land_holdings().front().id;
        RealmPrivateHousingTestAccess::set_consumed_plain_glass(glassOverflow,std::numeric_limits<int64_t>::max());
        const auto overflowBefore=glassOverflow.fingerprint();
        const auto overflowResult=glassOverflow.execute(PrivateResidenceBuildCommand{
            overflowOwner,glassOverflow.person(overflowOwner).decisions+1,overflowHolding});
        check(!overflowResult.success&&glassOverflow.fingerprint()==overflowBefore,
              "PlainGlass consumption overflow is rejected atomically");

        Realm world(20261024,26,256);
        const auto [owner,tenant]=RealmPrivateHousingTestAccess::prepare(world);
        const auto tenantHousehold=world.household_of(tenant);
        const auto holding=world.land_holdings().front().id;
        const int64_t landlordBefore=world.person(owner).cash;
        const auto buildCandidates=world.candidates(owner);
        const auto buildAction=std::find_if(buildCandidates.begin()+realm_private_build_candidate_begin,
            buildCandidates.begin()+realm_private_offer_candidate_begin,[](const Candidate&candidate){return candidate.kind==Intent::PrivateResidenceBuild;});
        check(buildAction!=buildCandidates.begin()+realm_private_offer_candidate_begin,"owned urban holding appears among learned-policy build choices");
        const auto buildIndex=int(buildAction-buildCandidates.begin());
        check(world.mask(owner)[size_t(buildIndex)]>0,"build candidate mask matches an affordable legal build");
        const auto build=world.execute({owner,world.person(owner).decisions+1,*buildAction});
        check(build.success&&world.private_residences().size()==1,"owner can build a private home on their urban title");
        check(world.person(owner).cash==landlordBefore-kPrivateResidenceBuildCost,"construction charges the title holder's personal cash");
        check(world.person(owner).inventory[size_t(GoodId::ConstructionMaterial)]==0&&world.person(owner).inventory[size_t(GoodId::Tools)]==0,"construction consumes the exact personal materials");
        check(world.person(owner).inventory[size_t(GoodId::PlainGlass)]==0&&
              RealmPrivateHousingTestAccess::consumed_plain_glass(world)==1,
              "construction consumes one personally owned PlainGlass and records it in the goods ledger");
        check(!world.private_residences().front().completed&&world.invariant(),"home remains under construction and world accounting is valid");

        const auto blockedBuild=world.execute(PrivateResidenceBuildCommand{owner,world.person(owner).decisions+1,holding});
        check(!blockedBuild.success,"duplicate home on one holding is rejected");
        world.advance(kPrivateResidenceBuildDays);
        check(world.private_residences().front().completed&&world.invariant(),"home completes after thirty game days");
        const auto home=world.private_residences().front();

        const auto offerCandidates=world.candidates(owner);
        const auto offerAction=std::find_if(offerCandidates.begin()+realm_private_offer_candidate_begin,
            offerCandidates.begin()+realm_private_response_candidate_begin,[&](const Candidate&candidate){return candidate.kind==Intent::PrivateHousing&&candidate.good==int(PrivateHousingOperation::Offer)&&candidate.target==int(home.id)&&candidate.amount==int64_t(tenantHousehold);});
        check(offerAction!=offerCandidates.begin()+realm_private_response_candidate_begin,"a completed vacant home and local household appear as a learned-policy offer choice");
        check(world.mask(owner)[size_t(offerAction-offerCandidates.begin())]>0,"offer candidate mask matches the typed offer rules");
        const auto offer=world.execute({owner,world.person(owner).decisions+1,*offerAction});
        check(offer.success&&world.private_residences().front().offered_household==tenantHousehold,"landlord makes a voluntary offer to a local household");
        const auto responseCandidates=world.candidates(tenant);
        const auto declineAction=std::find_if(responseCandidates.begin()+realm_private_response_candidate_begin,
            responseCandidates.begin()+realm_private_end_lease_candidate_begin,[&](const Candidate&candidate){return candidate.kind==Intent::PrivateHousing&&candidate.good==int(PrivateHousingOperation::Decline)&&candidate.target==int(home.id);});
        const auto acceptAction=std::find_if(responseCandidates.begin()+realm_private_response_candidate_begin,
            responseCandidates.begin()+realm_private_end_lease_candidate_begin,[&](const Candidate&candidate){return candidate.kind==Intent::PrivateHousing&&candidate.good==int(PrivateHousingOperation::Accept)&&candidate.target==int(home.id);});
        check(declineAction!=responseCandidates.begin()+realm_private_end_lease_candidate_begin&&acceptAction!=responseCandidates.begin()+realm_private_end_lease_candidate_begin,
            "the tenant receives separate accept and decline choices for the same offer");
        check(world.execute({tenant,world.person(tenant).decisions+1,*declineAction}).success&&
              world.private_residences().front().offered_household==0,"tenant can decline through the ordinary candidate command");
        const auto renewedCandidates=world.candidates(owner);
        const auto renewedOffer=std::find_if(renewedCandidates.begin()+realm_private_offer_candidate_begin,
            renewedCandidates.begin()+realm_private_response_candidate_begin,[&](const Candidate&candidate){return candidate.kind==Intent::PrivateHousing&&candidate.good==int(PrivateHousingOperation::Offer)&&candidate.target==int(home.id)&&candidate.amount==int64_t(tenantHousehold);});
        check(renewedOffer!=renewedCandidates.begin()+realm_private_response_candidate_begin&&
              world.execute({owner,world.person(owner).decisions+1,*renewedOffer}).success,"landlord can send a new offer after a decline");
        const auto nextResponses=world.candidates(tenant);
        const auto nextAccept=std::find_if(nextResponses.begin()+realm_private_response_candidate_begin,
            nextResponses.begin()+realm_private_end_lease_candidate_begin,[&](const Candidate&candidate){return candidate.kind==Intent::PrivateHousing&&candidate.good==int(PrivateHousingOperation::Accept)&&candidate.target==int(home.id);});
        check(nextAccept!=nextResponses.begin()+realm_private_end_lease_candidate_begin,"renewed offer has a legal acceptance choice");
        const auto accept=world.execute({tenant,world.person(tenant).decisions+1,*nextAccept});
        check(accept.success&&world.private_residences().front().tenant_household==tenantHousehold,"tenant household authority accepts the offer through the candidate pipeline");
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
        check(RealmPrivateHousingTestAccess::consumed_plain_glass(restored)==1,
              "save/load retains the PlainGlass consumption ledger without adding a new save field");
        const int64_t ownerCash=restored.person(owner).cash;
        RealmPrivateHousingTestAccess::make_rent_due_now(restored);
        check(restored.person(owner).cash==ownerCash+kPrivateResidenceRentPerPeriod,"rent transfers to the current title holder when due");
        check(restored.private_residences().front().next_rent_day==restored.day()+kPrivateResidenceRentPeriodDays,"rent schedule advances by one period");

        const auto endCandidates=restored.candidates(owner);
        const auto endAction=std::find_if(endCandidates.begin()+realm_private_end_lease_candidate_begin,
            endCandidates.end(),[&](const Candidate&candidate){return candidate.kind==Intent::PrivateHousing&&
                candidate.good==int(PrivateHousingOperation::EndLease)&&candidate.target==int(home.id);});
        check(endAction!=endCandidates.end()&&restored.mask(owner)[size_t(endAction-endCandidates.begin())]>0,
            "landlord receives a legal end-lease choice");
        check(restored.execute({owner,restored.person(owner).decisions+1,*endAction}).success&&
              restored.private_residences().front().tenant_household==0,"landlord ends the lease through the learned-policy command path");
        const auto reofferCandidates=restored.candidates(owner);
        const auto reofferAction=std::find_if(reofferCandidates.begin()+realm_private_offer_candidate_begin,
            reofferCandidates.begin()+realm_private_response_candidate_begin,[&](const Candidate&candidate){return candidate.kind==Intent::PrivateHousing&&
                candidate.good==int(PrivateHousingOperation::Offer)&&candidate.target==int(home.id)&&candidate.amount==int64_t(tenantHousehold);});
        check(reofferAction!=reofferCandidates.begin()+realm_private_response_candidate_begin&&
              restored.execute({owner,restored.person(owner).decisions+1,*reofferAction}).success,"vacant home can be offered again after the lease ends");
        const auto renterResponses=restored.candidates(tenant);
        const auto renterAccept=std::find_if(renterResponses.begin()+realm_private_response_candidate_begin,
            renterResponses.begin()+realm_private_end_lease_candidate_begin,[&](const Candidate&candidate){return candidate.kind==Intent::PrivateHousing&&
                candidate.good==int(PrivateHousingOperation::Accept)&&candidate.target==int(home.id);});
        check(renterAccept!=renterResponses.begin()+realm_private_end_lease_candidate_begin&&
              restored.execute({tenant,restored.person(tenant).decisions+1,*renterAccept}).success,"tenant can accept a later offer through the candidate path");

        RealmPrivateHousingTestAccess::make_renter_poor(restored,tenant);
        RealmPrivateHousingTestAccess::make_rent_due_now(restored);
        check(restored.private_residences().front().tenant_household==0,"lease ends when household cannot pay rent");
        check(restored.invariant(),"ending a lease leaves a valid world");

        Realm rotating(20261025,26,256);
        const auto [rotatingOwner,rotatingTenant]=RealmPrivateHousingTestAccess::prepare(rotating);
        (void)rotatingTenant;
        RealmPrivateHousingTestAccess::add_extra_urban_titles(rotating,rotatingOwner,16);
        check(rotating.invariant(),"many titles for one owner remain valid before candidate rotation");
        const auto firstWindow=rotating.candidates(rotatingOwner);
        std::vector<int> firstTargets;
        for(int slot=realm_private_build_candidate_begin;slot<realm_private_offer_candidate_begin;++slot)
            if(firstWindow[size_t(slot)].kind==Intent::PrivateResidenceBuild)
                firstTargets.push_back(firstWindow[size_t(slot)].target);
        check(firstTargets.size()==16,"one observation contains no more than sixteen build targets");
        check(rotating.execute({rotatingOwner,rotating.person(rotatingOwner).decisions+1,Candidate{}}).success,
              "an ordinary committed decision advances the candidate window");
        const auto secondWindow=rotating.candidates(rotatingOwner);
        std::vector<int> secondTargets;
        for(int slot=realm_private_build_candidate_begin;slot<realm_private_offer_candidate_begin;++slot)
            if(secondWindow[size_t(slot)].kind==Intent::PrivateResidenceBuild)
                secondTargets.push_back(secondWindow[size_t(slot)].target);
        std::set<int> allTargets(firstTargets.begin(),firstTargets.end());
        allTargets.insert(secondTargets.begin(),secondTargets.end());
        check(secondTargets.size()==16&&allTargets.size()==17,
              "stable-ID candidate window rotates so the seventeenth title appears after the next decision");
        std::cout<<"PRIVATE HOUSING TESTS: "<<checks<<" checks passed\n";
        return 0;
    }catch(const std::exception&error){std::cerr<<"FAIL after "<<checks<<": "<<error.what()<<'\n';return 1;}
}
