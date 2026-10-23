#pragma once

#include "polyec/realm.hpp"
#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace polyec {
namespace detail {

// Writes typed Knowledge rows. Confidence -1 means no measured confidence exists.
inline void write_knowledge_section(std::ostream& out,
                                   const std::vector<Knowledge>& knowledge) {
    out << "KNOWLEDGE_3 " << knowledge.size() << ' ';
    for (const auto& k : knowledge) {
        out << k.observer << ' ' << k.fact << ' ' << k.arrives << ' '
            << k.source_investigation_id << ' ' << unsigned(k.kind) << ' '
            << k.confidence_basis_points << ' ';
    }
}

// Reads into a temporary vector. Realm rebuilds knowledge_index_ from every
// row after parsing; the stateless codec does not own that derived index.
inline void read_knowledge_section(std::istream& in,
                                   std::vector<Knowledge>& knowledge,
                                   int schema=33) {
    if(schema>=31){
        std::string section;
        in>>section;
        const std::string expected=schema>=33?"KNOWLEDGE_3":"KNOWLEDGE_2";
        if(!in||section!=expected)throw std::runtime_error("Save knowledge section");
    }
    size_t count = 0;
    in >> count;
    if (!in || count > 1000000) throw std::runtime_error("Save count limit");

    std::vector<Knowledge> staged(count);
    for (auto& k : staged) {
        if (!(in >> k.observer >> k.fact >> k.arrives) ||
            (schema>=31&&!(in>>k.source_investigation_id)))
            throw std::runtime_error("Save count limit");
        if(schema>=33){
            unsigned kind=0;
            if(!(in>>kind>>k.confidence_basis_points)||kind>static_cast<unsigned>(KnowledgeKind::SecretEvidence)||
               k.confidence_basis_points<kUnknownKnowledgeConfidence||
               k.confidence_basis_points>kKnowledgeConfidenceBasisPointMaximum)
                throw std::runtime_error("Save knowledge fields");
            k.kind=static_cast<KnowledgeKind>(kind);
        }else if(k.source_investigation_id){
            k.kind=KnowledgeKind::SecretEvidence;
            k.confidence_basis_points=kUnknownKnowledgeConfidence;
        }
        if((k.kind==KnowledgeKind::SecretEvidence&&k.source_investigation_id==0)||
           (k.kind==KnowledgeKind::UnknownLegacy&&(k.source_investigation_id!=0||
              k.confidence_basis_points!=kUnknownKnowledgeConfidence)))
            throw std::runtime_error("Save knowledge fields");
    }
    knowledge = std::move(staged);
}

} // namespace detail
} // namespace polyec
