#pragma once
#include "economy.hpp"
namespace pe {
struct Body { double hunger=30,thirst=25,fatigue=12,health=100,joy=50; void elapse(); };
struct Experience { double farming=0,crafting=0,communication=0; double efficiency(bool farm,double joy)const; };
struct Tendencies { int security=50,pleasure=50,mobility=50; };
struct Fact { int event=0; std::string text; Id source=0; int happened=0,received=0; };
enum class ActivityKind:int {Idle,Work,Move,Meal,Drink,Rest,Talk,Study,Collect,Care};
struct Activity { ActivityKind kind=ActivityKind::Idle; Id target=0; double progress=0,required=0; Money escrow=0; Inventory reserved; int option=0,started=0; std::string cause; };
struct Person { Id id=0; std::string name; Id location=10; Account account; Body body; Experience experience; Tendencies tendencies; std::string goal; Activity activity; std::vector<Fact> knowledge; std::map<Id,int> relations; };
}
