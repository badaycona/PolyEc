#pragma once
#include <array>
#include <cstdint>
#include <random>
#include <string>
#include <vector>
#include <unordered_map>

namespace polyec {
inline constexpr const char* realm_schema="realm-v5";
inline constexpr int realm_actions=32, realm_state=128, realm_features=12;
inline constexpr int realm_obs=realm_state+realm_actions*realm_features;
enum class Family:int { Governance=0,Economic=1,Civilian=2 };
enum class Intent:int { Hold,Buy,Sell,Travel,Borrow,Repay,Project,Hire,Tax,Aid,Investigate,Offer,Accept,Reject,Work,Study,Rest,Social,Join,Migrate,Support,Appoint,Mobilize };
struct Stats { std::array<int,6> values{50,50,50,50,50,50}; };
struct Person {
    uint64_t id=0,household=0,decisions=0,sampling=0;
    int region=0,age_days=0,health=100,skill=0,job=-1,busy_until=0,destination=-1;
    int goal=0,school_days=0,social=0; bool alive=true;
    int tax_remainder=0;
    int organization=0;
    int job_kind=0;
    bool taxing_authority=false,sovereign=false;
    Family family=Family::Civilian; Stats stats;
    int64_t cash=100; std::array<int64_t,7> inventory{};
};
struct Province { int64_t treasury=10000,firm=30000;std::array<int64_t,7> stock{};std::array<int,7> price{};int tax=10,school=8,school_used=0,jobs=32,employed=0,road=0,unrest=0,teachers=0;bool charter=true; };
struct Edge { uint64_t from=0,to=0;int kind=0; };
struct Loan { uint64_t id=0,debtor=0;int creditor=0,due=0;int64_t principal=0,interest=0,paid=0;bool defaulted=false; };
struct Project {uint64_t id=0,owner=0;int region=0,due=0,kind=0;int64_t budget=0;bool completed=false;};
struct Fact {uint64_t id=0,parent=0,actor=0,target=0;int day=0,region=0,kind=0;int64_t amount=0;bool secret=false;};
struct Knowledge {uint64_t observer=0,fact=0;int arrives=0;};
struct Proposal {uint64_t id=0,from=0,to=0;int64_t amount=0;int due=0;bool resolved=false;};
struct Candidate {Intent kind=Intent::Hold;int target=0,good=0;int64_t amount=0;int term=0;};
struct RealmCommand {uint64_t actor=0,sequence=0;Candidate action;};
struct RealmResult {bool success=false;std::string reason;uint64_t fact=0;};
struct Draft {int tier=0;std::array<int,3> cards{};int seen=0,chosen=-1;bool rerolled=false;};
struct RealmView {int day=0,regions=0;uint64_t persons=0,alive=0,children=0,decisions=0;int64_t money=0;std::vector<Province> provinces;std::vector<Fact> facts;Draft draft;};
// Owns all mutable campaign state. Snapshots and observations are copies.
class Realm {
public:
    Realm(uint64_t seed=1,int regions=6,int people=10000);
    // Explicit initial-condition fixtures for paired eval. Never mutates a live world.
    static Realm scenario(uint64_t seed,uint64_t actor,int age=-1,int stat=-1,int goal=-1,int budget=-1,int schools=-1,int jobs=-1);
    const Person& person(uint64_t id) const;
    std::vector<uint64_t> due(Family family) const;
    std::array<Candidate,realm_actions> candidates(uint64_t actor) const;
    std::array<float,realm_actions> mask(uint64_t actor) const;
    std::array<float,realm_obs> observation(uint64_t actor) const;
    RealmResult execute(const RealmCommand& command);
    void advance(int days=1);
    RealmView view(uint64_t observer=1) const;
    bool invariant() const;
    uint64_t fingerprint() const;
    int64_t total_money() const;
    double utility(uint64_t actor) const;
    bool reroll(int slot);
    RealmResult route_command(uint64_t actor,int from,int to,bool open);
    int distance(int from,int to)const;
    Draft draft()const{return draft_;}
    bool choose(int card);
    bool save(const std::string& path,const std::string& bundle,std::string& error) const;
    bool load(const std::string& path,const std::string& bundle,std::string& error);
    std::string serialize(const std::string& bundle) const;
    bool deserialize(const std::string& data,const std::string& bundle,std::string& error);
    int day() const {return day_;}
    int regions() const {return regions_;}
    int count() const {return int(people_.size());}
    int64_t debt(uint64_t actor) const;
    int64_t available_funds(uint64_t actor)const;
    std::vector<Loan> loans() const {return loans_;}
    std::vector<Project> projects() const {return projects_;}
    std::vector<Edge> relations() const {return edges_;}
    static std::string action_name(const Candidate& c);
private:
    int day_=0,regions_=6;uint64_t seed_=0,next_id_=1,next_fact_=1;
    std::mt19937_64 rng_;Draft draft_;
    std::vector<Person> people_;std::vector<Province> provinces_;
    std::vector<Edge> edges_;std::vector<Loan> loans_;std::vector<Project> projects_;
    std::vector<Fact> facts_;std::vector<Knowledge> knowledge_;std::vector<Proposal> proposals_;
    std::unordered_map<uint64_t,std::unordered_map<uint64_t,int>> knowledge_index_;
    std::array<std::array<bool,7>,7> routes_{};
    std::array<int64_t,28> organization_funds_{};
    std::array<int64_t,7> opening_{},produced_{},consumed_{};
    int64_t opening_money_=0,carrier_=0,estate_=0,endowment_=0;
    Person& mutable_person(uint64_t id);
    uint64_t guardian(const Person& person)const;
    void spend(Person& person,int64_t& destination,int64_t amount);
    void payroll(int region,bool public_employer,const std::vector<std::pair<uint64_t,int64_t>>& claims);
    bool legal(uint64_t actor,const Candidate& c,std::string& reason) const;
    void daily();void prices();void event(uint64_t actor,int region,int kind,int64_t amount=0,uint64_t target=0,bool secret=false,uint64_t parent=0);
    void transfer(int64_t& from,int64_t& to,int64_t amount);
    bool knows(uint64_t observer,const Fact& f) const;
};
}
