#pragma once
#include <memory>
#include <string>
#include <vector>

namespace polyec {
struct PolicyOutput {
    bool success=false;
    std::string error;
    std::vector<float> logits,hidden;
};
class NativePolicy {
public:
    NativePolicy(int observations=310,int actions=21);
    ~NativePolicy();
    NativePolicy(const NativePolicy&)=delete;
    NativePolicy& operator=(const NativePolicy&)=delete;
    bool load(const std::string& path);
    PolicyOutput predict(const std::vector<float>& observation,const std::vector<float>& hidden,int batch);
    const std::string& error() const;
    static std::string version();
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
    std::string error_;
    int observations_,actions_;
};
}
