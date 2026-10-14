#include "polyec/realm_runtime.hpp"
#include <fstream>
#include <iostream>
int main(int argc,char**argv){
    if(argc!=6){std::cerr<<"usage: realm_replay governance.onnx economic.onnx civilian.onnx bundle output.json\n";return 2;}
    polyec::RealmRuntime runtime;
    if(!runtime.load_models({argv[1],argv[2],argv[3]},argv[4],37,10000)){std::cerr<<runtime.error();return 1;}
    auto draft=runtime.world()->draft();runtime.reroll(0);draft=runtime.world()->draft();runtime.choose(draft.cards[0]);
    std::ofstream out(argv[5]);out<<"[\n";
    for(int day=1;day<=64;++day){if(!runtime.advance(1)){std::cerr<<runtime.error();return 1;}auto v=runtime.world()->view();out<<"{\"day\":"<<day<<",\"fingerprint\":\""<<runtime.world()->fingerprint()<<"\",\"money\":"<<v.money<<",\"alive\":"<<v.alive<<",\"decisions\":"<<v.decisions<<"}"<<(day==64?"\n":",\n");}
    out<<"]\n";std::cout<<"REALM REPLAY:64 days, "<<runtime.inference_count()<<" learned decisions\n";
}
