#include "polyec/realm.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <thread>
#include <exception>
#ifdef POLYEC_AI_ENABLED
#include "polyec/policy.hpp"
#endif
namespace py=pybind11;using namespace polyec;
py::dict realm_view(const Realm&w,uint64_t observer){auto v=w.view(observer);py::dict d;d["day"]=v.day;d["persons"]=v.persons;d["alive"]=v.alive;d["children"]=v.children;d["money"]=v.money;d["decisions"]=v.decisions;d["cards"]=v.draft.cards;d["chosen"]=v.draft.chosen;py::list ps;for(auto&p:v.provinces){py::dict x;x["stock"]=p.stock;x["prices"]=p.price;x["treasury"]=p.treasury;x["firm"]=p.firm;x["unrest"]=p.unrest;x["road"]=p.road;x["jobs"]=p.jobs;x["employed"]=p.employed;x["school"]=p.school;x["school_used"]=p.school_used;ps.append(x);}d["provinces"]=ps;py::list fs;for(auto&f:v.facts){py::dict x;x["id"]=f.id;x["parent"]=f.parent;x["kind"]=f.kind;x["actor"]=f.actor;x["day"]=f.day;x["amount"]=f.amount;fs.append(x);}d["facts"]=fs;return d;}
class RealmBatch {
    struct Outcome {bool alive=false,insolvent=false;int health=0,nutrition=0,skill=0,days=0,due=0,defaults=0,decisions=0;int64_t cash=0,debt=0;bool authority=false;double reference=1,raw_utility=0;};
    std::vector<Realm> worlds;std::vector<uint64_t> actors;std::vector<int> steps;std::vector<double> opening;std::vector<Outcome> outcomes_;
    std::vector<uint8_t> finished_;std::vector<float> final_scores_;std::vector<std::mt19937_64> partner_rng_;
    uint64_t seed_;int role_,regions_,horizon_,workers_;
    uint64_t actor(uint64_t seed){if(role_==0)return seed%2==0?128:1+seed%(regions_+1);if(role_==1)return 2+regions_+seed%(regions_+1);return 2*(regions_+1)+1+seed%(128-2*(regions_+1));}
    void reset(int i){uint64_t seed=seed_++;worlds[i]=Realm(seed,regions_,128,role_==0&&seed%2==0);worlds[i].choose(worlds[i].draft().cards[seed%3]);actors[i]=actor(seed);if(role_==2)while(worlds[i].person(actors[i]).candidate){actors[i]=2*(regions_+1)+1+(actors[i]-2*(regions_+1))%(128-2*(regions_+1));}steps[i]=0;opening[i]=worlds[i].utility(actors[i]);finished_[i]=0;final_scores_[i]=0;partner_rng_[i].seed(seed^0xd0a1b41e91ull);}
    void transition(int i,int64_t action,float&reward,bool&done,float&score){
        if(finished_[i]){reward=0;done=true;score=final_scores_[i];return;}
        auto&w=worlds[i];auto id=actors[i];double before=w.utility(id);
        auto result=w.execute({id,w.person(id).decisions+1,w.candidates(id)[action]});if(!result.success)throw std::runtime_error(result.reason);
        for(int day=0;day<(role_==1?7:30);++day){
            // Explicit training partners only: civilian work/study; other families
            // uniformly sample legal actions. App uses learned actors instead.
            for(auto family:{Family::Governance,Family::Economic,Family::Civilian})for(auto peer:w.due(family))if(peer!=id){
                auto m=w.mask(peer);int a=0;
                if(family==Family::Civilian){int work=1+w.person(peer).region;a=m[work]?work:(m[15]?15:0);}
                else {std::vector<int> legal;for(int j=0;j<realm_actions;++j)if(m[j])legal.push_back(j);a=legal[partner_rng_[i]()%legal.size()];}
                auto r=w.execute({peer,w.person(peer).decisions+1,w.candidates(peer)[a]});if(!r.success)throw std::runtime_error("Training partner commit failed");
            }
            w.advance();if(!w.person(id).alive)break;
        }
        if(!w.invariant())throw std::runtime_error("Realm invariant failure");
        reward=float((w.utility(id)-before)/std::max(1.,std::abs(opening[i])));done=++steps[i]>=horizon_||!w.person(id).alive;score=done?float((w.utility(id)-opening[i])/std::max(1.,std::abs(opening[i]))):0;
        if(done){auto&p=w.person(id);auto&o=outcomes_[i];o={};o.alive=p.alive;o.health=p.health;o.nutrition=p.nutrition;o.cash=p.cash;o.skill=p.skill;o.debt=w.debt(id);o.days=w.day();o.decisions=steps[i];o.authority=p.taxing_authority;o.reference=std::max(1.,std::abs(opening[i]));o.raw_utility=w.utility(id)-opening[i];o.insolvent=p.cash<o.debt;for(auto&l:w.loans())if(l.debtor==id&&l.due<=w.day()){++o.due;o.defaults+=l.defaulted;}finished_[i]=1;final_scores_[i]=score;}
    }
public:
    RealmBatch(int count,uint64_t seed,int role,int regions,int horizon,int workers=4):seed_(seed),role_(role),regions_(regions),horizon_(horizon),workers_(workers){
        if(count<1||count>128||role<0||role>2||horizon<1||horizon>1024||workers<1||workers>8)throw std::invalid_argument("Realm batch bounds");
        for(int i=0;i<count;++i)worlds.emplace_back(1,regions,128);actors.resize(count);steps.resize(count);opening.resize(count);outcomes_.resize(count);finished_.resize(count);final_scores_.resize(count);partner_rng_.resize(count);for(int i=0;i<count;++i)reset(i);
    }
    py::tuple observe(){py::array_t<float>obs({int(worlds.size()),realm_obs}),mask({int(worlds.size()),realm_actions});for(int i=0;i<int(worlds.size());++i){auto o=worlds[i].observation(actors[i]);auto m=worlds[i].mask(actors[i]);if(finished_[i]){m.fill(0);m[0]=1;}std::copy(o.begin(),o.end(),obs.mutable_data()+i*realm_obs);std::copy(m.begin(),m.end(),mask.mutable_data()+i*realm_actions);}return py::make_tuple(obs,mask);}
    py::list outcomes()const{py::list result;for(auto&o:outcomes_){py::dict d;d["alive"]=o.alive;d["health"]=o.health;d["nutrition"]=o.nutrition;d["cash"]=o.cash;d["skill"]=o.skill;d["debt"]=o.debt;d["days"]=o.days;d["decisions"]=o.decisions;d["taxing_authority"]=o.authority;d["reference_utility"]=o.reference;d["raw_utility_delta"]=o.raw_utility;d["insolvent"]=o.insolvent;d["obligations_due"]=o.due;d["defaults"]=o.defaults;result.append(d);}return result;}
    py::tuple step(py::array_t<int64_t,py::array::c_style|py::array::forcecast>actions,bool auto_reset=true){
        int n=int(worlds.size());if(actions.ndim()!=1||actions.size()!=n)throw std::invalid_argument("Realm action shape");
        for(int i=0;i<n;++i)if(actions.data()[i]<0||actions.data()[i]>=realm_actions||(finished_[i]?actions.data()[i]!=0:!worlds[i].mask(actors[i])[actions.data()[i]]))throw std::invalid_argument("Invalid realm action");
        py::array_t<float> rewards(n),scores(n);py::array_t<bool>dones(n);
        auto*act=actions.data();auto*r=rewards.mutable_data();auto*sc=scores.mutable_data();auto*done=dones.mutable_data();
        std::array<std::exception_ptr,8>errors{};
        {py::gil_scoped_release release;std::vector<std::jthread>threads;int count=std::min(workers_,n);
            for(int worker=0;worker<count;++worker)threads.emplace_back([&,worker,count]{try{for(int i=worker;i<n;i+=count)transition(i,act[i],r[i],done[i],sc[i]);}catch(...){errors[worker]=std::current_exception();}});
        }
        for(auto&e:errors)if(e)std::rethrow_exception(e);
        if(auto_reset)for(int i=0;i<n;++i)if(done[i])reset(i);
        auto o=observe();return py::make_tuple(o[0],o[1],rewards,dones,scores);
    }
};
class RealmNativeTag {};
#ifdef POLYEC_AI_ENABLED
class RealmNativePolicy:public NativePolicy {public:using NativePolicy::NativePolicy;};
#endif
void bind_realm(py::module_&m){m.attr("REALM_OBS")=realm_obs;m.attr("REALM_ACTIONS")=realm_actions;m.attr("REALM_SCHEMA")=realm_schema;
    py::class_<Realm>(m,"Realm").def(py::init<uint64_t,int,int>(),py::arg("seed")=1,py::arg("regions")=6,py::arg("people")=10000)
    .def("view",&realm_view,py::arg("observer")=1).def("observe",&Realm::observation).def("mask",&Realm::mask).def("advance",&Realm::advance,py::arg("days")=1).def("invariant",&Realm::invariant).def("fingerprint",&Realm::fingerprint).def("due",[](Realm&w,int role){return w.due(Family(role));})
    .def("step",[](Realm&w,uint64_t id,int a){if(a<0||a>=realm_actions)throw std::invalid_argument("Action bounds");auto result=w.execute({id,w.person(id).decisions+1,w.candidates(id)[a]});return py::make_tuple(result.success,result.reason);})
    .def("person",[](Realm&w,uint64_t id){auto&p=w.person(id);py::dict d;d["id"]=id;d["region"]=p.region;d["age_days"]=p.age_days;d["cash"]=p.cash;d["health"]=p.health;d["nutrition"]=p.nutrition;d["job_kind"]=p.job_kind;d["taxing_authority"]=p.taxing_authority;d["skill"]=p.skill;d["goal"]=p.goal;d["stats"]=p.stats.values;d["family"]=int(p.family);d["alive"]=p.alive;d["decisions"]=p.decisions;d["sampling"]=p.sampling;return d;})
    .def("utility",&Realm::utility).def("choose",&Realm::choose).def("reroll",&Realm::reroll).def("serialize",&Realm::serialize).def("deserialize",[](Realm&w,const std::string&s,const std::string&b){std::string error;bool ok=w.deserialize(s,b,error);return py::make_tuple(ok,error);})
    .def_static("scenario",&Realm::scenario,py::arg("seed"),py::arg("actor"),py::arg("age")=-1,py::arg("stat")=-1,py::arg("goal")=-1,py::arg("budget")=-1,py::arg("schools")=-1,py::arg("jobs")=-1,py::arg("nutrition")=-1,py::arg("health")=-1)
    .def("route",[](Realm&w,uint64_t actor,int from,int to,bool open){return w.route_command(actor,from,to,open).success;});
    py::class_<RealmBatch>(m,"RealmBatch").def(py::init<int,uint64_t,int,int,int,int>(),py::arg("count"),py::arg("seed"),py::arg("role"),py::arg("regions"),py::arg("horizon"),py::arg("workers")=4).def("observe",&RealmBatch::observe).def("step",&RealmBatch::step,py::arg("actions"),py::arg("auto_reset")=true).def("outcomes",&RealmBatch::outcomes);
#ifdef POLYEC_AI_ENABLED
    py::class_<RealmNativePolicy>(m,"RealmNativePolicy").def(py::init<int,int>(),py::arg("observations")=realm_obs,py::arg("actions")=realm_actions).def("load",&NativePolicy::load).def("error",&NativePolicy::error).def("predict",[](RealmNativePolicy&p,py::array_t<float,py::array::c_style|py::array::forcecast>o,py::array_t<float,py::array::c_style|py::array::forcecast>h){if(o.ndim()!=2||o.shape(1)!=realm_obs||h.ndim()!=2||h.shape(0)!=o.shape(0)||h.shape(1)!=64)throw std::invalid_argument("Realm ONNX input shape");auto r=p.predict({o.data(),o.data()+o.size()},{h.data(),h.data()+h.size()},int(o.shape(0)));if(!r.success)throw std::runtime_error(r.error);py::array_t<float>logits({int(o.shape(0)),realm_actions}),memory({int(o.shape(0)),64});std::copy(r.logits.begin(),r.logits.end(),logits.mutable_data());std::copy(r.hidden.begin(),r.hidden.end(),memory.mutable_data());return py::make_tuple(logits,memory);});
#endif
}
