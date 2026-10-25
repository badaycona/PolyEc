#include "polyec/realm.hpp"
#include "polyec/managers/finance_manager.hpp"
#include <algorithm>
#include <limits>
#include <type_traits>

namespace polyec {
namespace {
template<class T>void reserve_append(std::vector<T>& records,size_t limit){
    if(records.size()>=limit)throw std::length_error("record count limit");
    if(records.size()==records.capacity()){
        const size_t grown=records.capacity()>limit/2?limit:std::max<size_t>(8,records.capacity()*2);
        records.reserve(std::min(limit,grown));
    }
}
}
bool Realm::manages_establishment(const Person& actor,const Establishment& site)const{
    if(!actor.alive||actor.destination!=-1||actor.region!=site.region)return false;
    switch(site.owner_kind){
    case EstablishmentOwnerKind::Person:return site.owner_id==actor.id;
    case EstablishmentOwnerKind::Public:return actor.family==Family::Governance&&has_local_government_authority(actor)&&
        provinces_[site.region].charter&&site.owner_id==uint64_t(site.region);
    case EstablishmentOwnerKind::Municipal:return actor.family==Family::Governance&&has_local_government_authority(actor)&&
        provinces_[site.region].charter&&site.owner_id==uint64_t(site.region);
    case EstablishmentOwnerKind::Organization:return site.owner_id>=1&&site.owner_id<=kOrganizationCount&&
        actor.organization==int(site.owner_id)&&site.region==int((site.owner_id-1)/kOrganizationGroupsPerTerritory);
    default:return false;
    }
}
RealmResult Realm::execute(const EstablishmentCommand& cmd){
    if (cmd.actor == 0 || cmd.actor > people_.size()) return {false, "Unknown actor"};
    auto& p = mutable_person(cmd.actor);
    if (p.decisions == UINT64_MAX || p.sampling == UINT64_MAX) return {false, "Decision counter overflow"};
    if (cmd.sequence != p.decisions + 1) return {false, "Stale or duplicate decision ID"};
    if (!p.alive) return {false, "Actor deceased"};
    if (cmd.region < 0 || cmd.region > regions_) return {false, "Invalid region"};
    if (cmd.owner_kind != EstablishmentOwnerKind::Person &&
        cmd.owner_kind != EstablishmentOwnerKind::Public &&
        cmd.owner_kind != EstablishmentOwnerKind::Organization &&
        cmd.owner_kind != EstablishmentOwnerKind::Municipal) return {false, "Invalid owner kind"};
    if (cmd.industry != EstablishmentIndustry::ClothWorkshop &&
        cmd.industry != EstablishmentIndustry::ToolWorkshop &&
        cmd.industry != EstablishmentIndustry::MilitarySupplyWorkshop &&
        cmd.industry != EstablishmentIndustry::Saltworks &&
        cmd.industry != EstablishmentIndustry::Forestry &&
        cmd.industry != EstablishmentIndustry::Mining &&
        cmd.industry != EstablishmentIndustry::PaperMill &&
        cmd.industry != EstablishmentIndustry::TeaGarden &&
        cmd.industry != EstablishmentIndustry::IncenseWorks &&
        cmd.industry != EstablishmentIndustry::WarhorseStable &&
        cmd.industry != EstablishmentIndustry::WeaponForge &&
        cmd.industry != EstablishmentIndustry::Glassworks &&
        cmd.industry != EstablishmentIndustry::PotteryKiln &&
        cmd.industry != EstablishmentIndustry::Tannery &&
        cmd.industry != EstablishmentIndustry::MedicinalGarden &&
        cmd.industry != EstablishmentIndustry::SeedPressWorks &&
        cmd.industry != EstablishmentIndustry::SawmillWorks &&
        cmd.industry != EstablishmentIndustry::Apiary &&
        cmd.industry != EstablishmentIndustry::SilkwormFarm &&
        cmd.industry != EstablishmentIndustry::GoldMine &&
        cmd.industry != EstablishmentIndustry::LivestockRanch &&
        cmd.industry != EstablishmentIndustry::Butchery &&
        cmd.industry != EstablishmentIndustry::DecorativeGlassWorks &&
        cmd.industry != EstablishmentIndustry::FinePorcelainWorks &&
        cmd.industry != EstablishmentIndustry::GeneralLuxuryAtelier) return {false, "Invalid industry"};
    if(cmd.industry==EstablishmentIndustry::Saltworks&&cmd.region!=8)return {false,"Saltworks requires atlas region 8"};
    if(cmd.industry==EstablishmentIndustry::Forestry){
        if(cmd.region!=6)return {false,"Forestry requires atlas region 6"};
        if(std::any_of(establishments_.begin(),establishments_.end(),[](const Establishment& site){
            return site.industry==EstablishmentIndustry::Forestry&&site.region==6;
        }))return {false,"Forestry resource site already exists"};
    }
    if(cmd.industry==EstablishmentIndustry::Mining){
        if(cmd.region!=7&&cmd.region!=23)return {false,"Mining requires an atlas region with a defined metal source"};
        if(std::any_of(establishments_.begin(),establishments_.end(),[&](const Establishment& site){
            return site.industry==EstablishmentIndustry::Mining&&site.region==cmd.region;
        }))return {false,"Mining source site already exists in this region"};
    }
    if(cmd.industry==EstablishmentIndustry::PaperMill){
        if(cmd.region!=13)return {false,"Paper Mill requires atlas region 13"};
        if(std::any_of(establishments_.begin(),establishments_.end(),[](const Establishment& site){
            return site.industry==EstablishmentIndustry::PaperMill&&site.region==13;
        }))return {false,"Paper Mill already exists in region 13"};
    }
    if(cmd.industry==EstablishmentIndustry::TeaGarden){
        if(cmd.region!=14)return {false,"Tea Garden requires atlas region 14"};
        if(std::any_of(establishments_.begin(),establishments_.end(),[](const Establishment& site){
            return site.industry==EstablishmentIndustry::TeaGarden&&site.region==14;
        }))return {false,"Tea Garden source site already exists in region 14"};
    }
    if(cmd.industry==EstablishmentIndustry::IncenseWorks){
        if(cmd.region!=11)return {false,"Incense Works requires atlas region 11"};
        if(std::any_of(establishments_.begin(),establishments_.end(),[](const Establishment& site){
            return site.industry==EstablishmentIndustry::IncenseWorks&&site.region==11;
        }))return {false,"Incense source site already exists in region 11"};
    }
    if(cmd.industry==EstablishmentIndustry::WarhorseStable){
        if(cmd.region!=16)return {false,"Warhorse Stable requires atlas region 16"};
        if(std::any_of(establishments_.begin(),establishments_.end(),[](const Establishment& site){
            return site.industry==EstablishmentIndustry::WarhorseStable&&site.region==16;
        }))return {false,"Warhorse Stable already exists in region 16"};
    }
    if(cmd.industry==EstablishmentIndustry::WeaponForge){
        if(cmd.region!=19)return {false,"Weapon Forge requires atlas region 19"};
        if(std::any_of(establishments_.begin(),establishments_.end(),[](const Establishment& site){
            return site.industry==EstablishmentIndustry::WeaponForge&&site.region==19;
        }))return {false,"Weapon Forge already exists in region 19"};
    }
    if(cmd.industry==EstablishmentIndustry::Glassworks){
        if(cmd.region!=10)return {false,"Glassworks requires atlas region 10"};
        if(std::any_of(establishments_.begin(),establishments_.end(),[](const Establishment& site){
            return site.industry==EstablishmentIndustry::Glassworks&&site.region==10;
        }))return {false,"Glassworks source site already exists in region 10"};
    }
    if(cmd.industry==EstablishmentIndustry::PotteryKiln){
        if(cmd.region!=15)return {false,"Pottery Kiln requires atlas region 15"};
        if(std::any_of(establishments_.begin(),establishments_.end(),[](const Establishment& site){
            return site.industry==EstablishmentIndustry::PotteryKiln&&site.region==15;
        }))return {false,"Pottery source site already exists in region 15"};
    }
    if(cmd.industry==EstablishmentIndustry::Tannery){
        if(cmd.region!=17)return {false,"Tannery requires atlas region 17"};
        if(std::any_of(establishments_.begin(),establishments_.end(),[](const Establishment& site){
            return site.industry==EstablishmentIndustry::Tannery&&site.region==17;
        }))return {false,"Tannery source site already exists in region 17"};
    }
    if(cmd.industry==EstablishmentIndustry::MedicinalGarden){
        if(cmd.region!=20)return {false,"Medicinal Garden requires Atlas region 20"};
        if(std::any_of(establishments_.begin(),establishments_.end(),[](const Establishment& site){
            return site.industry==EstablishmentIndustry::MedicinalGarden&&site.region==20;
        }))return {false,"Medicinal source site already exists in region 20"};
    }
    if(cmd.industry==EstablishmentIndustry::SeedPressWorks){
        if(cmd.region!=2)return {false,"Seed Press Works requires Atlas region 2"};
        if(std::any_of(establishments_.begin(),establishments_.end(),[](const Establishment& site){
            return site.industry==EstablishmentIndustry::SeedPressWorks&&site.region==2;
        }))return {false,"SupplementaryFood source site already exists in region 2"};
    }
    if(cmd.industry==EstablishmentIndustry::SawmillWorks){
        if(regions_!=int(kAtlasTerritoryCount)-1||cmd.region!=6)return {false,"Sawmill Works requires Atlas region 6"};
        if(std::any_of(establishments_.begin(),establishments_.end(),[](const Establishment& site){
            return site.industry==EstablishmentIndustry::SawmillWorks&&site.region==6;
        }))return {false,"Sawmill source site already exists in region 6"};
    }
    if(cmd.industry==EstablishmentIndustry::Apiary){
        if(regions_!=int(kAtlasTerritoryCount)-1||cmd.region!=21)
            return {false,"Apiary requires Atlas region 21"};
        if(std::any_of(establishments_.begin(),establishments_.end(),[](const Establishment& site){
            return site.industry==EstablishmentIndustry::Apiary&&site.region==21;
        }))return {false,"Beeswax source site already exists in region 21"};
    }
    if(cmd.industry==EstablishmentIndustry::SilkwormFarm){
        if(regions_!=int(kAtlasTerritoryCount)-1||cmd.region!=12)
            return {false,"Silkworm Farm requires Atlas region 12"};
        if(std::any_of(establishments_.begin(),establishments_.end(),[](const Establishment& site){
            return site.industry==EstablishmentIndustry::SilkwormFarm&&site.region==12;
        }))return {false,"Silk source site already exists in region 12"};
    }
    if(cmd.industry==EstablishmentIndustry::GoldMine){
        if(regions_!=int(kAtlasTerritoryCount)-1||cmd.region!=23)
            return {false,"Gold Mine requires Atlas region 23"};
        if(std::any_of(establishments_.begin(),establishments_.end(),[](const Establishment& site){
            return site.industry==EstablishmentIndustry::GoldMine&&site.region==23;
        }))return {false,"Gold source site already exists in region 23"};
    }
    if(cmd.industry==EstablishmentIndustry::LivestockRanch||cmd.industry==EstablishmentIndustry::Butchery){
        if(regions_!=int(kAtlasTerritoryCount)-1||cmd.region!=18)
            return {false,"Livestock Ranch and Butchery require Atlas region 18"};
        if(std::any_of(establishments_.begin(),establishments_.end(),[&](const Establishment& site){
            return site.industry==cmd.industry&&site.region==18;
        }))return {false,cmd.industry==EstablishmentIndustry::LivestockRanch?
            "Livestock Ranch already exists in region 18":"Butchery already exists in region 18"};
    }
    if(cmd.industry==EstablishmentIndustry::DecorativeGlassWorks){
        if(regions_!=int(kAtlasTerritoryCount)-1||cmd.region!=10)
            return {false,"Decorative Glass Works requires Atlas region 10"};
        if(std::any_of(establishments_.begin(),establishments_.end(),[](const Establishment& site){
            return site.industry==EstablishmentIndustry::DecorativeGlassWorks&&site.region==10;
        }))return {false,"Decorative Glass Works already exists in region 10"};
    }
    if(cmd.industry==EstablishmentIndustry::GeneralLuxuryAtelier){
        if(regions_!=int(kAtlasTerritoryCount)-1||cmd.region!=12)
            return {false,"General Luxury Atelier requires Atlas region 12"};
        if(std::any_of(establishments_.begin(),establishments_.end(),[](const Establishment& site){
            return site.industry==EstablishmentIndustry::GeneralLuxuryAtelier&&site.region==12;
        }))return {false,"General Luxury Atelier already exists in region 12"};
    }
    if(cmd.industry==EstablishmentIndustry::FinePorcelainWorks){
        if(regions_!=int(kAtlasTerritoryCount)-1||cmd.region!=15)
            return {false,"Fine Porcelain Works requires Atlas region 15"};
        if(std::any_of(establishments_.begin(),establishments_.end(),[](const Establishment& site){
            return site.industry==EstablishmentIndustry::FinePorcelainWorks&&site.region==15;
        }))return {false,"Fine Porcelain Works already exists in region 15"};
    }
    const int cap = cmd.capacity==-1?config_.default_capacity():cmd.capacity;
    if (cap < 1 || cap > kEstablishmentCapacityMax) return {false, "Invalid capacity"};
    if (cmd.funding < 1) return {false, "Insufficient funding"};
    if (next_id_ == UINT64_MAX) return {false, "ID counter overflow"};
    if (next_fact_ == UINT64_MAX) return {false, "Fact counter overflow"};
    if (establishments_.size() >= 1000000) return {false, "Establishment count limit"};

    // Resolve source and validate authorization.
    int64_t* source = nullptr;
    uint64_t ownerId = cmd.owner_id;
    if (cmd.owner_kind == EstablishmentOwnerKind::Person) {
        if (ownerId != cmd.actor) return {false, "Person owner must be actor"};
        source = &p.cash;
    } else if (cmd.owner_kind == EstablishmentOwnerKind::Public) {
        if (p.family != Family::Governance) return {false, "Public owner requires governance"};
        if (!has_local_government_authority(p)) return {false, "Public owner requires local authority and an unoccupied region"};
        if (p.region != cmd.region) return {false, "Public owner region mismatch"};
        if (!provinces_[cmd.region].charter) return {false, "Public owner requires charter"};
        if (ownerId == 0) ownerId = static_cast<uint64_t>(cmd.region);
        if (ownerId != static_cast<uint64_t>(cmd.region)) return {false, "Public owner id mismatch"};
        source = &provinces_[cmd.region].treasury;
    } else if(cmd.owner_kind == EstablishmentOwnerKind::Organization) {
        if (ownerId < 1 || ownerId > kOrganizationCount) return {false, "Invalid organization id"};
        if (p.organization != static_cast<int>(ownerId)) return {false, "Actor not organization member"};
        if (cmd.region != static_cast<int>((ownerId - 1) / kOrganizationGroupsPerTerritory)) return {false, "Organization home region mismatch"};
        source = &organization_funds_[ownerId - 1];
    } else {
        if(p.family!=Family::Governance)return {false,"Municipal owner requires governance"};
        if(!has_local_government_authority(p))return {false,"Municipal owner requires local authority and an unoccupied region"};
        if(p.region!=cmd.region)return {false,"Municipal owner region mismatch"};
        if(!provinces_[cmd.region].charter)return {false,"Municipal owner requires charter"};
        if(ownerId==0)ownerId=uint64_t(cmd.region);
        if(ownerId!=uint64_t(cmd.region))return {false,"Municipal owner id mismatch"};
        source=&municipal_funds_[size_t(cmd.region)];
    }
    if (*source < cmd.funding) return {false, "Insufficient source funds"};

    // Prepare all potentially throwing allocations before money, IDs or facts.
    static_assert(std::is_copy_constructible_v<Establishment>);
    const uint64_t newId = next_id_;
    Establishment e{};
    e.id = newId;
    e.region = cmd.region;
    e.owner_kind = cmd.owner_kind;
    e.owner_id = ownerId;
    e.industry = cmd.industry;
    e.capacity = cap;
    e.cash = cmd.funding;
    const int sourceGood=establishment_source_good(cmd.industry,cmd.region);
    if(sourceGood!=kNoEstablishmentSourceGood){
        e.source_good=sourceGood;
        e.source_units=(cmd.industry==EstablishmentIndustry::TeaGarden||cmd.industry==EstablishmentIndustry::Apiary||
                        cmd.industry==EstablishmentIndustry::SilkwormFarm||cmd.industry==EstablishmentIndustry::LivestockRanch)?25:kEstablishmentSourceUnitsMaximum;
    }
    RealmResult result{true, "Committed", 0, newId};
    reserve_append(establishments_,1000000);
    if(cmd.owner_kind==EstablishmentOwnerKind::Person)
        reserve_append(personal_establishment_indices_[std::size_t(ownerId-1)],1000000);
    reserve_append(facts_,8193);
    // Stable IDs are monotonically allocated; append preserves ID order.
    // MSVC's map move may allocate. Copy append has the vector's strong
    // exception guarantee; if it fails no record or scalar has been published.
    // After it succeeds, every remaining operation is nonallocating.
    establishments_.push_back(e);
    if(cmd.owner_kind==EstablishmentOwnerKind::Person)
        personal_establishment_indices_[std::size_t(ownerId-1)].push_back(establishments_.size()-1);
    *source -= cmd.funding;
    ++next_id_;
    p.last_decision_day = day_;
    ++p.decisions;
    ++p.sampling;
    event(p.id, cmd.region, 203, cmd.funding, newId);
    return result;
}

RealmResult Realm::execute(const MunicipalBudgetCommand& cmd){
    if(cmd.actor==0||cmd.actor>people_.size())return {false,"Unknown actor"};
    auto& actor=mutable_person(cmd.actor);
    if(actor.decisions==UINT64_MAX||actor.sampling==UINT64_MAX)return {false,"Decision counter overflow"};
    if(cmd.sequence!=actor.decisions+1)return {false,"Stale or duplicate decision ID"};
    if(!actor.alive)return {false,"Actor deceased"};
    if(cmd.region<0||cmd.region>regions_)return {false,"Invalid region"};
    if(actor.family!=Family::Governance||!has_local_government_authority(actor))return {false,"Municipal fund requires local governance authority in an unoccupied region"};
    if(actor.region!=cmd.region)return {false,"Municipal fund region mismatch"};
    if(!provinces_[size_t(cmd.region)].charter)return {false,"Municipal fund requires charter"};
    if(actor.destination!=-1||actor.busy_until>day_)return {false,"Actor is unavailable"};
    if(cmd.amount<=0)return {false,"Municipal transfer must be positive"};
    const int period=day_/30;
    if(municipal_last_funded_period_[size_t(cmd.region)]==period)return {false,"Municipal fund already received this period's grant"};
    const auto treasury=provinces_[size_t(cmd.region)].treasury;
    const int64_t limit=treasury/20; // floor(treasury * 5%); division avoids overflow.
    if(cmd.amount>limit)return {false,"Municipal grant exceeds 5% of the current treasury"};
    if(treasury<cmd.amount)return {false,"Insufficient regional treasury"};
    if(next_fact_==UINT64_MAX)return {false,"Fact counter overflow"};

    transfer(provinces_[size_t(cmd.region)].treasury,municipal_funds_[size_t(cmd.region)],cmd.amount);
    municipal_last_funded_period_[size_t(cmd.region)]=period;
    actor.last_decision_day=day_;
    ++actor.decisions;
    ++actor.sampling;
    event(actor.id,cmd.region,229,cmd.amount,uint64_t(period));
    return {true,"Committed",next_fact_-1};
}

RealmResult Realm::execute(const EstablishmentTransferCommand& cmd){
    if(cmd.actor==0||cmd.actor>people_.size())return {false,"Unknown actor"};
    auto& actor=mutable_person(cmd.actor);
    if(actor.decisions==UINT64_MAX||actor.sampling==UINT64_MAX)return {false,"Decision counter overflow"};
    if(cmd.sequence!=actor.decisions+1)return {false,"Stale or duplicate decision ID"};
    if(!actor.alive)return {false,"Actor deceased"};
    const auto found=std::lower_bound(establishments_.begin(),establishments_.end(),cmd.establishment_id,
        [](const Establishment& site,uint64_t id){return site.id<id;});
    if(found==establishments_.end()||found->id!=cmd.establishment_id)return {false,"Unknown establishment"};
    auto& site=*found;
    if(actor.destination!=-1||actor.region!=site.region)return {false,"Actor not present at establishment"};
    if(!manages_establishment(actor,site))return {false,"Actor cannot manage establishment"};
    const int operation=static_cast<int>(cmd.operation);
    if(operation<0||operation>3)return {false,"Invalid transfer operation"};
    const bool cash=operation<2;
    const bool deposit=operation==0||operation==2;
    if((cash&&cmd.good!=-1)||(!cash&&(cmd.good<0||cmd.good>=int(kGoodIdCount))))return {false,"Invalid transfer good"};
    if(cmd.amount<=0)return {false,"Transfer amount must be positive"};
    if(next_fact_==UINT64_MAX)return {false,"Fact counter overflow"};
    int64_t* external=nullptr;
    switch(site.owner_kind){
    case EstablishmentOwnerKind::Person:
        external=cash?&actor.cash:&actor.inventory[size_t(cmd.good)];
        break;
    case EstablishmentOwnerKind::Public:{
        external=cash?&provinces_[site.region].treasury:&provinces_[site.region].stock[size_t(cmd.good)];
        break;
    }
    case EstablishmentOwnerKind::Municipal:
        external=cash?&municipal_funds_[size_t(site.region)]:&provinces_[site.region].stock[size_t(cmd.good)];
        break;
    case EstablishmentOwnerKind::Organization:
        if(!cash)return {false,"Organization inventory is not available"};
        external=&organization_funds_[site.owner_id-1];
        break;
    default:return {false,"Invalid owner kind"};
    }
    int64_t* internal=cash?&site.cash:&site.stock[size_t(cmd.good)];
    int64_t& source=deposit?*external:*internal;
    int64_t& destination=deposit?*internal:*external;
    if(source<cmd.amount)return {false,"Insufficient source balance"};
    if(destination<0||destination>std::numeric_limits<int64_t>::max()-cmd.amount)return {false,"Transfer destination overflow"};
    if(!cash&&!deposit&&site.owner_kind==EstablishmentOwnerKind::Person){
        int64_t load=0;
        for(auto quantity:actor.inventory){
            if(quantity<0||quantity>16-load)return {false,"Personal carrying capacity exceeded"};
            load+=quantity;
        }
        if(cmd.amount>16-load)return {false,"Personal carrying capacity exceeded"};
    }
    RealmResult result{true,"Committed",next_fact_};
    reserve_append(facts_,8193);
    // No allocations or validation remain after the first authoritative write.
    source-=cmd.amount;destination+=cmd.amount;
    actor.last_decision_day=day_;++actor.decisions;++actor.sampling;
    event(actor.id,site.region,204+operation,cmd.amount,site.id);
    return result;
}
RealmResult Realm::execute(const WorkAuthorizationCommand& cmd){
    if(cmd.actor==0||cmd.actor>people_.size())return {false,"Unknown actor"};
    auto& actor=mutable_person(cmd.actor);
    if(actor.decisions==UINT64_MAX||actor.sampling==UINT64_MAX)return {false,"Decision counter overflow"};
    if(cmd.sequence!=actor.decisions+1)return {false,"Stale or duplicate decision ID"};
    if(!actor.alive)return {false,"Actor deceased"};
    auto found=std::lower_bound(establishments_.begin(),establishments_.end(),cmd.establishment_id,
        [](const Establishment& site,uint64_t id){return site.id<id;});
    if(found==establishments_.end()||found->id!=cmd.establishment_id)return {false,"Unknown establishment"};
    auto& site=*found;
    if(!manages_establishment(actor,site))return {false,"Actor cannot manage establishment"};
    if(cmd.worker==0||cmd.worker>people_.size())return {false,"Unknown worker"};
    if(std::any_of(apprenticeships_.begin(),apprenticeships_.end(),[&](const auto& course){
        return course.status==ApprenticeshipStatus::Active&&course.establishment_id==site.id&&
            course.apprentice==cmd.worker;
    }))return {false,"Apprentice work permission is controlled by the active training contract"};
    const bool revoke=cmd.operation==WorkAuthorizationOperation::Revoke;
    if(!revoke&&cmd.operation!=WorkAuthorizationOperation::Set)return {false,"Invalid authorization operation"};
    if(revoke){if(cmd.begin!=0||cmd.end!=0)return {false,"Invalid revoke range"};}
    else{
        const auto& worker=person(cmd.worker);
        if(!worker.alive||worker.destination!=-1||worker.region!=site.region)return {false,"Worker not present at establishment"};
        if(cmd.begin<schedule_book_.cursor||cmd.end<=cmd.begin||cmd.end>365000LL*kSubdayUnitsPerDay)
            return {false,"Invalid authorization range"};
    }
    auto previous=site.work_authorizations.find(cmd.worker);
    if(revoke&&previous==site.work_authorizations.end())return {false,"Authorization absent"};
    if(!revoke&&previous!=site.work_authorizations.end()&&previous->second.size()==1&&
       previous->second.front()==WorkAuthorizationWindow{cmd.begin,cmd.end})return {false,"Authorization unchanged"};
    const size_t oldWindows=previous==site.work_authorizations.end()?0:previous->second.size();
    const size_t newWindowCount=site.authorization_window_count-oldWindows+(revoke?0:1);
    if(newWindowCount>1000000)return {false,"Authorization window limit"};
    const auto sessions=schedule_book_.sessions.find(cmd.worker);
    if(sessions!=schedule_book_.sessions.end())for(const auto& session:sessions->second){
        if(session.retired||session.kind!=ActivityKind::Work||session.establishment_id!=site.id||session.end<=schedule_book_.cursor)continue;
        if(revoke||cmd.begin>std::max(session.begin,schedule_book_.cursor)||cmd.end<session.end)
            return {false,"Authorization would cut accepted Work"};
    }
    if(next_fact_==UINT64_MAX)return {false,"Fact counter overflow"};
    RealmResult result{true,"Committed",next_fact_};
    std::vector<WorkAuthorizationWindow> staged;
    if(!revoke)staged.push_back({cmd.begin,cmd.end});
    // Preallocate a node for new workers; existing windows use noexcept swap.
    decltype(site.work_authorizations) node_source;
    if(!revoke&&previous==site.work_authorizations.end())node_source.emplace(cmd.worker,std::move(staged));
    reserve_append(facts_,8193);
    if(revoke)site.work_authorizations.erase(previous);
    else if(previous!=site.work_authorizations.end())previous->second.swap(staged);
    else site.work_authorizations.insert(node_source.extract(cmd.worker));
    site.authorization_window_count=newWindowCount;
    actor.last_decision_day=day_;++actor.decisions;++actor.sampling;
    event(actor.id,site.region,revoke?209:208,revoke?0:cmd.end-cmd.begin,cmd.worker,true,0);
    return result;
}
} // namespace polyec
