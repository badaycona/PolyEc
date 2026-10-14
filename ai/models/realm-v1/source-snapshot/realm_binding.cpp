#include "polyec/realm.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#ifdef POLYEC_AI_ENABLED
#include "polyec/policy.hpp"
#endif
namespace py=pybind11;using namespace polyec;
py::dict realm_view(const Realm&w,uint64_t observer){auto v=w.view(observer);py::dict d;d["day"]=v.day;d["persons"]=v.persons;d["alive"]=v.alive;d["children"]=v.children;d["money"]=v.money;d["decisions"]=v.decisions;d["cards"]=v.draft.cards;d["chosen"]=v.draft.chosen;py::list ps;for(auto&p:v.provinces){py::dict x;x["stock"]=p.stock;x["prices"]=p.price;x["treasury"]=p.treasury;x["firm"]=p.firm;x["unrest"]=p.unrest;x["road"]=p.road;x["jobs"]=p.jobs;x["employed"]=p.employed;x["school"]=p.school;x["school_used"]=p.school_used;ps.append(x);}d["provinces"]=ps;py::list fs;for(auto&f:v.facts){py::dict x;x["id"]=f.id;x["parent"]=f.parent;x["kind"]=f.kind;x["actor"]=f.actor;x["day"]=f.day;x["amount"]=f.amount;fs.append(x);}d["facts"]=fs;return d;}
class RealmBatch {
    std::vector<Realm> worlds;std::vector<uint64_t> actors;std::vector<int> steps;std::vector<double> opening;
    uint64_t seed_;int role_,regions_,horizon_;
    uint64_t actor(const Realm&w,uint64_t seed){if(role_==0)return 1+seed%(regions_+1);if(role_==1)return 2+regions_+seed%(regions_+1);return 2*(regions_+1)+1+seed%(128-2*(regions_+1));}
    void reset(int i){uint64_t seed=seed_++;worlds[i]=Realm(seed,regions_,128);worlds[i].choose(worlds[i].view().draft.cards[seed%3]);actors[i]=actor(worlds[i],seed);steps[i]=0;opening[i]=worlds[i].utility(actors[i]);}
public:
    RealmBatch(int count,uint64_t seed,int role,int regions,int horizon):seed_(seed),role_(role),regions_(regions),horizon_(horizon){if(count<1||count>128||role<0||role>2||horizon<1||horizon>256)throw std::invalid_argument("Realm batch bounds");for(int i=0;i<count;++i)worlds.emplace_back(1,regions,128);actors.resize(count);steps.resize(count);opening.resize(count);for(int i=0;i<count;++i)reset(i);}
    py::tuple observe(){py::array_t<float> obs({int(worlds.size()),realm_obs}),mask({int(worlds.size()),realm_actions});for(int i=0;i<int(worlds.size());++i){auto o=worlds[i].observation(actors[i]);auto m=worlds[i].mask(actors[i]);std::copy(o.begin(),o.end(),obs.mutable_data()+i*realm_obs);std::copy(m.begin(),m.end(),mask.mutable_data()+i*realm_actions);}return py::make_tuple(obs,mask);}
    py::tuple step(py::array_t<int64_t,py::array::c_style|py::array::forcecast> acts){int n=int(worlds.size());if(acts.ndim()!=1||acts.size()!=n)throw std::invalid_argument("realm action shape");for(int i=0;i<n;++i)if(acts.data()[i]<0||acts.data()[i]>=realm_actions||!worlds[i].mask(actors[i])[acts.data()[i]])throw std::invalid_argument("Invalid realm action");py::array_t<float> rewards(n),scores(n);py::array_t<bool> done(n);for(int i=0;i<n;++i){auto&w=worlds[i];auto id=actors[i];double before=w.utility(id);auto result=w.execute({id,w.person(id).decisions+1,w.candidates(id)[acts.data()[i]]});if(!result.success)throw std::runtime_error(result.reason);w.advance(role_==1?7:30);if(!w.invariant())throw std::runtime_error("Realm invariant failure");rewards.mutable_data()[i]=float(w.utility(id)-before)/10.f;bool end=++steps[i]>=horizon_;done.mutable_data()[i]=end;scores.mutable_data()[i]=end?float(w.utility(id)-opening[i]):0;if(end)reset(i);}auto o=observe();return py::make_tuple(o[0],o[1],rewards,done,scores);}
};
class RealmNativeTag {};
#ifdef POLYEC_AI_ENABLED
class RealmNativePolicy:public NativePolicy {public:using NativePolicy::NativePolicy;};
#endif
void bind_realm(py::module_&m){m.attr("REALM_OBS")=realm_obs;m.attr("REALM_ACTIONS")=realm_actions;
    py::class_<Realm>(m,"Realm").def(py::init<uint64_t,int,int>(),py::arg("seed")=1,py::arg("regions")=6,py::arg("people")=10000)
    .def("view",&realm_view,py::arg("observer")=1).def("observe",&Realm::observation).def("mask",&Realm::mask).def("advance",&Realm::advance,py::arg("days")=1).def("invariant",&Realm::invariant).def("fingerprint",&Realm::fingerprint).def("due",[](Realm&w,int role){return w.due(Family(role));})
    .def("step",[](Realm&w,uint64_t id,int a){if(a<0||a>=realm_actions)throw std::invalid_argument("Action bounds");auto result=w.execute({id,w.person(id).decisions+1,w.candidates(id)[a]});return py::make_tuple(result.success,result.reason);})
    .def("person",[](Realm&w,uint64_t id){auto&p=w.person(id);py::dict d;d["id"]=id;d["region"]=p.region;d["age_days"]=p.age_days;d["cash"]=p.cash;d["health"]=p.health;d["skill"]=p.skill;d["goal"]=p.goal;d["stats"]=p.stats.values;d["family"]=int(p.family);d["alive"]=p.alive;d["decisions"]=p.decisions;d["sampling"]=p.sampling;return d;})
    .def("utility",&Realm::utility).def("choose",&Realm::choose).def("reroll",&Realm::reroll).def("serialize",&Realm::serialize).def("deserialize",[](Realm&w,const std::string&s,const std::string&b){std::string error;bool ok=w.deserialize(s,b,error);return py::make_tuple(ok,error);});
    py::class_<RealmBatch>(m,"RealmBatch").def(py::init<int,uint64_t,int,int,int>()).def("observe",&RealmBatch::observe).def("step",&RealmBatch::step);
#ifdef POLYEC_AI_ENABLED
    py::class_<RealmNativePolicy>(m,"RealmNativePolicy").def(py::init<int,int>(),py::arg("observations")=realm_obs,py::arg("actions")=realm_actions).def("load",&NativePolicy::load).def("error",&NativePolicy::error).def("predict",[](NativePolicy&p,py::array_t<float,py::array::c_style|py::array::forcecast>o,py::array_t<float,py::array::c_style|py::array::forcecast>h){if(o.ndim()!=2||o.shape(1)!=realm_obs||h.ndim()!=2||h.shape(0)!=o.shape(0)||h.shape(1)!=64)throw std::invalid_argument("Realm ONNX input shape");auto r=p.predict({o.data(),o.data()+o.size()},{h.data(),h.data()+h.size()},int(o.shape(0)));if(!r.success)throw std::runtime_error(r.error);py::array_t<float>logits({int(o.shape(0)),realm_actions}),memory({int(o.shape(0)),64});std::copy(r.logits.begin(),r.logits.end(),logits.mutable_data());std::copy(r.hidden.begin(),r.hidden.end(),memory.mutable_data());return py::make_tuple(logits,memory);});
#endif
}
