#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>
#include <span>
#include "../realm.hpp"

namespace polyec {

class FinanceManager;

// Immutable, opaque payroll plan. Only FinanceManager::prepare can populate it
// (private fields, no public constructor/setters), so apply() may trust the
// precomputed writes after revalidating the exact read set against live state.
// Const accessors expose scalar results for independent oracles/tests only.
class PayrollPlan {
public:
    bool noop() const { return noop_; }
    int day() const { return day_; }
    int region() const { return region_; }
    bool public_employer() const { return public_employer_; }
    size_t claim_count() const { return claims_.size(); }
    size_t recipient_count() const { return recipients_.size(); }
    uint64_t claim_id(size_t i) const { return claims_.at(i).id; }
    int64_t paid(size_t i) const { return claims_.at(i).paid; }
    int64_t tax(size_t i) const { return claims_.at(i).tax; }
    int64_t final_account() const { return final_account_; }
    int64_t final_treasury() const { return final_treasury_; }
    int64_t final_tax_receipts() const { return final_tax_receipts_; }

private:
    friend class FinanceManager;
    // Only FinanceManager::prepare may create a plan; callers cannot forge one.
    PayrollPlan() = default;
    struct ClaimPaid { uint64_t id = 0; int64_t paid = 0; int64_t tax = 0; };
    // Snapshot of every unique affected person: index + canonical id let apply()
    // reject a reordered/replaced people vector before writing any value.
    struct Recipient {
        size_t index = 0;
        uint64_t id = 0;
        int64_t cash_initial = 0;
        int remainder_initial = 0;
        int64_t final_cash = 0;
        int final_remainder = 0;
    };
    bool noop_ = false;
    int day_ = 0;
    int region_ = 0;
    bool public_employer_ = false;
    int64_t tax_rate_ = 0;
    int64_t account_initial_ = 0;
    int64_t treasury_initial_ = 0;
    int64_t receipts_initial_ = 0;
    int64_t final_account_ = 0;
    int64_t final_treasury_ = 0;
    int64_t final_tax_receipts_ = 0;
    std::vector<ClaimPaid> claims_;
    std::vector<Recipient> recipients_;
};

// Ordered input group for one staged batch. Claims are (recipient id, gross
// claim) in the exact order Realm wants FinanceManager to allocate them.
struct PayrollBatchGroup {
    int region = 0;
    bool public_employer = false;
    std::vector<std::pair<uint64_t,int64_t>> claims;
    uint64_t establishment_id = 0; // zero: legacy Province source
};

// Opaque staged batch. Only FinanceManager::prepare_batch may construct it;
// apply_batch validates the complete read set before the first write, then
// commits scalar values only. Claim order matches the concatenated input group
// claim order, so Realm can align non-wage/wage allocation tags by index.
class PayrollBatchPlan {
public:
    bool noop() const { return noop_; }
    int day() const { return day_; }
    size_t claim_count() const { return claims_.size(); }
    size_t group_count() const { return groups_.size(); }
    uint64_t claim_id(size_t i) const { return claims_.at(i).id; }
    int claim_region(size_t i) const { return claims_.at(i).region; }
    bool claim_public(size_t i) const { return claims_.at(i).public_employer; }
    uint64_t claim_establishment(size_t i) const { return claims_.at(i).establishment_id; }
    size_t establishment_count() const { return establishments_.size(); }
    int64_t paid(size_t i) const { return claims_.at(i).paid; }
    int64_t tax(size_t i) const { return claims_.at(i).tax; }

private:
    friend class FinanceManager;
    PayrollBatchPlan() = default;
    struct ClaimPaid {
        uint64_t id = 0;
        int region = 0;
        bool public_employer = false;
        uint64_t establishment_id = 0;
        int64_t paid = 0;
        int64_t tax = 0;
    };
    struct Recipient {
        size_t index = 0;
        uint64_t id = 0;
        int64_t cash_initial = 0;
        int remainder_initial = 0;
        int64_t final_cash = 0;
        int final_remainder = 0;
    };
    struct ProvinceDelta {
        int region = 0;
        bool firm_read = false;
        int64_t firm_initial = 0;
        int64_t firm_final = 0;
        int64_t treasury_initial = 0;
        int64_t treasury_final = 0;
        int64_t tax_rate = 0;
        int64_t receipts_initial = 0;
        int64_t receipts_final = 0;
    };
    struct EstablishmentDelta {
        size_t index=0;
        uint64_t id=0;
        int region=0;
        int64_t cash_initial=0,cash_final=0;
    };
    bool noop_ = false;
    int day_ = 0;
    std::vector<ProvinceDelta> groups_;
    std::vector<ClaimPaid> claims_;
    std::vector<Recipient> recipients_;
    std::vector<EstablishmentDelta> establishments_;
};

// Pure helper over Realm-owned containers. Realm stays the sole owner; these
// functions never keep state. FinanceManager::prepare splits payroll calculation
// from mutation: prepare reads const inputs and returns a PayrollPlan, apply
// revalidates the plan's exact read set and commits precomputed scalar writes.
// Realm::payroll calls the thin payroll() wrapper; no parallel algorithm remains.
class FinanceManager {
public:
    // Same semantics as the original Realm::transfer.
    static void transfer(int64_t &from, int64_t &to, int64_t amount) {
        if (amount < 0 || from < amount || to < 0) throw std::logic_error("unfunded transfer");
        if (&from == &to) return;
        if (to > std::numeric_limits<int64_t>::max() - amount)
            throw std::overflow_error("transfer destination overflow");
        from -= amount;
        to += amount;
    }

    // Calculate payroll without mutating anything. Preserves the original
    // early-return order (empty -> region -> per-claim validation -> due==0),
    // claim order/duplicates, pro-rata integer division, day-based rotation and
    // budget-before-tax. New state-domain validation and checked arithmetic run
    // only on the due>0 path, before any write.
    static PayrollPlan prepare(int day, int region, bool public_employer,
                               const std::vector<std::pair<uint64_t,int64_t>> &claims,
                               const std::vector<Person> &people,
                               const std::vector<Province> &provinces) {
        PayrollPlan plan;
        plan.day_ = day;
        plan.public_employer_ = public_employer;
        if (claims.empty()) { plan.noop_ = true; return plan; }
        if (region < 0 || size_t(region) >= provinces.size())
            throw std::out_of_range("province region");
        for (const auto &claim : claims) {
            if (claim.first == 0 || claim.first > people.size())
                throw std::out_of_range("person ID");
            if (claim.second < 0)
                throw std::logic_error("negative payroll claim");
        }
        int64_t due = 0;
        for (const auto &claim : claims) due = checked_add(due, claim.second);
        if (due == 0) { plan.noop_ = true; return plan; }

        const Province &v = provinces[region];
        if (v.tax < 0) throw std::logic_error("negative tax rate");
        int64_t account = public_employer ? v.treasury : v.firm;
        if (account < 0) throw std::logic_error("negative payroll account");
        if (v.treasury < 0) throw std::logic_error("negative payroll treasury");
        if (v.tax_receipts < 0) throw std::logic_error("negative tax receipts");

        plan.region_ = region;
        plan.tax_rate_ = v.tax;
        plan.account_initial_ = account;
        plan.treasury_initial_ = v.treasury;
        plan.receipts_initial_ = v.tax_receipts;

        int64_t budget = std::min(account, due), left = budget;
        std::vector<int64_t> paid(claims.size());
        for (size_t i = 0; i < claims.size(); ++i) {
            paid[i] = checked_mul_nonneg(budget, claims[i].second) / due;
            left -= paid[i];
        }
        // Public pro-rata clearing with rotating rounding, never permanent ID
        // priority. Rotation phase is the game DAY (unchanged).
        for (size_t offset = 0; left > 0 && offset < claims.size(); ++offset) {
            size_t i = (size_t(day) + offset) % claims.size();
            if (paid[i] < claims[i].second) { ++paid[i]; --left; }
        }

        int64_t acct = account;       // running source balance
        int64_t tres = v.treasury;    // running destination balance
        int64_t receipts = v.tax_receipts;
        std::vector<PayrollPlan::Recipient> recip;
        // Sparse average-O(1) lookup from person index to recipient slot. Only
        // lookups are used; recipient order stays first-encounter deterministic.
        std::unordered_map<size_t, size_t> recip_slot;
        for (size_t i = 0; i < claims.size(); ++i) {
            size_t idx = size_t(claims[i].first - 1);
            auto it = recip_slot.find(idx);
            size_t pos;
            if (it == recip_slot.end()) {
                const Person &p = people[idx];
                if (p.id != claims[i].first)
                    throw std::logic_error("inconsistent payroll person id");
                if (p.cash < 0) throw std::logic_error("negative payroll cash");
                if (p.tax_remainder < 0 || p.tax_remainder > 99)
                    throw std::logic_error("invalid tax remainder");
                PayrollPlan::Recipient r;
                r.index = idx;
                r.id = p.id;
                r.cash_initial = p.cash;
                r.remainder_initial = p.tax_remainder;
                r.final_cash = p.cash;
                r.final_remainder = p.tax_remainder;
                recip.push_back(r);
                pos = recip.size() - 1;
                recip_slot.emplace(idx, pos);
            } else {
                pos = it->second;
            }
            PayrollPlan::Recipient &r = recip[pos];
            const int64_t pay = paid[i];
            if (acct < pay) throw std::logic_error("unfunded transfer");
            acct -= pay;
            if (public_employer) tres -= pay; // source == destination field
            r.final_cash = checked_add(r.final_cash, pay);
            const int64_t assessed =
                checked_add(checked_mul_nonneg(pay, v.tax), r.final_remainder);
            const int64_t tax = assessed / 100;
            r.final_remainder = int(assessed % 100);
            if (r.final_cash < tax) throw std::logic_error("unfunded transfer");
            r.final_cash -= tax;
            tres = checked_add(tres, tax);
            if (public_employer) acct = checked_add(acct, tax);
            receipts = checked_add(receipts, tax);
            PayrollPlan::ClaimPaid cp; cp.id = claims[i].first; cp.paid = pay; cp.tax = tax;
            plan.claims_.push_back(cp);
        }

        plan.recipients_ = std::move(recip);
        plan.final_account_ = acct;
        plan.final_treasury_ = tres;
        plan.final_tax_receipts_ = receipts;
        plan.noop_ = false;
        return plan;
    }

    // Validate the plan's exact read set against live state, then commit
    // precomputed scalar writes. Allocates nothing and performs no throwing
    // operation after the first write, so a fresh plan cannot partially commit.
    static void apply(const PayrollPlan &plan, int64_t current_day,
                      std::vector<Person> &people,
                      std::vector<Province> &provinces) {
        if (plan.noop_) return;
        if (current_day != plan.day_)
            throw std::logic_error("stale payroll plan day");
        if (plan.region_ < 0 || size_t(plan.region_) >= provinces.size())
            throw std::out_of_range("province region");
        Province &v = provinces[plan.region_];
        if (plan.public_employer_) {
            if (v.treasury != plan.account_initial_ ||
                v.treasury != plan.treasury_initial_)
                throw std::logic_error("stale payroll account");
        } else {
            if (v.firm != plan.account_initial_)
                throw std::logic_error("stale payroll account");
            if (v.treasury != plan.treasury_initial_)
                throw std::logic_error("stale payroll treasury");
        }
        if (v.tax != plan.tax_rate_)
            throw std::logic_error("stale payroll tax");
        if (v.tax_receipts != plan.receipts_initial_)
            throw std::logic_error("stale payroll receipts");
        for (const auto &r : plan.recipients_) {
            if (r.index >= people.size())
                throw std::out_of_range("payroll recipient");
            if (people[r.index].id != r.id)
                throw std::logic_error("stale payroll person id");
            if (people[r.index].cash != r.cash_initial)
                throw std::logic_error("stale payroll cash");
            if (people[r.index].tax_remainder != r.remainder_initial)
                throw std::logic_error("stale payroll remainder");
        }
        if (plan.public_employer_) {
            v.treasury = plan.final_treasury_; // == final_account_
        } else {
            v.firm = plan.final_account_;
            v.treasury = plan.final_treasury_;
        }
        v.tax_receipts = plan.final_tax_receipts_;
        for (const auto &r : plan.recipients_) {
            people[r.index].cash = r.final_cash;
            people[r.index].tax_remainder = r.final_remainder;
        }
    }

    // Stage every ordered group against one overlay. Each group is allocated
    // against the running staged accounts/treasuries/tax receipts, so later
    // groups cannot spend money already promised to earlier groups. Nothing in
    // the live people/provinces containers is written here.
    static PayrollBatchPlan prepare_batch(
            int day, const std::vector<PayrollBatchGroup> &groups,
            const std::vector<Person> &people,
            const std::vector<Province> &provinces,
            std::span<const Establishment> establishments={}) {
        PayrollBatchPlan plan;
        plan.day_ = day;
        std::vector<Province> staged = provinces;
        std::vector<PayrollBatchPlan::ProvinceDelta> deltas(provinces.size());
        std::vector<char> region_seen(provinces.size(), 0);
        std::unordered_map<size_t, size_t> recip_slot;
        std::vector<PayrollBatchPlan::Recipient> recip;
        std::unordered_map<uint64_t,size_t> establishment_slots;

        for (const auto &group : groups) {
            if (group.claims.empty()) continue;
            if(group.establishment_id&&group.public_employer)
                throw std::logic_error("ambiguous payroll source");
            const int region = group.region;
            if (region < 0 || size_t(region) >= provinces.size())
                throw std::out_of_range("province region");
            size_t input_establishment_index=0;
            if(group.establishment_id){
                const auto source=std::lower_bound(establishments.begin(),establishments.end(),group.establishment_id,
                    [](const Establishment& site,uint64_t id){return site.id<id;});
                if(source==establishments.end()||source->id!=group.establishment_id)
                    throw std::out_of_range("establishment payroll source");
                if(source->region!=region)throw std::logic_error("payroll establishment region mismatch");
                input_establishment_index=size_t(source-establishments.begin());
            }
            for (const auto &claim : group.claims) {
                if (claim.first == 0 || claim.first > people.size())
                    throw std::out_of_range("person ID");
                if (claim.second < 0)
                    throw std::logic_error("negative payroll claim");
            }
            int64_t due = 0;
            for (const auto &claim : group.claims)
                due = checked_add(due, claim.second);
            if (due == 0) {
                for (const auto &claim : group.claims) {
                    PayrollBatchPlan::ClaimPaid cp;
                    cp.id = claim.first;
                    cp.region = region;
                    cp.public_employer = group.public_employer;
                    cp.establishment_id = group.establishment_id;
                    plan.claims_.push_back(cp);
                }
                continue;
            }

            if (!region_seen[size_t(region)]) {
                const Province &live = provinces[size_t(region)];
                if (live.tax < 0) throw std::logic_error("negative tax rate");
                if (live.treasury < 0)
                    throw std::logic_error("negative payroll treasury");
                if (live.tax_receipts < 0)
                    throw std::logic_error("negative tax receipts");
                PayrollBatchPlan::ProvinceDelta delta;
                delta.region = region;
                delta.treasury_initial = live.treasury;
                delta.treasury_final = live.treasury;
                delta.tax_rate = live.tax;
                delta.receipts_initial = live.tax_receipts;
                delta.receipts_final = live.tax_receipts;
                delta.firm_initial = live.firm;
                delta.firm_final = live.firm;
                deltas[size_t(region)] = delta;
                region_seen[size_t(region)] = 1;
            }
            PayrollBatchPlan::ProvinceDelta &delta = deltas[size_t(region)];
            size_t establishment_slot=0;
            if(group.establishment_id){
                const auto known=establishment_slots.find(group.establishment_id);
                if(known==establishment_slots.end()){
                    const auto& source=establishments[input_establishment_index];
                    if(source.cash<0)throw std::logic_error("negative payroll account");
                    establishment_slot=plan.establishments_.size();
                    plan.establishments_.push_back({input_establishment_index,source.id,region,source.cash,source.cash});
                    establishment_slots.emplace(source.id,establishment_slot);
                }else establishment_slot=known->second;
                if(plan.establishments_[establishment_slot].region!=region)
                    throw std::logic_error("payroll establishment region mismatch");
            }
            if (group.public_employer) {
                if (staged[size_t(region)].treasury < 0)
                    throw std::logic_error("negative payroll account");
            } else if(!group.establishment_id) {
                if (staged[size_t(region)].firm < 0)
                    throw std::logic_error("negative payroll account");
                delta.firm_read = true;
            }

            const Province &v = staged[size_t(region)];
            const int64_t account = group.establishment_id
                ? plan.establishments_[establishment_slot].cash_final
                : group.public_employer ? v.treasury : v.firm;
            int64_t budget = std::min(account, due), left = budget;
            std::vector<int64_t> paid(group.claims.size());
            for (size_t i = 0; i < group.claims.size(); ++i) {
                paid[i] = checked_mul_nonneg(budget, group.claims[i].second) / due;
                left -= paid[i];
            }
            for (size_t offset = 0; left > 0 && offset < group.claims.size();
                 ++offset) {
                size_t i = (size_t(day) + offset) % group.claims.size();
                if (paid[i] < group.claims[i].second) {
                    ++paid[i];
                    --left;
                }
            }

            int64_t acct = account;
            int64_t tres = v.treasury;
            int64_t receipts = v.tax_receipts;
            for (size_t i = 0; i < group.claims.size(); ++i) {
                size_t idx = size_t(group.claims[i].first - 1);
                auto it = recip_slot.find(idx);
                size_t pos;
                if (it == recip_slot.end()) {
                    const Person &p = people[idx];
                    if (p.id != group.claims[i].first)
                        throw std::logic_error("inconsistent payroll person id");
                    if (p.cash < 0) throw std::logic_error("negative payroll cash");
                    if (p.tax_remainder < 0 || p.tax_remainder > 99)
                        throw std::logic_error("invalid tax remainder");
                    PayrollBatchPlan::Recipient r;
                    r.index = idx;
                    r.id = p.id;
                    r.cash_initial = p.cash;
                    r.remainder_initial = p.tax_remainder;
                    r.final_cash = p.cash;
                    r.final_remainder = p.tax_remainder;
                    recip.push_back(r);
                    pos = recip.size() - 1;
                    recip_slot.emplace(idx, pos);
                } else {
                    pos = it->second;
                }
                PayrollBatchPlan::Recipient &r = recip[pos];
                const int64_t pay = paid[i];
                if (acct < pay) throw std::logic_error("unfunded transfer");
                acct -= pay;
                if (group.public_employer) tres -= pay;
                r.final_cash = checked_add(r.final_cash, pay);
                const int64_t assessed =
                    checked_add(checked_mul_nonneg(pay, v.tax), r.final_remainder);
                const int64_t tax = assessed / 100;
                r.final_remainder = int(assessed % 100);
                if (r.final_cash < tax) throw std::logic_error("unfunded transfer");
                r.final_cash -= tax;
                tres = checked_add(tres, tax);
                if (group.public_employer) acct = checked_add(acct, tax);
                receipts = checked_add(receipts, tax);
                PayrollBatchPlan::ClaimPaid cp;
                cp.id = group.claims[i].first;
                cp.region = region;
                cp.public_employer = group.public_employer;
                cp.establishment_id = group.establishment_id;
                cp.paid = pay;
                cp.tax = tax;
                plan.claims_.push_back(cp);
            }

            if(group.establishment_id){
                plan.establishments_[establishment_slot].cash_final=acct;
                staged[size_t(region)].treasury=tres;
            }else if (group.public_employer) {
                staged[size_t(region)].treasury = acct;
            } else {
                staged[size_t(region)].firm = acct;
                staged[size_t(region)].treasury = tres;
            }
            staged[size_t(region)].tax_receipts = receipts;
            delta.treasury_final = staged[size_t(region)].treasury;
            delta.firm_final = staged[size_t(region)].firm;
            delta.receipts_final = receipts;
        }

        for (size_t i = 0; i < provinces.size(); ++i)
            if (region_seen[i]) plan.groups_.push_back(deltas[i]);
        plan.recipients_ = std::move(recip);
        plan.noop_ = plan.claims_.empty();
        return plan;
    }

    // Validate the complete batch read set before the first write. After that
    // point only precomputed scalar assignments and no throwing operations run.
    static void apply_batch(const PayrollBatchPlan &plan, int64_t current_day,
                            std::vector<Person> &people,
                            std::vector<Province> &provinces,
                            std::span<Establishment> establishments={}) {
        if (plan.noop_) return;
        if (current_day != plan.day_)
            throw std::logic_error("stale payroll batch day");
        for (const auto &delta : plan.groups_) {
            if (delta.region < 0 || size_t(delta.region) >= provinces.size())
                throw std::out_of_range("province region");
            const Province &v = provinces[size_t(delta.region)];
            if (delta.firm_read && v.firm != delta.firm_initial)
                throw std::logic_error("stale payroll batch account");
            if (v.treasury != delta.treasury_initial)
                throw std::logic_error("stale payroll batch treasury");
            if (v.tax != delta.tax_rate)
                throw std::logic_error("stale payroll batch tax");
            if (v.tax_receipts != delta.receipts_initial)
                throw std::logic_error("stale payroll batch receipts");
        }
        for(const auto& source:plan.establishments_){
            if(source.index>=establishments.size())throw std::out_of_range("establishment payroll source");
            const auto& site=establishments[source.index];
            if(site.id!=source.id||site.region!=source.region||site.cash!=source.cash_initial)
                throw std::logic_error("stale payroll establishment");
        }
        for (const auto &r : plan.recipients_) {
            if (r.index >= people.size())
                throw std::out_of_range("payroll recipient");
            if (people[r.index].id != r.id)
                throw std::logic_error("stale payroll person id");
            if (people[r.index].cash != r.cash_initial)
                throw std::logic_error("stale payroll cash");
            if (people[r.index].tax_remainder != r.remainder_initial)
                throw std::logic_error("stale payroll remainder");
        }
        for (const auto &delta : plan.groups_) {
            Province &v = provinces[size_t(delta.region)];
            if (delta.firm_read) v.firm = delta.firm_final;
            v.treasury = delta.treasury_final;
            v.tax_receipts = delta.receipts_final;
        }
        for(const auto& source:plan.establishments_)establishments[source.index].cash=source.cash_final;
        for (const auto &r : plan.recipients_) {
            people[r.index].cash = r.final_cash;
            people[r.index].tax_remainder = r.final_remainder;
        }
    }

    // Thin wrapper preserving the original caller API.
    static void payroll(int day, int region, bool public_employer,
                        const std::vector<std::pair<uint64_t,int64_t>> &claims,
                        std::vector<Person> &people,
                        std::vector<Province> &provinces) {
        apply(prepare(day, region, public_employer, claims, people, provinces),
              day, people, provinces);
    }

private:
    static int64_t checked_add(int64_t a, int64_t b) {
        const int64_t hi = std::numeric_limits<int64_t>::max();
        const int64_t lo = std::numeric_limits<int64_t>::min();
        if (b > 0 && a > hi - b) throw std::overflow_error("payroll overflow");
        if (b < 0 && a < lo - b) throw std::overflow_error("payroll overflow");
        return a + b;
    }
    static int64_t checked_mul_nonneg(int64_t a, int64_t b) {
        if (a < 0 || b < 0) throw std::logic_error("payroll negative product");
        if (a != 0 && b > std::numeric_limits<int64_t>::max() / a)
            throw std::overflow_error("payroll overflow");
        return a * b;
    }
};

}
