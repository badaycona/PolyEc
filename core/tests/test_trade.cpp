#include "polyec/trade.hpp"
#include <iostream>
#include <cmath>
#include <random>
using namespace polyec;
int checks=0, failures=0;
#define CHECK(x) do {++checks;if(!(x)){++failures;std::cerr<<"FAIL "<<__LINE__<<": "<<#x<<'\n';}}while(0)
int main() {
    TradeWorld w(21); CHECK(w.invariant());
    auto hash=w.fingerprint(); CHECK(!w.commit(-1)); CHECK(!w.commit(99)); CHECK(hash==w.fingerprint());
    CHECK(!w.commit(8)); CHECK(hash==w.fingerprint());
    auto before=w.view(); CHECK(w.commit(1)); CHECK(w.commit(8));
    CHECK(w.view().cargo[0]==0); CHECK(w.view().cash==before.cash-8); CHECK(w.invariant());
    int target=(w.view().region+1)%6; int origin=w.view().region;
    CHECK(w.commit(15+target)); CHECK(w.view().region==origin); CHECK(w.view().transit_days>0);
    w.step(0); CHECK(w.view().region==target); CHECK(w.view().transit_days==0); CHECK(w.invariant());
    TradeWorld a(123,6,64),b(123,6,64);
    std::mt19937 gen(932);
    for(int t=0;t<64;++t) {
        auto mask=a.mask(); std::vector<int> valid;
        for(int i=0;i<21;++i) if(mask[i]>0.5f) valid.push_back(i);
        int action=valid[gen()%valid.size()];
        auto ar=a.step(action),br=b.step(action);
        CHECK(ar.reward==br.reward); CHECK(a.fingerprint()==b.fingerprint()); CHECK(a.invariant());
        for(float value:a.observation()) CHECK(std::isfinite(value));
    }
    CHECK(a.view().done); auto end=a.fingerprint(); CHECK(!a.commit(0)); CHECK(a.fingerprint()==end);
    //20 years of weekly rules, several seeds; not a full M1-quality claim.
    for(int seed=0;seed<8;++seed) {
        TradeWorld long_world(seed,6,1030);
        int64_t money=long_world.total_money();
        for(int t=0;t<1030;++t) {
            auto m=long_world.mask(); std::vector<int> valid;
            for(int i=0;i<21;++i) if(m[i]) valid.push_back(i);
            long_world.step(valid[gen()%valid.size()]);
            CHECK(long_world.invariant()); CHECK(long_world.total_money()==money);
        }
        CHECK(long_world.view().day>=7200);
    }
    for(int n:{1,2,6}) {TradeWorld env(42,n,1); env.step(0); CHECK(env.view().done);CHECK(env.invariant());}
    std::cout<<checks-failures<<'/'<<checks<<" trade checks passed\n";
    return failures?1:0;
}
