#pragma once
#include "economy.hpp"
namespace pe {
struct Office { Id holder=0; bool can_grant=false; };
struct Organization { Id id=100; std::string name="Public treasury"; Account account{100}; Office office{1,true}; };
}
