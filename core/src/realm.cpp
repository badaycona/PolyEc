*** Begin Patch
*** Update File: core/src/realm.cpp
@@
-#include "polyec/realm.hpp"
-#include "polyec/trade.hpp"
+#include "polyec/realm.hpp"
+#include "polyec/trade.hpp"
+#include "polyec/managers/household_index.hpp"
+#include "polyec/managers/finance_manager.hpp"
+#include "polyec/managers/project_manager.hpp"
@@
-void Realm::transfer(int64_t&from,int64_t&to,int64_t amount){if(amount<0||from<amount)throw std::logic_error("unfunded transfer");from-=amount;to+=amount;}
+void Realm::transfer(int64_t&from,int64_t&to,int64_t amount){
+    // Delegate to FinanceManager for a single authority of transfer logic.
+    FinanceManager::transfer(from,to,amount);
+}
@@
-void Realm::index_household(uint64_t household,uint64_t person){
-    if(!household||!person)throw std::logic_error("household membership label");
-    auto&members=households_[household];
-    if(members.empty()||members.back()<person)members.push_back(person);
-    else if(std::find(members.begin(),members.end(),person)==members.end())members.insert(std::lower_bound(members.begin(),members.end(),person),person);
-}
-
-void Realm::reindex_households(){households_.clear();households_.reserve(people_.size());for(auto&p:people_)index_household(p.household,p.id);}
+void Realm::index_household(uint64_t household,uint64_t person){
+    if(!household||!person)throw std::logic_error("household membership label");
+    auto&members=households_[household];
+    if(members.empty()||members.back()<person)members.push_back(person);
+    else if(std::find(members.begin(),members.end(),person)==members.end())members.insert(std::lower_bound(members.begin(),members.end(),person),person);
+}
+
+void Realm::reindex_households(){
+    // Build households_ using the extracted helper to keep behaviour identical
+    households_ = HouseholdIndex::build(people_);
+}
@@
-    int active=0;for(auto&j:projects_)active+=!j.completed&&j.region==p.region&&j.kind==c.good;
-    if(active<(c.good==3&&draft_.chosen==10?2:1))return true;break;}
+    int active = ProjectManager::active_projects_count(projects_, p.region, c.good);
+    if(active < (c.good==3 && draft_.chosen==10 ? 2 : 1)) return true; break; }
*** End Patch
