#pragma once
#include "types.hpp"
namespace pe {
struct Command { Id actor=0; std::string verb; Id target=0; int amount=1,option=0; std::uint64_t revision=0; std::string text; std::map<std::string,int> params; int param(const std::string& name,int fallback)const {auto i=params.find(name);return i==params.end()?fallback:i->second;} };
struct ActionDefinition { std::string verb,parameters,description; };
struct Result { bool ok=false; std::string message; };
struct State;
using ActionHandler=void(*)(State&,const Command&);
struct RegisteredAction {ActionDefinition definition; ActionHandler handler;std::vector<std::string> named_parameters;};
const std::map<std::string,RegisteredAction>& action_registry();
namespace resources {void manage(State&,const Command&);void transfer(State&,const Command&);void equip(State&,const Command&);}
namespace production {void produce(State&,const Command&);void revoke(State&,const Command&);}
namespace life {void collect(State&,const Command&);void care(State&,const Command&);}
namespace civic {void dismiss(State&,const Command&);void announce(State&,const Command&);void invitation(State&,const Command&);void inspect(State&,const Command&);}
}
