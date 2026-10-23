#include "work_authorization_fixture.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace polyec {
struct RealmObservationTestAccess {
    static Person& mutable_person(Realm& world,uint64_t actor){return world.mutable_person(actor);}
};
}

using namespace polyec;

int main(){
    try {
    struct RecipeCase{uint64_t seed;EstablishmentIndustry industry;GoodId first,second,output;const char* name;};
    constexpr RecipeCase recipes[]{
        {53,EstablishmentIndustry::ToolWorkshop,GoodId::Iron,GoodId::Fuel,GoodId::Tools,"Tools"},
        {55,EstablishmentIndustry::MilitarySupplyWorkshop,GoodId::Tools,GoodId::Cloth,GoodId::MilitarySupply,"MilitarySupply"}
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
        std::vector<std::string> tokens;{std::istringstream input(base.serialize("secondary-recipe"));std::string token;while(input>>token)tokens.push_back(token);}
        const auto section=std::find(tokens.begin(),tokens.end(),"ESTABLISH_14");
        if(section==tokens.end())throw std::runtime_error("current save lacks the establishment section");
        const size_t record=size_t(section-tokens.begin())+2;
        for(const GoodId input:{recipe.first,recipe.second}){
            const size_t good=size_t(input);tokens.at(record+7+good)="1";
            tokens.at(18+good)=std::to_string(std::stoll(tokens.at(18+good))+1);
        }
        std::ostringstream seeded;for(const auto& token:tokens)seeded<<token<<' ';
        Realm prepared(2,1,128);std::string error;
        if(!prepared.deserialize(seeded.str(),"secondary-recipe",error))throw std::runtime_error(error);
        Realm uninterrupted=prepared,split=prepared;
        uninterrupted.advance_ticks(64);split.advance_ticks(16);
        Realm resumed(2,1,128);
        if(!resumed.deserialize(split.serialize("secondary-recipe"),"secondary-recipe",error))throw std::runtime_error(error);
        resumed.advance_ticks(48);
        const auto site=resumed.establishments().at(0);
        if(uninterrupted.fingerprint()!=resumed.fingerprint()||
           site.stock[size_t(recipe.first)]!=0||site.stock[size_t(recipe.second)]!=0||
           site.stock[size_t(recipe.output)]!=1||!resumed.invariant()){
            throw std::runtime_error(std::string(recipe.name)+" recipe mismatch: first="+
                std::to_string(site.stock[size_t(recipe.first)])+" second="+
                std::to_string(site.stock[size_t(recipe.second)])+" output="+
                std::to_string(site.stock[size_t(recipe.output)])+" day="+std::to_string(resumed.day()));
        }
        std::cout<<recipe.name<<" recipe passed\n";
    }
    } catch(const std::exception& error) {
        std::cerr<<error.what()<<'\n';
        return 1;
    }
}
