*** Begin Patch
*** Update File: core/src/realm.cpp
@@
 bool Realm::reindex_households(){households_.clear();households_.reserve(people_.size());for(auto&p:people_)index_household(p.household,p.id);}
*** End Patch
