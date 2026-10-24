#include "polyec/realm.hpp"
#include "work_authorization_fixture.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

using namespace polyec;

namespace polyec {
struct RealmMedicineTestAccess {
    static void prepare_and_run_day(Realm& world,uint64_t actor,int health,int medicine){
        auto& p=world.people_.at(size_t(actor-1));
        p.health=health;
        p.nutrition=100;
        p.inventory[size_t(GoodId::Medicine)]=medicine;
        world.day_people();
    }
    static int64_t consumed(const Realm& world){return world.consumed_[size_t(GoodId::Medicine)];}
};
}

namespace {
int checks=0;
void check(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
std::vector<std::string> tokens(const std::string& input){
    std::istringstream stream(input);std::vector<std::string> result;std::string token;
    while(stream>>token)result.push_back(token);return result;
}
std::string join(const std::vector<std::string>& values){
    std::ostringstream stream;for(const auto& value:values)stream<<value<<' ';return stream.str();
}
uint64_t find_person(const Realm& world,int region,Family family,bool adult=false){
    for(uint64_t id=1;id<=uint64_t(world.count());++id){
        const auto& p=world.person(id);
        if(p.alive&&p.destination==-1&&p.region==region&&p.family==family&&int(id%7)!=world.day()%7&&
           (!adult||p.age_days>=14*365))return id;
    }
    return 0;
}
}

int main(){try{
    constexpr int region=20;
    constexpr size_t medicine=size_t(GoodId::Medicine);
    static_assert(int(GoodId::Medicine)==21);
    static_assert(realm_actions==400&&realm_obs==5182);

    Realm world(73021,26,512);
    const uint64_t authority=uint64_t(region+2);
    const uint64_t worker=find_person(world,region,Family::Civilian,true);
    const uint64_t buyer=find_person(world,region,Family::Economic,true);
    check(world.person(authority).taxing_authority&&worker&&buyer,
          "Atlas region 20 has a public authority, adult worker and adult market participant");
    const auto candidates=world.candidates(authority);
    const auto& create=candidates[size_t(realm_specialty_product_candidate_begin+2)];
    check(create.kind==Intent::CreateEstablishment&&create.target==region&&
          create.good==int(EstablishmentIndustry::MedicinalGarden)&&
          world.mask(authority,candidates)[size_t(realm_specialty_product_candidate_begin+2)]==1.f,
          "the region 20 authority receives the 100-unit Medicinal Garden choice");
    const auto created=world.execute({authority,world.person(authority).decisions+1,create});
    check(created.success&&world.establishments().size()==1,
          "the authority creates one Medicinal Garden through CreateEstablishment");
    const uint64_t site=created.session;
    check(world.establishments()[0].source_good==int(GoodId::Medicine)&&
          world.establishments()[0].source_units==100&&world.establishments()[0].stock[medicine]==0,
          "the garden starts with 100 finite source units separate from its Medicine inventory");
    const auto wrongRegion=world.execute(EstablishmentCommand{authority,world.person(authority).decisions+1,
        EstablishmentOwnerKind::Public,20,19,EstablishmentIndustry::MedicinalGarden,1,25});
    check(!wrongRegion.success&&world.establishments().size()==1&&world.invariant(),
          "a direct command cannot create this source outside region 20 or partially change the world");
    const auto duplicate=world.execute(EstablishmentCommand{authority,world.person(authority).decisions+1,
        EstablishmentOwnerKind::Public,20,20,EstablishmentIndustry::MedicinalGarden,1,25});
    check(!duplicate.success&&world.establishments().size()==1,
          "a second Medicinal Garden cannot duplicate the unique regional source");

    check(world.execute(WorkAuthorizationCommand{authority,world.person(authority).decisions+1,site,worker,
        WorkAuthorizationOperation::Set,world.game_tick(),world.game_tick()+32}).success,
        "the regional authority authorizes a worker for the upcoming interval");
    check(world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
        ScheduleCommand::Operation::Submit,0,world.game_tick(),world.game_tick()+32,0,1000,
        ActivityKind::Work,region,site}).success,
        "the worker schedules Work through the normal schedule command");
    world.advance_ticks(64);
    check(world.establishments()[0].source_units==99&&world.establishments()[0].stock[medicine]==1&&
          world.establishments()[0].stock[size_t(GoodId::Tea)]==0&&world.invariant(),
          "one complete work batch consumes one source unit and creates exactly one Medicine");

    auto resumedSave=world.serialize("medicine-source-and-treatment-v1");
    check(resumedSave.rfind("POLYEC_REALM_70 ",0)==0,
          "the current schema 64 save contains the new industry and urban housing section");
    Realm resumed(1,1,128);std::string error;
    check(resumed.deserialize(resumedSave,"medicine-source-and-treatment-v1",error),error.c_str());
    check(resumed.fingerprint()==world.fingerprint()&&resumed.establishments()[0].source_units==99&&
          resumed.establishments()[0].stock[medicine]==1,
          "save/load preserves the garden source and Medicine output without producing twice");
    std::string schema62=resumedSave;
    test_fixtures::downgrade_save_64_to_63(schema62);
    schema62.replace(0,std::string("POLYEC_REALM_63").size(),"POLYEC_REALM_62");
    Realm migrated62(1,1,128);
    check(migrated62.deserialize(schema62,"medicine-source-and-treatment-v1",error)&&
          migrated62.establishments().size()==1&&
          migrated62.establishments()[0].industry==EstablishmentIndustry::MedicinalGarden&&
          migrated62.establishments()[0].source_units==99&&
          migrated62.establishments()[0].stock[medicine]==1&&
          migrated62.serialize("medicine-source-and-treatment-v1").rfind("POLYEC_REALM_70 ",0)==0&&
          migrated62.invariant(),
          "a schema 62 Medicinal Garden loads with its source and Medicine stock intact, then saves as schema 64");
    auto malformed=tokens(resumedSave);
    const auto section=std::find(malformed.begin(),malformed.end(),"ESTABLISH_14");
    check(section!=malformed.end()&&size_t(section-malformed.begin())+3<malformed.size(),
          "the serialized establishment section can be located for a corruption check");
    malformed[size_t(section-malformed.begin())+3]="19";
    const auto beforeBadLoad=resumed.fingerprint();
    check(!resumed.deserialize(join(malformed),"medicine-source-and-treatment-v1",error)&&
          resumed.fingerprint()==beforeBadLoad,
          "a save placing Medicinal Garden at region 19 is rejected without changing the loaded world");
    check(resumed.execute(EstablishmentTransferCommand{authority,resumed.person(authority).decisions+1,
        site,EstablishmentTransferOperation::WithdrawGoods,int(GoodId::Medicine),1}).success&&
        resumed.view(authority).provinces[region].stock[medicine]==1,
        "the owner moves the actual Medicine product into the regional market using the ordinary transfer command");
    check(resumed.execute({buyer,resumed.person(buyer).decisions+1,{Intent::Borrow,region,0,300,180}}).success,
          "the market participant can obtain money through the existing loan action");
    check(resumed.execute({buyer,resumed.person(buyer).decisions+1,
        {Intent::Buy,region,int(GoodId::Medicine),1}}).success&&resumed.person(buyer).inventory[medicine]==1,
        "the person buys the real Medicine good through the ordinary market action");

    const int64_t consumedBefore=RealmMedicineTestAccess::consumed(resumed);
    RealmMedicineTestAccess::prepare_and_run_day(resumed,buyer,55,1);
    check(resumed.person(buyer).health==65&&resumed.person(buyer).inventory[medicine]==0&&
          RealmMedicineTestAccess::consumed(resumed)==consumedBefore+1,
          "a living person at health 55 consumes only their own Medicine and recovers 10 points");
    const auto treatmentView=resumed.view(buyer);
    check(std::any_of(treatmentView.facts.begin(),treatmentView.facts.end(),[&](const Fact& fact){
        return fact.actor==buyer&&fact.kind==237&&fact.amount==1;
    }),"the person can read a dated history event identifying their own Medicine treatment");
    const std::string treatedSave=resumed.serialize("medicine-source-and-treatment-v1");
    Realm treatedAgain(1,1,128);
    check(treatedAgain.deserialize(treatedSave,"medicine-source-and-treatment-v1",error)&&
          treatedAgain.person(buyer).health==65&&treatedAgain.person(buyer).inventory[medicine]==0&&
          RealmMedicineTestAccess::consumed(treatedAgain)==consumedBefore+1,
          "save/load preserves the completed treatment and does not repeat its consumption");

    Realm oldSchema(73029,26,512);
    std::string schema61=oldSchema.serialize("medicine-source-and-treatment-v1");
    const size_t residence=schema61.rfind(" URBAN_RESIDENCE_1 ");
    check(residence!=std::string::npos,"schema 64 contains the residence section needed to construct a schema 61 fixture");
    (void)residence; // Residence data was introduced in schema 61 and remains part of its fixture.
    test_fixtures::downgrade_save_64_to_63(schema61);
    schema61.replace(0,std::string("POLYEC_REALM_63").size(),"POLYEC_REALM_61");
    Realm migrated(1,1,128);
    check(migrated.deserialize(schema61,"medicine-source-and-treatment-v1",error)&&
          migrated.serialize("medicine-source-and-treatment-v1").rfind("POLYEC_REALM_70 ",0)==0&&
          migrated.establishments().empty()&&migrated.invariant(),
          "schema 61 migrates to 64 without inventing a garden, Medicine or residence data");
    for(const auto [health,stock,label]:{std::tuple{61,1,"health above 60"},
                                        std::tuple{0,1,"health zero"},
                                        std::tuple{40,0,"no personal Medicine"}}){
        Realm edge(73022+uint64_t(health+stock),26,512);
        const uint64_t actor=find_person(edge,region,Family::Civilian,true);
        check(actor!=0,"an adult person is available for each treatment boundary case");
        const int64_t before=RealmMedicineTestAccess::consumed(edge);
        RealmMedicineTestAccess::prepare_and_run_day(edge,actor,health,stock);
        const bool noTreatment=health==0?!edge.person(actor).alive:
            edge.person(actor).health==health&&edge.person(actor).inventory[medicine]==stock;
        check(noTreatment&&
              RealmMedicineTestAccess::consumed(edge)==before,
              label);
    }

    check(resumed.invariant()&&treatedAgain.invariant(),
          "the production, market, treatment and reload ledgers remain internally consistent");
    std::cout<<"Medicine production/treatment checks="<<checks<<'\n';return 0;
}catch(const std::exception& error){
    std::cerr<<"Medicine production/treatment failure after "<<checks<<": "<<error.what()<<'\n';return 1;
}}
