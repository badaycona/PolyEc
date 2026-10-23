#pragma once

#include "polyec/campaign.hpp" // MapRoute, Cargo, Case; realm.hpp supplies Fact

#include <array>
#include <cstdint>
#include <iomanip>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace polyec {
namespace detail {

// Campaign-owned save payload: the bytes between the "POLYEC_M4_1 " magic token
// and the trailing std::quoted Realm save string of Campaign::state(). This
// codec is stateless and grammar-only: Campaign keeps the magic wrapper, the
// quoted Realm emission/consumption, the whole-state invariant check and the
// atomic move-commit; Realm keeps owning and serializing the world. The codec
// must not see into the Realm.

struct CampaignStatePayload {
    uint64_t seed=0,revision=0,sequence=1,next_record=1;
    int inspecting_until=0;
    bool surveyed=false;
    int64_t workshop_output=0;
    std::vector<MapRoute> routes;
    std::vector<Cargo> cargo;
    std::vector<Case> cases;
    std::vector<Fact> journal;
    std::vector<uint64_t> workers;
    std::array<int64_t,kGoodIdCount> bonded_goods{}; // reader-derived output only
};

// Pure emitter: no reordering, no normalization, no validation, no range
// enforcement; counts are the container sizes as-is. Group order and the one
// trailing space before the wrapper's quoted Realm string match
// Campaign::state() byte-for-byte.
inline void write_campaign_state(std::ostream& out,const CampaignStatePayload& payload){
    out<<payload.seed<<' '<<payload.revision<<' '<<payload.sequence<<' '<<payload.next_record<<' '<<payload.inspecting_until<<' '<<payload.surveyed<<' '<<payload.workshop_output<<' ';
    out<<payload.routes.size()<<' ';for(auto& r:payload.routes)out<<r.from<<' '<<r.to<<' '<<r.days<<' '<<r.capacity<<' '<<std::quoted(r.terrain)<<' '<<r.open<<' '<<r.claimed<<' '<<r.works_due<<' ';
    out<<payload.cargo.size()<<' ';for(auto& c:payload.cargo){out<<c.id<<' '<<c.cause<<' '<<c.from<<' '<<c.to<<' '<<c.good<<' '<<c.depart<<' '<<c.arrives<<' '<<c.quantity<<' '<<c.cost<<' '<<c.delivered<<' '<<c.path.size()<<' ';for(int r:c.path)out<<r<<' ';for(auto* p:{&c.leg_starts,&c.leg_ends,&c.leg_arrivals})for(int t:*p)out<<t<<' ';}
    out<<payload.cases.size()<<' ';for(auto& k:payload.cases)out<<k.id<<' '<<k.cause<<' '<<k.region<<' '<<k.opened<<' '<<k.reported<<' '<<k.closed<<' '<<k.evidence<<' '<<std::quoted(k.title)<<' ';
    out<<payload.journal.size()<<' ';for(auto& f:payload.journal)out<<f.id<<' '<<f.parent<<' '<<f.day<<' '<<f.region<<' '<<f.kind<<' '<<f.amount<<' ';
    out<<payload.workers.size()<<' ';for(auto id:payload.workers)out<<id<<' ';
}

// Streaming reader using the exact same expressions in the same order as the
// previous inline Campaign::deserialize() payload block so stream-state
// behavior (failbit stickiness, whitespace skipping, eof) stays bit-compatible.
// Throws std::runtime_error("M4 save record bound"|"M4 path missing"|"M4 manifest
// invalid") with the same first-failure ordering; a malformed or truncated token
// inside a group surfaces at the next count token as "M4 save record bound". The
// reader stages into a local payload and publishes to out only on full payload
// success. It must not add a success-side !in check of its own (that behavior
// belongs to the wrapper's quoted Realm read), must not consume the Realm quote,
// must not call Realm::deserialize or anything else on the streamed suffix, and
// must not touch any live Campaign. Bonded totals accumulate per cargo record,
// after that record's manifest check and only if(!c.delivered), in cargo write
// order, with plain int64_t += semantics as before.
inline void read_campaign_state(std::istream& in,CampaignStatePayload& out){
    CampaignStatePayload p;
    in>>p.seed>>p.revision>>p.sequence>>p.next_record>>p.inspecting_until>>p.surveyed>>p.workshop_output;
    auto bounds=[&](size_t maximum){size_t n=0;in>>n;if(!in||n>maximum)throw std::runtime_error("M4 save record bound");return n;};
    p.routes.resize(bounds(9));for(auto& r:p.routes)in>>r.from>>r.to>>r.days>>r.capacity>>std::quoted(r.terrain)>>r.open>>r.claimed>>r.works_due;
    p.cargo.resize(bounds(100000));for(auto& c:p.cargo){in>>c.id>>c.cause>>c.from>>c.to>>c.good>>c.depart>>c.arrives>>c.quantity>>c.cost>>c.delivered;c.path.resize(bounds(7));for(int& r:c.path)in>>r;if(c.path.size()<2)throw std::runtime_error("M4 path missing");for(auto* q:{&c.leg_starts,&c.leg_ends,&c.leg_arrivals}){q->resize(c.path.size()-1);for(int& t:*q)in>>t;}if(c.good<0||c.good>=kLegacyGoodCount||c.quantity<1||c.quantity>5000)throw std::runtime_error("M4 manifest invalid");if(!c.delivered){const int good=canonical_from_legacy_good(c.good);if(good<0)throw std::runtime_error("M4 manifest good");p.bonded_goods[size_t(good)]+=c.quantity;}}
    p.cases.resize(bounds(100000));for(auto& k:p.cases)in>>k.id>>k.cause>>k.region>>k.opened>>k.reported>>k.closed>>k.evidence>>std::quoted(k.title);
    p.journal.resize(bounds(1000000));for(auto& f:p.journal)in>>f.id>>f.parent>>f.day>>f.region>>f.kind>>f.amount;
    p.workers.resize(bounds(6));for(auto& id:p.workers)in>>id;
    out=std::move(p);
}

} // namespace detail
} // namespace polyec
