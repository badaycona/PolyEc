#pragma once
#include "types.hpp"
namespace pe {
enum class Good:int {Food,Seed,Wood,Tool,Water,Count};
constexpr int goods=5;
struct Inventory { std::array<int,goods> units{}; int& operator[](Good g){return units.at(int(g));} int operator[](Good g)const{return units.at(int(g));} };
struct Account { Id id=0; Money money=0; Inventory inventory; };

struct State;
struct MoneyLeg { Id from,to; Money amount; };
struct GoodsLeg { Inventory* from; Inventory* to; Good good; int amount; };
class Transaction { std::vector<MoneyLeg> money_; std::vector<GoodsLeg> goods_; public: Transaction& money(Id,Id,Money); Transaction& goods(Inventory&,Inventory&,Good,int); void commit(State&); };
}
