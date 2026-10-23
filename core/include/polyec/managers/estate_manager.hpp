#pragma once

#include "../realm.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <set>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace polyec {

class EstateDeathPlan {
    friend class EstateManager;
    EstateDeathPlan() = default;
    uint64_t person_id_ = 0;
    int day_ = 0, region_ = 0;
    int64_t cash_initial_ = 0, estate_initial_ = 0, estate_final_ = 0;
    std::array<int64_t, kGoodIdCount> inventory_initial_{};
    std::vector<EstateCase> cases_initial_, cases_final_;
};

class EstateCashClaimPlan {
    friend class EstateManager;
    EstateCashClaimPlan() = default;
    uint64_t decedent_ = 0;
    bool noop_ = false;
    int64_t estate_initial_ = 0, estate_final_ = 0;
    int64_t legacy_initial_ = 0, credit_pool_initial_ = 0, credit_pool_final_ = 0;
    std::vector<EstateCase> cases_initial_, cases_final_;
    std::vector<Loan> loans_initial_, loans_final_;
    std::vector<int64_t> province_firms_initial_, province_firms_final_;
};

enum class EstateBeneficiaryTier : uint8_t { Children, Spouse, Parents, Siblings, PublicEstate };
struct EstateBeneficiaries {
    EstateBeneficiaryTier tier = EstateBeneficiaryTier::PublicEstate;
    std::vector<uint64_t> recipients;
};

class EstateDistributionPlan {
    friend class EstateManager;
    EstateDistributionPlan() = default;
    uint64_t decedent_ = 0;
    int64_t estate_initial_ = 0, estate_final_ = 0;
    int64_t legacy_initial_ = 0, legacy_final_ = 0;
    std::vector<EstateCase> cases_initial_, cases_final_;
    std::vector<std::pair<uint64_t, int64_t>> recipient_cash_initial_, recipient_cash_final_;
    std::vector<std::pair<uint64_t, std::array<int64_t, kGoodIdCount>>> recipient_inventory_initial_, recipient_inventory_final_;
    int public_region_ = -1;
    std::array<int64_t, kGoodIdCount> public_stock_initial_{}, public_stock_final_{};
    std::vector<Loan> loans_initial_;
    std::vector<std::pair<uint64_t, uint64_t>> establishment_owners_initial_;
    std::vector<uint64_t> establishments_to_transfer_;
    std::vector<LandHolding> holdings_initial_, holdings_final_;
    std::vector<LandUseRight> use_rights_initial_, use_rights_final_;
};

// Stages the transfer of a deceased Person's cash and goods into a saveable case,
// and transfers land titles only when one heir is eligible and all estate claims
// are paid. Shared or encumbered titles remain referenced by the case.
class EstateManager {
public:
    // Moves remaining cash and goods from one estate case. The first living
    // legal beneficiary tier receives equal shares; indivisible units go to
    // the lowest IDs first. With no living heir, cash enters the public estate
    // fund and goods enter the decedent's regional public stock.
    static EstateDistributionPlan prepare_distribution(uint64_t decedent, int64_t estate_balance,
        int64_t legacy_unassigned, const std::vector<EstateCase>& cases,
        const std::vector<Person>& people, const std::vector<Edge>& relations,
        const std::vector<Province>& provinces, const std::vector<Loan>& loans,
        const std::vector<Establishment>& establishments,
        const std::vector<LandHolding>& holdings, const std::vector<LandUseRight>& use_rights) {
        if (decedent == 0 || estate_balance < 0 || legacy_unassigned < 0)
            throw std::logic_error("invalid estate distribution input");
        EstateDistributionPlan plan;
        plan.decedent_ = decedent; plan.estate_initial_ = estate_balance; plan.estate_final_ = estate_balance;
        plan.legacy_initial_ = legacy_unassigned; plan.legacy_final_ = legacy_unassigned;
        plan.loans_initial_ = loans;
        plan.holdings_initial_ = holdings; plan.holdings_final_ = holdings;
        plan.use_rights_initial_ = use_rights; plan.use_rights_final_ = use_rights;
        plan.cases_initial_ = cases; plan.cases_final_ = cases;
        int64_t accounted = legacy_unassigned;
        uint64_t previous = 0;
        size_t case_index = cases.size();
        for (size_t i = 0; i < cases.size(); ++i) {
            const auto& item = cases[i];
            if (item.decedent == 0 || item.decedent <= previous || item.cash < 0 ||
                item.cash > std::numeric_limits<int64_t>::max() - accounted)
                throw std::logic_error("invalid estate distribution cases");
            for (const auto quantity : item.inventory)
                if (quantity < 0) throw std::logic_error("negative estate distribution inventory");
            accounted += item.cash;
            previous = item.decedent;
            if (item.decedent == decedent) case_index = i;
        }
        if (accounted != estate_balance || case_index == cases.size())
            throw std::logic_error("estate distribution ledger mismatch");
        const auto beneficiaries = resolve_beneficiaries(decedent, people, relations);
        if (beneficiaries.recipients.size() > static_cast<size_t>(std::numeric_limits<int64_t>::max()))
            throw std::overflow_error("too many estate beneficiaries");
        const auto& estate_case = cases[case_index];
        bool unpaid_claim = false;
        for (const auto loan_id : estate_case.loans) {
            const auto loan = std::find_if(loans.begin(), loans.end(), [loan_id, decedent](const Loan& value) {
                return value.id == loan_id && value.debtor == decedent;
            });
            if (loan == loans.end() || loan->principal <= 0 || loan->interest < 0 || loan->paid < 0 ||
                loan->interest > std::numeric_limits<int64_t>::max() - loan->principal ||
                loan->paid > loan->principal + loan->interest)
                throw std::logic_error("invalid estate land claim reference");
            if (loan->paid != loan->principal + loan->interest) unpaid_claim = true;
        }
        const bool sole_heir = beneficiaries.recipients.size() == 1;
        const bool sole_adult_heir = sole_heir &&
            people.at(static_cast<size_t>(beneficiaries.recipients.front() - 1)).age_days >= 18 * 365;
        if (sole_heir && !unpaid_claim) {
            const auto heir = beneficiaries.recipients.front();
            if (sole_adult_heir) {
                for (const auto establishment_id : estate_case.establishments) {
                    const auto site = std::find_if(establishments.begin(), establishments.end(),
                        [establishment_id](const Establishment& value) { return value.id == establishment_id; });
                    if (site == establishments.end() || site->owner_kind != EstablishmentOwnerKind::Person ||
                        site->owner_id != decedent)
                        throw std::logic_error("estate establishment reference mismatch");
                    plan.establishment_owners_initial_.emplace_back(site->id, site->owner_id);
                    plan.establishments_to_transfer_.push_back(site->id);
                }
                plan.cases_final_[case_index].establishments.clear();
            }
            for (const auto holding_id : estate_case.land_holdings) {
                const auto holding = std::lower_bound(plan.holdings_final_.begin(), plan.holdings_final_.end(), holding_id,
                    [](const LandHolding& value, uint64_t id) { return value.id < id; });
                if (holding == plan.holdings_final_.end() || holding->id != holding_id || holding->title_holder != decedent)
                    throw std::logic_error("estate land title reference mismatch");
                holding->title_holder = heir;
                const auto right = std::find_if(plan.use_rights_final_.begin(), plan.use_rights_final_.end(),
                    [holding_id](const LandUseRight& value) { return value.holding_id == holding_id; });
                if (right != plan.use_rights_final_.end()) {
                    if (right->grantor != decedent) throw std::logic_error("estate land use grantor mismatch");
                    right->grantor = heir;
                }
            }
            plan.cases_final_[case_index].land_holdings.clear();
            plan.cases_final_[case_index].land_use_rights.clear();
        }
        const int64_t cash = estate_case.cash;
        const int64_t count = static_cast<int64_t>(beneficiaries.recipients.size());
        for (const auto id : beneficiaries.recipients) {
            const auto& recipient = people.at(static_cast<size_t>(id - 1));
            if (recipient.cash < 0) throw std::logic_error("negative estate recipient cash");
            plan.recipient_cash_initial_.emplace_back(id, recipient.cash);
            plan.recipient_cash_final_.emplace_back(id, recipient.cash);
            plan.recipient_inventory_initial_.emplace_back(id, recipient.inventory);
            plan.recipient_inventory_final_.emplace_back(id, recipient.inventory);
        }
        if (cash > 0) {
            plan.cases_final_[case_index].cash = 0;
            if (beneficiaries.recipients.empty()) {
                if (cash > std::numeric_limits<int64_t>::max() - legacy_unassigned)
                    throw std::overflow_error("public estate fund overflow");
                plan.legacy_final_ += cash;
            } else {
                const int64_t share = cash / count, remainder = cash % count;
                for (size_t i = 0; i < beneficiaries.recipients.size(); ++i) {
                    const auto id = beneficiaries.recipients[i];
                    const auto& recipient = people.at(static_cast<size_t>(id - 1));
                    const int64_t amount = share + (static_cast<int64_t>(i) < remainder ? 1 : 0);
                    if (amount > std::numeric_limits<int64_t>::max() - recipient.cash)
                        throw std::overflow_error("estate recipient cash overflow");
                    plan.recipient_cash_final_[i].second = recipient.cash + amount;
                }
                plan.estate_final_ -= cash;
            }
        }
        const auto& inventory = estate_case.inventory;
        if (beneficiaries.recipients.empty()) {
            if (estate_case.region < 0 || static_cast<size_t>(estate_case.region) >= provinces.size())
                throw std::logic_error("invalid public estate goods region");
            plan.public_region_ = estate_case.region;
            plan.public_stock_initial_ = provinces[static_cast<size_t>(plan.public_region_)].stock;
            plan.public_stock_final_ = plan.public_stock_initial_;
            for (size_t good = 0; good < kGoodIdCount; ++good) {
                const int64_t quantity = inventory[good];
                const int64_t stock = plan.public_stock_initial_[good];
                if (stock < 0) throw std::logic_error("negative public estate goods stock");
                if (quantity > std::numeric_limits<int64_t>::max() - stock)
                    throw std::overflow_error("public estate goods stock overflow");
                plan.public_stock_final_[good] += quantity;
            }
            plan.cases_final_[case_index].inventory.fill(0);
        } else {
            for (size_t i = 0; i < beneficiaries.recipients.size(); ++i) {
                const auto id = beneficiaries.recipients[i];
                const auto& recipient = people.at(static_cast<size_t>(id - 1));
                auto final_inventory = recipient.inventory;
                for (size_t good = 0; good < kGoodIdCount; ++good) {
                    if (recipient.inventory[good] < 0) throw std::logic_error("negative estate recipient goods");
                    const int64_t quantity = inventory[good] / count +
                        (static_cast<int64_t>(i) < inventory[good] % count ? 1 : 0);
                    if (quantity > std::numeric_limits<int64_t>::max() - recipient.inventory[good])
                        throw std::overflow_error("estate recipient goods overflow");
                    final_inventory[good] += quantity;
                }
                plan.recipient_inventory_final_[i].second = final_inventory;
            }
            plan.cases_final_[case_index].inventory.fill(0);
        }
        return plan;
    }

    static EstateDistributionPlan prepare_distribution(uint64_t decedent, int64_t estate_balance,
        int64_t legacy_unassigned, const std::vector<EstateCase>& cases,
        const std::vector<Person>& people, const std::vector<Edge>& relations,
        const std::vector<Province>& provinces) {
        static const std::vector<Loan> no_loans;
        static const std::vector<Establishment> no_establishments;
        static const std::vector<LandHolding> no_holdings;
        static const std::vector<LandUseRight> no_rights;
        return prepare_distribution(decedent, estate_balance, legacy_unassigned, cases, people, relations,
            provinces, no_loans, no_establishments, no_holdings, no_rights);
    }

    static EstateDistributionPlan prepare_distribution(uint64_t decedent, int64_t estate_balance,
        int64_t legacy_unassigned, const std::vector<EstateCase>& cases,
        const std::vector<Person>& people, const std::vector<Edge>& relations,
        const std::vector<Province>& provinces, const std::vector<Loan>& loans,
        const std::vector<LandHolding>& holdings, const std::vector<LandUseRight>& use_rights) {
        static const std::vector<Establishment> no_establishments;
        return prepare_distribution(decedent, estate_balance, legacy_unassigned, cases, people, relations,
            provinces, loans, no_establishments, holdings, use_rights);
    }

    static void apply_distribution(const EstateDistributionPlan& plan, int64_t& estate_balance,
        int64_t& legacy_unassigned, std::vector<EstateCase>& cases, std::vector<Person>& people,
        std::vector<Province>& provinces, std::vector<Loan>& loans,
        std::vector<Establishment>& establishments,
        std::vector<LandHolding>& holdings, std::vector<LandUseRight>& use_rights) {
        if (estate_balance != plan.estate_initial_ || legacy_unassigned != plan.legacy_initial_ ||
            cases != plan.cases_initial_ || holdings != plan.holdings_initial_ ||
            use_rights != plan.use_rights_initial_)
            throw std::logic_error("stale estate distribution plan");
        if (loans.size() != plan.loans_initial_.size()) throw std::logic_error("stale estate distribution loans");
        for (size_t i = 0; i < loans.size(); ++i) {
            const auto& actual = loans[i];
            const auto& expected = plan.loans_initial_[i];
            if (actual.id != expected.id || actual.debtor != expected.debtor || actual.creditor != expected.creditor ||
                actual.due != expected.due || actual.principal != expected.principal || actual.interest != expected.interest ||
                actual.paid != expected.paid || actual.defaulted != expected.defaulted ||
                actual.royal_pool != expected.royal_pool || actual.cause != expected.cause)
                throw std::logic_error("stale estate distribution loan");
        }
        for (const auto& [id, expected_owner] : plan.establishment_owners_initial_) {
            const auto site = std::find_if(establishments.begin(), establishments.end(),
                [id](const Establishment& value) { return value.id == id; });
            if (site == establishments.end() || site->owner_kind != EstablishmentOwnerKind::Person ||
                site->owner_id != expected_owner || expected_owner != plan.decedent_)
                throw std::logic_error("stale estate establishment owner");
        }
        if (plan.recipient_cash_initial_.size() != plan.recipient_cash_final_.size() ||
            plan.recipient_inventory_initial_.size() != plan.recipient_inventory_final_.size() ||
            plan.recipient_cash_initial_.size() != plan.recipient_inventory_initial_.size())
            throw std::logic_error("invalid estate recipient plan");
        for (size_t i = 0; i < plan.recipient_cash_initial_.size(); ++i) {
            const auto [id, expected_cash] = plan.recipient_cash_initial_[i];
            if (id == 0 || id > people.size() || people[id - 1].id != id || !people[id - 1].alive ||
                people[id - 1].cash != expected_cash || plan.recipient_inventory_initial_[i].first != id ||
                plan.recipient_inventory_final_[i].first != id ||
                people[id - 1].inventory != plan.recipient_inventory_initial_[i].second)
                throw std::logic_error("stale estate beneficiary");
        }
        if (plan.public_region_ >= 0 &&
            (static_cast<size_t>(plan.public_region_) >= provinces.size() ||
             provinces[static_cast<size_t>(plan.public_region_)].stock != plan.public_stock_initial_))
            throw std::logic_error("stale public estate goods stock");
        auto staged_cases = plan.cases_final_;
        auto staged_holdings = plan.holdings_final_;
        auto staged_use_rights = plan.use_rights_final_;
        for (size_t i = 0; i < plan.recipient_cash_final_.size(); ++i) {
            const auto id = plan.recipient_cash_final_[i].first;
            people[id - 1].cash = plan.recipient_cash_final_[i].second;
            people[id - 1].inventory = plan.recipient_inventory_final_[i].second;
        }
        for (const auto id : plan.establishments_to_transfer_) {
            const auto site = std::find_if(establishments.begin(), establishments.end(),
                [id](const Establishment& value) { return value.id == id; });
            site->owner_id = plan.recipient_cash_final_.front().first;
        }
        if (plan.public_region_ >= 0)
            provinces[static_cast<size_t>(plan.public_region_)].stock = plan.public_stock_final_;
        estate_balance = plan.estate_final_; legacy_unassigned = plan.legacy_final_;
        cases.swap(staged_cases);
        holdings.swap(staged_holdings);
        use_rights.swap(staged_use_rights);
    }

    static void apply_distribution(const EstateDistributionPlan& plan, int64_t& estate_balance,
        int64_t& legacy_unassigned, std::vector<EstateCase>& cases, std::vector<Person>& people,
        std::vector<Province>& provinces) {
        static std::vector<Loan> no_loans;
        static std::vector<Establishment> no_establishments;
        static std::vector<LandHolding> no_holdings;
        static std::vector<LandUseRight> no_rights;
        apply_distribution(plan, estate_balance, legacy_unassigned, cases, people, provinces,
            no_loans, no_establishments, no_holdings, no_rights);
    }

    static void apply_distribution(const EstateDistributionPlan& plan, int64_t& estate_balance,
        int64_t& legacy_unassigned, std::vector<EstateCase>& cases, std::vector<Person>& people,
        std::vector<Province>& provinces, std::vector<Loan>& loans,
        std::vector<LandHolding>& holdings, std::vector<LandUseRight>& use_rights) {
        static std::vector<Establishment> no_establishments;
        apply_distribution(plan, estate_balance, legacy_unassigned, cases, people, provinces,
            loans, no_establishments, holdings, use_rights);
    }

    // Pays every outstanding loan referenced by one estate case only when that
    // case's cash can cover all claims. This avoids inventing creditor priority
    // when the estate cannot pay in full.
    static EstateCashClaimPlan prepare_cash_claims(uint64_t decedent, int64_t estate_balance,
        int64_t legacy_unassigned, int64_t credit_pool, const std::vector<EstateCase>& cases,
        const std::vector<Loan>& loans, const std::vector<Province>& provinces) {
        if (decedent == 0 || estate_balance < 0 || legacy_unassigned < 0 || credit_pool < 0)
            throw std::logic_error("invalid estate claim input");
        EstateCashClaimPlan plan;
        plan.decedent_ = decedent; plan.estate_initial_ = estate_balance; plan.estate_final_ = estate_balance;
        plan.legacy_initial_ = legacy_unassigned; plan.credit_pool_initial_ = credit_pool; plan.credit_pool_final_ = credit_pool;
        plan.cases_initial_ = cases; plan.cases_final_ = cases; plan.loans_initial_ = loans; plan.loans_final_ = loans;
        plan.province_firms_initial_.reserve(provinces.size());
        for (const auto& province : provinces) {
            if (province.firm < 0) throw std::logic_error("negative estate creditor account");
            plan.province_firms_initial_.push_back(province.firm);
        }
        plan.province_firms_final_ = plan.province_firms_initial_;

        int64_t accounted = legacy_unassigned;
        uint64_t previous_decedent = 0;
        size_t case_index = cases.size();
        for (size_t i = 0; i < cases.size(); ++i) {
            const auto& item = cases[i];
            if (item.decedent == 0 || item.decedent <= previous_decedent || item.cash < 0 ||
                item.cash > std::numeric_limits<int64_t>::max() - accounted)
                throw std::logic_error("invalid estate claim cases");
            for (const auto quantity : item.inventory) if (quantity < 0) throw std::logic_error("negative estate inventory");
            accounted += item.cash;
            previous_decedent = item.decedent;
            if (item.decedent == decedent) case_index = i;
        }
        if (accounted != estate_balance || case_index == cases.size()) throw std::logic_error("estate claim ledger mismatch");

        std::vector<uint64_t> loan_ids;
        loan_ids.reserve(loans.size());
        for (const auto& loan : loans) {
            if (loan.id == 0 || loan.principal <= 0 || loan.interest < 0 ||
                loan.interest > std::numeric_limits<int64_t>::max() - loan.principal || loan.paid < 0 ||
                loan.paid > loan.principal + loan.interest || loan.creditor < 0 ||
                static_cast<size_t>(loan.creditor) >= provinces.size())
                throw std::logic_error("invalid estate loan record");
            loan_ids.push_back(loan.id);
        }
        std::sort(loan_ids.begin(), loan_ids.end());
        if (std::adjacent_find(loan_ids.begin(), loan_ids.end()) != loan_ids.end())
            throw std::logic_error("duplicate estate loan ID");

        const auto& estate_case = cases[case_index];
        if (!std::is_sorted(estate_case.loans.begin(), estate_case.loans.end()) ||
            std::adjacent_find(estate_case.loans.begin(), estate_case.loans.end()) != estate_case.loans.end())
            throw std::logic_error("noncanonical estate loan references");
        int64_t total_due = 0;
        for (const auto id : estate_case.loans) {
            const auto found = std::find_if(loans.begin(), loans.end(), [id](const Loan& loan) { return loan.id == id; });
            if (found == loans.end() || found->debtor != decedent) throw std::logic_error("estate loan reference mismatch");
            const int64_t due = found->principal + found->interest - found->paid;
            if (due > std::numeric_limits<int64_t>::max() - total_due) throw std::overflow_error("estate claims overflow");
            total_due += due;
        }
        if (total_due == 0) { plan.noop_ = true; return plan; }
        if (estate_case.cash < total_due) { plan.noop_ = true; return plan; }

        for (auto& item : plan.cases_final_) if (item.decedent == decedent) item.cash -= total_due;
        plan.estate_final_ -= total_due;
        for (const auto id : estate_case.loans) {
            const auto found = std::find_if(plan.loans_final_.begin(), plan.loans_final_.end(), [id](const Loan& loan) { return loan.id == id; });
            const int64_t due = found->principal + found->interest - found->paid;
            if (found->royal_pool) {
                if (due > std::numeric_limits<int64_t>::max() - plan.credit_pool_final_)
                    throw std::overflow_error("estate credit pool overflow");
                plan.credit_pool_final_ += due;
            } else {
                const auto region = static_cast<size_t>(found->creditor);
                if (due > std::numeric_limits<int64_t>::max() - plan.province_firms_final_[region])
                    throw std::overflow_error("estate creditor balance overflow");
                plan.province_firms_final_[region] += due;
            }
            found->paid += due;
        }
        return plan;
    }

    static void apply_cash_claims(const EstateCashClaimPlan& plan, int64_t& estate_balance,
        int64_t legacy_unassigned, int64_t& credit_pool, std::vector<EstateCase>& cases,
        std::vector<Loan>& loans, std::vector<Province>& provinces) {
        const auto loans_equal = [](const std::vector<Loan>& left, const std::vector<Loan>& right) {
            if (left.size() != right.size()) return false;
            for (size_t i = 0; i < left.size(); ++i) {
                const auto& a = left[i]; const auto& b = right[i];
                if (a.id != b.id || a.debtor != b.debtor || a.creditor != b.creditor || a.due != b.due ||
                    a.principal != b.principal || a.interest != b.interest || a.paid != b.paid ||
                    a.defaulted != b.defaulted || a.royal_pool != b.royal_pool || a.cause != b.cause) return false;
            }
            return true;
        };
        if (estate_balance != plan.estate_initial_ || legacy_unassigned != plan.legacy_initial_ ||
            credit_pool != plan.credit_pool_initial_ || cases != plan.cases_initial_ ||
            !loans_equal(loans, plan.loans_initial_) || provinces.size() != plan.province_firms_initial_.size())
            throw std::logic_error("stale estate cash claim plan");
        for (size_t i = 0; i < provinces.size(); ++i)
            if (provinces[i].firm != plan.province_firms_initial_[i]) throw std::logic_error("stale estate creditor balance");
        if (plan.noop_) return;
        auto staged_cases = plan.cases_final_;
        auto staged_loans = plan.loans_final_;
        for (size_t i = 0; i < provinces.size(); ++i) provinces[i].firm = plan.province_firms_final_[i];
        estate_balance = plan.estate_final_; credit_pool = plan.credit_pool_final_;
        cases.swap(staged_cases); loans.swap(staged_loans);
    }

    static EstateBeneficiaries resolve_beneficiaries(uint64_t deceased,
                                                     const std::vector<Person>& people,
                                                     const std::vector<Edge>& relations) {
        if (deceased == 0 || deceased > people.size() || people[deceased - 1].id != deceased || people[deceased - 1].alive)
            throw std::invalid_argument("invalid estate decedent");
        for (size_t i = 0; i < people.size(); ++i)
            if (people[i].id != i + 1) throw std::invalid_argument("noncanonical estate person IDs");
        for (const auto& edge : relations)
            if (edge.from == 0 || edge.to == 0 || edge.from > people.size() || edge.to > people.size() ||
                edge.kind > RelationKind::Spouse || (edge.legal_parentage &&
                (edge.kind != RelationKind::ParentChild || edge.from == edge.to)))
                throw std::invalid_argument("invalid estate relation edge");
        const auto living = [&](uint64_t id) { return id != deceased && people[id - 1].alive; };
        const auto result = [](EstateBeneficiaryTier tier, const std::set<uint64_t>& ids) {
            return EstateBeneficiaries{tier, std::vector<uint64_t>(ids.begin(), ids.end())};
        };
        std::set<uint64_t> candidates;
        for (const auto& edge : relations) if (edge.kind == RelationKind::ParentChild && edge.legal_parentage &&
            edge.to == deceased && living(edge.from)) candidates.insert(edge.from);
        if (!candidates.empty()) return result(EstateBeneficiaryTier::Children, candidates);
        for (const auto& edge : relations) if (edge.kind == RelationKind::Spouse) {
            if (edge.from == deceased && living(edge.to)) candidates.insert(edge.to);
            if (edge.to == deceased && living(edge.from)) candidates.insert(edge.from);
        }
        if (!candidates.empty()) return result(EstateBeneficiaryTier::Spouse, candidates);
        std::set<uint64_t> parents;
        for (const auto& edge : relations) if (edge.kind == RelationKind::ParentChild && edge.legal_parentage &&
            edge.from == deceased && edge.to != deceased) parents.insert(edge.to);
        for (auto parent : parents) if (living(parent)) candidates.insert(parent);
        if (!candidates.empty()) return result(EstateBeneficiaryTier::Parents, candidates);
        candidates.clear();
        for (const auto& edge : relations) {
            if (edge.kind == RelationKind::ParentChild && edge.legal_parentage &&
                parents.contains(edge.to) && living(edge.from)) candidates.insert(edge.from);
        }
        candidates.erase(deceased);
        if (!candidates.empty()) return result(EstateBeneficiaryTier::Siblings, candidates);
        return {};
    }

    static EstateDeathPlan prepare_death(const Person& person, int day, int64_t estate_balance,
        int64_t legacy_unassigned, const std::vector<EstateCase>& existing_cases,
        const std::vector<Establishment>& establishments, const std::vector<LandHolding>& holdings,
        const std::vector<LandUseRight>& rights, const std::vector<Loan>& loans) {
        if (!person.alive || person.id == 0 || person.region < 0 || day < 0 || person.cash < 0 || estate_balance < 0 || legacy_unassigned < 0)
            throw std::logic_error("invalid estate death input");
        int64_t accounted = legacy_unassigned;
        uint64_t previous = 0;
        for (const auto& item : existing_cases) {
            if (item.decedent == 0 || item.decedent <= previous || item.decedent == person.id || item.cash < 0 ||
                item.cash > std::numeric_limits<int64_t>::max() - accounted)
                throw std::logic_error("invalid existing estate cases");
            accounted += item.cash;
            previous = item.decedent;
        }
        if (accounted != estate_balance) throw std::logic_error("estate ledger mismatch");
        if (estate_balance > std::numeric_limits<int64_t>::max() - person.cash) throw std::overflow_error("estate balance overflow");
        for (const auto quantity : person.inventory) if (quantity < 0) throw std::logic_error("negative estate goods");

        EstateCase record{};
        record.decedent = person.id; record.opened_day = day; record.region = person.region;
        record.cash = person.cash; record.inventory = person.inventory;
        for (const auto& site : establishments)
            if (site.owner_kind == EstablishmentOwnerKind::Person && site.owner_id == person.id) record.establishments.push_back(site.id);
        for (const auto& holding : holdings) if (holding.title_holder == person.id) record.land_holdings.push_back(holding.id);
        for (const auto& right : rights) if (right.grantor == person.id) record.land_use_rights.push_back(right.id);
        for (const auto& loan : loans) {
            if (loan.debtor == person.id && loan.principal > 0 && loan.interest >= 0 &&
                loan.principal <= std::numeric_limits<int64_t>::max() - loan.interest &&
                loan.principal + loan.interest > loan.paid) record.loans.push_back(loan.id);
        }
        std::sort(record.establishments.begin(), record.establishments.end());
        std::sort(record.land_holdings.begin(), record.land_holdings.end());
        std::sort(record.land_use_rights.begin(), record.land_use_rights.end());
        std::sort(record.loans.begin(), record.loans.end());
        EstateDeathPlan plan;
        plan.person_id_ = person.id; plan.day_ = day; plan.region_ = person.region;
        plan.cash_initial_ = person.cash; plan.inventory_initial_ = person.inventory;
        plan.estate_initial_ = estate_balance; plan.estate_final_ = estate_balance + person.cash;
        plan.cases_initial_ = existing_cases; plan.cases_final_ = existing_cases;
        plan.cases_final_.push_back(std::move(record));
        std::sort(plan.cases_final_.begin(), plan.cases_final_.end(), [](const auto& a, const auto& b) { return a.decedent < b.decedent; });
        return plan;
    }

    static void apply_death(const EstateDeathPlan& plan, Person& person, int64_t& estate_balance,
                            std::vector<EstateCase>& cases) {
        if (person.id != plan.person_id_ || !person.alive || person.region != plan.region_ || person.cash != plan.cash_initial_ ||
            person.inventory != plan.inventory_initial_ || estate_balance != plan.estate_initial_ || cases != plan.cases_initial_)
            throw std::logic_error("stale estate death plan");
        auto staged_cases = plan.cases_final_;
        person.alive = false; person.cash = 0; person.inventory.fill(0);
        estate_balance = plan.estate_final_; cases.swap(staged_cases);
    }
};

static_assert(!std::is_default_constructible_v<EstateDeathPlan>, "EstateDeathPlan must only be produced by EstateManager");

} // namespace polyec
