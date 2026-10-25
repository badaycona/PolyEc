#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>
namespace pe {
using Id=int; using Money=std::int64_t;
struct RuleError:std::runtime_error {using std::runtime_error::runtime_error;};
inline void require(bool test,const std::string& why){if(!test)throw RuleError(why);}
}
