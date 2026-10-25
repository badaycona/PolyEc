#pragma once
#include "economy.hpp"
namespace pe {
enum class SiteKind:int {Other,Court,Market,Well,Residence,Production,Camp,Road};
enum class WorkSkill:int {Farming,Crafting};
struct Recipe { Inventory input,output; double work=0; WorkSkill skill=WorkSkill::Crafting; };
struct Location {
    Id id=0,owner=0; std::string name; int x=0,y=0; Inventory stock;
    bool tool=false,job=false; Money wage=6; int tool_uses=0; std::vector<Id> residents;
    Id region=1; SiteKind kind=SiteKind::Other; int lodging_capacity=0;
    bool has_sale=false; Good sale_good=Good::Food; int price=1; Recipe recipe;
};
struct Territory { Id id=1; std::string name="Thanh Dien"; std::map<Id,Location> locations; Id treasury=100; };
}
