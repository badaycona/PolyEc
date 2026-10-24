#include "work_authorization_fixture.hpp"
#include "polyec/realm.hpp"
#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
using namespace polyec;
int checks=0;
void check(bool ok,const char* label){++checks;if(!ok)throw std::runtime_error(label);}

void test_land_title_registry(){
    auto world=std::make_unique<Realm>(91,2,128);
    const auto titles=world->land_holdings();
    check(titles.size()==world->household_records(),"genesis creates one abstract title per founding household");
    check(!titles.empty(),"land registry has initial endowment");
    for(const auto& title:titles){
        const auto& owner=world->person(title.title_holder);
        check(title.id>uint64_t(world->count())&&title.region==owner.region&&title.area_units==1&&
                  owner.household_membership.household_id==owner.id,
              "genesis title has stable ID, home region, unit area and founder owner");
    }
    check(world->invariant(),"genesis land registry satisfies Realm invariant");

    const auto current=world->serialize("land-title");
    check(current.rfind("POLYEC_REALM_68 ",0)==0&&current.find(" LAND_1 ")!=std::string::npos&&
              current.find(" LANDUSE_1 0 ")!=std::string::npos,
          "land titles and empty use-right registry persist in current schema68");
    auto restored=std::make_unique<Realm>(1,1,128);std::string error;
    const bool initialRoundTrip=restored->deserialize(current,"land-title",error);
    check(initialRoundTrip&&restored->land_holdings().size()==titles.size()&&
              restored->serialize("land-title")==current,
          initialRoundTrip?"land title save/load preserves registry exactly":error.c_str());

    auto schema21=test_fixtures::save_as_v25(current);
    const auto landUse=schema21.find(" LANDUSE_1 ");
    check(landUse!=std::string::npos,"schema21 migration fixture finds land-use section");
    schema21.erase(landUse);schema21.replace(0,15,"POLYEC_REALM_21");
    auto migrated21=std::make_unique<Realm>(1,1,128);
    check(migrated21->deserialize(schema21,"land-title",error)&&migrated21->land_use_rights().empty()&&
              migrated21->land_holdings()==titles,
          "schema21 migration preserves titles and initializes no active use rights");

    auto schema20=test_fixtures::save_as_v25(current);
    const auto land=schema20.find(" LAND_1 ");
    check(land!=std::string::npos,"schema20 migration fixture finds land section");
    schema20.erase(land);schema20.replace(0,15,"POLYEC_REALM_20");
    auto migrated=std::make_unique<Realm>(1,1,128);
    check(migrated->deserialize(schema20,"land-title",error)&&migrated->land_holdings().empty(),
          "schema20 migrates without fabricating land titles");

    auto malformed=current;
    const auto tag=malformed.find(" LAND_1 ");
    const auto countEnd=malformed.find(' ',tag+8);
    const auto idEnd=malformed.find(' ',countEnd+1);
    const auto regionEnd=malformed.find(' ',idEnd+1);
    const auto areaEnd=malformed.find(' ',regionEnd+1);
    check(areaEnd!=std::string::npos,"land title corruption fixture locates area");
    malformed.replace(regionEnd+1,areaEnd-regionEnd-1,"0");
    const auto before=restored->fingerprint();
    check(!restored->deserialize(malformed,"land-title",error)&&restored->fingerprint()==before,
          "invalid land title area rejects load atomically");

    uint64_t adult = 0, minor = 0, otherAdult = 0;
    for (uint64_t id = 1; id <= uint64_t(world->count()); ++id) {
        const auto& candidate = world->person(id);
        if (!candidate.alive) continue;
        if (candidate.age_days >= 18 * 365) {
            if (!adult) adult = id;
            else if (!otherAdult) otherAdult = id;
        } else if (!minor) minor = id;
    }
    check(adult && minor && otherAdult, "land transfer fixture has adults and a dependent recipient");
    const auto owned = std::find_if(titles.begin(), titles.end(), [adult](const LandHolding& h) {
        return h.title_holder == adult;
    });
    check(owned != titles.end(), "adult transfer fixture owns an initial title");
    const auto titleId = owned->id;
    const auto totalMoney = world->total_money();
    auto grant = world->execute(LandUseGrantCommand{
        adult, world->person(adult).decisions + 1, titleId, minor});
    check(grant.success && world->land_use_rights().size() == 1 && world->invariant(),
          "adult title holder grants whole-holding use to a living dependent");
    const auto firstRightId = world->land_use_rights().front().id;
    const auto replacement = world->execute(LandUseGrantCommand{
        adult, world->person(adult).decisions + 1, titleId, otherAdult});
    check(replacement.success && world->land_use_rights().size() == 1 &&
              world->land_use_rights().front().grantee == otherAdult &&
              world->land_use_rights().front().id > firstRightId && world->invariant(),
          "grant replaces the unique active user without retaining stale right records");
    const auto replacementId = world->land_use_rights().front().id;
    const auto restoreDependent = world->execute(LandUseGrantCommand{
        adult, world->person(adult).decisions + 1, titleId, minor});
    check(restoreDependent.success && world->land_use_rights().size() == 1 &&
              world->land_use_rights().front().grantee == minor &&
              world->land_use_rights().front().id > replacementId,
          "title holder can replace a license with a new dependent grantee");
    const auto useSave = world->serialize("land-use");
    auto useReload = std::make_unique<Realm>(1, 1, 128);
    check(useReload->deserialize(useSave, "land-use", error) &&
              useReload->land_use_rights() == world->land_use_rights() &&
              useReload->fingerprint() == world->fingerprint(),
          "schema22 round-trip preserves title, grant and replay fingerprint");
    const auto useBeforeRejects = world->fingerprint();
    const auto staleGrant = world->execute(LandUseGrantCommand{
        adult, world->person(adult).decisions, titleId, otherAdult});
    check(!staleGrant.success && staleGrant.reason == "Stale or duplicate decision ID" &&
              world->fingerprint() == useBeforeRejects,
          "stale land-use grant rejects atomically");
    const auto selfGrant = world->execute(LandUseGrantCommand{
        adult, world->person(adult).decisions + 1, titleId, adult});
    check(!selfGrant.success && selfGrant.reason == "Title holder cannot grant use to self" &&
              world->fingerprint() == useBeforeRejects,
          "title holder cannot create a redundant self-use right");
    auto malformedUse = useSave;
    const auto useTag = malformedUse.find(" LANDUSE_1 ");
    const auto useCountEnd = malformedUse.find(' ', useTag + std::string(" LANDUSE_1 ").size());
    check(useCountEnd != std::string::npos, "land-use corruption fixture locates bounded record count");
    malformedUse.replace(useTag + std::string(" LANDUSE_1 ").size(),
                         useCountEnd - useTag - std::string(" LANDUSE_1 ").size(), "1000001");
    const auto malformedBefore = useReload->fingerprint();
    check(!useReload->deserialize(malformedUse, "land-use", error) &&
              useReload->fingerprint() == malformedBefore,
          "excessive LANDUSE_1 count rejects load without replacing live world");
    const auto blockedBefore = world->fingerprint();
    const auto blockedTransfer = world->execute(LandTitleTransferCommand{
        adult, world->person(adult).decisions + 1, titleId, otherAdult});
    check(!blockedTransfer.success && blockedTransfer.reason == "Revoke land use right before title transfer" &&
              world->fingerprint() == blockedBefore,
          "active use right blocks title transfer without partial mutation");
    const auto revoke = world->execute(LandUseRevokeCommand{
        adult, world->person(adult).decisions + 1, titleId});
    check(revoke.success && world->land_use_rights().empty() && world->invariant(),
          "current title holder revokes use permission through Realm command");
    const auto beforeTransfer = world->fingerprint();
    const auto first = world->execute(LandTitleTransferCommand{adult, world->person(adult).decisions + 1, titleId, minor});
    check(first.success && world->land_holdings()[size_t(owned - titles.begin())].title_holder == minor,
          "adult owner transfers title to a living dependent through Realm command");
    const auto updatedTitles = world->land_holdings();
    const auto& transferred = updatedTitles[size_t(owned - titles.begin())];
    check(transferred.id == titleId && transferred.region == owned->region &&
              transferred.area_units == owned->area_units && world->total_money() == totalMoney &&
              world->fingerprint() != beforeTransfer && world->invariant(),
          "title transfer preserves identity, land attributes and money while changing fingerprint");
    const auto transferSave = world->serialize("land-transfer");
    auto transferReload=std::make_unique<Realm>(1, 1, 128);
    check(transferReload->deserialize(transferSave, "land-transfer", error) &&
              transferReload->land_holdings() == world->land_holdings() &&
              transferReload->fingerprint() == world->fingerprint(),
          "title transfer persists and continues through schema22 save/load");

    std::vector<std::string> householdTokens;
    { std::istringstream input(transferSave); std::string item; while (input >> item) householdTokens.push_back(item); }
    const size_t personCount = size_t(world->count());
    const size_t personWidth = 23 + Person{}.stats.values.size() + kGoodIdCount;
    size_t personStart = householdTokens.size();
    for (size_t i = 0; i + 1 + personWidth * personCount <= householdTokens.size(); ++i) {
        if (householdTokens[i] != std::to_string(personCount)) continue;
        bool dense = true;
        for (size_t j = 0; j < personCount; ++j)
            if (householdTokens[i + 1 + personWidth * j] != std::to_string(j + 1)) { dense = false; break; }
        if (dense) { personStart = i + 1; break; }
    }
    check(personStart < householdTokens.size(), "guardian title fixture locates Person save block");
    householdTokens[personStart + personWidth * (minor - 1) + 1] = std::to_string(adult);
    std::ostringstream guardianSave;
    for (size_t i = 0; i < householdTokens.size(); ++i) {
        if (i) guardianSave << ' ';
        guardianSave << householdTokens[i];
    }
    auto guardianWorld = std::make_unique<Realm>(1, 1, 128);
    check(guardianWorld->deserialize(guardianSave.str(), "land-transfer", error) &&
              guardianWorld->guardian(minor) == adult,
          "adult member is recomputed as dependent's current guardian");
    const auto guardianBefore = guardianWorld->fingerprint();
    const auto guardianDeed = guardianWorld->execute(LandTitleTransferCommand{
        adult, guardianWorld->person(adult).decisions + 1, titleId, otherAdult});
    check(!guardianDeed.success && guardianDeed.reason == "Actor is not title holder" &&
              guardianWorld->fingerprint() == guardianBefore,
          "guardian cannot transfer dependent-owned title");
    const auto guardianTitles = guardianWorld->land_holdings();
    const auto childHolding = std::find_if(guardianTitles.begin(), guardianTitles.end(), [minor](const LandHolding& holding) {
        return holding.title_holder == minor;
    });
    check(childHolding != guardianTitles.end(), "dependent holds an abstract land title");
    const int64_t guardianMoney = guardianWorld->total_money();
    const auto unauthorizedBefore = guardianWorld->fingerprint();
    const auto unauthorizedGrant = guardianWorld->execute(LandUseGrantCommand{
        otherAdult, guardianWorld->person(otherAdult).decisions + 1, childHolding->id, adult});
    check(!unauthorizedGrant.success &&
              unauthorizedGrant.reason == "Actor is not title holder or dependent guardian" &&
              guardianWorld->fingerprint() == unauthorizedBefore,
          "non-guardian adult cannot administer a dependent's permission");
    const auto guardianGrant = guardianWorld->execute(LandUseGrantCommand{
        adult, guardianWorld->person(adult).decisions + 1, childHolding->id, otherAdult});
    const auto guardianRights = guardianWorld->land_use_rights();
    const auto guardianFacts = guardianWorld->view(adult).facts;
    const auto guardianGrantFact = std::find_if(guardianFacts.begin(), guardianFacts.end(), [&](const Fact& fact) {
        return fact.id == guardianGrant.fact;
    });
    check(guardianGrant.success && guardianRights.size() == 1 &&
              guardianRights.front().holding_id == childHolding->id &&
              guardianRights.front().grantee == otherAdult && guardianRights.front().grantor == minor &&
              guardianWorld->land_holdings()[size_t(childHolding - guardianTitles.begin())].title_holder == minor &&
              guardianWorld->total_money() == guardianMoney && guardianWorld->invariant() &&
              guardianGrantFact != guardianFacts.end() && guardianGrantFact->actor == adult &&
              guardianGrantFact->target == otherAdult && guardianGrantFact->kind == 213,
          "guardian grants unpriced use while dependent retains title and audit actor");
    const auto guardianLandSave = guardianWorld->serialize("guardian-land-use");
    auto guardianLandRestore = std::make_unique<Realm>(1, 1, 128);
    check(guardianLandRestore->deserialize(guardianLandSave, "guardian-land-use", error) &&
              guardianLandRestore->land_use_rights() == guardianRights &&
              guardianLandRestore->guardian(minor) == adult &&
              guardianLandRestore->fingerprint() == guardianWorld->fingerprint(),
          "guardian permission and current authority survive save/load without schema change");
    const auto selfBefore = guardianWorld->fingerprint();
    const auto guardianSelfGrant = guardianWorld->execute(LandUseGrantCommand{
        adult, guardianWorld->person(adult).decisions + 1, childHolding->id, adult});
    check(!guardianSelfGrant.success && guardianSelfGrant.reason == "Title holder cannot grant use to self" &&
              guardianWorld->fingerprint() == selfBefore,
          "guardian cannot give themselves use permission over dependent property");
    const auto guardianRevoke = guardianWorld->execute(LandUseRevokeCommand{
        adult, guardianWorld->person(adult).decisions + 1, childHolding->id});
    check(guardianRevoke.success && guardianWorld->land_use_rights().empty() &&
              guardianWorld->land_holdings()[size_t(childHolding - guardianTitles.begin())].title_holder == minor &&
              guardianWorld->total_money() == guardianMoney && guardianWorld->invariant(),
          "current guardian revokes permission while preserving dependent ownership");
    const auto restoredRevoke = guardianLandRestore->execute(LandUseRevokeCommand{
        adult, guardianLandRestore->person(adult).decisions + 1, childHolding->id});
    check(restoredRevoke.success && guardianLandRestore->land_use_rights().empty() &&
              guardianLandRestore->land_holdings()[size_t(childHolding - guardianTitles.begin())].title_holder == minor &&
              guardianLandRestore->invariant(),
          "saved guardian permission can be revoked by the continuing guardian");

    auto unchanged = world->fingerprint();
    auto stale = world->execute(LandTitleTransferCommand{adult, world->person(adult).decisions, titleId, otherAdult});
    check(!stale.success && stale.reason == "Stale or duplicate decision ID" &&
              world->fingerprint() == unchanged,
          "stale title transfer rejects atomically");
    auto wrongHolder = world->execute(LandTitleTransferCommand{otherAdult, world->person(otherAdult).decisions + 1, titleId, adult});
    check(!wrongHolder.success && wrongHolder.reason == "Actor is not title holder" &&
              world->fingerprint() == unchanged,
          "non-holder cannot transfer another person's title");
    auto minorHolder = world->execute(LandTitleTransferCommand{minor, world->person(minor).decisions + 1,
                                      world->land_holdings()[size_t(owned - titles.begin())].id, adult});
    check(!minorHolder.success && minorHolder.reason == "Title transfer requires adult holder" &&
              world->fingerprint() == unchanged,
          "dependent title holder cannot self-transfer while underage");
    auto selfTransfer = world->execute(LandTitleTransferCommand{adult, world->person(adult).decisions + 1, titleId, otherAdult});
    check(!selfTransfer.success && selfTransfer.reason == "Actor is not title holder" &&
              world->fingerprint() == unchanged,
          "former holder cannot transfer a title after ownership changed");

    const auto otherTitle = std::find_if(titles.begin(), titles.end(), [otherAdult](const LandHolding& h) {
        return h.title_holder == otherAdult;
    });
    check(otherTitle != titles.end(), "second adult owns a distinct initial title");
    auto invalidRecipient = world->execute(LandTitleTransferCommand{
        otherAdult, world->person(otherAdult).decisions + 1, otherTitle->id, 0});
    check(!invalidRecipient.success && invalidRecipient.reason == "Unknown recipient" &&
              world->fingerprint() == unchanged,
          "invalid transfer recipient rejects atomically");
    auto unknownTitle = world->execute(LandTitleTransferCommand{
        otherAdult, world->person(otherAdult).decisions + 1, UINT64_MAX, adult});
    check(!unknownTitle.success && unknownTitle.reason == "Unknown land title" &&
              world->fingerprint() == unchanged,
          "unknown title rejects atomically");

    auto patch_person = [&](uint64_t id, size_t field, const std::string& value) {
        std::vector<std::string> tokens;
        { std::istringstream input(transferSave); std::string item; while (input >> item) tokens.push_back(item); }
        size_t start = tokens.size();
        for (size_t i = 0; i + 1 + personWidth * personCount <= tokens.size(); ++i) {
            if (tokens[i] != std::to_string(personCount)) continue;
            bool dense = true;
            for (size_t j = 0; j < personCount; ++j)
                if (tokens[i + 1 + personWidth * j] != std::to_string(j + 1)) { dense = false; break; }
            if (dense) { start = i + 1; break; }
        }
        if (!id || id > personCount || field >= personWidth || start == tokens.size())
            throw std::runtime_error("land transfer Person fixture index");
        tokens[start + personWidth * (id - 1) + field] = value;
        std::ostringstream output;
        for (size_t i = 0; i < tokens.size(); ++i) { if (i) output << ' '; output << tokens[i]; }
        return output.str();
    };
    auto deadActor = std::make_unique<Realm>(1,1,128);
    check(deadActor->deserialize(patch_person(otherAdult,14,"0"),"land-transfer",error),
          "dead title actor fixture loads");
    const auto deadBefore = deadActor->fingerprint();
    const auto deadResult = deadActor->execute(LandTitleTransferCommand{
        otherAdult,deadActor->person(otherAdult).decisions+1,otherTitle->id,adult});
    check(!deadResult.success&&deadResult.reason=="Actor deceased"&&deadActor->fingerprint()==deadBefore,
          "deceased owner cannot transfer title");
    auto deadRecipient = std::make_unique<Realm>(1,1,128);
    check(deadRecipient->deserialize(patch_person(adult,14,"0"),"land-transfer",error),
          "deceased title recipient fixture loads");
    const auto recipientBefore = deadRecipient->fingerprint();
    const auto recipientResult = deadRecipient->execute(LandTitleTransferCommand{
        otherAdult,deadRecipient->person(otherAdult).decisions+1,otherTitle->id,adult});
    check(!recipientResult.success&&recipientResult.reason=="Recipient deceased"&&
              deadRecipient->fingerprint()==recipientBefore,
          "title transfer to a deceased recipient rejects atomically");
    auto overflowActor = std::make_unique<Realm>(1,1,128);
    check(overflowActor->deserialize(patch_person(otherAdult,2,
          std::to_string(std::numeric_limits<uint64_t>::max())),"land-transfer",error),
          "decision overflow title actor fixture loads");
    const auto decisionBefore = overflowActor->fingerprint();
    const auto decisionResult = overflowActor->execute(LandTitleTransferCommand{
        otherAdult,UINT64_MAX,otherTitle->id,adult});
    check(!decisionResult.success&&decisionResult.reason=="Decision counter overflow"&&
              overflowActor->fingerprint()==decisionBefore,
          "title transfer rejects actor counter overflow atomically");

    std::vector<std::string> overflowTokens;
    { auto overflowBase=std::make_unique<Realm>(91,2,128);std::istringstream input(overflowBase->serialize("land-transfer"));
      std::string item; while (input >> item) overflowTokens.push_back(item); }
    overflowTokens[6] = std::to_string(std::numeric_limits<uint64_t>::max());
    std::ostringstream overflowSave;
    for (size_t i = 0; i < overflowTokens.size(); ++i) {
        if (i) overflowSave << ' ';
        overflowSave << overflowTokens[i];
    }
    auto factOverflow=std::make_unique<Realm>(1, 1, 128);
    check(factOverflow->deserialize(overflowSave.str(), "land-transfer", error),
          "fact overflow transfer fixture loads");
    const auto overflowBefore = factOverflow->fingerprint();
    const auto overflowTransfer = factOverflow->execute(LandTitleTransferCommand{adult, factOverflow->person(adult).decisions + 1,
                                                         titleId, minor});
    check(!overflowTransfer.success && overflowTransfer.reason == "Fact counter overflow" &&
              factOverflow->fingerprint() == overflowBefore,
          "title transfer rejects fact counter overflow atomically");
}
int main(){try{test_land_title_registry();std::cout<<"LAND TITLE TESTS: "<<checks<<" checks passed\n";return 0;}catch(const std::exception&e){std::cerr<<"FAIL after "<<checks<<": "<<e.what()<<'\n';return 1;}}






