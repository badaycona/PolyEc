#include "polyec/work_credit.hpp"
#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace polyec;
namespace {
int checks=0;
void check(bool ok,const char* label){++checks;if(!ok)throw std::runtime_error(label);}
template<class Function>void rejects(Function function){
    bool rejected=false;try{function();}catch(const std::logic_error&){rejected=true;}
    catch(const std::overflow_error&){rejected=true;}
    check(rejected,"invalid/overflow credit input must reject");
}
void exhaustive(){
    int scenarios=0;
    for(int t=1;t<=5;++t)for(int u=0;u<2*t;++u)for(int a=0;a<2*t;++a)for(int b=0;b<2*t;++b){
        const int64_t e0=(u+a+b)/t;
        const std::array<WorkCreditSource,2> old{{{11,a/t,0},{29,b/t,0}}};
        const auto initial=initialize_work_credit(e0,u/t,old);
        for(int du:{0,1,t})for(int da:{0,1,t})for(int db:{0,1,t}){
            const int64_t eu=(u+du)/t,ea=(a+da)/t,eb=(b+db)/t;
            const std::array<WorkCreditSource,2> sites{{{11,ea,initial.sites[0].credited_units},{29,eb,initial.sites[1].credited_units}}};
            const auto result=allocate_work_credit(initial.region,eu,sites,scenarios%13);
            const auto& credit=result.region;
            const int64_t total=result.regional_entitlement+ea-result.sites[0].credited_units+eb-result.sites[1].credited_units;
            check(total==std::max(e0,eu+ea+eb)&&total<=(u+du+a+da+b+db)/t,"entitlement matches independent floor oracle");
            check(credit.legacy_units==credit.unassigned_units+credit.bridge_units+
                result.sites[0].credited_units+result.sites[1].credited_units,"legacy credit conserved");
            check(result.regional_entitlement>=e0&&credit.unassigned_units<=eu&&result.sites[0].credited_units<=ea&&
                result.sites[1].credited_units<=eb,"legacy debt preserved and credit bounded");
            if(credit.bridge_units)check(result.regional_entitlement==e0&&result.sites[0].credited_units==ea&&
                result.sites[1].credited_units==eb,"bridge consumed before any new payable unit");
            const auto repeated=allocate_work_credit(credit,eu,result.sites,scenarios%13);
            check(repeated.region==credit&&repeated.sites==result.sites&&repeated.regional_entitlement==result.regional_entitlement,
                "credit retry without new attendance is idempotent");
            ++scenarios;
        }
    }
    check(scenarios==48600,"exhaustive scenario coverage matches independent Python oracle");
}
}
int main(){try{
    exhaustive();
    const std::array<WorkCreditSource,2> empty{{{11,0,0},{29,0,0}}};
    const auto baseline=initialize_work_credit(1,0,empty);
    const std::array<WorkCreditSource,2> future{{{11,1,0},{29,1,0}}};
    const auto day0=allocate_work_credit(baseline.region,0,future,0);
    const auto day1=allocate_work_credit(baseline.region,0,future,1);
    check(day0.sites[0].credited_units==1&&day0.sites[1].credited_units==0&&
        day1.sites[0].credited_units==0&&day1.sites[1].credited_units==1,"rounding rotates over positive canonical claims");
    const std::array<WorkCreditSource,2> weighted{{{11,10,0},{29,20,0}}};
    const auto weighted0=allocate_work_credit({2,0,2},0,weighted,0);
    const auto weighted1=allocate_work_credit({2,0,2},0,weighted,1);
    check(weighted0.sites[0].credited_units==1&&weighted0.sites[1].credited_units==1&&
        weighted1.sites[0].credited_units==0&&weighted1.sites[1].credited_units==2,"pro-rata shares precede rotating rounding");
    const std::array<WorkCreditSource,1> one{{{11,1,0}}};
    const auto partial=allocate_work_credit(initialize_work_credit(0,0,empty).region,0,one,0);
    check(partial.regional_entitlement==0&&partial.sites[0].credited_units==0,"old partial ticks finish a new site unit");
    const auto split=allocate_work_credit(baseline.region,0,future,0);
    const int64_t oldPaid=1,wage=3;
    check(split.regional_entitlement*wage-oldPaid==2&&
        ((1-split.sites[0].credited_units)+(1-split.sites[1].credited_units))*wage==3,"partial historical money debt and exactly one new unit survive");
    const auto staged1=allocate_work_credit(baseline.region,0,one,0);
    const auto staged2=allocate_work_credit(staged1.region,1,staged1.sites,1);
    check(staged1.regional_entitlement==1&&staged1.sites[0].credited_units==1&&staged2.regional_entitlement==2,
        "unassigned and site fractions survive successive updates");
    rejects([&]{initialize_work_credit(-1,0,empty);});
    rejects([&]{initialize_work_credit(0,1,empty);});
    rejects([&]{initialize_work_credit(1,0,std::array<WorkCreditSource,1>{{{1,1,1}}});});
    for(const auto bad:std::array<WorkCreditSource,4>{{{0,1,0},{11,-1,0},{11,0,1},{11,1,-1}}})
        rejects([&]{allocate_work_credit({},0,std::array<WorkCreditSource,1>{{bad}},0);});
    rejects([&]{allocate_work_credit({},0,std::array<WorkCreditSource,2>{{{11,0,0},{11,0,0}}},0);});
    rejects([&]{allocate_work_credit({},0,std::array<WorkCreditSource,2>{{{29,0,0},{11,0,0}}},0);});
    rejects([&]{allocate_work_credit({1,0,0},0,empty,0);});
    rejects([&]{allocate_work_credit({1,1,0},0,empty,0);});
    rejects([&]{allocate_work_credit({0,0,-1},0,empty,0);});
    rejects([&]{allocate_work_credit({},0,empty,-1);});
    const auto maximum=std::numeric_limits<int64_t>::max();
    rejects([&]{initialize_work_credit(maximum,maximum,one);});
    const std::array<WorkCreditSource,1> huge{{{11,maximum,0}}};
    rejects([&]{allocate_work_credit({maximum/2,0,maximum/2},0,huge,0);});
    const auto all=allocate_work_credit({maximum,0,maximum},0,huge,0);
    check(all.sites[0].credited_units==maximum&&all.regional_entitlement==maximum&&all.region.bridge_units==0,
        "full allocation avoids needless overflowing product");
    rejects([&]{allocate_work_credit({maximum,0,0},1,std::array<WorkCreditSource,1>{{{11,maximum,maximum}}},0);});
    std::cout<<"Work credit checks="<<checks<<"; exhaustive scenarios=48600\n";return 0;
}catch(const std::exception& error){std::cerr<<"FAIL after "<<checks<<": "<<error.what()<<'\n';return 1;}}






