#pragma once
#include "geography.hpp"
namespace pe {
struct Journey { Id route=0,origin=0,destination=0; int coordinate=0,steps=0,leg_start=0; bool paused=false; Money toll_paid=0; int started=0; };
struct Route { Id id=1,from=13,to=23,controller=1,collector=100; std::string name="Thanh Dien - Tung Lam"; int length=12,capacity=12; Money toll=2; bool open=true; Location camp,road; };
struct State; struct Person; struct Command;
Route& route(State&,Id); const Route& route(const State&,Id);
namespace transport {
int carried_weight(const Person&);
void travel(State&,const Command&);void pause(State&,const Command&);void resume(State&,const Command&);void turnback(State&,const Command&);void control(State&,const Command&);
void advance(State&,Person&);void interrupt(State&,Person&,const std::string&);void validate(const State&);
}
}
