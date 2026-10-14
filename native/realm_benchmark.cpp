#include "polyec/realm_runtime.hpp"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
int main(int argc,char**argv){if(argc<6||argc>8)return 2;int days=argc>=7?std::stoi(argv[6]):365;uint64_t seed=argc>=8?std::stoull(argv[7]):37;if(days<1||days>36500)return 2;polyec::RealmRuntime runtime;
    if(!runtime.load_models({argv[1],argv[2],argv[3]},argv[4],seed,10000)){std::cerr<<runtime.error();return 1;}
    runtime.choose(runtime.world()->draft().cards[0]);auto start=runtime.world()->view();std::vector<double> times;
    for(int day=0;day<days;++day){if(day==30)runtime.route(1,0,1,false);auto t=std::chrono::steady_clock::now();if(!runtime.advance(1)){std::cerr<<runtime.error();return 1;}times.push_back(std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t).count());}
    auto v=runtime.world()->view();std::sort(times.begin(),times.end());size_t households=0;for(int i=129;i<=runtime.world()->count();++i)if(runtime.world()->person(i).household==uint64_t(i))++households;
    std::ofstream out(argv[5]);out<<"{\"profile\":\"A native learned co-play\",\"days\":"<<days<<",\"population_opening\":"<<start.persons<<",\"population_end\":"<<v.persons<<",\"alive_end\":"<<v.alive<<",\"children_end\":"<<v.children<<",\"complex_actors_opening\":128,\"households\":"<<households<<",\"graph_edges\":"<<runtime.world()->relations().size()<<",\"accounts\":"<<v.persons+44<<",\"facilities\":7,\"decisions\":"<<runtime.inference_count()<<",\"total_inference_ms\":"<<runtime.inference_ms()<<",\"full_day_p50_ms\":"<<times[times.size()/2]<<",\"full_day_p95_ms\":"<<times[size_t(times.size()*.95)]<<",\"full_day_max_ms\":"<<times.back()<<",\"money_conserved\":"<<(start.money==v.money?"true":"false")<<",\"invariant\":"<<(runtime.world()->invariant()?"true":"false")<<",\"route_shock_day\":30,\"render_included\":false,\"save_included\":false,\"account_balances\":[";
    auto balances=runtime.world()->balances();for(size_t i=0;i<balances.size();++i){if(i)out<<',';out<<balances[i];}out<<"],\"food_stock_end\":[";for(size_t i=0;i<v.provinces.size();++i){if(i)out<<',';out<<v.provinces[i].stock[0];}out<<"],\"family_outcomes\":[";
    for(int role=0;role<3;++role){int alive=0,count=0,working=0;int64_t health=0,nutrition=0,cash=0;for(int i=1;i<=runtime.world()->count();++i){auto&p=runtime.world()->person(i);if(int(p.family)!=role)continue;++count;if(p.alive){++alive;health+=p.health;nutrition+=p.nutrition;working+=p.job>=0;}cash+=p.cash;}if(role)out<<',';out<<"{\"role\":"<<role<<",\"persons\":"<<count<<",\"alive\":"<<alive<<",\"working\":"<<working<<",\"mean_living_health\":"<<double(health)/std::max(1,alive)<<",\"mean_living_nutrition\":"<<double(nutrition)/std::max(1,alive)<<",\"cash\":"<<cash<<'}';}out<<"]}";
    std::cout<<"PROFILE A:"<<days<<" days,"<<runtime.inference_count()<<" learned decisions,p95 "<<times[size_t(times.size()*.95)]<<"ms\n";
}
