#pragma once
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>
#include "../realm.hpp"

namespace polyec {

// Pure helper over Realm-owned containers. Realm stays the sole owner; these
// functions never keep state and are not a second payroll implementation —
// Realm::payroll delegates here with exact d5751bb semantics.
class FinanceManager {
public:
    // Same semantics as the original Realm::transfer.
    static void transfer(int64_t &from, int64_t &to, int64_t amount) {
        if (amount < 0 || from < amount) throw std::logic_error("unfunded transfer");
        from -= amount;
        to += amount;
    }

    // Full-parity extraction of Realm::payroll (d5751bb):
    // - pro-rata settlement budget = min(account, due);
    // - rotating rounding remainder keyed on DAY (not region), so no permanent
    //   priority for any claim position;
    // - tax assessed per person via province.tax + person.tax_remainder;
    //   the tax is debited from the person's cash into the province treasury
    //   (money conservation) and counted in province.tax_receipts (counter
    //   only — Realm::total_money excludes tax_receipts).
    // Invalid inputs (region out of range, unknown person ID, negative claim)
    // are rejected BEFORE any mutation so a throw never leaves partial state.
    // Valid inputs behave exactly like the original inline implementation.
    static void payroll(int day, int region, bool public_employer,
                        const std::vector<std::pair<uint64_t,int64_t>> &claims,
                        std::vector<Person> &people,
                        std::vector<Province> &provinces) {
        if (claims.empty()) return;
        if (region < 0 || size_t(region) >= provinces.size())
            throw std::out_of_range("province region");
        for (const auto &claim : claims) {
            if (claim.first == 0 || claim.first > people.size())
                throw std::out_of_range("person ID");
            if (claim.second < 0)
                throw std::logic_error("negative payroll claim");
        }
        auto &v = provinces[region];
        auto &account = public_employer ? v.treasury : v.firm;
        int64_t due = 0;
        for (const auto &claim : claims) due += claim.second;
        if (due == 0) return; // nothing owed and nothing payable: safe no-op
        int64_t budget = std::min(account, due), left = budget;
        std::vector<int64_t> paid(claims.size());
        for (size_t i = 0; i < claims.size(); ++i) {
            paid[i] = budget * claims[i].second / due;
            left -= paid[i];
        }
        // Public pro-rata clearing with rotating rounding, never permanent ID
        // priority. Rotation phase is the game DAY.
        for (size_t offset = 0; left > 0 && offset < claims.size(); ++offset) {
            size_t i = (size_t(day) + offset) % claims.size();
            if (paid[i] < claims[i].second) { ++paid[i]; --left; }
        }
        for (size_t i = 0; i < claims.size(); ++i) {
            Person &p = people[claims[i].first - 1];
            transfer(account, p.cash, paid[i]);
            int64_t assessed = paid[i] * v.tax + p.tax_remainder;
            int64_t tax = assessed / 100;
            p.tax_remainder = int(assessed % 100);
            transfer(p.cash, v.treasury, tax);
            v.tax_receipts += tax;
        }
    }
};

}
