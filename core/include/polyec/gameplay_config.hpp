#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <iosfwd>

namespace polyec {
// A validated value snapshot. Realm exposes only const access; changing a file
// cannot alter a running world's rules. Historical defaults never follow files.
class GameplayConfig {
public:
    using Values = std::array<int64_t, 18>;
    GameplayConfig() = default;
    static GameplayConfig historical();
    static GameplayConfig from_values(const Values& values);
    static GameplayConfig parse(std::string_view text);
    static GameplayConfig load(const std::string& path);
    int64_t ticks_per_unit() const { return values_[0]; }
    int64_t wage_per_unit() const { return values_[1]; }
    int default_capacity() const { return static_cast<int>(values_[2]); }
    bool market_enabled() const { return values_[11] == 1; }
    int64_t market_response_basis_points() const { return values_[12]; }
    int64_t market_maximum_change_basis_points() const { return values_[13]; }
    int64_t market_price_floor() const { return values_[14]; }
    int64_t market_price_ceiling() const { return values_[15]; }
    int64_t salt_fuel_input() const { return values_[16]; }
    int64_t salt_output() const { return values_[17]; }
    const Values& values() const { return values_; }
    std::string canonical() const;
    uint64_t fingerprint() const;
    bool historical_defaults() const { return values_ == Values{16,1,1,1,1,1,1,1,1,1,1,0,1000,1000,1,1000000,1,1}; }
    bool operator==(const GameplayConfig&) const = default;
private:
    Values values_{16,1,1,1,1,1,1,1,1,1,1,1,1000,1000,1,1000000,1,1};
};
void write_gameplay_config(std::ostream& out, const GameplayConfig& config);
GameplayConfig read_gameplay_config(std::istream& in);
} // namespace polyec
