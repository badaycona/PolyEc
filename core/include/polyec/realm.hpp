*** Begin Patch
*** Update File: core/include/polyec/realm.hpp
@@
 class Realm {
     friend class Campaign;
 public:
     Realm(uint64_t seed=1,int regions=6,int people=10000,bool advisor_fixture=false);
@@
     std::vector<Proposal> offers(uint64_t observer)const;
     std::vector<Investigation> investigations()const{return investigations_;}
     double utility(uint64_t actor) const;
     bool reroll(int slot);
     RealmResult route_command(uint64_t actor,int from,int to,bool open);
     int distance(int from,int to)const;
     Draft draft()const{return draft_;}
     bool choose(int card);
     bool save(const std::string& path,const std::string& bundle,std::string& error) const;
     bool load(const std::string& path,const std::string& bundle,std::string& error);
     std::string serialize(const std::string& bundle) const;
     bool deserialize(const std::string& data,const std::string& bundle,std::string& error,const std::array<int64_t,7>& bonded_goods={});
     int day() const {return day_;}
     int regions() const {return regions_;}
     int count() const {return int(people_.size());}
@@
 private:
@@
     Person& mutable_person(uint64_t id);
     uint64_t guardian_of(const Person& person)const;
@@
     void transfer(int64_t& from,int64_t& to,int64_t amount);
@@
 };
+
+    // Mutable accessors for managers or external refactor helpers. These keep
+    // the internal containers available for managed refactors and do not
+    // change the authoritative ownership: Realm still owns its data.
+    public:
+    std::vector<Person>& people_mutable() { return people_; }
+    std::vector<Province>& provinces_mutable() { return provinces_; }
+    std::vector<Edge>& edges_mutable() { return edges_; }
+    std::vector<Loan>& loans_mutable() { return loans_; }
+    std::vector<Project>& projects_mutable() { return projects_; }
+    std::vector<Fact>& facts_mutable() { return facts_; }
+    std::vector<Knowledge>& knowledge_mutable() { return knowledge_; }
+    std::vector<Proposal>& proposals_mutable() { return proposals_; }
+    std::unordered_map<uint64_t,std::vector<uint64_t>>& households_mutable() { return households_; }
+    std::unordered_map<uint64_t,std::unordered_map<uint64_t,int>>& knowledge_index_mutable() { return knowledge_index_; }
 };
*** End Patch
