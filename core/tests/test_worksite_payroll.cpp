#include "polyec/realm.hpp"
#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace polyec;
namespace {
int checks=0;
void check(bool ok,const char* label){++checks;if(!ok)throw std::runtime_error(label);}
int64_t at(const std::map<uint64_t,int64_t>& map,uint64_t actor=65){const auto found=map.find(actor);return found==map.end()?0:found->second;}
int64_t regional_paid(const Realm& world){const auto key=WorkRegionKey{65,world.person(65).region};const auto& map=world.schedule_book().paid_work_units_by_region;
    const auto found=map.find(key);return found==map.end()?0:found->second;}
int64_t site_paid(const Realm& world){int64_t units=0;for(const auto& site:world.establishments())units+=at(site.work_pay.paid_units);return units;}
Realm fixture(const std::string& name){
    std::ifstream input(std::filesystem::path(POLYEC_TEST_FIXTURE_DIR)/"work-v16"/(name+".save"));
    const std::string source((std::istreambuf_iterator<char>(input)),{});
    check(source.rfind("POLYEC_REALM_16 ",0)==0,"frozen oracle is actual accepted old schema16");
    Realm world(1,1,128);std::string error;
    check(world.deserialize(source,"migration-v16",error),error.c_str());check(world.invariant(),"migrated oracle invariant");return world;
}
void work(Realm& world,uint64_t site,int64_t begin,int64_t end,uint64_t worker=65){
    const auto result=world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,ScheduleCommand::Operation::Submit,
        0,begin,end,0,1000,ActivityKind::Work,world.person(worker).region,site});
    check(result.success,result.reason.c_str());
}
void advance_to(Realm& world,int64_t tick){world.advance_ticks(tick-world.game_tick());check(world.invariant(),"payroll day invariant");}
void save_retry(Realm& world,int64_t tick){
    auto control=world;advance_to(control,tick);
    const auto saved=world.serialize("worksite-payroll");Realm loaded(1,1,128);std::string error;
    check(loaded.deserialize(saved,"worksite-payroll",error),error.c_str());advance_to(loaded,tick);
    check(loaded.fingerprint()==control.fingerprint(),"mid-shift/retry save continues exactly");world=std::move(loaded);
}
void fund_legacy_firm(Realm& world,int64_t amount){
    uint64_t funder=0;for(uint64_t actor=1;actor<=uint64_t(world.count());++actor)
        if(world.person(actor).family==Family::Economic&&world.person(actor).region==world.person(65).region){funder=actor;break;}
    check(funder!=0,"local economic actor can fund legacy firm");
    const auto result=world.execute(RealmCommand{funder,world.person(funder).decisions+1,{Intent::Hire,world.person(65).region,0,amount}});
    check(result.success,"actual Hire transfers money into legacy private account");
}
std::vector<std::string> tokens(const Realm& world){
    std::istringstream input(world.serialize("worksite-payroll"));std::vector<std::string> words;std::string word;
    while(input>>word)words.push_back(word);return words;
}
std::string joined(const std::vector<std::string>& words){std::ostringstream out;for(const auto& word:words)out<<word<<' ';return out.str();}
std::size_t find(const std::vector<std::string>& words,const char* tag){
    const auto found=std::find(words.begin(),words.end(),tag);if(found==words.end())throw std::runtime_error("missing fixture tag");return std::size_t(found-words.begin());
}
std::size_t first_site_pay(const std::vector<std::string>& words){
    auto at=find(words,"ESTABLISH_14")+2;at+=39+2*std::stoull(words.at(at+38));
    const auto workers=std::stoull(words.at(at++));
    for(std::size_t row=0;row<workers;++row)at+=2+2*std::stoull(words.at(at+1));return at;
}
void reject_save(Realm& world,const std::vector<std::string>& words){
    const auto before=world.fingerprint();std::string error;
    check(!world.deserialize(joined(words),"worksite-payroll",error)&&world.fingerprint()==before,"corrupt credits reject without replacing live world");
}
}
int main(){try{
    {
        auto world=fixture("partial-site");const auto id=world.establishments()[0].id;
        check(world.work_credits().empty()&&regional_paid(world)==0,"old8 ticks creates no phantom unit");
        work(world,id,64,72);advance_to(world,68);save_retry(world,128);
        check(site_paid(world)==1&&regional_paid(world)==0&&world.establishments()[0].cash==9,"old partial8 plus new8 pays one site unit");
    }
    {
        auto world=fixture("split-paid");const auto sites=world.establishments();const auto key=WorkRegionKey{65,world.person(65).region};
        check(world.work_credits().at(key)==WorkRegionCredit{1,0,1}&&regional_paid(world)==1,"old regional unit remains accounted with one bridge");
        auto control=world;advance_to(control,128);const auto firm=control.view().provinces[key.second].firm;const auto money=world.total_money();
        work(world,sites[0].id,64,72);work(world,sites[1].id,72,80);advance_to(world,70);save_retry(world,128);
        check(site_paid(world)==1&&regional_paid(world)==1,"two partial sites add only one new paid unit");
        check(world.view().provinces[key.second].firm==firm&&world.total_money()==money,"site pays its own wage, no extra Province debit");
        check(world.work_credits().at(key).bridge_units==0,"bridge exhausted before new unit payment");
        const auto paid=site_paid(world);advance_to(world,192);check(site_paid(world)==paid,"empty later day cannot duplicate wages");
    }
    {
        auto world=fixture("mixed-unassigned");const auto id=world.establishments()[0].id;
        work(world,id,64,72);advance_to(world,128);
        check(regional_paid(world)==1&&site_paid(world)==0,"first completed site unit consumes historical bridge");
        save_retry(world,192);
        check(regional_paid(world)==2&&site_paid(world)==0,"accepted legacy unassigned shift preserves other partial unit");
    }
    {
        auto world=fixture("legacy-debt");const auto sites=world.establishments();
        check(regional_paid(world)==1&&at(sites[0].work_pay.legacy_credit)==2&&at(sites[1].work_pay.legacy_credit)==1,"three old units credited while only one money unit was paid");
        fund_legacy_firm(world,2);save_retry(world,128);
        check(regional_paid(world)==3&&site_paid(world)==0&&world.establishments()[0].cash==10&&world.establishments()[1].cash==10,
            "legacy debt2 still paid from old source without billing either site");
    }
    {
        auto world=fixture("split-partial-money");const auto sites=world.establishments();const int region=world.person(65).region;
        check(world.gameplay_config().wage_per_unit()==3&&world.work_pay_remainder(65,region)==1&&regional_paid(world)==0,"old one money unit is not rounded up or erased");
        fund_legacy_firm(world,2);work(world,sites[0].id,64,72);work(world,sites[1].id,72,80);save_retry(world,128);
        check(regional_paid(world)==1&&site_paid(world)==1&&world.work_pay_remainder(65,region)==0,"wage3 lifetime entitlement is six money, not nine");
        check(world.establishments()[0].cash+world.establishments()[1].cash==17,"only three new money units leave sites");
    }
    for(const auto ownerKind:{EstablishmentOwnerKind::Person,EstablishmentOwnerKind::Public,EstablishmentOwnerKind::Organization}){
        Realm world(71,2,128);const uint64_t owner=ownerKind==EstablishmentOwnerKind::Public?1:65;
        const int region=world.person(65).region;uint64_t ownerId=65;
        if(ownerKind==EstablishmentOwnerKind::Organization){
            check(world.execute(RealmCommand{65,1,{Intent::Join,region,0,20,30}}).success,"real Join funds organization");ownerId=uint64_t(1+4*region);
        }else if(ownerKind==EstablishmentOwnerKind::Public)ownerId=uint64_t(region);
        const auto created=world.execute(EstablishmentCommand{owner,world.person(owner).decisions+1,ownerKind,ownerId,region,
            EstablishmentIndustry::ClothWorkshop,1,10});check(created.success,"owner funds actual payroll site");
        if(ownerKind==EstablishmentOwnerKind::Public)
            check(world.execute(WorkAuthorizationCommand{owner,world.person(owner).decisions+1,created.session,65,WorkAuthorizationOperation::Set,0,32}).success,"public owner authorizes worker");
        auto control=world;
        check(control.execute(ScheduleCommand{65,control.person(65).decisions+1,ScheduleCommand::Operation::Submit,
            0,0,32,0,1000}).success,"paired control enters same timeline without work wages");
        advance_to(control,64);
        const auto firm=control.view().provinces[region].firm,money=world.total_money();
        work(world,created.session,0,32);advance_to(world,64);
        check(world.establishments()[0].cash==8&&site_paid(world)==2&&regional_paid(world)==0,"every ownership kind pays two units only from site cash");
        check(world.view().provinces[region].firm==firm&&world.total_money()==money,"cash source and conservation independent of owner kind");
    }
    {
        Realm world(71,2,128);const int region=world.person(65).region;
        const auto created=world.execute(EstablishmentCommand{65,1,EstablishmentOwnerKind::Person,65,region,EstablishmentIndustry::ClothWorkshop,1,1});
        check(created.success,"shortage site funded by one actual money");work(world,created.session,0,32);advance_to(world,64);
        check(site_paid(world)==1&&world.establishments()[0].cash==0,"cash shortage keeps second work unit pending");
        check(world.execute(EstablishmentTransferCommand{65,world.person(65).decisions+1,created.session,
            EstablishmentTransferOperation::DepositCash,-1,1}).success,"real refill funds pending unit");
        save_retry(world,128);check(site_paid(world)==2&&world.establishments()[0].cash==0,"retry pays exactly one missing unit");
    }
    {
        Realm world(71,2,128);const int region=world.person(65).region;std::vector<uint64_t> sites;
        for(int n=0;n<2;++n){const auto result=world.execute(EstablishmentCommand{65,world.person(65).decisions+1,
            EstablishmentOwnerKind::Person,65,region,EstablishmentIndustry::ClothWorkshop,1,10});check(result.success,"distinct actual wage account created");sites.push_back(result.session);}
        work(world,sites[0],0,16);work(world,sites[1],16,32);save_retry(world,64);
        const auto actual=world.establishments();
        check(actual[0].cash==9&&actual[1].cash==9&&at(actual[0].work_pay.paid_units)==1&&at(actual[1].work_pay.paid_units)==1&&regional_paid(world)==0,
            "new cross-site wages debit each source exactly once");
    }
    {
        auto world=fixture("split-paid");const auto original=tokens(world);const auto credit=find(original,"WORKCREDIT_1");
        for(const auto [offset,value]:std::array<std::pair<std::size_t,const char*>,7>{{{2,"0"},{3,"7"},{4,"2"},{5,"1"},{6,"2"},{6,"-1"},{1,"9999999"}}}){
            auto changed=original;changed[credit+offset]=value;reject_save(world,changed);
        }
        auto duplicate=original;duplicate[credit+1]="2";
        duplicate.insert(duplicate.end(),original.begin()+std::ptrdiff_t(credit+2),original.begin()+std::ptrdiff_t(credit+7));
        reject_save(world,duplicate);
        auto debt=fixture("legacy-debt");const auto debtWords=tokens(debt);const auto sitePay=first_site_pay(debtWords);
        for(const auto value:{"0","1","3"}){auto changed=debtWords;changed[sitePay+2]=value;reject_save(debt,changed);}
        auto paidOld=debtWords;const auto paidMap=sitePay+1+2*std::stoull(debtWords.at(sitePay));
        paidOld[paidMap]="1";paidOld.insert(paidOld.begin()+std::ptrdiff_t(paidMap+1),{"65","1"});reject_save(debt,paidOld);
        auto badAttribution=debtWords;const auto schedule=find(badAttribution,"SCHEDULE_7")+1;
        const auto siteAttendance=schedule+10+17*std::stoull(badAttribution[schedule+4])+2*std::stoull(badAttribution[schedule+5])+
            2*std::stoull(badAttribution[schedule+6])+3*std::stoull(badAttribution[schedule+7])+3*std::stoull(badAttribution[schedule+8]);
        badAttribution.at(siteAttendance+5)="32";reject_save(debt,badAttribution);
        // Conservation alone is insufficient: bridge cannot coexist with new
        // site money already paid, including a partially paid wage.
        const auto sites=world.establishments();work(world,sites[0].id,64,72);work(world,sites[1].id,72,80);advance_to(world,128);
        auto unpaidBridge=tokens(world);const auto oldCredit=first_site_pay(unpaidBridge);
        check(unpaidBridge[oldCredit]=="1","day2 rotating bridge fixture credits first source");
        unpaidBridge[oldCredit]="0";unpaidBridge.erase(unpaidBridge.begin()+std::ptrdiff_t(oldCredit+1),unpaidBridge.begin()+std::ptrdiff_t(oldCredit+3));
        unpaidBridge[find(unpaidBridge,"WORKCREDIT_1")+6]="1";reject_save(world,unpaidBridge);
    }
    {
        Realm world(71,2,128);const auto created=world.execute(EstablishmentCommand{1,1,EstablishmentOwnerKind::Public,2,2,
            EstablishmentIndustry::ClothWorkshop,1,10});check(created.success,"live public employer for dead-worker fixture");
        check(world.execute(WorkAuthorizationCommand{1,2,created.session,65,WorkAuthorizationOperation::Set,0,32}).success,"authorize actual worker before death");
        work(world,created.session,0,32);advance_to(world,16);
        auto words=tokens(world);std::size_t start=0;
        constexpr std::size_t person_width=59; // 23 scalar fields + 6 stats + 30 goods
        for(std::size_t index=100;index+1+person_width*128<words.size();++index){
            if(words[index]!="128")continue;bool dense=true;
            for(std::size_t row=0;row<128;++row)if(words[index+1+person_width*row]!=std::to_string(row+1)){dense=false;break;}
            if(dense){start=index+1;break;}
        }
        check(start!=0,"death snapshot locates Person records");words[start+person_width*64+14]="0";std::string error;
        check(world.deserialize(joined(words),"worksite-payroll",error),error.c_str());advance_to(world,64);
        check(world.establishments()[0].cash==10&&site_paid(world)==0,"pending site wage freezes on worker death under existing contract");
    }
    std::cout<<"Worksite payroll checks="<<checks<<"; frozen v16 migration oracles=5\n";return 0;
}catch(const std::exception& error){std::cerr<<"FAIL after "<<checks<<": "<<error.what()<<'\n';return 1;}}






