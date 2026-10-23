#pragma once
#include "work_authorization_fixture.hpp"
#include "polyec/establishment_section_codec.hpp"

void authorization_replay(){
    Realm world(71,2,128);const auto site=create(world,1,EstablishmentOwnerKind::Public,2);
    uint64_t worker=0;for(uint64_t id=2;id<=uint64_t(world.count());++id)
        if(world.person(id).alive&&world.person(id).region==2){worker=id;break;}
    auto point=[&](int index){std::cout<<index<<' '<<world.fingerprint()<<' '<<worker<<' '<<site<<'\n';};
    point(0);
    if(!world.execute(WorkAuthorizationCommand{1,world.person(1).decisions+1,site,worker,WorkAuthorizationOperation::Set,0,96}).success)throw std::runtime_error("replay grant");
    point(1);
    if(!world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,ScheduleCommand::Operation::Submit,0,16,64,0,1000,ActivityKind::Work,2,site}).success)throw std::runtime_error("replay Work");
    point(2);
    if(world.execute(WorkAuthorizationCommand{1,world.person(1).decisions+1,site,worker,WorkAuthorizationOperation::Revoke}).success)throw std::runtime_error("replay revoke should reject");
    point(3);world.advance_ticks(24);point(4);
    Realm loaded(1,1,128);std::string error;if(!loaded.deserialize(world.serialize("authorization-replay"),"authorization-replay",error))throw std::runtime_error(error);
    world=std::move(loaded);world.advance_ticks(40);point(5);
    if(!world.execute(WorkAuthorizationCommand{1,world.person(1).decisions+1,site,worker,WorkAuthorizationOperation::Revoke}).success)throw std::runtime_error("replay revoke after shift");
    point(6);
}

// Included by the transfer executable to reuse its allocation-failure harness.
void authorization_checks(){
    Realm world(71,2,256);const uint64_t owner=1;
    const auto site=create(world,owner,EstablishmentOwnerKind::Public,2);
    uint64_t worker=0;for(uint64_t id=2;id<=uint64_t(world.count());++id)
        if(world.person(id).alive&&world.person(id).region==2){worker=id;break;}
    check(worker!=0,"authorization fixture worker present");
    auto grant=[&](WorkAuthorizationOperation operation,int64_t begin=0,int64_t end=0){
        return world.execute(WorkAuthorizationCommand{owner,world.person(owner).decisions+1,site,worker,operation,begin,end});
    };
    auto work=[&](int64_t begin,int64_t end){return world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,
        ScheduleCommand::Operation::Submit,0,begin,end,0,1000,ActivityKind::Work,2,site});};
    auto rejectWork=[&](int64_t begin,int64_t end){const auto before=world.fingerprint();const auto result=work(begin,end);
        check(!result.success&&result.reason=="Work requires owner authorization"&&world.fingerprint()==before,"unauthorized Work rejects before mutation");};
    rejectWork(0,16);
    check(grant(WorkAuthorizationOperation::Set,16,96).success,"owner grants exact window");
    {
        const auto ownerFacts=world.view(owner).facts;
        const auto fact=ownerFacts.back();const auto workerFacts=world.view(worker).facts;
        const auto outsiderFacts=world.view(2).facts;
        check(fact.kind==208&&fact.target==worker&&fact.secret,"grant fact targets worker Person and is private");
        check(std::any_of(workerFacts.begin(),workerFacts.end(),[&](const auto& f){return f.id==fact.id;})&&
              std::none_of(outsiderFacts.begin(),outsiderFacts.end(),[&](const auto& f){return f.id==fact.id;}),"grant visible to worker without leaking to outsider");
    }
    rejectWork(0,16);rejectWork(80,112);
    auto before=world.fingerprint();check(!grant(WorkAuthorizationOperation::Set,16,96).success&&world.fingerprint()==before,"no-op grant rejects without fact spam");
    const auto accepted=work(16,96);check(accepted.success,"worker consents with exact-boundary Work");
    before=world.fingerprint();check(!grant(WorkAuthorizationOperation::Revoke).success&&world.fingerprint()==before,"revoke cannot cut accepted Work");
    check(!grant(WorkAuthorizationOperation::Set,32,96).success&&world.fingerprint()==before,"replacement cannot cut accepted beginning");
    check(!grant(WorkAuthorizationOperation::Set,16,64).success&&world.fingerprint()==before,"replacement cannot cut accepted end");
    check(grant(WorkAuthorizationOperation::Set,0,128).success,"owner can extend accepted window");
    check(world.execute(ScheduleCommand{worker,world.person(worker).decisions+1,ScheduleCommand::Operation::Cancel,accepted.session}).success,"worker cancels before Work starts");
    check(grant(WorkAuthorizationOperation::Revoke).success,"owner revokes after cancellation");
    rejectWork(16,32);
    before=world.fingerprint();check(!grant(WorkAuthorizationOperation::Revoke).success&&world.fingerprint()==before,"absent revoke atomic");
    const auto malformed=WorkAuthorizationCommand{owner,world.person(owner).decisions+1,site,worker,WorkAuthorizationOperation::Set,0,128};
    for(int variant=0;variant<7;++variant){
        auto command=malformed;
        switch(variant){case 0:command.worker=0;break;case 1:command.establishment_id=0;break;
        case 2:command.sequence=0;break;case 3:command.end=365000LL*64+1;break;
        case 4:command.operation=static_cast<WorkAuthorizationOperation>(99);break;
        case 5:command.begin=-1;break;case 6:command.actor=worker;command.sequence=world.person(worker).decisions+1;break;}
        before=world.fingerprint();const auto result=world.execute(command);
        check(!result.success&&world.fingerprint()==before,"invalid authorization atomic");
    }
    auto deceased=dead_owner(world,worker);before=deceased.fingerprint();
    check(!deceased.execute(malformed).success&&deceased.fingerprint()==before,"deceased worker cannot receive grant");
    check(grant(WorkAuthorizationOperation::Set,0,96).success,"migration fixture grant");
    check(work(0,16).success&&work(32,64).success,"migration fixture separated shifts");
    world.advance_ticks(8);
    Realm migrated(1,1,128);std::string error;
    check(migrated.deserialize(polyec::test_fixtures::save_as_v15(world.serialize("authorization")),"authorization",error),error.c_str());
    const auto windows=migrated.establishments()[0].work_authorizations.at(worker);
    check(windows==std::vector<WorkAuthorizationWindow>{{8,16},{32,64}},"legacy migration clips consumed ticks and preserves gaps");
    before=migrated.fingerprint();
    check(!migrated.execute(ScheduleCommand{worker,migrated.person(worker).decisions+1,ScheduleCommand::Operation::Submit,
        0,16,32,0,1000,ActivityKind::Work,2,site}).success&&migrated.fingerprint()==before,"legacy gap grants no Work right");
    const auto saved=migrated.serialize("authorization");Realm loaded(1,1,128);
    check(loaded.deserialize(saved,"authorization",error)&&loaded.serialize("authorization")==saved,"authorization roundtrip exact");
    {
        // Independent literal grammar oracle, including a deliberate gap.
        std::string prefix=" ESTABLISH_5 1 129 2 1 2 1 1 10 ";
        for(int n=0;n<27;++n)prefix+="0 ";
        prefix+="0 0 ";
        const std::string literal=prefix+"1 6 2 8 16 32 64 0 0 0 ";
        std::istringstream input(literal);std::vector<Establishment> decoded;
        check(detail::read_establishment_section(input,128,2,17,decoded,error),"literal authorization grammar accepted");
        std::ostringstream output;detail::write_establishment_section(output,decoded);
        std::string currentLiteral=" ESTABLISH_14 1 129 2 1 2 1 1 10 ";
        for(int n=0;n<30;++n)currentLiteral+="0 ";
        currentLiteral+="0 0 1 6 2 8 16 32 64 0 0 0 -1 0 ";
        check(output.str()==currentLiteral,"authorization codec writes the current source-ledger record exactly");
        for(const auto tail:{"1 0 1 8 16 ","1 6 0 ","1 6 2 8 16 16 64 ","1 6 2 8 16 12 64 ",
                             "1 6 1 -1 16 ","1 6 1 8 23360001 ","1 6 1000001 ","129 "}){
            const auto prior=decoded.front().work_authorizations;
            std::istringstream corrupt(prefix+tail);
            check(!detail::read_establishment_section(corrupt,128,2,17,decoded,error)&&decoded.front().work_authorizations==prior,
                "corrupt authorization grammar leaves previous records intact");
        }
    }
    check(loaded.execute(RealmClockCommand{RealmClockCommand::Operation::Pause}).success,"authorization pause");
    before=loaded.fingerprint();bool pausedRejected=false;
    try{loaded.advance_ticks(128);}catch(const std::runtime_error&){pausedRejected=true;}
    check(pausedRejected&&loaded.fingerprint()==before,"pause does not expire authorization");
    migrated.advance_ticks(120);auto uninterrupted=world;uninterrupted.advance_ticks(120);
    check(migrated.schedule_book().work_present_ticks.at(worker)==uninterrupted.schedule_book().work_present_ticks.at(worker)&&migrated.invariant(),"legacy accepted shifts keep attendance");
    for(int variant=0;variant<3;++variant){
        const auto operation=variant==2?WorkAuthorizationOperation::Revoke:WorkAuthorizationOperation::Set;
        bool committed=false;int failures=0;
        for(int allocation=0;allocation<64;++allocation){
            Realm probe(71,2,256);const auto probeSite=create(probe,owner,EstablishmentOwnerKind::Public,2);
            if(variant>0)polyec::test_fixtures::authorize_work(probe,probeSite,worker,128);
            const auto original=probe.fingerprint();RealmResult result;bool failed=false;
            fault_phase="work authorization";fault_index=allocation;allocations_until_failure=allocation;
            try{result=probe.execute(WorkAuthorizationCommand{owner,probe.person(owner).decisions+1,probeSite,worker,operation,0,
                operation==WorkAuthorizationOperation::Set?(variant==1?256:128):0});}catch(const std::bad_alloc&){failed=true;}
            allocations_until_failure=-1;fault_phase="none";
            if(failed){++failures;check(probe.fingerprint()==original,"authorization allocation failure preserves state");}
            else{committed=result.success;break;}
        }
        check(committed&&(variant!=0||failures>=2),"authorization allocation sweep reaches commit");
    }
    for(const auto operation:{ScheduleCommand::Operation::Submit,ScheduleCommand::Operation::Cancel,ScheduleCommand::Operation::Reprioritize}){
        bool committed=false;int failures=0;
        for(int allocation=0;allocation<128;++allocation){
            Realm probe(71,2,256);const auto probeSite=create(probe,owner,EstablishmentOwnerKind::Public,2);
            polyec::test_fixtures::authorize_work(probe,probeSite,worker,128);
            if(operation!=ScheduleCommand::Operation::Submit){
                const auto accepted=probe.execute(ScheduleCommand{worker,probe.person(worker).decisions+1,ScheduleCommand::Operation::Submit,
                    0,16,64,0,1000,ActivityKind::Work,2,probeSite});
                check(accepted.success,"schedule fault fixture accepted Work");
            }
            const auto original=probe.fingerprint();RealmResult result;bool failed=false;
            fault_phase="authorized schedule";fault_index=allocation;allocations_until_failure=allocation;
            try{result=probe.execute(ScheduleCommand{worker,probe.person(worker).decisions+1,operation,
                1,16,64,3,1000,ActivityKind::Work,2,probeSite});}catch(const std::bad_alloc&){failed=true;}
            allocations_until_failure=-1;fault_phase="none";
            if(failed){++failures;check(probe.fingerprint()==original,"schedule allocation failure preserves IDs, sessions, facts and decisions");}
            else{committed=result.success;break;}
        }
        check(committed&&failures>0,"schedule allocation sweep reaches successful commit");
    }
}




