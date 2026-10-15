#pragma once
#include "polyec/realm.hpp"

namespace polyec {
// M4 adds a versioned scenario/controller around the shared Realm, not a second
// economy. Every transfer, production and consumption stays in this C++ core.
struct MapPoint { double x=0,y=0; };
struct MapRegion { std::string name,terrain; MapPoint center; std::vector<MapPoint> boundary; int harvest=100; };
struct MapRoute { int from=0,to=0,days=1,capacity=200; std::string terrain; bool open=true,claimed=false; int works_due=0; };
struct Cargo { uint64_t id=0,cause=0; int from=0,to=0,good=0,depart=0,arrives=0; int64_t quantity=0,cost=0; bool delivered=false; std::vector<int> path,leg_starts,leg_ends,leg_arrivals; };
struct Case { uint64_t id=0,cause=0; int region=0,opened=0,reported=0,closed=-1; bool evidence=false; std::string title; };
enum class CampaignAction { Relief, Road, Negotiate, Arbitrate, Survey, Refinance, Weave };
struct CampaignCommand { uint64_t actor=1,sequence=0; CampaignAction action=CampaignAction::Relief; int target=1; int64_t quantity=200; };
struct CampaignResult { bool success=false; std::string reason; };
struct RoutePreview { bool reachable=false; int days=0; int64_t fee=0; std::vector<int> regions,leg_starts,leg_ends,leg_arrivals; };
struct CampaignView {
    RealmView realm; uint64_t revision=0; std::vector<MapRegion> regions;
    std::vector<MapRoute> routes; std::vector<Cargo> cargo; std::vector<Case> cases;
    std::vector<Loan> loans; std::vector<Fact> journal;
    int64_t crown=0,workshop_cash=0,cloth=0; int workshop_workers=0,inspecting_until=0;
    uint64_t player_sequence=1; bool invariant=false;
};
class Campaign {
public:
    explicit Campaign(uint64_t seed=37,int population=10000);
    static const std::vector<MapRegion>& geography();
    CampaignView view()const;
    CampaignResult execute(const CampaignCommand&);
    void advance(int days=1);
    RoutePreview preview(int from,int to,int64_t quantity=200)const;
    bool invariant()const;
    uint64_t fingerprint()const;
    uint64_t revision()const{return revision_;}
    bool save(const std::string&,std::string&)const;
    bool load(const std::string&,std::string&);
    std::string serialize()const;
    bool deserialize(const std::string&,std::string&);
private:
    Realm world_;
    std::vector<MapRoute> routes_;
    std::vector<Cargo> cargo_;
    std::vector<Case> cases_;
    std::vector<Fact> journal_;
    uint64_t seed_=37,revision_=0,sequence_=1,next_record_=1;
    int inspecting_until_=0;
    bool surveyed_=false;
    int64_t workshop_output_=0;
    std::vector<uint64_t> workers_;
    void update_cases();
    void controller();
    void record(int kind,int region,int64_t amount=0,uint64_t parent=0);
    uint64_t latest(int region,int kind)const;
    std::string state()const;
};
}
