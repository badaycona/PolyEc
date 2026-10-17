#pragma once
#include <vector>
#include <stdexcept>
#include "../types.hpp"
#include "../realm.hpp"

namespace polyec {

class FinanceManager {
public:
    // Simple safe transfer helper. Throws on invalid transfer.
    static void transfer(int64_t &from, int64_t &to, int64_t amount) {
        if (amount < 0 || from < amount) throw std::logic_error("unfunded transfer");
        from -= amount;
        to += amount;
    }

    // Payroll helper that mirrors the original Realm::payroll semantics closely.
    // Transfers amounts from the account into people's cash pro-rata and applies
    // tax rounding behavior via province.tax_remainder. This is intentionally
    // a drop-in replacement for the existing Realm implementation to enable an
    // incremental extraction without changing behaviour.
    static void payroll(int region, bool public_employer,
                        const std::vector<std::pair<uint64_t,int64_t>>& claims,
                        std::vector<Person>& people,
                        std::vector<Province>& provinces) {
        if (claims.empty()) return;
        auto &v = provinces[region];
        auto &account = public_employer ? v.treasury : v.firm;
        int64_t due = 0; for (auto &claim : claims) due += claim.second;
        int64_t budget = std::min(account, due), left = budget;
        std::vector<int64_t> paid(claims.size());
        for (size_t i = 0; i < claims.size(); ++i) { paid[i] = budget * claims[i].second / due; left -= paid[i]; }
        for (size_t offset = 0; left > 0 && offset < claims.size(); ++offset) { size_t i = (region + offset) % claims.size(); if (paid[i] < claims[i].second) { ++paid[i]; --left; } }
        for (size_t i = 0; i < claims.size(); ++i) {
            uint64_t id = claims[i].first; int64_t amount = paid[i];
            if (id == 0 || id > people.size()) continue;
            Person &p = people[id - 1];
            transfer(account, p.cash, paid[i]);
            int64_t assessed = paid[i] * v.tax + p.tax_remainder; int64_t tax = assessed / 100; p.tax_remainder = int(assessed % 100);
            // Collected tax goes to treasury
            v.treasury += tax;
        }
    }
};

}
