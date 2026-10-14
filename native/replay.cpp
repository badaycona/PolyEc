#include "polyec/policy.hpp"
#include "polyec/trade.hpp"
#include <iostream>
#include <algorithm>
int main(int argc,char**argv) {
    if(argc<2) return 2;
    polyec::NativePolicy policy;
    if(!policy.load(argv[1])) {std::cerr<<policy.error()<<'\n';return 1;}
    polyec::TradeWorld world(876543,6,64);
    std::vector<float> hidden(64,0.f);
    for(int t=0;t<64;++t) {
        auto observation=world.observation();auto mask=world.mask();
        auto output=policy.predict(std::vector<float>(observation.begin(),observation.end()),hidden,1);
        if(!output.success) {std::cerr<<output.error<<'\n';return 1;}
        int action=0;float best=-1e30f;
        for(int a=0;a<polyec::kActions;++a) if(mask[a] && output.logits[a]>best) {best=output.logits[a];action=a;}
        hidden=output.hidden;
        auto step=world.step(action);
        std::cout<<t<<' '<<action<<' '<<world.fingerprint()<<' '<<world.view().cash<<'\n';
    }
}
