#include "work_authorization_fixture.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace polyec {
struct RealmObservationTestAccess {
    static Person& mutable_person(Realm& world,uint64_t actor){return world.mutable_person(actor);}
    static void add_personal_good(Realm& world,uint64_t actor,GoodId good){
        ++world.mutable_person(actor).inventory[size_t(good)];
        ++world.opening_[size_t(good)];
    }
};
}

using namespace polyec;

int main(){
    try {
    struct RecipeCase{uint64_t seed;EstablishmentIndustry industry;std::array<GoodId,3> inputs;size_t input_count;GoodId output;const char* name;};
    constexpr RecipeCase recipes[]{
        {53,EstablishmentIndustry::ToolWorkshop,{GoodId::Iron,GoodId::Copper,GoodId::Fuel},3,GoodId::Tools,"Tools"},
        {55,EstablishmentIndustry::MilitarySupplyWorkshop,{GoodId::Tools,GoodId::Cloth,GoodId::Fuel},2,GoodId::MilitarySupply,"MilitarySupply"}
    };
    for(const auto& recipe:recipes){
        Realm base(recipe.seed,2,128);const uint64_t worker=65;
        RealmObservationTestAccess::mutable_person(base,worker).skill=100;
        const int region=base.person(worker).region;
        const auto created=base.execute(EstablishmentCommand{worker,base.person(worker).decisions+1,
            EstablishmentOwnerKind::Person,worker,region,recipe.industry,1,1});
        if(!created.success)throw std::runtime_error(std::string(recipe.name)+" workshop creation failed: "+created.reason);
        test_fixtures::authorize_work(base,created.session,worker);
        const auto scheduled=base.execute(ScheduleCommand{worker,base.person(worker).decisions+1,
            ScheduleCommand::Operation::Submit,0,0,32,0,1000,ActivityKind::Work,region,created.session});
        if(!scheduled.success)throw std::runtime_error(std::string(recipe.name)+" Work scheduling failed: "+scheduled.reason);
        for(size_t input=0;input<recipe.input_count;++input)
            RealmObservationTestAccess::add_personal_good(base,worker,recipe.inputs[input]);
        const int worksiteCandidate=realm_worksite_candidate_begin;
        auto candidates=base.candidates(worker);
        if(candidates[size_t(worksiteCandidate)].kind!=Intent::TransferEstablishmentGoods||
           candidates[size_t(worksiteCandidate)].good!=int(recipe.inputs[0]))
            throw std::runtime_error(std::string(recipe.name)+" first input candidate mismatch");
        auto moved=base.execute({worker,base.person(worker).decisions+1,candidates[size_t(worksiteCandidate)]});
        if(!moved.success)throw std::runtime_error(std::string(recipe.name)+" first input transfer failed: "+moved.reason);
        for(size_t input=1;input<recipe.input_count;++input){
            candidates=base.candidates(worker);
            const auto&candidate=candidates[size_t(worksiteCandidate+1)];
            if(candidate.kind!=Intent::TransferEstablishmentGoods||candidate.good!=int(recipe.inputs[input]))
                throw std::runtime_error(std::string(recipe.name)+" next input candidate mismatch");
            moved=base.execute({worker,base.person(worker).decisions+1,candidate});
            if(!moved.success)throw std::runtime_error(std::string(recipe.name)+" input transfer failed: "+moved.reason);
        }
        if(recipe.industry==EstablishmentIndustry::ToolWorkshop){
            candidates=base.candidates(worker);
            if(candidates[size_t(worksiteCandidate+1)].good!=int(GoodId::Fuel))
                throw std::runtime_error("Tools candidate does not switch from Copper to Fuel after Copper is supplied");
        }
        std::string error;
        Realm prepared(2,1,128);
        if(!prepared.deserialize(base.serialize("secondary-recipe"),"secondary-recipe",error))
            throw std::runtime_error(error);
        Realm uninterrupted=prepared,split=prepared;
        uninterrupted.advance_ticks(64);split.advance_ticks(16);
        Realm resumed(2,1,128);
        if(!resumed.deserialize(split.serialize("secondary-recipe"),"secondary-recipe",error))throw std::runtime_error(error);
        resumed.advance_ticks(48);
        const auto site=resumed.establishments().at(0);
        if(uninterrupted.fingerprint()!=resumed.fingerprint()||
           std::any_of(recipe.inputs.begin(),recipe.inputs.begin()+std::ptrdiff_t(recipe.input_count),
                       [&](GoodId input){return site.stock[size_t(input)]!=0;})||
           site.stock[size_t(recipe.output)]!=1||!resumed.invariant()){
            throw std::runtime_error(std::string(recipe.name)+" recipe mismatch: first="+
                std::to_string(site.stock[size_t(recipe.inputs[0])])+" second="+
                std::to_string(recipe.input_count>1?site.stock[size_t(recipe.inputs[1])]:0)+" output="+
                std::to_string(site.stock[size_t(recipe.output)])+" day="+std::to_string(resumed.day()));
        }
        std::cout<<recipe.name<<" recipe passed\n";
    }
    } catch(const std::exception& error) {
        std::cerr<<error.what()<<'\n';
        return 1;
    }
}
