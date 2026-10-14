#include "polyec/trade.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <chrono>

namespace polyec {
TradeWorld::TradeWorld(uint64_t seed,int regions,int horizon) { reset(seed,regions,horizon); }
void TradeWorld::reset(uint64_t seed,int regions,int horizon) {
    if(regions<1 || regions>6 || horizon<1 || horizon>2048) throw std::invalid_argument("trade configuration");
    rng_.seed(seed); horizon_=horizon; state_=TradeView{};
    state_.markets.resize(regions); initial_stock_.fill(0);
    for(int r=0;r<regions;++r) for(int g=0;g<kGoods;++g) {
        auto& m=state_.markets[r];
        m.stock[g]=80+int(rng_()%101);
        m.geography[g]=0.65f+float(rng_()%81)/100.f;
        m.price[g]=std::max(2,int(std::lround(kReference[g]*m.geography[g])));
        initial_stock_[g]+=m.stock[g];
    }
    state_.region=int(rng_()%regions); state_.destination=state_.region;
    initial_money_=total_money();
}
int64_t TradeWorld::total_money() const {
    int64_t result=state_.cash+state_.carrier_cash;
    for(const auto& m:state_.markets) result+=m.firm_cash+m.household_cash;
    return result;
}
int64_t TradeWorld::wealth() const {
    int64_t value=state_.cash;
    for(int g=0;g<kGoods;++g) value+=state_.cargo[g]*kReference[g];
    return value;
}
int64_t TradeWorld::liquidation_value() const {
    int64_t value=state_.cash, budget=state_.markets[state_.region].firm_cash;
    for(int g=0;g<kGoods;++g) {
        int price=std::max(1,state_.markets[state_.region].price[g]-1);
        int64_t qty=std::min(state_.cargo[g],budget/price);
        budget-=qty*price; value+=qty*price;
    }
    return value;
}
int TradeWorld::travel_distance(int destination) const {
    int difference=std::abs(destination-state_.region), n=int(state_.markets.size());
    return std::min(difference,n-difference);
}
int TradeWorld::travel_fee(int destination) const { return 4+2*travel_distance(destination); }
int TradeWorld::lot(int action) const {
    const auto& m=state_.markets[state_.region];
    if(action>=1 && action<=7) {
        int g=action-1;
        int64_t free=16-std::accumulate(state_.cargo.begin(),state_.cargo.end(),int64_t(0));
        return int(std::max<int64_t>(0,std::min({int64_t(4),free,m.stock[g],state_.cash/(m.price[g]+1)})));
    }
    if(action>=8 && action<=14) {
        int g=action-8;
        return int(std::min({int64_t(4),state_.cargo[g],m.firm_cash/std::max(1,m.price[g]-1)}));
    }
    return 0;
}
std::array<float,kActions> TradeWorld::mask() const {
    std::array<float,kActions> result{};
    result[0]=1;
    if(state_.done || state_.transit_days>0) return result;
    for(int a=1;a<15;++a) result[a]=lot(a)>0 ? 1.f:0.f;
    for(int r=0;r<int(state_.markets.size());++r)
        result[15+r]=(r!=state_.region && state_.cash>=travel_fee(r)) ? 1.f:0.f;
    return result;
}
bool TradeWorld::commit(int action) {
    if(state_.done || action<0 || action>=kActions || mask()[action]<0.5f) return false;
    auto& m=state_.markets[state_.region];
    if(action==0) { ++state_.holds; return true; }
    if(action<8) {
        int g=action-1, qty=lot(action), payment=qty*(m.price[g]+1);
        state_.cash-=payment; m.firm_cash+=payment;
        m.stock[g]-=qty; state_.cargo[g]+=qty; ++state_.trades;
    } else if(action<15) {
        int g=action-8, qty=lot(action), payment=qty*std::max(1,m.price[g]-1);
        state_.cash+=payment; m.firm_cash-=payment;
        m.stock[g]+=qty; state_.cargo[g]-=qty; ++state_.trades;
    } else {
        int destination=action-15, fee=travel_fee(destination);
        state_.cash-=fee; state_.carrier_cash+=fee;
        state_.destination=destination;
        state_.transit_days=travel_distance(destination)+1;
        ++state_.travels;
    }
    return true;
}
void TradeWorld::consume(Market& m,int g,int64_t quantity) {
    m.stock[g]-=quantity; state_.consumed[g]+=quantity;
}
void TradeWorld::produce(Market& m,int g,int64_t quantity) {
    m.stock[g]+=quantity; state_.produced[g]+=quantity;
}
void TradeWorld::advance_day() {
    ++state_.day;
    if(state_.transit_days>0 && --state_.transit_days==0) state_.region=state_.destination;
    for(int r=0;r<int(state_.markets.size());++r) {
        auto& m=state_.markets[r];
        // Fixed labor contracts and consumption baskets are laws of this
        // curriculum, not learned civilian/firm decision making.
        if(state_.day%7==0) {
            int64_t wage=std::min<int64_t>(m.firm_cash,180);
            m.firm_cash-=wage; m.household_cash+=wage;
        }
        if(state_.day%90==0) {
            int harvest=140+int(rng_()%121); // exogenous realized yield, never in observation early
            produce(m,0,harvest); produce(m,1,harvest/2);
            if(m.stock[0]>=4) { consume(m,0,4); produce(m,6,1); }
        }
        if(state_.day%7==0) {
            produce(m,4,8); // declared primary fuel source
            if(m.stock[1]>=2 && m.stock[4]>=1) { consume(m,1,2); consume(m,4,1); produce(m,2,1); }
            if(m.stock[4]>=2) { consume(m,4,2); produce(m,3,1); }
            if(m.stock[3]>=1 && m.stock[4]>=1) { consume(m,3,1); consume(m,4,1); produce(m,5,1); }
        }
        int g=state_.day%7;
        int64_t qty=std::min({int64_t(g==0?2:1),m.stock[g],m.household_cash/m.price[g]});
        int64_t payment=qty*m.price[g];
        m.household_cash-=payment; m.firm_cash+=payment; consume(m,g,qty);
        for(int good=0;good<kGoods;++good) {
            double scarcity=std::clamp((120.0-double(m.stock[good]))/120.0,-1.0,1.0);
            double target=kReference[good]*std::clamp(double(m.geography[good])+0.25*scarcity,0.45,1.8);
            m.price[good]=std::max(2,int(std::lround(0.9*m.price[good]+0.1*target)));
        }
    }
}
void TradeWorld::liquidate() {
    auto& m=state_.markets[state_.region];
    for(int g=0;g<kGoods;++g) {
        int price=std::max(1,m.price[g]-1);
        int64_t qty=std::min(state_.cargo[g],m.firm_cash/price);
        m.firm_cash-=qty*price; state_.cash+=qty*price;
        m.stock[g]+=qty; state_.cargo[g]-=qty;
    }
}
TradeStep TradeWorld::step(int action) {
    int64_t before=wealth();
    if(!commit(action)) throw std::invalid_argument("invalid trade action: rejected without mutation");
    for(int day=0;day<7;++day) advance_day();
    ++state_.decision;
    state_.done=state_.decision>=horizon_;
    if(state_.done) liquidate();
    if(!invariant()) throw std::runtime_error("trade accounting invariant violated");
    // At gamma=1 this telescopes to terminal funded net wealth change.
    int64_t after=state_.done?state_.cash:wealth();
    return {float(after-before)/100.f,state_.done,action};
}
std::array<float,kObservation> TradeWorld::observation() const {
    std::array<float,kObservation> result{};
    result[0]=float(state_.cash)/500.f;
    result[1]=float(horizon_-state_.decision)/horizon_;
    result[2+state_.region]=1;
    for(int g=0;g<7;++g) result[8+g]=float(state_.cargo[g])/16.f;
    int index=15;
    for(int r=0;r<6;++r) for(int g=0;g<7;++g)
        result[index++]=r<int(state_.markets.size()) ? float(state_.markets[r].price[g])/kReference[g]:0.f;
    for(int r=0;r<6;++r) for(int g=0;g<7;++g)
        result[index++]=r<int(state_.markets.size()) ? std::min(5.f,float(state_.markets[r].stock[g])/120.f):0.f;
    result[99]=float(state_.transit_days)/7.f;
    const auto& home=state_.markets[state_.region];
    int64_t cargo=std::accumulate(state_.cargo.begin(),state_.cargo.end(),int64_t(0));
    result[kState+3]=1; // hold one-hot
    for(int a=1;a<15;++a) {
        int g=a<8?a-1:a-8, offset=kState+a*kFeatures;
        result[offset+(a<8?0:1)]=1;
        result[offset+4]=float(home.price[g]+(a<8?1:-1))/kReference[g];
        result[offset+5]=float(lot(a))/4.f;
        result[offset+6]=float(state_.cash)/500.f;
        result[offset+7]=float(state_.cargo[g])/16.f;
        result[offset+8]=float(home.stock[g])/120.f;
    }
    for(int r=0;r<int(state_.markets.size());++r) {
        int offset=kState+(15+r)*kFeatures;
        result[offset+2]=1;
        result[offset+5]=float(travel_distance(r))/3.f;
        result[offset+6]=float(state_.cash)/500.f;
        result[offset+7]=float(cargo)/16.f;
        int64_t difference=0;
        for(int g=0;g<7;++g) difference+=state_.cargo[g]*(state_.markets[r].price[g]-home.price[g]);
        // Public quote feature, not a selected/optimized strategy or secret.
        result[offset+8]=float(difference)/100.f;
        result[offset+9]=float(travel_fee(r))/100.f;
    }
    return result;
}
bool TradeWorld::invariant() const {
    if(total_money()!=initial_money_ || state_.cash<0 || state_.carrier_cash<0) return false;
    for(const auto& m:state_.markets) if(m.firm_cash<0 || m.household_cash<0) return false;
    for(int g=0;g<7;++g) {
        int64_t total=state_.cargo[g];
        if(total<0) return false;
        for(const auto& m:state_.markets) { if(m.stock[g]<0 || m.price[g]<1) return false; total+=m.stock[g]; }
        if(total!=initial_stock_[g]+state_.produced[g]-state_.consumed[g]) return false;
    }
    return true;
}
uint64_t TradeWorld::fingerprint() const {
    uint64_t hash=1469598103934665603ULL;
    auto add=[&](int64_t v){ hash^=uint64_t(v); hash*=1099511628211ULL; };
    add(state_.day); add(state_.decision); add(state_.region); add(state_.cash); add(state_.carrier_cash);
    add(state_.transit_days); add(state_.destination); add(state_.trades); add(state_.travels); add(state_.holds);
    for(auto c:state_.cargo) add(c);
    for(int g=0;g<7;++g) {add(state_.produced[g]);add(state_.consumed[g]);}
    for(const auto& m:state_.markets) {add(m.firm_cash);add(m.household_cash); for(int g=0;g<7;++g){add(m.stock[g]);add(m.price[g]);}}
    return hash;
}
std::string TradeWorld::action_name(int a) {
    static const char* goods[]={"food","fiber","cloth","iron","fuel","tools","horses"};
    if(a==0) return "hold";
    if(a>=1 && a<8) return std::string("buy_")+goods[a-1];
    if(a>=8 && a<15) return std::string("sell_")+goods[a-8];
    if(a>=15 && a<21) return "travel_region_"+std::to_string(a-15);
    return "invalid";
}
ScaleResult citizen_primitive_spike(uint64_t count,int iterations) {
    if(count<1 || count>1000000 || iterations<1 || iterations>100) throw std::invalid_argument("spike bounds");
    struct Citizen { uint64_t id; uint32_t age_days,household,job; uint16_t health,skill; uint64_t counter; };
    auto start=std::chrono::steady_clock::now();
    std::vector<Citizen> citizens(count);
    std::vector<uint32_t> neighbors(count*4);
    for(uint64_t i=0;i<count;++i) {
        citizens[i]={i+1,uint32_t((i*17)%28000),uint32_t(i/4),uint32_t(i%97),1000,0,0};
        for(int e=0;e<4;++e) neighbors[i*4+e]=uint32_t((i+1+e*7919)%count);
    }
    uint64_t checksum=0;
    for(int t=0;t<iterations;++t) for(uint64_t i=0;i<count;++i) {
        auto& c=citizens[i]; c.age_days+=1; ++c.counter;
        if(c.age_days>18*360) c.skill=uint16_t(std::min(1000,int(c.skill)+1));
        checksum+=c.id+c.counter+neighbors[i*4+(t%4)]+c.skill;
    }
    return {count,count*4,count*sizeof(Citizen)+neighbors.size()*sizeof(uint32_t),checksum,
            std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()};
}
}
