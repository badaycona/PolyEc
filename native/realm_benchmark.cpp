#include "polyec/realm_runtime.hpp"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
int main(int argc,char**argv){if(argc!=6)return 2;polyec::RealmRuntime runtime;
    if(!runtime.load_models({argv[1],argv[2],argv[3]},argv[4],37,10000)){std::cerr<<runtime.error();return 1;}
    runtime.choose(runtime.world()->draft().cards[0]);auto start=runtime.world()->view();std::vector<double> times;
    for(int day=0;day<365;++day){if(day==30)runtime.route(1,0,1,false);auto t=std::chrono::steady_clock::now();if(!runtime.advance(1)){std::cerr<<runtime.error();return 1;}times.push_back(std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t).count());}
    auto v=runtime.world()->view();std::sort(times.begin(),times.end());size_t households=0;for(int i=129;i<=runtime.world()->count();++i)if(runtime.world()->person(i).household==uint64_t(i))++households;
    std::ofstream out(argv[5]);out<<"{\"profile\":\"A native learned co-play\",\"days\":365,\"population_opening\":"<<start.persons<<",\"population_end\":"<<v.persons<<",\"alive_end\":"<<v.alive<<",\"children_end\":"<<v.children<<",\"complex_actors_opening\":128,\"households\":"<<households<<",\"graph_edges\":"<<runtime.world()->relations().size()<<",\"accounts\":"<<v.persons+16<<",\"facilities\":7,\"decisions\":"<<runtime.inference_count()<<",\"total_inference_ms\":"<<runtime.inference_ms()<<",\"full_day_p50_ms\":"<<times[182]<<",\"full_day_p95_ms\":"<<times[346]<<",\"full_day_max_ms\":"<<times.back()<<",\"money_conserved\":"<<(start.money==v.money?"true":"false")<<",\"invariant\":"<<(runtime.world()->invariant()?"true":"false")<<",\"route_shock_day\":30,\"render_included\":false,\"save_included\":false}";
    std::cout<<"PROFILE A:365 days,"<<runtime.inference_count()<<" learned decisions,p95 "<<times[346]<<"ms\n";
}
