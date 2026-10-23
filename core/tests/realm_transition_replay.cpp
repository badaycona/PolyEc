#include "polyec/realm.hpp"
#include <iostream>
int main(){polyec::Realm w(37,6,256);for(int day=0;day<64;++day){for(auto family:{polyec::Family::Governance,polyec::Family::Economic,polyec::Family::Civilian})for(auto id:w.due(family)){auto mask=w.mask(id);int a=int((id+3*day)%polyec::realm_actions);if(!mask[a])a=0;auto r=w.execute({id,w.person(id).decisions+1,w.candidates(id)[a]});if(!r.success)return 1;}w.advance();if(!w.invariant())return 1;std::cout<<w.day()<<' '<<w.fingerprint()<<' '<<w.total_money()<<'\n';}}

