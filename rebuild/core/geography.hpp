#pragma once
#include "economy.hpp"
namespace pe {
struct Location { Id id=0,owner=0; std::string name; int x=0,y=0; Inventory stock; bool tool=false,job=false; Money wage=6; int tool_uses=0; std::vector<Id> residents; };
struct Territory { Id id=1; std::string name="Thanh Dien - trial-a2"; std::map<Id,Location> locations; };
}
