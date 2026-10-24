#include "polyec/realm.hpp"
#include "work_authorization_fixture.hpp"

#include <cstdint>
#include <iostream>
#include <string>

namespace polyec {
struct RealmUrbanResidenceTestAccess {
    static uint64_t prepare_housing_project(Realm& world,int& region) {
        uint64_t owner=0;
        for(int candidate_region=0;candidate_region<=world.regions_;++candidate_region){
            const bool has_urban=std::any_of(world.people_.begin(),world.people_.end(),[&](const Person& person){
                return person.alive&&person.region==candidate_region&&person.residence_kind==ResidenceKind::Urban;
            });
            if(!has_urban||world.provinces_[size_t(candidate_region)].firm<kUrbanHousingProjectBudget)continue;
            for(auto& person:world.people_)if(person.alive&&person.family==Family::Governance&&person.region==candidate_region){
                owner=person.id;region=candidate_region;break;
            }
            if(owner)break;
        }
        if(!owner)return 0;
        auto& province=world.provinces_[size_t(region)];
        world.people_[size_t(owner-1)].taxing_authority=true;
        province.charter=true;
        world.urban_housing_capacity_[size_t(region)]=0;
        if(province.firm<kUrbanHousingProjectBudget)return 0;
        world.transfer(province.firm,world.municipal_funds_[size_t(region)],kUrbanHousingProjectBudget);
        province.stock[size_t(GoodId::ConstructionMaterial)]+=4;
        province.stock[size_t(GoodId::Tools)]+=2;
        world.opening_[size_t(GoodId::ConstructionMaterial)]+=4;
        world.opening_[size_t(GoodId::Tools)]+=2;
        world.refresh_demography();
        return owner;
    }
    static void finish_housing_projects_after_trial_term(Realm& world) {
        world.day_+=kUrbanHousingProjectTermDays;
        world.schedule_book_.cursor=int64_t(world.day_)*kSubdayUnitsPerDay;
        world.market_settled_day_=world.day_;
        world.market_processed_month_=world.day_/30;
        world.finish_projects();
        world.refresh_demography();
    }
    static std::array<int64_t,kGoodIdCount> goods_at(const Realm& world,int region){
        return world.provinces_[size_t(region)].stock;
    }
    static int64_t& housing_capacity(Realm& world,int region){return world.urban_housing_capacity_[size_t(region)];}
    static int64_t reserved(const Realm& world,int region){return world.urban_arrivals_reserved_[size_t(region)];}
    static void try_birth(Realm& world, uint64_t parent_id) {
        for (auto& person : world.people_) person.age_days = 365;
        auto& parent = world.people_[size_t(parent_id - 1)];
        parent.age_days = 30 * 365;
        parent.health = 100;
        parent.nutrition = 100;
        parent.cash = 100;
        world.day_ = 0;
        world.day_births();
        world.refresh_demography();
    }
};
}

namespace {
int failures = 0;
void check(bool value, const char* message) {
    if (!value) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
}

int main() {
    using namespace polyec;
    Realm first(20261023, 26, 512);
    Realm same(20261023, 26, 512);
    Realm other_seed(20261024, 26, 512);
    check(first.invariant(), "new Atlas realm satisfies residence invariants");
    check(first.urban_census().size() == kAtlasTerritoryCount, "Atlas census includes all 27 sites");
    check(first.urban_census() == same.urban_census(), "starting residence assignment is deterministic for a seed");
    check(first.urban_census() != other_seed.urban_census(), "different seed changes at least one aggregate assignment");

    uint64_t alive = 0, urban = 0, rural = 0, unknown = 0;
    for (const auto& row : first.urban_census()) {
        urban += row.urban_residents;
        rural += row.rural_residents;
        unknown += row.unknown_residence;
    }
    for (uint64_t id = 1; id <= uint64_t(first.count()); ++id) {
        const auto& person = first.person(id);
        if (person.alive) ++alive;
    }
    check(alive == urban + rural + unknown, "living census categories sum to living population");
    check(unknown == 0 && urban > 0 && rural > 0, "new Atlas starting population has known urban and rural residences");
    for(const auto& row:first.urban_census())
        check(row.housing_capacity%kUrbanHousingBedsPerBlock==0&&row.housing_capacity>=row.urban_residents&&
              row.unhoused_residents==0,"opening housing provides beds for current urban residents in blocks of eight");
    const auto before_query = first.fingerprint();
    (void)first.urban_census();
    check(first.fingerprint() == before_query, "census query does not mutate the world");

    const std::string current = first.serialize("urban-residence-test");
    check(current.rfind("POLYEC_REALM_70 ", 0) == 0, "new save uses schema 69");
    Realm restored;
    std::string error;
    const bool loaded = restored.deserialize(current, "urban-residence-test", error);
    if (!loaded) std::cerr << "schema 69 roundtrip error: " << error << '\n';
    check(loaded, "schema 69 roundtrip loads");
    check(restored.invariant() && restored.urban_census() == first.urban_census(), "roundtrip preserves residence and census");

    std::string legacy64=current;
    test_fixtures::downgrade_save_66_to_65(legacy64);
    const auto council_section=legacy64.rfind(" MUNICIPAL_COUNCIL_1 ");
    check(council_section!=std::string::npos,"schema 66 fixture contains municipal council section");
    if(council_section!=std::string::npos){
        legacy64.erase(council_section);
        legacy64.replace(0,std::string("POLYEC_REALM_67").size(),"POLYEC_REALM_64");
        std::string legacy63=legacy64;
        const auto housing_section=legacy63.rfind(" URBAN_HOUSING_1 ");
        check(housing_section!=std::string::npos,"schema 64 save contains housing capacity section");
        if(housing_section!=std::string::npos){
        legacy63.erase(housing_section);
        legacy63.replace(0,std::string("POLYEC_REALM_64").size(),"POLYEC_REALM_63");
        Realm migrated;
        check(migrated.deserialize(legacy63,"urban-residence-test",error),"schema 63 save migrates to current housing rules");
        check(migrated.urban_census()==first.urban_census(),"schema 63 migration initializes enough beds without changing residents");
        check(migrated.invariant(),"schema 63 housing migration preserves world invariants");
        }
    }

    Realm housing(20261026,26,512);
    int housing_region=-1;
    const uint64_t housing_owner=RealmUrbanResidenceTestAccess::prepare_housing_project(housing,housing_region);
    if(!housing_owner)std::cerr<<"housing fixture could not find a funded Governance owner in a settlement with Urban residents\n";
    check(housing_owner!=0,"housing fixture has a local Governance owner, Urban residents and enough project funds");
    if(housing_owner){
        check(housing.invariant(),"prepared housing fixture preserves money and goods ledgers");
        auto options=housing.candidates(housing_owner);
        Candidate project=options[281];
        const auto before_funds=housing.municipal_funds()[size_t(housing_region)];
        const auto before_goods=RealmUrbanResidenceTestAccess::goods_at(housing,housing_region);
        check(Realm::action_name(project)=="Xây nhà ở công cộng","the housing candidate has a player-readable action name");
        const auto started=housing.execute({housing_owner,housing.person(housing_owner).decisions+1,project});
        check(started.success,"eligible local Governance can start the public housing project");
        if(started.success){
            check(housing.municipal_funds()[size_t(housing_region)]==before_funds-kUrbanHousingProjectBudget,
                  "housing construction takes exactly the stated civic fund budget");
            const auto pending_stock=RealmUrbanResidenceTestAccess::goods_at(housing,housing_region);
            check(pending_stock[size_t(GoodId::ConstructionMaterial)]==before_goods[size_t(GoodId::ConstructionMaterial)]-4&&
                  pending_stock[size_t(GoodId::Tools)]==before_goods[size_t(GoodId::Tools)]-2,
                  "housing construction consumes four real construction materials and two tools");
            check(housing.urban_census()[size_t(housing_region)].housing_capacity==0,"housing is not counted before the construction term ends");
            check(housing.invariant(),"active housing project preserves money, goods and project invariants");
            const auto duplicate_before=housing.fingerprint();
            check(!housing.execute({housing_owner,housing.person(housing_owner).decisions+1,project}).success&&
                  housing.fingerprint()==duplicate_before,"a second public housing project is rejected atomically while one is active");
            Realm activeRoundtrip;
            const auto activeSave=housing.serialize("urban-housing-test");
            check(activeRoundtrip.deserialize(activeSave,"urban-housing-test",error)&&
                  activeRoundtrip.fingerprint()==housing.fingerprint(),"save/load preserves a housing project before completion");
            RealmUrbanResidenceTestAccess::finish_housing_projects_after_trial_term(housing);
            check(housing.urban_census()[size_t(housing_region)].housing_capacity==kUrbanHousingBedsPerBlock,
                  "housing project adds eight beds after sixty game days");
            RealmUrbanResidenceTestAccess::finish_housing_projects_after_trial_term(activeRoundtrip);
            check(activeRoundtrip.fingerprint()==housing.fingerprint(),
                  "a saved in-progress project completes on the same day with the same result");
            check(housing.invariant(),"completed housing project preserves world invariants");
            Realm roundtrip;
            const auto saved=housing.serialize("urban-housing-test");
            check(roundtrip.deserialize(saved,"urban-housing-test",error),"save/load preserves completed housing capacity");
            check(roundtrip.urban_census()==housing.urban_census(),"housing census remains identical after save/load");
        }
    }

    std::string legacy = current;
    const auto residence_section = legacy.rfind(" URBAN_RESIDENCE_1 ");
    check(residence_section != std::string::npos, "schema 61 save contains a residence section");
    if (residence_section != std::string::npos) {
        legacy.erase(residence_section);
        legacy.replace(0, std::string("POLYEC_REALM_63").size(), "POLYEC_REALM_60");
        Realm migrated;
        check(migrated.deserialize(legacy, "urban-residence-test", error), "schema 60 save migrates");
        uint64_t migrated_unknown = 0;
        for (const auto& row : migrated.urban_census()) migrated_unknown += row.unknown_residence;
        check(migrated_unknown == alive, "schema 60 residents become unknown rather than being mislabeled rural");
        check(migrated.invariant(), "migrated legacy save satisfies invariants");
    }

    std::string malformed = current;
    const auto section = malformed.rfind(" URBAN_RESIDENCE_1 ");
    if (section != std::string::npos) {
        size_t count_end = malformed.find(' ', section + 1);
        count_end = malformed.find(' ', count_end + 1);
        std::size_t pos = count_end + 1;
        bool corrupted = false;
        for (int row = 0; row < first.count(); ++row) {
            const auto id_end = malformed.find(' ', pos);
            const auto kind_end = malformed.find(' ', id_end + 1);
            const auto site_end = malformed.find(' ', kind_end + 1);
            const unsigned kind = unsigned(std::stoul(malformed.substr(id_end + 1, kind_end - id_end - 1)));
            if (kind == static_cast<unsigned>(ResidenceKind::Urban)) {
                const auto site = unsigned(std::stoul(malformed.substr(kind_end + 1, site_end - kind_end - 1)));
                malformed.replace(kind_end + 1, site_end - kind_end - 1, std::to_string((site + 1) % kAtlasTerritoryCount));
                corrupted = true;
                break;
            }
            pos = site_end + 1;
        }
        check(corrupted, "test found an urban residence row to corrupt");
        const auto target_before = restored.fingerprint();
        check(!restored.deserialize(malformed, "urban-residence-test", error), "wrong site to territory is rejected");
        check(restored.fingerprint() == target_before, "malformed residence load leaves destination world unchanged");
    }

    Realm small(7, 6, 256);
    check(small.urban_census().empty(), "profiles without Atlas sites do not fabricate census rows");
    for (uint64_t id = 1; id <= uint64_t(small.count()); ++id)
        check(small.person(id).residence_kind == ResidenceKind::Unknown, "small profile residence stays unknown");

    bool inherited_urban_birth = false;
    for (uint64_t seed = 1; seed <= 32 && !inherited_urban_birth; ++seed) {
        Realm birth_world(seed, 26, 512);
        uint64_t parent_id = 0;
        for (uint64_t id = 1; id <= uint64_t(birth_world.count()); ++id)
            if (birth_world.person(id).residence_kind == ResidenceKind::Urban) { parent_id = id; break; }
        if (!parent_id) continue;
        const auto parent_residence = birth_world.person(parent_id).residence_kind;
        const auto parent_site = birth_world.person(parent_id).urban_site_id;
        const int before = birth_world.count();
        RealmUrbanResidenceTestAccess::try_birth(birth_world, parent_id);
        if (birth_world.count() == before + 1) {
            const auto& child = birth_world.person(uint64_t(before + 1));
            inherited_urban_birth = child.residence_kind == parent_residence && child.urban_site_id == parent_site;
            check(birth_world.invariant(), "birth with inherited urban residence preserves world invariants");
        }
    }
    check(inherited_urban_birth, "newborn inherits the urban site of its parent");

    Realm traveler(20261025, 26, 512);
    bool completed_urban_migration = false;
    for (uint64_t id = 1; id <= uint64_t(traveler.count()) && !completed_urban_migration; ++id) {
        const auto& person = traveler.person(id);
        if (person.residence_kind != ResidenceKind::Urban || person.age_days < 14 * 365) continue;
        const auto candidates = traveler.candidates(id);
        for (const auto& candidate : candidates) {
            if (candidate.kind != Intent::Migrate) continue;
            const int origin = person.region;
            const uint8_t origin_site = person.urban_site_id;
            const auto result = traveler.execute({id, person.decisions + 1, candidate});
            if (!result.success) continue;
            check(traveler.person(id).region == origin && traveler.person(id).urban_site_id == origin_site,
                  "urban traveler remains counted at the origin before arrival");
            const int arrival_day = traveler.person(id).busy_until;
            traveler.advance(arrival_day - traveler.day() + 1);
            const auto& arrived = traveler.person(id);
            completed_urban_migration = arrived.region == candidate.target &&
                arrived.urban_site_id == static_cast<uint8_t>(candidate.target);
            check(completed_urban_migration, "urban migration changes site only after arrival");
            break;
        }
    }
    check(completed_urban_migration, "test completes one valid urban migration");

    Realm full_city(20261027,26,512);
    uint64_t blocked_migrant=0;size_t blocked_slot=0;
    const auto full_census=full_city.urban_census();
    for(uint64_t id=1;id<=uint64_t(full_city.count())&&!blocked_migrant;++id){
        const auto& person=full_city.person(id);
        if(!person.alive||person.destination!=-1||person.residence_kind!=ResidenceKind::Urban||person.age_days<14*365)continue;
        const auto choices=full_city.candidates(id);
        for(size_t slot=0;slot<choices.size();++slot)if(choices[slot].kind==Intent::Migrate&&choices[slot].target!=person.region){
            const size_t target=size_t(choices[slot].target);
            auto& capacity=RealmUrbanResidenceTestAccess::housing_capacity(full_city,choices[slot].target);
            const auto row=std::find_if(full_census.begin(),full_census.end(),[&](const UrbanCensus& value){return value.territory_id==target;});
            const int64_t original=capacity;
            if(full_city.mask(id,choices)[slot]!=1.f)continue;
            capacity=int64_t(row->urban_residents)+RealmUrbanResidenceTestAccess::reserved(full_city,choices[slot].target);
            if(full_city.mask(id,choices)[slot]==0.f){blocked_migrant=id;blocked_slot=slot;break;}
            capacity=original;
        }
    }
    check(blocked_migrant!=0,"test finds an adult urban migrant and fills a reachable destination");
    if(blocked_migrant){
        const auto choices=full_city.candidates(blocked_migrant);const auto before=full_city.fingerprint();
        const auto result=full_city.execute({blocked_migrant,full_city.person(blocked_migrant).decisions+1,choices[blocked_slot]});
        check(!result.success&&full_city.fingerprint()==before,"migration into a full settlement rejects without charging travel money or changing the world");
    }
    return failures == 0 ? 0 : 1;
}
