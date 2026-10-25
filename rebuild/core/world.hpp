#pragma once
#include "person.hpp"
#include "geography.hpp"
#include "institutions.hpp"
#include "contracts.hpp"
#include "actions.hpp"
namespace pe {
struct Event { int id=0,tick=0; std::string text; std::vector<Id> witnesses; bool public_notice=false; };
struct State { std::uint32_t seed=251025; int tick=0; std::uint64_t revision=0; int next_event=1,next_request=1; std::map<Id,Person> people; Territory territory; Organization government; std::vector<Request> requests; std::vector<Event> events; Money initial_money=556; Inventory created,consumed; int next_contract=1,next_debt=1,next_lease=1; std::vector<Proposal> proposals; std::vector<Debt> debts; std::vector<Lease> leases; };
Person& person(State&,Id); const Person& person(const State&,Id);
Location& location(State&,Id); const Location& location(const State&,Id);
Account& account(State&,Id);
void emit(State&,std::string,std::vector<Id>,bool public_notice=false);
void observe(State&,Id);
void idle(const Person&);
void start(State&,Person&,ActivityKind,Id,double,int option=0);
void cancel_activity(State&,Id,const std::string&);
namespace production { void offer(State&,const Command&); void work(State&,const Command&); void buy(State&,const Command&); void advance(State&,Person&); void cancel(State&,Person&); }
namespace life { void move(State&,const Command&); void meal(State&,const Command&); void drink(State&,const Command&); void rest(State&,const Command&); void advance(State&,Person&); }
namespace civic { void grant(State&,const Command&); void talk(State&,const Command&); void accept(State&,const Command&); void study(State&,const Command&); void goal(State&,const Command&); void advance(State&,Person&); }
class World { State state_; public: explicit World(std::uint32_t seed=251025); const State& state()const{return state_;} Result submit(const Command&); Result advance(int steps); std::string save()const; Result load(const std::string&); std::string view(Id viewer,bool debug)const; static void validate(const State&); static std::vector<ActionDefinition> definitions(); };
std::string encode(const State&); State decode(const std::string&); std::string project(const State&,Id,bool);
std::string activity_name(ActivityKind);}
