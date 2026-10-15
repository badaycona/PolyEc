#pragma once
#include <array>
#include <cstdint>
#include <random>
#include <string>
#include <vector>
#include <unordered_map>
#include <iosfwd>

namespace polyec {
inline constexpr const char* realm_schema="realm-v6";
inline constexpr int realm_actions=32, realm_state=128, realm_features=12;
inline constexpr int realm_obs=realm_state+realm_actions*realm_features;
enum class Family:int { Governance=0,Economic=1,Civilian=2 };
enum class Intent:int { Hold,Buy,Sell,Travel,Borrow,Repay,Project,Hire,Tax,Aid,Investigate,Offer,Accept,Reject,Work,Study,Rest,Social,Join,Migrate,Support,Appoint,Mobilize,Resign };
struct Stats { std::array<int,6> values{50,50,50,50,50,50}; };
struct Person {
    uint64_t id=0,household=0,decisions=0,sampling=0;
    int region=0,age_days=0,health=100,skill=0,job=-1,busy_until=0,destination=-1;
    int goal=0,school_days=0,social=0; bool alive=true;
    int tax_remainder=0;
    int organization=0;
    int job_kind=0;
    bool taxing_authority=false,sovereign=false;
    int nutrition=100;
    bool candidate=false,office=false,special_advisor=false;
    uint64_t hired_by=0;int salary_due=30,office_notice=0,last_decision_day=-1;
    Family family=Family::Civilian; Stats stats;
    int64_t cash=100; std::array<int64_t,7> inventory{};
};
struct Province { int64_t treasury=10000,firm=30000;std::array<int64_t,7> stock{};std::array<int,7> price{};int tax=10,school=8,school_used=0,jobs=32,employed=0,road=0,unrest=0,teachers=0;bool charter=true;int residents=0;int64_t health_sum=0;int sales_tax_remainder=0;int64_t tax_receipts=0,food_capacity=0;int teacher_skill_sum=0; };
struct Edge { uint64_t from=0,to=0;int kind=0; };
// creditor is the provincial firm-account index, not a Person ID.
struct Loan { uint64_t id=0,debtor=0;int creditor=0,due=0;int64_t principal=0,interest=0,paid=0;bool defaulted=false,royal_pool=false;uint64_t cause=0; };
struct Project {uint64_t id=0,owner=0;int region=0,due=0,kind=0;int64_t budget=0;bool completed=false,engineer=false,stalled=false;uint64_t cause=0;};
struct Fact {uint64_t id=0,parent=0,actor=0,target=0;int day=0,region=0,kind=0;int64_t amount=0;bool secret=false;};
struct Knowledge {uint64_t observer=0,fact=0;int arrives=0;};
struct Proposal {uint64_t id=0,from=0,to=0;int64_t amount=0;int due=0;bool resolved=false;int kind=0,ready=0;int64_t escrow=0;bool public_source=false,mediation=false;int source_region=0;uint64_t cause=0;std::array<uint64_t,3> parties{};std::array<int,3> regions{};std::array<bool,3> pledged{};};
struct Investigation {uint64_t id=0,owner=0,cause=0;int region=0,due=0;bool completed=false;};
struct Candidate {Intent kind=Intent::Hold;int target=0,good=0;int64_t amount=0;int term=0;};
struct RealmCommand {uint64_t actor=0,sequence=0;Candidate action;};
struct RealmResult {bool success=false;std::string reason;uint64_t fact=0;};
struct Draft {int tier=0;std::array<int,3> cards{};int seen=0,chosen=-1;bool rerolled=false;};
struct RealmView {int day=0,regions=0;uint64_t persons=0,alive=0,children=0,decisions=0;int64_t money=0;std::vector<Province> provinces;std::vector<Fact> facts;Draft draft;};
// Owns all mutable campaign state. Snapshots and observations are copies.
class Realm {
    friend class Campaign;
public:
    Realm(uint64_t seed=1,int regions=6,int people=10000,bool advisor_fixture=false);
    // Explicit initial-condition fixtures for paired eval. Never mutates a live world.
    static Realm scenario(uint64_t seed,uint64_t actor,int age=-1,int stat=-1,int goal=-1,int budget=-1,int schools=-1,int jobs=-1,int nutrition=-1,int health=-1);
    const Person& person(uint64_t id) const;
    std::vector<uint64_t> due(Family family) const;
    std::array<Candidate,realm_actions> candidates(uint64_t actor) const;
    std::array<float,realm_actions> mask(uint64_t actor) const;
    std::array<float,realm_obs> observation(uint64_t actor) const;
    RealmResult execute(const RealmCommand& command);
    void advance(int days=1);
    RealmView view(uint64_t observer=1) const;
    // A containing domain may own bonded goods (e.g. M4 manifests in transit).
    bool invariant(const std::array<int64_t,7>& bonded_goods={}) const;
    uint64_t fingerprint() const;
    int64_t total_money() const;
    std::array<int64_t,9> balances()const;
    int64_t royal_reference()const{return royal_reference_;}
    std::array<int64_t,2> royal_funds()const{return {credit_pool_,restricted_fund_};}
    std::vector<Proposal> offers(uint64_t observer)const;
    std::vector<Investigation> investigations()const{return investigations_;}
    double utility(uint64_t actor) const;
    bool reroll(int slot);
    RealmResult route_command(uint64_t actor,int from,int to,bool open);
    int distance(int from,int to)const;
    Draft draft()const{return draft_;}
    bool choose(int card);
    bool save(const std::string& path,const std::string& bundle,std::string& error) const;
    bool load(const std::string& path,const std::string& bundle,std::string& error);
    std::string serialize(const std::string& bundle) const;
    bool deserialize(const std::string& data,const std::string& bundle,std::string& error,const std::array<int64_t,7>& bonded_goods={});
    int day() const {return day_;}
    int regions() const {return regions_;}
    int count() const {return int(people_.size());}
    int64_t debt(uint64_t actor) const;
    int64_t available_funds(uint64_t actor)const;
    // Sparse household membership. A household label is the ID of its founding member,
    // but membership is a dynamic record: a birth joins the parent household with a
    // fresh non-consecutive ID, so never scan household..household+3.
    uint64_t household_of(uint64_t person) const;
    std::vector<uint64_t> household_members(uint64_t household) const;
    bool same_household(uint64_t a,uint64_t b) const;
    uint64_t guardian(uint64_t person) const;
    // Representative authority of a household: the living adult founding member,
    // otherwise the oldest living adult member; 0 when the household has none.
    uint64_t household_authority(uint64_t household) const;
    size_t household_records() const;
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
    // Derived household membership index: label -> ascending member IDs. It adds no
    // money and no second world; it is rebuilt from Person::household after any load.
    std::unordered_map<uint64_t,std::vector<uint64_t>> households_;
    std::array<std::array<bool,7>,7> routes_{};
    std::array<int64_t,28> organization_funds_{};
    int64_t royal_reference_=0,credit_pool_=0,restricted_fund_=0;
    int mediation_ready_=0,bonus_offices_=0;bool pact_=false,reference_scenario_=false;
    std::vector<uint64_t> referrals_;std::vector<Investigation> investigations_;
    std::array<int64_t,7> opening_{},produced_{},consumed_{};
    int64_t opening_money_=0,carrier_=0,estate_=0,endowment_=0;
    Person& mutable_person(uint64_t id);
    uint64_t guardian_of(const Person& person)const;
    void index_household(uint64_t household,uint64_t person);
    void reindex_households();
    bool households_consistent()const;
    void spend(Person& person,int64_t& destination,int64_t amount);
    void payroll(int region,bool public_employer,const std::vector<std::pair<uint64_t,int64_t>>& claims);
    void refresh_demography();
    void sales_tax(Province& province,int64_t gross);
    int64_t harvest_quantity(int region,int good,int residents,int weather=-1,int productivity=100)const;
    void initialize_court_reference();
    void apply_augment(int card);
    int report_delay(bool inventory=false)const;
    int office_capacity(uint64_t actor)const;
    int office_used(uint64_t actor)const;
    bool project_materials(int region,int kind)const;
    void start_project(uint64_t owner,int region,int kind,int64_t budget,int term,bool from_escrow=false,uint64_t cause=0);
    void finish_projects();
    bool legal_proposal_response(uint64_t actor,const Candidate& action)const;
    void create_proposal(uint64_t actor,const Candidate& action);
    void resolve_proposal(uint64_t actor,const Candidate& action);
    void refund_proposal(Proposal& proposal);
    void write_court(std::ostream& stream)const;
    bool read_court(std::istream& stream);
    bool legal(uint64_t actor,const Candidate& c,std::string& reason) const;
    void daily();void prices();void event(uint64_t actor,int region,int kind,int64_t amount=0,uint64_t target=0,bool secret=false,uint64_t parent=0);
    void transfer(int64_t& from,int64_t& to,int64_t amount);
    bool knows(uint64_t observer,const Fact& f) const;
};
}
