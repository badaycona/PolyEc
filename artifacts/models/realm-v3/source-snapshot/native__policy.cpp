#include "polyec/policy.hpp"
#include "polyec/trade.hpp"
#include <onnxruntime_cxx_api.h>
#include <array>
#include <cmath>
#include <filesystem>
#include <stdexcept>

namespace polyec {
class NativePolicy::Impl {
public:
    Ort::Env env{ORT_LOGGING_LEVEL_WARNING,"PolyEcCPU"};
    Ort::SessionOptions options;
    std::unique_ptr<Ort::Session> session;
    Ort::MemoryInfo memory=Ort::MemoryInfo::CreateCpu(OrtArenaAllocator,OrtMemTypeDefault);
    explicit Impl(const std::string& path,int observations,int actions) {
        options.SetIntraOpNumThreads(2);
        options.SetInterOpNumThreads(1);
        options.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
        options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
        auto file=std::filesystem::u8path(path);
        session=std::make_unique<Ort::Session>(env,file.c_str(),options);
        if(session->GetInputCount()!=2 || session->GetOutputCount()!=2) throw std::runtime_error("policy IO count mismatch");
        Ort::AllocatorWithDefaultOptions allocator;
        const char* expected_inputs[]={"obs","hidden"};
        const char* expected_outputs[]={"logits","next_hidden"};
        int widths[]={observations,64,actions,64};
        for(int i=0;i<4;++i) {
            auto info=i<2?session->GetInputTypeInfo(i):session->GetOutputTypeInfo(i-2);
            auto tensor=info.GetTensorTypeAndShapeInfo();auto shape=tensor.GetShape();
            if(shape.size()!=2 || shape[1]!=widths[i] || tensor.GetElementType()!=ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
                throw std::runtime_error("policy tensor shape/dtype mismatch");
            auto name=i<2?session->GetInputNameAllocated(i,allocator):session->GetOutputNameAllocated(i-2,allocator);
            if(std::string(name.get())!=(i<2?expected_inputs[i]:expected_outputs[i-2])) throw std::runtime_error("policy tensor name mismatch");
        }
    }
};
NativePolicy::NativePolicy(int observations,int actions):observations_(observations),actions_(actions){}
NativePolicy::~NativePolicy()=default;
bool NativePolicy::load(const std::string& path) {
    try { impl_=std::make_unique<Impl>(path,observations_,actions_); error_.clear();return true; }
    catch(const std::exception& e) {impl_.reset();error_=e.what();return false;}
}
PolicyOutput NativePolicy::predict(const std::vector<float>& observation,const std::vector<float>& hidden,int batch) {
    PolicyOutput result;
    try {
        if(!impl_) throw std::runtime_error("policy is not loaded; inference stopped");
        if(batch<1 || batch>128 || observation.size()!=size_t(batch*observations_) || hidden.size()!=size_t(batch*64))
            throw std::runtime_error("policy input batch/shape mismatch");
        for(float v:observation) if(!std::isfinite(v)) throw std::runtime_error("non-finite observation");
        for(float v:hidden) if(!std::isfinite(v)) throw std::runtime_error("non-finite recurrent memory");
        std::array<int64_t,2> obs_shape{batch,observations_},memory_shape{batch,64};
        std::array<Ort::Value,2> inputs{
            Ort::Value::CreateTensor<float>(impl_->memory,const_cast<float*>(observation.data()),observation.size(),obs_shape.data(),2),
            Ort::Value::CreateTensor<float>(impl_->memory,const_cast<float*>(hidden.data()),hidden.size(),memory_shape.data(),2)};
        const char* input_names[]={"obs","hidden"};const char* output_names[]={"logits","next_hidden"};
        auto outputs=impl_->session->Run(Ort::RunOptions{nullptr},input_names,inputs.data(),2,output_names,2);
        if(outputs[0].GetTensorTypeAndShapeInfo().GetElementCount()!=size_t(batch*actions_) ||
           outputs[1].GetTensorTypeAndShapeInfo().GetElementCount()!=size_t(batch*64)) throw std::runtime_error("policy output shape mismatch");
        auto logits=outputs[0].GetTensorData<float>();auto memory=outputs[1].GetTensorData<float>();
        result.logits.assign(logits,logits+batch*actions_);result.hidden.assign(memory,memory+batch*64);
        for(float v:result.logits) if(!std::isfinite(v)) throw std::runtime_error("non-finite policy logits");
        for(float v:result.hidden) if(!std::isfinite(v)) throw std::runtime_error("non-finite policy memory");
        result.success=true;
    } catch(const std::exception& e) {error_=e.what();result.error=error_;result.logits.clear();result.hidden.clear();}
    return result;
}
const std::string& NativePolicy::error() const {return error_;}
std::string NativePolicy::version() {return OrtGetApiBase()->GetVersionString();}
}
