#include "polyec/trade.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <memory>
#ifdef POLYEC_AI_ENABLED
#include "polyec/policy.hpp"
#endif
namespace py=pybind11;
using namespace polyec;
py::dict view_dict(const TradeView& v) {
    py::dict result;
    result["day"]=v.day;result["decision"]=v.decision;result["region"]=v.region;
    result["cash"]=v.cash;result["cargo"]=v.cargo;result["carrier_cash"]=v.carrier_cash;
    result["trades"]=v.trades;result["travels"]=v.travels;result["holds"]=v.holds;
    result["done"]=v.done;result["transit_days"]=v.transit_days;
    py::list markets;
    for(const auto& m:v.markets) {
        py::dict item;item["prices"]=m.price;item["stock"]=m.stock;
        item["firm_cash"]=m.firm_cash;item["household_cash"]=m.household_cash;markets.append(item);
    }
    result["markets"]=markets;result["produced"]=v.produced;result["consumed"]=v.consumed;
    return result;
}
class BatchEnv {
    std::vector<TradeWorld> worlds;
    uint64_t next_seed;
    int regions,horizon;
public:
    BatchEnv(int n,uint64_t seed,int r,int h):next_seed(seed),regions(r),horizon(h) {
        if(n<1||n>512) throw std::invalid_argument("batch bounds");
        for(int i=0;i<n;++i) worlds.emplace_back(next_seed++,r,h);
    }
    py::tuple observe() {
        py::array_t<float> obs({int(worlds.size()),kObservation}), mask_array({int(worlds.size()),kActions});
        for(int i=0;i<int(worlds.size());++i) {
            auto o=worlds[i].observation();auto m=worlds[i].mask();
            std::copy(o.begin(),o.end(),obs.mutable_data()+i*kObservation);
            std::copy(m.begin(),m.end(),mask_array.mutable_data()+i*kActions);
        }
        return py::make_tuple(obs,mask_array);
    }
    py::tuple step(py::array_t<int64_t,py::array::c_style|py::array::forcecast> actions) {
        if(actions.ndim()!=1||actions.size()!=int(worlds.size())) throw std::invalid_argument("batch action shape");
        // Validate the complete batch first; malformed input never partially commits.
        for(int i=0;i<int(worlds.size());++i) {
            auto a=actions.data()[i];
            if(a<0||a>=kActions||!worlds[i].mask()[a]) throw std::invalid_argument("invalid batched action");
        }
        py::array_t<float> rewards(int(worlds.size())),profits(int(worlds.size()));
        py::array_t<bool> dones(int(worlds.size()));
        for(int i=0;i<int(worlds.size());++i) {
            auto r=worlds[i].step(int(actions.data()[i]));
            rewards.mutable_data()[i]=r.reward;dones.mutable_data()[i]=r.done;
            profits.mutable_data()[i]=r.done?float(worlds[i].view().cash-500):0.f;
            if(r.done) worlds[i].reset(next_seed++,regions,horizon);
        }
        auto observed=observe();
        return py::make_tuple(observed[0],observed[1],rewards,dones,profits);
    }
};
PYBIND11_MODULE(polyec_env,m) {
#ifdef POLYEC_AI_ENABLED
    py::class_<NativePolicy>(m,"NativePolicy").def(py::init<>()).def("load",&NativePolicy::load)
        .def("error",&NativePolicy::error).def_static("version",&NativePolicy::version)
        .def("predict",[](NativePolicy& policy,py::array_t<float,py::array::c_style|py::array::forcecast> obs,
                         py::array_t<float,py::array::c_style|py::array::forcecast> hidden){
            if(obs.ndim()!=2 || hidden.ndim()!=2 || obs.shape(1)!=kObservation || hidden.shape(1)!=64 || obs.shape(0)!=hidden.shape(0))
                throw std::invalid_argument("native policy array shape");
            auto r=policy.predict(std::vector<float>(obs.data(),obs.data()+obs.size()),
                                  std::vector<float>(hidden.data(),hidden.data()+hidden.size()),int(obs.shape(0)));
            if(!r.success) throw std::runtime_error(r.error);
            py::array_t<float> logits({int(obs.shape(0)),kActions}),memory({int(obs.shape(0)),64});
            std::copy(r.logits.begin(),r.logits.end(),logits.mutable_data());
            std::copy(r.hidden.begin(),r.hidden.end(),memory.mutable_data());
            return py::make_tuple(logits,memory);
        });
#endif
    m.attr("OBS")=kObservation;m.attr("ACTIONS")=kActions;m.attr("STATE")=kState;
    m.attr("SCHEMA")="trade-v1";m.attr("REWARD")="reference-wealth-gamma1-terminal-cash-v1";
    py::class_<TradeWorld>(m,"TradeWorld")
        .def(py::init<uint64_t,int,int>(),py::arg("seed")=1,py::arg("regions")=6,py::arg("horizon")=64)
        .def("reset",&TradeWorld::reset,py::arg("seed"),py::arg("regions")=6,py::arg("horizon")=64)
        .def("observation",&TradeWorld::observation).def("mask",&TradeWorld::mask)
        .def("step",[](TradeWorld& w,int action){auto r=w.step(action);return py::make_tuple(r.reward,r.done);})
        .def("view",[](const TradeWorld&w){return view_dict(w.view());})
        .def("fingerprint",&TradeWorld::fingerprint).def("invariant",&TradeWorld::invariant)
        .def("wealth",&TradeWorld::wealth).def("liquidation_value",&TradeWorld::liquidation_value);
    py::class_<BatchEnv>(m,"BatchEnv").def(py::init<int,uint64_t,int,int>())
        .def("observe",&BatchEnv::observe).def("step",&BatchEnv::step);
    m.def("action_name",&TradeWorld::action_name);
    m.def("citizen_spike",[](uint64_t n,int iterations){
        auto r=citizen_primitive_spike(n,iterations);py::dict d;
        d["persons"]=r.persons;d["edges"]=r.edges;d["bytes"]=r.bytes;d["checksum"]=r.checksum;d["milliseconds"]=r.milliseconds;return d;
    },py::arg("count"),py::arg("iterations")=12);
}
