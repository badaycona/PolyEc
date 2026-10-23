#include "polyec/trade.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
int main() {
    polyec::TradeWorld world(876543,6,64);
    std::cout<<std::setprecision(9);
    for(int t=0;t<64;++t) {
        std::vector<int> valid; auto mask=world.mask();
        for(int a=0;a<polyec::kActions;++a) if(mask[a]) valid.push_back(a);
        int action=valid[(t*17+3)%valid.size()];
        auto r=world.step(action);auto v=world.view();
        std::cout<<t<<' '<<action<<' '<<world.fingerprint()<<' '<<r.reward<<' '<<v.cash<<' '<<v.region<<'\n';
    }
}

