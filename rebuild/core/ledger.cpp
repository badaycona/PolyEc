#include "model.hpp"
namespace pe {
Transaction& Transaction::money(Id from,Id to,Money amount){money_.push_back({from,to,amount});return *this;}
Transaction& Transaction::goods(Inventory&from,Inventory&to,Good good,int amount){goods_.push_back({&from,&to,good,amount});return *this;}
void Transaction::commit(State&s){std::map<Id,Money>dm;std::map<std::pair<Inventory*,int>,std::int64_t>dg;
 for(auto&l:money_){require(l.amount>0&&l.amount<=1000000,"Invalid transfer amount");account(s,l.from);account(s,l.to);dm[l.from]-=l.amount;dm[l.to]+=l.amount;}
 for(auto&l:goods_){require(l.amount>0&&l.amount<=1000000,"Invalid goods amount");dg[{l.from,int(l.good)}]-=l.amount;dg[{l.to,int(l.good)}]+=l.amount;}
 for(auto&[id,d]:dm){auto n=account(s,id).money+d;require(n>=0,"Insufficient funds");require(n<=1000000000,"Account overflow");}
 for(auto&[key,d]:dg){auto n=key.first->units.at(key.second)+d;require(n>=0,"Insufficient goods");require(n<=1000000,"Inventory overflow");}
 for(auto&[id,d]:dm)account(s,id).money+=d;for(auto&[key,d]:dg)key.first->units.at(key.second)+=int(d);
}
}
