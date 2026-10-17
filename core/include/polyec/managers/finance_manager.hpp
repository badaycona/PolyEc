#pragma once
#include <vector>
#include "../types.hpp"

namespace polyec {

class FinanceManager {
public:
    // Simple safe transfer helper. Throws on invalid transfer.
    static void transfer(int64_t &from, int64_t &to, int64_t amount) {
        if (amount < 0 || from < amount) throw std::logic_error("unfunded transfer");
        from -= amount;
        to += amount;
    }

    // Placeholder payroll: caller should supply claim list. This function keeps
    // a minimal semantic parity with the core: it transfers amounts from
    // account to recipients' cash values in a pro-rata manner.
    static void payroll(int64_t &account, const std::vector<std::pair<uint64_t,int64_t>>& claims, std::vector<Person>& people) {
        if (claims.empty()) return;
        int64_t due = 0; for (auto &c : claims) due += c.second;
        int64_t budget = std::min(account, due);
        int64_t left = budget;
        std::vector<int64_t> paid(claims.size());
        for (size_t i=0;i<claims.size();++i) { paid[i] = budget * claims[i].second / due; left -= paid[i]; }
        for (size_t offset=0; left>0 && offset<claims.size(); ++offset) {
            size_t i = (offset) % claims.size();
            if (paid[i] < claims[i].second) { ++paid[i]; --left; }
        }
        for (size_t i=0;i<claims.size();++i) {
            uint64_t id = claims[i].first; int64_t amount = paid[i];
            if (id==0 || id > people.size()) continue;
            Person &p = people[id-1];
            transfer(account, p.cash, amount);
        }
    }
};

}
