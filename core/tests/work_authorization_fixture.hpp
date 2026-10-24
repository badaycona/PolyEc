#pragma once
#include "polyec/realm.hpp"
#include "polyec/realm_primary_header_codec.hpp"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace polyec::test_fixtures {
inline void normalize_schema68_fixture(std::string& source){
    if(source.rfind("POLYEC_REALM_71 ",0)==0)
        source.replace(0,std::string("POLYEC_REALM_71").size(),"POLYEC_REALM_70");
    if(source.rfind("POLYEC_REALM_70 ",0)==0)
        source.replace(0,std::string("POLYEC_REALM_70").size(),"POLYEC_REALM_69");
    if(source.rfind("POLYEC_REALM_69 ",0)==0)
        source.replace(0,std::string("POLYEC_REALM_69").size(),"POLYEC_REALM_68");
    if(source.rfind("POLYEC_REALM_68 ",0)==0)
        source.replace(0,std::string("POLYEC_REALM_68").size(),"POLYEC_REALM_67");
}
inline void downgrade_save_67_to_66(std::string& source){
        normalize_schema68_fixture(source);if(source.rfind("POLYEC_REALM_67 ",0)!=0)throw std::runtime_error("fixture is not schema67");
    const size_t section=source.rfind(" PRIVATE_HOUSING_1 ");
    if(section==std::string::npos)throw std::runtime_error("schema67 fixture lacks PRIVATE_HOUSING_1");
    std::istringstream in(source.substr(section+1));std::string tag;size_t count=0;in>>tag>>count;
    if(!in||tag!="PRIVATE_HOUSING_1"||count!=0)
        throw std::runtime_error("schema67 legacy fixture must not contain private homes");
    source.erase(section);
    source.replace(0,std::string("POLYEC_REALM_67").size(),"POLYEC_REALM_66");
}
inline void downgrade_save_66_to_65(std::string& source){
        normalize_schema68_fixture(source);if(source.rfind("POLYEC_REALM_67 ",0)==0)downgrade_save_67_to_66(source);
    if(source.rfind("POLYEC_REALM_66 ",0)!=0)throw std::runtime_error("fixture is not schema66");
    const size_t section=source.rfind(" MILITARY_RAID_2 ");
    if(section==std::string::npos)throw std::runtime_error("schema66 fixture lacks MILITARY_RAID_2");
    std::istringstream in(source.substr(section+1));std::string tag;size_t count=0;in>>tag>>count;
    if(!in||tag!="MILITARY_RAID_2"||count>1000000)throw std::runtime_error("schema66 raid header invalid");
    const size_t section_end=source.find(" URBAN_RESIDENCE_1 ",section+1);
    if(section_end==std::string::npos)throw std::runtime_error("schema66 fixture lacks URBAN_RESIDENCE_1 after raids");
    std::ostringstream out;out<<" MILITARY_RAID_1 "<<count<<' ';
    for(size_t i=0;i<count;++i){
        uint64_t id=0;int partner=0,target=0,created=0,resolved=0,force=0,score=0,losses=0,casualties=0;
        int64_t salt=0;unsigned status=0;size_t defenders=0,supplied=0;
        in>>id>>partner>>target>>created>>resolved>>force>>score>>losses>>casualties>>salt>>status>>defenders;
        if(!in||defenders>16)throw std::runtime_error("schema66 raid record invalid");
        std::vector<uint64_t> members(defenders);for(auto& member:members)in>>member;
        in>>supplied;if(!in||supplied>defenders)throw std::runtime_error("schema66 supplied defender count invalid");
        for(size_t j=0;j<supplied;++j){uint64_t ignored=0;in>>ignored;}
        if(!in)throw std::runtime_error("schema66 raid supply IDs invalid");
        out<<id<<' '<<partner<<' '<<target<<' '<<created<<' '<<resolved<<' '<<force<<' '<<score<<' '
           <<losses<<' '<<casualties<<' '<<salt<<' '<<status<<' '<<members.size()<<' ';
        for(const auto member:members)out<<member<<' ';
    }
    source.replace(section,section_end-section,out.str());
    source.replace(0,std::string("POLYEC_REALM_66").size(),"POLYEC_REALM_65");
}
inline void downgrade_save_64_to_63(std::string& source){
        normalize_schema68_fixture(source);if(source.rfind("POLYEC_REALM_67 ",0)==0||source.rfind("POLYEC_REALM_66 ",0)==0)downgrade_save_66_to_65(source);
    if(source.rfind("POLYEC_REALM_65 ",0)==0){
        const auto council=source.rfind(" MUNICIPAL_COUNCIL_1 ");
        if(council==std::string::npos)throw std::runtime_error("schema65 fixture lacks MUNICIPAL_COUNCIL_1");
        source.erase(council);
        source.replace(0,std::string("POLYEC_REALM_65").size(),"POLYEC_REALM_64");
    }
    if(source.rfind("POLYEC_REALM_64 ",0)!=0)throw std::runtime_error("fixture is not schema64");
    const auto section=source.rfind(" URBAN_HOUSING_1 ");
    if(section==std::string::npos)throw std::runtime_error("schema64 fixture lacks URBAN_HOUSING_1");
    source.erase(section);
    source.replace(0,std::string("POLYEC_REALM_64").size(),"POLYEC_REALM_63");
}
inline void downgrade_save_60_to_59(std::string& source){
        normalize_schema68_fixture(source);if(source.rfind("POLYEC_REALM_67 ",0)==0||source.rfind("POLYEC_REALM_66 ",0)==0)downgrade_save_66_to_65(source);
    if(source.rfind("POLYEC_REALM_65 ",0)==0)downgrade_save_64_to_63(source);
    if(source.rfind("POLYEC_REALM_64 ",0)==0)downgrade_save_64_to_63(source);
    if((source.rfind("POLYEC_REALM_63 ",0)==0||source.rfind("POLYEC_REALM_62 ",0)==0))
        source.replace(0,std::string("POLYEC_REALM_62").size(),"POLYEC_REALM_61");
    if(source.rfind("POLYEC_REALM_61 ",0)==0){
        const size_t residence=source.rfind(" URBAN_RESIDENCE_1 ");
        if(residence==std::string::npos)throw std::runtime_error("schema61 fixture lacks URBAN_RESIDENCE_1");
        source.erase(residence);
        source.replace(0,std::string("POLYEC_REALM_61").size(),"POLYEC_REALM_60");
    }
    if(source.rfind("POLYEC_REALM_60 ",0)!=0)throw std::runtime_error("fixture is not schema60");
    const size_t section=source.rfind(" MILITARY_RAID_1 ");
    if(section==std::string::npos)throw std::runtime_error("schema60 fixture lacks MILITARY_RAID_1");
    source.erase(section);
    source.replace(0,std::string("POLYEC_REALM_60").size(),"POLYEC_REALM_59");
}
inline void downgrade_save_59_to_58(std::string& source){
        normalize_schema68_fixture(source);if(source.rfind("POLYEC_REALM_67 ",0)==0||source.rfind("POLYEC_REALM_66 ",0)==0)downgrade_save_66_to_65(source);
    if(source.rfind("POLYEC_REALM_65 ",0)==0)downgrade_save_64_to_63(source);
    if(source.rfind("POLYEC_REALM_64 ",0)==0)downgrade_save_64_to_63(source);
    if((source.rfind("POLYEC_REALM_63 ",0)==0||source.rfind("POLYEC_REALM_62 ",0)==0)||source.rfind("POLYEC_REALM_61 ",0)==0||source.rfind("POLYEC_REALM_60 ",0)==0)downgrade_save_60_to_59(source);
    if(source.rfind("POLYEC_REALM_59 ",0)!=0)throw std::runtime_error("fixture is not schema59");
    struct Span{size_t begin=0,end=0;std::string value;};std::vector<Span> tokens;
    for(size_t pos=0;pos<source.size();){while(pos<source.size()&&std::isspace(static_cast<unsigned char>(source[pos])))++pos;if(pos==source.size())break;
        const size_t begin=pos;while(pos<source.size()&&!std::isspace(static_cast<unsigned char>(source[pos])))++pos;
        tokens.push_back({begin,pos,source.substr(begin,pos-begin)});}
    auto tag=std::find_if(tokens.begin(),tokens.end(),[](const Span& token){return token.value=="FOREIGN_TRADE_6";});
    if(tag==tokens.end())throw std::runtime_error("schema59 fixture lacks FOREIGN_TRADE_6");
    const size_t tag_index=size_t(tag-tokens.begin()),partner_width=3+kGoodIdCount+1+8;
    std::vector<Span> remove;
    for(size_t partner=0;partner<kForeignTradePartnerCount;++partner){
        const size_t first=tag_index+1+partner*partner_width+3+kGoodIdCount+1+6;
        if(first+2>tokens.size())throw std::runtime_error("truncated schema59 foreign trade partner");
        remove.push_back(tokens[first]);remove.push_back(tokens[first+1]);}
    size_t cursor=tag_index+1+kForeignTradePartnerCount*partner_width;
    if(cursor>=tokens.size())throw std::runtime_error("truncated schema59 foreign trade orders");
    const size_t order_count=std::stoull(tokens[cursor++].value);cursor+=order_count*11;
    if(cursor>=tokens.size())throw std::runtime_error("truncated schema59 foreign trade offers");
    const size_t offer_count=std::stoull(tokens[cursor].value);
    if(cursor+1+offer_count*16>tokens.size())throw std::runtime_error("truncated schema59 foreign trade offer records");
    for(size_t i=cursor;i<cursor+1+offer_count*16;++i)remove.push_back(tokens[i]);
    std::sort(remove.begin(),remove.end(),[](const Span& a,const Span& b){return a.begin>b.begin;});
    for(const auto& span:remove)source.erase(span.begin,span.end-span.begin);
    const size_t section=source.find(" FOREIGN_TRADE_6 ");
    source.replace(section+1,std::string("FOREIGN_TRADE_6").size(),"FOREIGN_TRADE_5");
    source.replace(0,std::string("POLYEC_REALM_59").size(),"POLYEC_REALM_58");
}
inline void downgrade_save_58_to_57(std::string& source){
        normalize_schema68_fixture(source);if((source.rfind("POLYEC_REALM_63 ",0)==0||source.rfind("POLYEC_REALM_62 ",0)==0)||source.rfind("POLYEC_REALM_61 ",0)==0||source.rfind("POLYEC_REALM_60 ",0)==0)downgrade_save_60_to_59(source);
    if((source.rfind("POLYEC_REALM_63 ",0)==0||source.rfind("POLYEC_REALM_62 ",0)==0)||source.rfind("POLYEC_REALM_61 ",0)==0||source.rfind("POLYEC_REALM_60 ",0)==0||source.rfind("POLYEC_REALM_59 ",0)==0)downgrade_save_59_to_58(source);
    if(source.rfind("POLYEC_REALM_58 ",0)!=0)throw std::runtime_error("fixture is not schema58");
    std::vector<std::pair<size_t,size_t>> tokens;
    for(size_t pos=0;pos<source.size();){
        while(pos<source.size()&&std::isspace(static_cast<unsigned char>(source[pos])))++pos;
        if(pos==source.size())break;const size_t begin=pos;
        while(pos<source.size()&&!std::isspace(static_cast<unsigned char>(source[pos])))++pos;
        tokens.emplace_back(begin,pos);
    }
    const auto tag=std::find_if(tokens.begin(),tokens.end(),[&](const auto& span){return source.substr(span.first,span.second-span.first)=="FOREIGN_TRADE_5";});
    if(tag==tokens.end())throw std::runtime_error("schema58 fixture lacks FOREIGN_TRADE_5");
    const size_t tag_index=size_t(tag-tokens.begin()), partner_width=3+kGoodIdCount+1+6;
    std::vector<std::pair<size_t,size_t>> remove;
    for(size_t partner=0;partner<kForeignTradePartnerCount;++partner){
        const size_t first=tag_index+1+partner*partner_width+3+kGoodIdCount+1;
        if(first+6>tokens.size())throw std::runtime_error("truncated schema58 trade history");
        for(size_t field=0;field<6;++field)remove.push_back(tokens[first+field]);
    }
    for(auto it=remove.rbegin();it!=remove.rend();++it)source.erase(it->first,it->second-it->first);
    const size_t section=source.find(" FOREIGN_TRADE_5 ");
    source.replace(section+1,std::string("FOREIGN_TRADE_5").size(),"FOREIGN_TRADE_4");
    source.replace(0,std::string("POLYEC_REALM_58").size(),"POLYEC_REALM_57");
}
inline void downgrade_save_57_to_56(std::string& source){
        normalize_schema68_fixture(source);if(source.rfind("POLYEC_REALM_57 ",0)!=0)throw std::runtime_error("fixture is not schema57");
    struct Span{size_t begin=0,end=0;std::string value;};
    std::vector<Span> words;
    for(size_t pos=0;pos<source.size();){
        while(pos<source.size()&&std::isspace(static_cast<unsigned char>(source[pos])))++pos;
        if(pos==source.size())break;const size_t begin=pos;
        if(source[pos]=='"'){
            ++pos;bool escaped=false;
            while(pos<source.size()){
                const char ch=source[pos++];
                if(escaped){escaped=false;continue;}if(ch=='\\'){escaped=true;continue;}if(ch=='"')break;
            }
        }else while(pos<source.size()&&!std::isspace(static_cast<unsigned char>(source[pos])))++pos;
        words.push_back({begin,pos,source.substr(begin,pos-begin)});
    }
    const auto tag=std::find_if(words.begin(),words.end(),[](const Span& word){return word.value=="FOREIGN_TRADE_4";});
    if(tag==words.end())throw std::runtime_error("schema57 fixture lacks FOREIGN_TRADE_4");
    const size_t tag_index=size_t(tag-words.begin());
    const size_t partner_width=3+kGoodIdCount+1;
    std::vector<Span> remove;
    for(size_t partner=0;partner<kForeignTradePartnerCount;++partner){
        const size_t suspension=tag_index+1+partner*partner_width+3+kGoodIdCount;
        if(suspension>=words.size())throw std::runtime_error("truncated schema57 foreign trade section");
        remove.push_back(words[suspension]);
    }
    for(auto it=remove.rbegin();it!=remove.rend();++it)source.erase(it->begin,it->end-it->begin);
    const size_t tag_position=source.find(" FOREIGN_TRADE_4 ");
    source.replace(tag_position+1,std::string("FOREIGN_TRADE_4").size(),"FOREIGN_TRADE_3");
    source.replace(0,std::string("POLYEC_REALM_57").size(),"POLYEC_REALM_56");
}
inline void downgrade_save_56_to_55(std::string& source){
        normalize_schema68_fixture(source);if((source.rfind("POLYEC_REALM_63 ",0)==0||source.rfind("POLYEC_REALM_62 ",0)==0)||source.rfind("POLYEC_REALM_61 ",0)==0||source.rfind("POLYEC_REALM_60 ",0)==0||source.rfind("POLYEC_REALM_59 ",0)==0)downgrade_save_58_to_57(source);if(source.rfind("POLYEC_REALM_58 ",0)==0)downgrade_save_58_to_57(source);
    if(source.rfind("POLYEC_REALM_57 ",0)==0)downgrade_save_57_to_56(source);
    if(source.rfind("POLYEC_REALM_56 ",0)!=0)throw std::runtime_error("fixture is not schema56");
    struct Span{size_t begin=0,end=0;std::string value;};
    std::vector<Span> words;
    for(size_t pos=0;pos<source.size();){
        while(pos<source.size()&&std::isspace(static_cast<unsigned char>(source[pos])))++pos;
        if(pos==source.size())break;
        const size_t begin=pos;
        while(pos<source.size()&&!std::isspace(static_cast<unsigned char>(source[pos])))++pos;
        words.push_back({begin,pos,source.substr(begin,pos-begin)});
    }
    const auto tag=std::find_if(words.begin(),words.end(),[](const Span& word){return word.value=="MILITARY_SERVICE_3";});
    if(tag==words.end())throw std::runtime_error("schema56 fixture lacks MILITARY_SERVICE_3");
    const size_t tag_index=size_t(tag-words.begin());
    const size_t count=std::stoull(words.at(tag_index+1).value);
    if(words.size()<tag_index+2+count*10)throw std::runtime_error("truncated schema56 military service section");
    std::vector<Span> remove;
    for(size_t row=0;row<count;++row)remove.push_back(words[tag_index+2+row*10+1]);
    for(auto it=remove.rbegin();it!=remove.rend();++it)source.erase(it->begin,it->end-it->begin);
    source.replace(0,std::string("POLYEC_REALM_56").size(),"POLYEC_REALM_55");
    const size_t service=source.find(" MILITARY_SERVICE_3 ");
    source.replace(service+1,std::string("MILITARY_SERVICE_3").size(),"MILITARY_SERVICE_2");
}
inline void downgrade_save_55_to_54(std::string& source){
        normalize_schema68_fixture(source);if(source.rfind("POLYEC_REALM_67 ",0)==0||source.rfind("POLYEC_REALM_66 ",0)==0)downgrade_save_66_to_65(source);
    if(source.rfind("POLYEC_REALM_65 ",0)==0)downgrade_save_64_to_63(source);
    if(source.rfind("POLYEC_REALM_64 ",0)==0)downgrade_save_64_to_63(source);
    if((source.rfind("POLYEC_REALM_63 ",0)==0||source.rfind("POLYEC_REALM_62 ",0)==0)||source.rfind("POLYEC_REALM_61 ",0)==0||source.rfind("POLYEC_REALM_60 ",0)==0||source.rfind("POLYEC_REALM_59 ",0)==0)downgrade_save_58_to_57(source);if(source.rfind("POLYEC_REALM_58 ",0)==0)downgrade_save_58_to_57(source);if(source.rfind("POLYEC_REALM_57 ",0)==0)downgrade_save_57_to_56(source);
    if(source.rfind("POLYEC_REALM_56 ",0)==0)downgrade_save_56_to_55(source);
    if(source.rfind("POLYEC_REALM_55 ",0)!=0)throw std::runtime_error("fixture is not schema55");
    struct Word{std::string value;size_t begin=0,end=0;};
    std::vector<Word> words;
    for(size_t pos=0;pos<source.size();){
        while(pos<source.size()&&std::isspace(static_cast<unsigned char>(source[pos])))++pos;
        if(pos==source.size())break;const size_t begin=pos;
        if(source[pos]=='"'){
            ++pos;bool escaped=false;
            while(pos<source.size()){
                const char ch=source[pos++];
                if(escaped){escaped=false;continue;}
                if(ch=='\\'){escaped=true;continue;}
                if(ch=='"')break;
            }
        }else while(pos<source.size()&&!std::isspace(static_cast<unsigned char>(source[pos])))++pos;
        words.push_back({source.substr(begin,pos-begin),begin,pos});
    }
    const auto number=[&](size_t index)->size_t{
        if(index>=words.size())throw std::runtime_error("truncated schema55 save while preparing schema54 fixture");
        return static_cast<size_t>(std::stoull(words[index].value));
    };
    std::vector<size_t> remove;
    // The three Realm-wide accounting arrays immediately follow the draft cards.
    size_t primary=18;
    for(int array=0;array<3;++array){for(size_t good=27;good<30;++good)remove.push_back(primary+good);primary+=30;}
    // Ask the production header reader for the exact end of the primary header.
    std::istringstream headerStream(source);std::string magic,bundle;
    detail::RealmPrimaryHeader header{};
    headerStream>>magic>>std::quoted(bundle);
    if(!headerStream||!detail::read_realm_primary_header_initial(headerStream,header)||
       !detail::read_realm_primary_header_tail(headerStream,header,55))
        throw std::runtime_error("cannot parse schema55 primary header for downgrade fixture");
    const auto headerEnd=headerStream.tellg();
    size_t cursor=0;while(cursor<words.size()&&words[cursor].begin<size_t(headerEnd))++cursor;
    if(cursor>=words.size())throw std::runtime_error("schema55 save has no person section");
    // Person rows: 23 scalars, the typed capability vector, then 30 inventory slots.
    const size_t people=number(cursor++);
    const size_t statCount=Person{}.stats.values.size();
    for(size_t row=0;row<people;++row){
        cursor+=23+statCount;
        for(size_t good=27;good<30;++good)remove.push_back(cursor+good);
        cursor+=30;
    }
    // Province rows: 12 scalar fields, 30 goods, then 30 matching prices.
    const size_t provinces=number(cursor++);
    for(size_t row=0;row<provinces;++row){
        cursor+=12;
        for(size_t good=27;good<30;++good)remove.push_back(cursor+good);
        cursor+=30;
        for(size_t good=27;good<30;++good)remove.push_back(cursor+good);
        cursor+=30;
    }
    const auto findTag=[&](const char* name)->size_t{
        for(size_t i=0;i<words.size();++i)if(words[i].value==name)return i;
        return words.size();
    };
    // Establishment stock is followed by variable-length work and payroll ledgers.
    size_t tag=findTag("ESTABLISH_14");
    if(tag==words.size())throw std::runtime_error("schema55 save lacks ESTABLISH_14");
    cursor=tag+2;const size_t sites=number(tag+1);
    for(size_t row=0;row<sites;++row){
        cursor+=7;
        for(size_t good=27;good<30;++good)remove.push_back(cursor+good);
        cursor+=30+2;const size_t processed=number(cursor-1);cursor+=2*processed;
        const size_t workers=number(cursor++);
        for(size_t i=0;i<workers;++i){++cursor;const size_t windows=number(cursor++);cursor+=2*windows;}
        for(int ledger=0;ledger<3;++ledger){const size_t entries=number(cursor++);cursor+=2*entries;}
        cursor+=2;
    }
    // Open-estate inventories also have one slot per canonical GoodId.
    tag=findTag("ESTATE_1");
    if(tag==words.size())throw std::runtime_error("schema55 save lacks ESTATE_1");
    cursor=tag+3;const size_t estates=number(tag+2);
    for(size_t row=0;row<estates;++row){
        cursor+=4;
        for(size_t good=27;good<30;++good)remove.push_back(cursor+good);
        cursor+=30;
        for(int list=0;list<4;++list){const size_t count=number(cursor++);cursor+=count;}
    }
    // Monthly market evidence is the only variable-width section outside records.
    tag=findTag("MARKET_2");
    if(tag==words.size())throw std::runtime_error("schema55 save lacks MARKET_2");
    cursor=tag+5;const size_t orders=number(tag+4);cursor+=14*orders;
    const size_t marketRegions=number(cursor++);
    for(size_t region=0;region<marketRegions;++region){
        for(size_t good=27;good<30;++good)for(size_t field=0;field<2;++field)
            remove.push_back(cursor+good*2+field);
        cursor+=30*2;
    }
    // Foreign accounts have three fixed partner records, each with a 30-slot ledger.
    tag=findTag("FOREIGN_TRADE_3");
    if(tag==words.size())throw std::runtime_error("schema55 save lacks FOREIGN_TRADE_3");
    cursor=tag+1;
    for(size_t partner=0;partner<kForeignTradePartnerCount;++partner){
        cursor+=3;
        for(size_t good=27;good<30;++good)remove.push_back(cursor+good);
        cursor+=30;
    }
    std::sort(remove.begin(),remove.end());
    if(std::adjacent_find(remove.begin(),remove.end())!=remove.end())
        throw std::runtime_error("overlapping schema55 inventory fields during downgrade");
    for(auto it=remove.rbegin();it!=remove.rend();++it){
        if(*it>=words.size())throw std::runtime_error("schema55 inventory field outside token stream");
        source.erase(words[*it].begin,words[*it].end-words[*it].begin);
    }
    source.replace(0,std::string("POLYEC_REALM_55").size(),"POLYEC_REALM_54");
    const std::array<std::pair<const char*,const char*>,3> tagDowngrades{{
        {"ESTABLISH_14","ESTABLISH_13"},{"MARKET_2","MARKET_1"},{"FOREIGN_TRADE_3","FOREIGN_TRADE_2"}}};
    for(const auto& [current,prior]:tagDowngrades){
        const size_t section=source.find(std::string(" ")+current+" ");
        if(section==std::string::npos)throw std::runtime_error("failed to downgrade schema55 section tag");
        source.replace(section+1,std::string(current).size(),prior);
    }
}
inline void downgrade_config3_to_v2(std::string& save){
    const auto config=save.find(" CONFIG_3 ");
    if(config==std::string::npos)return;
    const auto end=save.find(" WORKPAY_1 ",config);
    if(end==std::string::npos)throw std::runtime_error("fixture missing work-pay section after config v3");
    std::istringstream input(save.substr(config));std::string tag;int version=0;
    std::array<int64_t,18> values{};uint64_t ignored=0;input>>tag>>version;
    for(auto&value:values)input>>value;input>>ignored;
    if(!input||tag!="CONFIG_3"||version!=3)throw std::runtime_error("fixture config v3 section invalid");
    uint64_t hash=1469598103934665603ull;std::string canonical="GAMEPLAY_2";
    for(size_t i=0;i<16;++i)canonical+=" "+std::to_string(values[i]);
    for(unsigned char c:canonical){hash^=c;hash*=1099511628211ull;}
    std::ostringstream old;old<<" CONFIG_2 2";for(size_t i=0;i<16;++i)old<<' '<<values[i];old<<' '<<hash<<' ';
    save.replace(config,end-config,old.str());
}
inline std::string downgrade_establishments_7_to_6(const std::string& source){
    struct Word{std::string value;size_t begin=0,end=0;};
    std::vector<Word> words;
    for(size_t pos=0;pos<source.size();){
        while(pos<source.size()&&std::isspace(static_cast<unsigned char>(source[pos])))++pos;
        if(pos==source.size())break;const size_t begin=pos;
        while(pos<source.size()&&!std::isspace(static_cast<unsigned char>(source[pos])))++pos;
        words.push_back({source.substr(begin,pos-begin),begin,pos});
    }
    const auto tag=std::find_if(words.begin(),words.end(),[](const Word& value){return value.value=="ESTABLISH_7";});
    if(tag==words.end())throw std::runtime_error("fixture missing ESTABLISH_7 section");
    const size_t section=size_t(tag-words.begin());
    const size_t count=std::stoull(words.at(section+1).value);
    size_t cursor=section+2;std::vector<std::pair<size_t,size_t>> forestFields;forestFields.reserve(count);
    for(size_t row=0;row<count;++row){
        if(cursor+36>words.size())throw std::runtime_error("fixture truncated ESTABLISH_7 record");
        const size_t watermarkCount=std::stoull(words.at(cursor+35).value);
        cursor+=36+2*watermarkCount;
        const size_t authorizationWorkers=std::stoull(words.at(cursor++).value);
        for(size_t worker=0;worker<authorizationWorkers;++worker){
            ++cursor;const size_t windows=std::stoull(words.at(cursor++).value);cursor+=2*windows;
        }
        for(int ledger=0;ledger<3;++ledger){
            const size_t entries=std::stoull(words.at(cursor++).value);cursor+=2*entries;
        }
        const auto& forest=words.at(cursor++);
        size_t finish=forest.end;
        while(finish<source.size()&&std::isspace(static_cast<unsigned char>(source[finish])))++finish;
        forestFields.emplace_back(forest.begin,finish);
    }
    std::string result=source;
    result.replace(words.at(section).begin,words.at(section).value.size(),"ESTABLISH_6");
    result.replace(words.front().begin,words.front().value.size(),"POLYEC_REALM_36");
    std::sort(forestFields.rbegin(),forestFields.rend());
    for(const auto&[begin,finish]:forestFields)result.erase(begin,finish-begin);
    return result;
}
inline std::string downgrade_establishments_8_to_7(const std::string& source){
    struct Word{std::string value;size_t begin=0,end=0;};
    std::vector<Word> words;
    for(size_t pos=0;pos<source.size();){
        while(pos<source.size()&&std::isspace(static_cast<unsigned char>(source[pos])))++pos;
        if(pos==source.size())break;const size_t begin=pos;
        while(pos<source.size()&&!std::isspace(static_cast<unsigned char>(source[pos])))++pos;
        words.push_back({source.substr(begin,pos-begin),begin,pos});
    }
    const auto tag=std::find_if(words.begin(),words.end(),[](const Word& value){return value.value=="ESTABLISH_8";});
    if(tag==words.end())throw std::runtime_error("fixture missing ESTABLISH_8 section");
    const size_t section=size_t(tag-words.begin()),count=std::stoull(words.at(section+1).value);
    size_t cursor=section+2;std::vector<std::pair<size_t,size_t>> sourceGoodFields;sourceGoodFields.reserve(count);
    for(size_t row=0;row<count;++row){
        if(cursor+36>words.size())throw std::runtime_error("fixture truncated ESTABLISH_8 record");
        const int industry=std::stoi(words.at(cursor+4).value);
        const size_t watermarks=std::stoull(words.at(cursor+35).value);cursor+=36+2*watermarks;
        const size_t workers=std::stoull(words.at(cursor++).value);
        for(size_t worker=0;worker<workers;++worker){++cursor;const size_t windows=std::stoull(words.at(cursor++).value);cursor+=2*windows;}
        for(int ledger=0;ledger<3;++ledger){const size_t entries=std::stoull(words.at(cursor++).value);cursor+=2*entries;}
        // The ledger is the final pair in each record. Only the GoodId token
        // is removed: source_units becomes the legacy forest_units field.
        const int sourceGood=std::stoi(words.at(cursor).value);
        if((industry==int(EstablishmentIndustry::Forestry)&&sourceGood!=int(GoodId::Timber))||
           (industry!=int(EstablishmentIndustry::Forestry)&&sourceGood!=kNoEstablishmentSourceGood))
            throw std::runtime_error("fixture cannot represent non-forestry source in schema37");
        sourceGoodFields.emplace_back(words.at(cursor).begin,words.at(cursor).end);
        ++cursor;const int64_t units=std::stoll(words.at(cursor++).value);
        if(units<0||units>kEstablishmentSourceUnitsMaximum)throw std::runtime_error("fixture source units outside legacy range");
    }
    std::string result=source;
    result.replace(words.at(section).begin,words.at(section).value.size(),"ESTABLISH_7");
    result.replace(words.front().begin,words.front().value.size(),"POLYEC_REALM_37");
    std::sort(sourceGoodFields.rbegin(),sourceGoodFields.rend());
    for(const auto&[begin,end]:sourceGoodFields){size_t finish=end;while(finish<source.size()&&std::isspace(static_cast<unsigned char>(source[finish])))++finish;result.erase(begin,finish-begin);}
    return result;
}
inline std::string downgrade_establishments_9_to_8(const std::string& source){
    if(source.rfind("POLYEC_REALM_39 ",0)!=0)throw std::runtime_error("fixture is not schema39");
    std::string result=source;
    const auto tag=result.find(" ESTABLISH_9 ");
    if(tag==std::string::npos)throw std::runtime_error("fixture missing ESTABLISH_9 section");
    result.replace(tag+1,std::string("ESTABLISH_9").size(),"ESTABLISH_8");
    result.replace(0,std::string("POLYEC_REALM_39").size(),"POLYEC_REALM_38");
    return result;
}
inline std::string downgrade_establishments_11_to_10(const std::string& source){
    if(source.rfind("POLYEC_REALM_41 ",0)!=0)throw std::runtime_error("fixture is not schema41");
    std::string result=source;
    const auto tag=result.find(" ESTABLISH_11 ");
    if(tag==std::string::npos)throw std::runtime_error("fixture missing ESTABLISH_11 section");
    result.replace(tag+1,std::string("ESTABLISH_11").size(),"ESTABLISH_10");
    result.replace(0,std::string("POLYEC_REALM_41").size(),"POLYEC_REALM_40");
    return result;
}
inline std::string downgrade_establishments_12_to_11(const std::string& source){
    if(source.rfind("POLYEC_REALM_42 ",0)!=0)throw std::runtime_error("fixture is not schema42");
    std::string result=source;
    const auto tag=result.find(" ESTABLISH_12 ");
    if(tag==std::string::npos)throw std::runtime_error("fixture missing ESTABLISH_12 section");
    result.replace(tag+1,std::string("ESTABLISH_12").size(),"ESTABLISH_11");
    result.replace(0,std::string("POLYEC_REALM_42").size(),"POLYEC_REALM_41");
    return result;
}
inline std::string downgrade_organization_funds_44_to_43(const std::string& source);
inline void downgrade_empty_transport_shipment_2_to_1(std::string& source){
    const auto section=source.find(" TRANSPORT_SHIPMENT_2 ");
    if(section==std::string::npos)throw std::runtime_error("fixture missing TRANSPORT_SHIPMENT_2 section");
    std::istringstream input(source.substr(section+1));std::string tag;uint64_t next_lot=0;int64_t escrow=0;
    size_t shipment_count=0,lot_count=0;input>>tag>>next_lot>>escrow>>shipment_count>>lot_count;
    if(!input||tag!="TRANSPORT_SHIPMENT_2"||shipment_count||lot_count)
        throw std::runtime_error("legacy fixture conversion requires empty public transport section");
    source.replace(section+1,std::string("TRANSPORT_SHIPMENT_2").size(),"TRANSPORT_SHIPMENT_1");
}
inline void downgrade_military_service_2_to_1(std::string& source){
    const auto section=source.find(" MILITARY_SERVICE_2 ");
    if(section==std::string::npos)throw std::runtime_error("fixture missing MILITARY_SERVICE_2 section");
    struct Span{size_t begin=0,end=0;std::string value;};
    std::vector<Span> words;
    for(size_t pos=section+1;pos<source.size();){
        while(pos<source.size()&&std::isspace(static_cast<unsigned char>(source[pos])))++pos;
        if(pos==source.size())break;
        const size_t begin=pos;
        while(pos<source.size()&&!std::isspace(static_cast<unsigned char>(source[pos])))++pos;
        words.push_back({begin,pos,source.substr(begin,pos-begin)});
    }
    if(words.size()<2||words[0].value!="MILITARY_SERVICE_2")throw std::runtime_error("invalid MILITARY_SERVICE_2 fixture");
    const size_t count=std::stoull(words[1].value);
    if(words.size()<2+count*9)throw std::runtime_error("truncated MILITARY_SERVICE_2 fixture");
    std::vector<Span> remove;
    for(size_t row=0;row<count;++row){
        const auto& mobilized=words[2+row*9+5];
        if(mobilized.value!="0")throw std::runtime_error("cannot downgrade a save with an active muster window");
        remove.push_back(mobilized);
    }
    for(auto it=remove.rbegin();it!=remove.rend();++it)source.erase(it->begin,it->end-it->begin);
    source.replace(section+1,std::string("MILITARY_SERVICE_2").size(),"MILITARY_SERVICE_1");
}
inline void downgrade_save_51_to_50(std::string& source);
inline void downgrade_save_54_to_53(std::string& source){
        normalize_schema68_fixture(source);if((source.rfind("POLYEC_REALM_63 ",0)==0||source.rfind("POLYEC_REALM_62 ",0)==0)||source.rfind("POLYEC_REALM_61 ",0)==0||source.rfind("POLYEC_REALM_60 ",0)==0||source.rfind("POLYEC_REALM_59 ",0)==0)downgrade_save_58_to_57(source);if(source.rfind("POLYEC_REALM_58 ",0)==0)downgrade_save_58_to_57(source);if(source.rfind("POLYEC_REALM_57 ",0)==0)downgrade_save_57_to_56(source);
    if(source.rfind("POLYEC_REALM_56 ",0)==0)downgrade_save_56_to_55(source);
    if(source.rfind("POLYEC_REALM_55 ",0)==0)downgrade_save_55_to_54(source);
    if(source.rfind("POLYEC_REALM_54 ",0)!=0)throw std::runtime_error("fixture is not schema54");
    const auto section=source.find(" MILITARY_SUPPLY_OBLIGATION_1 ");
    if(section==std::string::npos)throw std::runtime_error("fixture lacks schema54 military supply obligation section");
    std::istringstream input(source.substr(section+1));
    std::string tag;size_t count=0;input>>tag>>count;
    if(!input||tag!="MILITARY_SUPPLY_OBLIGATION_1"||count!=0)
        throw std::runtime_error("cannot downgrade schema54 save with active military supply obligations");
    source.erase(section);
    source.replace(0,std::string("POLYEC_REALM_54").size(),"POLYEC_REALM_53");
}
inline void downgrade_save_53_to_52(std::string& source){
        normalize_schema68_fixture(source);if(source.rfind("POLYEC_REALM_67 ",0)==0||source.rfind("POLYEC_REALM_66 ",0)==0)downgrade_save_66_to_65(source);
    if(source.rfind("POLYEC_REALM_65 ",0)==0)downgrade_save_64_to_63(source);
    if(source.rfind("POLYEC_REALM_64 ",0)==0)downgrade_save_64_to_63(source);
    if((source.rfind("POLYEC_REALM_63 ",0)==0||source.rfind("POLYEC_REALM_62 ",0)==0)||source.rfind("POLYEC_REALM_61 ",0)==0||source.rfind("POLYEC_REALM_60 ",0)==0||source.rfind("POLYEC_REALM_59 ",0)==0)downgrade_save_58_to_57(source);if(source.rfind("POLYEC_REALM_58 ",0)==0)downgrade_save_58_to_57(source);if(source.rfind("POLYEC_REALM_57 ",0)==0)downgrade_save_57_to_56(source);
    if(source.rfind("POLYEC_REALM_56 ",0)==0||source.rfind("POLYEC_REALM_55 ",0)==0||source.rfind("POLYEC_REALM_54 ",0)==0)downgrade_save_54_to_53(source);
    if(source.rfind("POLYEC_REALM_53 ",0)!=0)throw std::runtime_error("fixture is not schema53: "+source.substr(0,15));
    const auto section=source.find(" MILITARY_RATION_DEPOT_2 ");
    const auto next=source.find(" MUNICIPAL_FUNDS_1 ",section);
    if(section==std::string::npos||next==std::string::npos)throw std::runtime_error("fixture lacks schema53 ration depot section");
    std::istringstream input(source.substr(section+1,next-section-1));
    std::string tag;size_t count=0;input>>tag>>count;
    if(!input||tag!="MILITARY_RATION_DEPOT_2")throw std::runtime_error("invalid schema53 ration section");
    std::ostringstream legacy;legacy<<" MILITARY_RATION_DEPOT_1 "<<count<<' ';
    for(size_t region=0;region<count;++region){int64_t dry=0,supplementary=0;input>>dry>>supplementary;if(!input)throw std::runtime_error("truncated schema53 ration section");legacy<<dry<<' ';}
    source.replace(section,next-section,legacy.str());
    source.replace(0,std::string("POLYEC_REALM_53").size(),"POLYEC_REALM_52");
}
inline void downgrade_save_52_to_51(std::string& source){
        normalize_schema68_fixture(source);if((source.rfind("POLYEC_REALM_63 ",0)==0||source.rfind("POLYEC_REALM_62 ",0)==0)||source.rfind("POLYEC_REALM_61 ",0)==0||source.rfind("POLYEC_REALM_60 ",0)==0||source.rfind("POLYEC_REALM_59 ",0)==0)downgrade_save_58_to_57(source);if(source.rfind("POLYEC_REALM_58 ",0)==0)downgrade_save_58_to_57(source);if(source.rfind("POLYEC_REALM_57 ",0)==0)downgrade_save_57_to_56(source);
    if(source.rfind("POLYEC_REALM_56 ",0)==0||source.rfind("POLYEC_REALM_55 ",0)==0||source.rfind("POLYEC_REALM_54 ",0)==0||source.rfind("POLYEC_REALM_53 ",0)==0)downgrade_save_53_to_52(source);
    if(source.rfind("POLYEC_REALM_52 ",0)!=0)throw std::runtime_error("fixture is not schema52");
    const std::string foreign_marker=" FOREIGN_TRADE_2 ";
    const size_t foreign_at=source.find(foreign_marker);
    const size_t obligation_at=source.find(" MILITARY_OBLIGATION_1 ",foreign_at);
    if(foreign_at==std::string::npos||obligation_at==std::string::npos)
        throw std::runtime_error("fixture lacks schema52 foreign sections");

    std::istringstream foreign(source.substr(foreign_at+foreign_marker.size()));
    std::array<ForeignTradePartner,kForeignTradePartnerCount> partners{};
    for(auto& partner:partners){
        foreign>>partner.money>>partner.escrow>>partner.salt_capacity;
        for(size_t good=0;good<27;++good)foreign>>partner.goods[good];
    }
    size_t order_count=0;foreign>>order_count;
    if(!foreign||order_count!=0)throw std::runtime_error("cannot downgrade a schema52 save with in-flight foreign orders");
    std::istringstream header(source);
    std::string magic,bundle;int day=0,regions=0,tier=0,seen=0,chosen=0;bool rerolled=false;
    uint64_t seed=0,nextId=0,nextFact=0;int64_t openingMoney=0,carrier=0,estate=0,endowment=0;
    header>>magic>>std::quoted(bundle)>>day>>regions>>seed>>nextId>>nextFact;
    header>>std::ws;const auto moneyPosition=header.tellg();
    header>>openingMoney>>carrier>>estate>>endowment>>tier>>seen>>chosen>>rerolled;
    int card=0;for(int i=0;i<3;++i)header>>card;
    int64_t openingDryFood=0;std::streampos dryFoodPosition{};
    for(size_t good=0;good<27;++good){
        header>>std::ws;const auto position=header.tellg();int64_t amount=0;header>>amount;
        if(good==size_t(GoodId::DryFood)){openingDryFood=amount;dryFoodPosition=position;}
    }
    if(!header||(regions!=1&&regions!=2&&regions!=6&&regions!=int(kAtlasTerritoryCount)-1))
        throw std::runtime_error("schema52 fixture header cannot be downgraded");
    if(regions==int(kAtlasTerritoryCount)-1){
        int64_t removedCash=0,removedDryFood=0,removedSalt=0;
        for(size_t id=1;id<partners.size();++id){
            removedCash+=partners[id].money+partners[id].escrow;
            removedDryFood+=partners[id].goods[size_t(GoodId::DryFood)];
            removedSalt+=partners[id].goods[size_t(GoodId::Salt)];
        }
        if(openingMoney<removedCash||openingDryFood<removedDryFood)
            throw std::runtime_error("schema52 Atlas fixture cannot balance removal of additional partners");
        source.replace(size_t(dryFoodPosition),std::to_string(openingDryFood).size(),std::to_string(openingDryFood-removedDryFood));
        source.replace(size_t(moneyPosition),std::to_string(openingMoney).size(),std::to_string(openingMoney-removedCash));
        if(removedSalt){
            std::istringstream refreshedHeader(source);std::string ignored,quoted;int parsedDay=0,parsedRegions=0,parsedTier=0,parsedSeen=0,parsedChosen=0;bool parsedRerolled=false;
            uint64_t parsedSeed=0,parsedNextId=0,parsedNextFact=0;int64_t parsedMoney=0,parsedCarrier=0,parsedEstate=0,parsedEndowment=0;
            refreshedHeader>>ignored>>std::quoted(quoted)>>parsedDay>>parsedRegions>>parsedSeed>>parsedNextId>>parsedNextFact
                >>parsedMoney>>parsedCarrier>>parsedEstate>>parsedEndowment>>parsedTier>>parsedSeen>>parsedChosen>>parsedRerolled;
            int parsedCard=0;for(int i=0;i<3;++i)refreshedHeader>>parsedCard;
            std::streampos saltPosition{};int64_t saltOpening=0;
            for(size_t good=0;good<27;++good){refreshedHeader>>std::ws;const auto position=refreshedHeader.tellg();int64_t amount=0;refreshedHeader>>amount;if(good==size_t(GoodId::Salt)){saltPosition=position;saltOpening=amount;}}
            if(!refreshedHeader||saltOpening<removedSalt)throw std::runtime_error("schema52 fixture cannot balance partner Salt");
            source.replace(size_t(saltPosition),std::to_string(saltOpening).size(),std::to_string(saltOpening-removedSalt));
        }
    }else{
        for(size_t id=0;id<partners.size();++id)
            if(partners[id].money||partners[id].escrow||partners[id].salt_capacity||
               std::any_of(partners[id].goods.begin(),partners[id].goods.end(),[](int64_t amount){return amount!=0;}))
                throw std::runtime_error("schema52 small-profile fixture unexpectedly has foreign accounts");
    }

    std::ostringstream legacy;
    const auto& partner=partners[0];
    legacy<<" FOREIGN_TRADE_1 "<<partner.money<<' '<<partner.escrow<<' '<<partner.salt_capacity<<' ';
    for(size_t good=0;good<27;++good)legacy<<partner.goods[good]<<' ';
    legacy<<"0 ";
    const size_t refreshedForeignAt=source.find(foreign_marker);
    const size_t refreshedObligationAt=source.find(" MILITARY_OBLIGATION_1 ",refreshedForeignAt);
    source.replace(refreshedForeignAt,refreshedObligationAt-refreshedForeignAt,legacy.str());
    source.replace(0,std::string("POLYEC_REALM_52").size(),"POLYEC_REALM_51");
}
inline void downgrade_save_50_to_49(std::string& source){
        normalize_schema68_fixture(source);if(source.rfind("POLYEC_REALM_67 ",0)==0||source.rfind("POLYEC_REALM_66 ",0)==0)downgrade_save_66_to_65(source);
    if(source.rfind("POLYEC_REALM_65 ",0)==0)downgrade_save_64_to_63(source);
    if(source.rfind("POLYEC_REALM_64 ",0)==0)downgrade_save_64_to_63(source);
    if((source.rfind("POLYEC_REALM_63 ",0)==0||source.rfind("POLYEC_REALM_62 ",0)==0)||source.rfind("POLYEC_REALM_61 ",0)==0||source.rfind("POLYEC_REALM_60 ",0)==0||source.rfind("POLYEC_REALM_59 ",0)==0)downgrade_save_58_to_57(source);if(source.rfind("POLYEC_REALM_58 ",0)==0)downgrade_save_58_to_57(source);if(source.rfind("POLYEC_REALM_57 ",0)==0)downgrade_save_57_to_56(source);
    if(source.rfind("POLYEC_REALM_56 ",0)==0||source.rfind("POLYEC_REALM_55 ",0)==0||source.rfind("POLYEC_REALM_54 ",0)==0||source.rfind("POLYEC_REALM_53 ",0)==0)downgrade_save_53_to_52(source);
    if(source.rfind("POLYEC_REALM_52 ",0)==0)downgrade_save_52_to_51(source);
    if(source.rfind("POLYEC_REALM_51 ",0)==0)downgrade_save_51_to_50(source);
    if(source.rfind("POLYEC_REALM_50 ",0)!=0)throw std::runtime_error("fixture is not schema50: "+source.substr(0,15));
    std::istringstream header(source);
    std::string magic,bundle;int day=0,regions=0,tier=0,seen=0,chosen=0;bool rerolled=false;
    uint64_t seed=0,nextId=0,nextFact=0;int64_t openingMoney=0,carrier=0,estate=0,endowment=0,openingDryFood=0;
    header>>magic>>std::quoted(bundle)>>day>>regions>>seed>>nextId>>nextFact;
    header>>std::ws;const auto moneyPosition=header.tellg();header>>openingMoney>>carrier>>estate>>endowment;
    header>>tier>>seen>>chosen>>rerolled;int card=0;for(int i=0;i<3;++i)header>>card;
    header>>std::ws;const auto dryFoodPosition=header.tellg();header>>openingDryFood;
    if(!header||(regions!=1&&regions!=2&&regions!=6&&regions!=int(kAtlasTerritoryCount)-1))
        throw std::runtime_error("schema50 fixture header cannot be downgraded");
    const auto replaceNumber=[&](std::streampos position,int64_t oldValue,int64_t decrement){
        const size_t begin=size_t(position);const std::string old=std::to_string(oldValue);
        source.replace(begin,old.size(),std::to_string(oldValue-decrement));
    };
    if(regions==int(kAtlasTerritoryCount)-1){
        if(openingMoney<kForeignTradeOpeningCash||openingDryFood<kForeignTradeOpeningDryFood)
            throw std::runtime_error("schema50 foreign endowment header cannot be downgraded");
        replaceNumber(dryFoodPosition,openingDryFood,kForeignTradeOpeningDryFood);
        replaceNumber(moneyPosition,openingMoney,kForeignTradeOpeningCash);
    }
    const auto section=source.find(" FOREIGN_TRADE_1 ");
    if(section==std::string::npos)throw std::runtime_error("fixture missing FOREIGN_TRADE_1 section");
    source.erase(section);
    source.replace(0,std::string("POLYEC_REALM_50").size(),"POLYEC_REALM_49");
}
inline void downgrade_save_51_to_50(std::string& source){
        normalize_schema68_fixture(source);if(source.rfind("POLYEC_REALM_51 ",0)!=0)throw std::runtime_error("fixture is not schema51");
    const auto section=source.find(" MILITARY_OBLIGATION_1 ");
    if(section==std::string::npos)throw std::runtime_error("fixture missing MILITARY_OBLIGATION_1 section");
    std::istringstream header(source.substr(section));std::string tag;size_t count=0;header>>tag>>count;
    if(!header||tag!="MILITARY_OBLIGATION_1"||count!=0)
        throw std::runtime_error("cannot downgrade a save with military obligations");
    source.erase(section);
    source.replace(0,std::string("POLYEC_REALM_51").size(),"POLYEC_REALM_50");
}
inline void downgrade_save_49_to_48(std::string& source);
inline void downgrade_save_48_to_47(std::string& source){
        normalize_schema68_fixture(source);if(source.rfind("POLYEC_REALM_67 ",0)==0||source.rfind("POLYEC_REALM_66 ",0)==0)downgrade_save_66_to_65(source);
    if(source.rfind("POLYEC_REALM_65 ",0)==0)downgrade_save_64_to_63(source);
    if(source.rfind("POLYEC_REALM_64 ",0)==0)downgrade_save_64_to_63(source);
    if((source.rfind("POLYEC_REALM_63 ",0)==0||source.rfind("POLYEC_REALM_62 ",0)==0)||source.rfind("POLYEC_REALM_61 ",0)==0||source.rfind("POLYEC_REALM_60 ",0)==0||source.rfind("POLYEC_REALM_59 ",0)==0)downgrade_save_58_to_57(source);if(source.rfind("POLYEC_REALM_58 ",0)==0)downgrade_save_58_to_57(source);if(source.rfind("POLYEC_REALM_57 ",0)==0)downgrade_save_57_to_56(source);
    if(source.rfind("POLYEC_REALM_56 ",0)==0||source.rfind("POLYEC_REALM_55 ",0)==0||source.rfind("POLYEC_REALM_54 ",0)==0||source.rfind("POLYEC_REALM_53 ",0)==0)downgrade_save_53_to_52(source);
    if(source.rfind("POLYEC_REALM_52 ",0)==0)downgrade_save_52_to_51(source);
    if(source.rfind("POLYEC_REALM_51 ",0)==0)downgrade_save_51_to_50(source);
    if(source.rfind("POLYEC_REALM_50 ",0)==0)downgrade_save_49_to_48(source);
    if(source.rfind("POLYEC_REALM_49 ",0)==0)downgrade_save_49_to_48(source);
    if(source.rfind("POLYEC_REALM_48 ",0)!=0)throw std::runtime_error("fixture is not schema48");
    downgrade_military_service_2_to_1(source);
    source.replace(0,std::string("POLYEC_REALM_48").size(),"POLYEC_REALM_47");
}
inline void downgrade_save_49_to_48(std::string& source){
        normalize_schema68_fixture(source);if(source.rfind("POLYEC_REALM_67 ",0)==0||source.rfind("POLYEC_REALM_66 ",0)==0)downgrade_save_66_to_65(source);
    if(source.rfind("POLYEC_REALM_65 ",0)==0)downgrade_save_64_to_63(source);
    if(source.rfind("POLYEC_REALM_64 ",0)==0)downgrade_save_64_to_63(source);
    if((source.rfind("POLYEC_REALM_63 ",0)==0||source.rfind("POLYEC_REALM_62 ",0)==0)||source.rfind("POLYEC_REALM_61 ",0)==0||source.rfind("POLYEC_REALM_60 ",0)==0||source.rfind("POLYEC_REALM_59 ",0)==0)downgrade_save_58_to_57(source);if(source.rfind("POLYEC_REALM_58 ",0)==0)downgrade_save_58_to_57(source);if(source.rfind("POLYEC_REALM_57 ",0)==0)downgrade_save_57_to_56(source);
    if(source.rfind("POLYEC_REALM_56 ",0)==0||source.rfind("POLYEC_REALM_55 ",0)==0||source.rfind("POLYEC_REALM_54 ",0)==0||source.rfind("POLYEC_REALM_53 ",0)==0)downgrade_save_53_to_52(source);
    if(source.rfind("POLYEC_REALM_52 ",0)==0)downgrade_save_52_to_51(source);
    if(source.rfind("POLYEC_REALM_51 ",0)==0)downgrade_save_51_to_50(source);
    if(source.rfind("POLYEC_REALM_50 ",0)==0)downgrade_save_50_to_49(source);
    if(source.rfind("POLYEC_REALM_49 ",0)!=0)throw std::runtime_error("fixture is not schema49");
    const auto section=source.find(" MUNICIPAL_FUNDS_1 ");
    if(section==std::string::npos)throw std::runtime_error("fixture missing MUNICIPAL_FUNDS_1 section");
    source.erase(section);
    source.replace(0,std::string("POLYEC_REALM_49").size(),"POLYEC_REALM_48");
}
inline std::string downgrade_establishments_13_to_12(const std::string& source){
    std::string current=source;
    normalize_schema68_fixture(current);
    if((current.rfind("POLYEC_REALM_63 ",0)==0||current.rfind("POLYEC_REALM_62 ",0)==0)||current.rfind("POLYEC_REALM_61 ",0)==0||current.rfind("POLYEC_REALM_60 ",0)==0||current.rfind("POLYEC_REALM_59 ",0)==0)downgrade_save_58_to_57(current);if(current.rfind("POLYEC_REALM_58 ",0)==0)downgrade_save_58_to_57(current);if(current.rfind("POLYEC_REALM_57 ",0)==0)downgrade_save_57_to_56(current);
    if(current.rfind("POLYEC_REALM_56 ",0)==0||current.rfind("POLYEC_REALM_55 ",0)==0||current.rfind("POLYEC_REALM_54 ",0)==0||current.rfind("POLYEC_REALM_53 ",0)==0)downgrade_save_53_to_52(current);
    if(current.rfind("POLYEC_REALM_52 ",0)==0)downgrade_save_52_to_51(current);
    if(current.rfind("POLYEC_REALM_51 ",0)==0)downgrade_save_51_to_50(current);
    if(current.rfind("POLYEC_REALM_50 ",0)==0)downgrade_save_49_to_48(current);
    if(current.rfind("POLYEC_REALM_49 ",0)==0)downgrade_save_49_to_48(current);
    if(current.rfind("POLYEC_REALM_48 ",0)==0)downgrade_save_48_to_47(current);
    const std::string normalized=current.rfind("POLYEC_REALM_47 ",0)==0||current.rfind("POLYEC_REALM_46 ",0)==0||current.rfind("POLYEC_REALM_45 ",0)==0||current.rfind("POLYEC_REALM_44 ",0)==0?
        downgrade_organization_funds_44_to_43(current):current;
    if(normalized.rfind("POLYEC_REALM_43 ",0)!=0)throw std::runtime_error("fixture is not schema43");
    std::string result=normalized;
    const auto tag=result.find(" ESTABLISH_13 ");
    if(tag==std::string::npos)throw std::runtime_error("fixture missing ESTABLISH_13 section");
    result.replace(tag+1,std::string("ESTABLISH_13").size(),"ESTABLISH_12");
    result.replace(0,std::string("POLYEC_REALM_43").size(),"POLYEC_REALM_42");
    return result;
}
inline std::string downgrade_organization_funds_44_to_43(const std::string& source){
    std::string normalized=source;
    normalize_schema68_fixture(normalized);
    // Current feature tests reuse old migration fixtures. Normalize the new
    // council/housing/Atlas save sections before walking back to schema44.
    if(normalized.rfind("POLYEC_REALM_67 ",0)==0||normalized.rfind("POLYEC_REALM_66 ",0)==0)downgrade_save_66_to_65(normalized);
    if(normalized.rfind("POLYEC_REALM_65 ",0)==0||normalized.rfind("POLYEC_REALM_64 ",0)==0)
        downgrade_save_64_to_63(normalized);
    if(normalized.rfind("POLYEC_REALM_63 ",0)==0||normalized.rfind("POLYEC_REALM_62 ",0)==0||
       normalized.rfind("POLYEC_REALM_61 ",0)==0||normalized.rfind("POLYEC_REALM_60 ",0)==0||
       normalized.rfind("POLYEC_REALM_59 ",0)==0||normalized.rfind("POLYEC_REALM_58 ",0)==0)
        downgrade_save_58_to_57(normalized);
    if(normalized.rfind("POLYEC_REALM_57 ",0)==0)downgrade_save_57_to_56(normalized);
    if(normalized.rfind("POLYEC_REALM_56 ",0)==0||normalized.rfind("POLYEC_REALM_55 ",0)==0||
       normalized.rfind("POLYEC_REALM_54 ",0)==0||normalized.rfind("POLYEC_REALM_53 ",0)==0)
        downgrade_save_53_to_52(normalized);
    if(normalized.rfind("POLYEC_REALM_52 ",0)==0)downgrade_save_52_to_51(normalized);
    if(normalized.rfind("POLYEC_REALM_51 ",0)==0)downgrade_save_51_to_50(normalized);
    if(normalized.rfind("POLYEC_REALM_50 ",0)==0)downgrade_save_49_to_48(normalized);
    if(normalized.rfind("POLYEC_REALM_49 ",0)==0)downgrade_save_49_to_48(normalized);
    if(normalized.rfind("POLYEC_REALM_48 ",0)==0)downgrade_save_48_to_47(normalized);
    if(normalized.rfind("POLYEC_REALM_47 ",0)==0){
        const auto depots=normalized.find(" MILITARY_RATION_DEPOT_1 ");
        if(depots==std::string::npos)throw std::runtime_error("fixture missing MILITARY_RATION_DEPOT_1 section");
        normalized.erase(depots);
        downgrade_empty_transport_shipment_2_to_1(normalized);
        normalized.replace(0,std::string("POLYEC_REALM_47").size(),"POLYEC_REALM_46");
    }
    if(normalized.rfind("POLYEC_REALM_46 ",0)==0){
        const auto depots=normalized.find(" MILITARY_RATION_DEPOT_1 ");
        if(depots!=std::string::npos)normalized.erase(depots);
        const auto service=normalized.find(" MILITARY_SERVICE_1 ");
        if(service==std::string::npos)throw std::runtime_error("fixture missing MILITARY_SERVICE_1 section");
        normalized.erase(service);
        normalized.replace(0,std::string("POLYEC_REALM_46").size(),"POLYEC_REALM_44");
    }
    if(normalized.rfind("POLYEC_REALM_45 ",0)==0){
        const auto service=normalized.find(" MILITARY_SERVICE_1 ");
        if(service==std::string::npos)throw std::runtime_error("fixture missing MILITARY_SERVICE_1 section");
        normalized.erase(service);
        normalized.replace(0,std::string("POLYEC_REALM_45").size(),"POLYEC_REALM_44");
    }
    struct Word{std::string value;size_t begin=0,end=0;};
    std::vector<Word> words;
    for(size_t pos=0;pos<normalized.size();){
        while(pos<normalized.size()&&std::isspace(static_cast<unsigned char>(normalized[pos])))++pos;
        if(pos==normalized.size())break;const size_t begin=pos;
        while(pos<normalized.size()&&!std::isspace(static_cast<unsigned char>(normalized[pos])))++pos;
        words.push_back({normalized.substr(begin,pos-begin),begin,pos});
    }
    constexpr size_t organizationBegin=2+9+7+3*kGoodIdCount+7*7;
    constexpr size_t legacyCount=28;
    if(words.empty()||words.front().value!="POLYEC_REALM_44"||
       words.size()<organizationBegin+kOrganizationCount+1)
        throw std::runtime_error("fixture expected schema44 organization balances");
    std::string result=normalized;
    result.replace(words.front().begin,words.front().value.size(),"POLYEC_REALM_43");
    const size_t begin=words[organizationBegin+legacyCount].begin;
    const size_t end=words[organizationBegin+kOrganizationCount].begin;
    result.erase(begin,end-begin);
    return result;
}
inline std::string downgrade_establishments_10_to_9(const std::string& source){
    if(source.rfind("POLYEC_REALM_40 ",0)!=0)throw std::runtime_error("fixture is not schema40");
    std::string result=source;
    const auto tag=result.find(" ESTABLISH_10 ");
    if(tag==std::string::npos)throw std::runtime_error("fixture missing ESTABLISH_10 section");
    result.replace(tag+1,std::string("ESTABLISH_10").size(),"ESTABLISH_9");
    result.replace(0,std::string("POLYEC_REALM_40").size(),"POLYEC_REALM_39");
    return result;
}
inline std::string save_as_v34(const std::string& source){
    std::string current=source;
    normalize_schema68_fixture(current);
    if(current.rfind("POLYEC_REALM_67 ",0)==0||current.rfind("POLYEC_REALM_66 ",0)==0)downgrade_save_66_to_65(current);
    if(current.rfind("POLYEC_REALM_65 ",0)==0)downgrade_save_64_to_63(current);
    if(current.rfind("POLYEC_REALM_64 ",0)==0)downgrade_save_64_to_63(current);
    if((current.rfind("POLYEC_REALM_63 ",0)==0||current.rfind("POLYEC_REALM_62 ",0)==0)||current.rfind("POLYEC_REALM_61 ",0)==0||current.rfind("POLYEC_REALM_60 ",0)==0||current.rfind("POLYEC_REALM_59 ",0)==0)downgrade_save_58_to_57(current);if(current.rfind("POLYEC_REALM_58 ",0)==0)downgrade_save_58_to_57(current);if(current.rfind("POLYEC_REALM_57 ",0)==0)downgrade_save_57_to_56(current);
    if(current.rfind("POLYEC_REALM_56 ",0)==0||current.rfind("POLYEC_REALM_55 ",0)==0||current.rfind("POLYEC_REALM_54 ",0)==0||current.rfind("POLYEC_REALM_53 ",0)==0)downgrade_save_53_to_52(current);
    if(current.rfind("POLYEC_REALM_52 ",0)==0)downgrade_save_52_to_51(current);
    if(current.rfind("POLYEC_REALM_51 ",0)==0)downgrade_save_51_to_50(current);
    if(current.rfind("POLYEC_REALM_50 ",0)==0)downgrade_save_49_to_48(current);
    if(current.rfind("POLYEC_REALM_49 ",0)==0)downgrade_save_49_to_48(current);
    if(current.rfind("POLYEC_REALM_48 ",0)==0)downgrade_save_48_to_47(current);
    if(current.rfind("POLYEC_REALM_47 ",0)==0){
        const auto depots=current.find(" MILITARY_RATION_DEPOT_1 ");
        if(depots==std::string::npos)throw std::runtime_error("fixture missing MILITARY_RATION_DEPOT_1 section");
        current.erase(depots);
        downgrade_empty_transport_shipment_2_to_1(current);
        current.replace(0,std::string("POLYEC_REALM_47").size(),"POLYEC_REALM_46");
    }
    if(current.rfind("POLYEC_REALM_46 ",0)==0){
        const auto depots=current.find(" MILITARY_RATION_DEPOT_1 ");
        if(depots!=std::string::npos)current.erase(depots);
        const auto service=current.find(" MILITARY_SERVICE_1 ");
        if(service==std::string::npos)throw std::runtime_error("fixture missing MILITARY_SERVICE_1 section");
        current.erase(service);
        current.replace(0,std::string("POLYEC_REALM_46").size(),"POLYEC_REALM_44");
    }
    if(current.rfind("POLYEC_REALM_45 ",0)==0){
        const auto service=current.find(" MILITARY_SERVICE_1 ");
        if(service==std::string::npos)throw std::runtime_error("fixture missing MILITARY_SERVICE_1 section");
        current.erase(service);
        current.replace(0,std::string("POLYEC_REALM_45").size(),"POLYEC_REALM_44");
    }
    const std::string normalized=current.rfind("POLYEC_REALM_44 ",0)==0?
        downgrade_organization_funds_44_to_43(current):current;
    std::string result=normalized.rfind("POLYEC_REALM_43 ",0)==0?
        downgrade_establishments_9_to_8(downgrade_establishments_10_to_9(downgrade_establishments_11_to_10(downgrade_establishments_12_to_11(downgrade_establishments_13_to_12(normalized))))):
        normalized.rfind("POLYEC_REALM_42 ",0)==0?
        downgrade_establishments_9_to_8(downgrade_establishments_10_to_9(downgrade_establishments_11_to_10(downgrade_establishments_12_to_11(normalized)))):
        normalized.rfind("POLYEC_REALM_41 ",0)==0?
        downgrade_establishments_9_to_8(downgrade_establishments_10_to_9(downgrade_establishments_11_to_10(normalized))):
        normalized.rfind("POLYEC_REALM_40 ",0)==0?
        downgrade_establishments_9_to_8(downgrade_establishments_10_to_9(normalized)):
        normalized.rfind("POLYEC_REALM_39 ",0)==0?downgrade_establishments_9_to_8(normalized):normalized;
    if(result.rfind("POLYEC_REALM_38 ",0)==0)
        result=downgrade_establishments_7_to_6(downgrade_establishments_8_to_7(result));
    if(result.rfind("POLYEC_REALM_36 ",0)==0){
        const auto establishment=result.find("ESTABLISH_6");
        if(establishment==std::string::npos)throw std::runtime_error("fixture missing ESTABLISH_6 section");
        result.replace(establishment,std::string("ESTABLISH_6").size(),"ESTABLISH_5");
        result.replace(0,15,"POLYEC_REALM_35");
    }
    if(result.rfind("POLYEC_REALM_35 ",0)==0){
        const auto traits=result.rfind(" PERSONALITY_1 ");
        if(traits==std::string::npos)throw std::runtime_error("fixture missing PERSONALITY_1 section");
        result.erase(traits);
        result.replace(0,15,"POLYEC_REALM_34");
    }
    if(result.rfind("POLYEC_REALM_34 ",0)==0)downgrade_config3_to_v2(result);
    return result;
}
inline std::string save_as_v33(const std::string& source){
    std::string schema33=save_as_v34(source);
    if(schema33.rfind("POLYEC_REALM_34 ",0)==0){
        struct Word {std::string value;size_t begin=0,end=0;};
        std::vector<Word> words;
        for(size_t pos=0;pos<schema33.size();){
            while(pos<schema33.size()&&std::isspace(static_cast<unsigned char>(schema33[pos])))++pos;
            if(pos==schema33.size())break;const size_t begin=pos;
            while(pos<schema33.size()&&!std::isspace(static_cast<unsigned char>(schema33[pos])))++pos;
            words.push_back({schema33.substr(begin,pos-begin),begin,pos});
        }
        const auto tag=std::find_if(words.begin(),words.end(),[](const Word& value){return value.value=="SCHEDULE_7";});
        const auto establishment=std::find_if(words.begin(),words.end(),[](const Word& value){return value.value=="ESTABLISH_5";});
        if(tag==words.end()||establishment==words.end())throw std::runtime_error("fixture missing SCHEDULE_7 section");
        const size_t at=size_t(tag-words.begin()),end=size_t(establishment-words.begin());
        if(at+13>end)throw std::runtime_error("fixture truncated SCHEDULE_7 header");
        const size_t sessions=std::stoull(words.at(at+5).value),attendance=std::stoull(words.at(at+6).value);
        const size_t settled=std::stoull(words.at(at+7).value),regional=std::stoull(words.at(at+8).value);
        const size_t paid=std::stoull(words.at(at+9).value),site=std::stoull(words.at(at+10).value);
        const size_t effort=std::stoull(words.at(at+11).value),productivity=std::stoull(words.at(at+12).value);
        const size_t rowBegin=at+13+17*sessions+2*attendance+2*settled+3*regional+3*paid+3*site+3*effort;
        const size_t rowEnd=rowBegin+3*productivity;
        if(rowEnd>end)throw std::runtime_error("fixture truncated SCHEDULE_7 productivity rows");
        std::vector<std::pair<size_t,size_t>> removals;
        if(productivity){size_t finish=words.at(rowEnd-1).end;if(finish<schema33.size()&&std::isspace(static_cast<unsigned char>(schema33[finish])))++finish;removals.emplace_back(words.at(rowBegin).begin,finish);}
        size_t countFinish=words.at(at+12).end;if(countFinish<schema33.size()&&std::isspace(static_cast<unsigned char>(schema33[countFinish])))++countFinish;
        removals.emplace_back(words.at(at+12).begin,countFinish);
        std::sort(removals.rbegin(),removals.rend());
        for(const auto&[begin,finish]:removals)schema33.erase(begin,finish-begin);
        const size_t schedule=schema33.find("SCHEDULE_7");schema33.replace(schedule,std::string("SCHEDULE_7").size(),"SCHEDULE_6");
        schema33.replace(0,15,"POLYEC_REALM_33");
    }
    return schema33;
}
inline std::string save_as_v29(const std::string& source){
    std::string schema30=save_as_v33(source);
    if(schema30.rfind("POLYEC_REALM_33 ",0)==0){
        // Schema33 adds kind/confidence after the schema32 investigation source.
        // Strip those fields to build an actual schema32 migration fixture.
        struct Word {std::string value;size_t begin=0,end=0;};
        std::vector<Word> words;
        for(size_t pos=0;pos<schema30.size();){
            while(pos<schema30.size()&&std::isspace(static_cast<unsigned char>(schema30[pos])))++pos;
            if(pos==schema30.size())break;const size_t begin=pos;
            while(pos<schema30.size()&&!std::isspace(static_cast<unsigned char>(schema30[pos])))++pos;
            words.push_back({schema30.substr(begin,pos-begin),begin,pos});
        }
        const auto knowledge=std::find_if(words.begin(),words.end(),[](const Word& word){return word.value=="KNOWLEDGE_3";});
        if(knowledge==words.end())throw std::runtime_error("fixture missing KNOWLEDGE_3 section");
        const size_t at=size_t(knowledge-words.begin()),count=std::stoull(words.at(at+1).value);
        const size_t rows=at+2,end=rows+6*count;
        if(end>words.size())throw std::runtime_error("fixture truncated KNOWLEDGE_3 rows");
        std::vector<std::pair<size_t,size_t>> removals;
        for(size_t row=0;row<count;++row){
            const size_t start=rows+6*row;
            removals.emplace_back(words[start+4].begin,words[start+5].end);
        }
        std::sort(removals.rbegin(),removals.rend());
        for(const auto&[begin,finish]:removals)schema30.erase(begin,finish-begin);
        const size_t tag=schema30.find("KNOWLEDGE_3");
        schema30.replace(tag,std::string("KNOWLEDGE_3").size(),"KNOWLEDGE_2");
        schema30.replace(0,15,"POLYEC_REALM_32");
    }
    if(schema30.rfind("POLYEC_REALM_32 ",0)==0){
        const size_t capability=schema30.rfind(" CAPABILITY_SCHEMA_1 ");
        if(capability==std::string::npos)throw std::runtime_error("fixture missing capability schema section");
        schema30.erase(capability);
        schema30.replace(0,15,"POLYEC_REALM_31");
    }
    if(schema30.rfind("POLYEC_REALM_31 ",0)==0){
        const std::string& schema31=schema30;
        struct Word {std::string value;size_t begin=0,end=0;};
        std::vector<Word> words;
        for(size_t pos=0;pos<schema31.size();){
            while(pos<schema31.size()&&std::isspace(static_cast<unsigned char>(schema31[pos])))++pos;
            if(pos==schema31.size())break;const size_t begin=pos;
            while(pos<schema31.size()&&!std::isspace(static_cast<unsigned char>(schema31[pos])))++pos;
            words.push_back({schema31.substr(begin,pos-begin),begin,pos});
        }
        const auto knowledge=std::find_if(words.begin(),words.end(),[](const Word& word){return word.value=="KNOWLEDGE_2";});
        if(knowledge==words.end())throw std::runtime_error("fixture missing KNOWLEDGE_2 section");
        const size_t at=size_t(knowledge-words.begin()),count=std::stoull(words.at(at+1).value);
        const size_t rows=at+2,end=rows+4*count;
        if(end>words.size())throw std::runtime_error("fixture truncated KNOWLEDGE_2 rows");
        std::vector<std::pair<size_t,size_t>> removals;
        removals.emplace_back(words[at].begin,words[at+1].begin);
        for(size_t row=0;row<count;++row){
            const size_t start=rows+4*row;
            removals.emplace_back(words[start+2].end,words[start+3].end);
        }
        std::sort(removals.rbegin(),removals.rend());
        for(const auto&[begin,finish]:removals)schema30.erase(begin,finish-begin);
        schema30.replace(0,15,"POLYEC_REALM_30");
    }
    if(schema30.rfind("POLYEC_REALM_30 ",0)!=0)return source;
    const std::string& current=schema30;
    struct Token {std::string value;size_t begin=0,end=0;};
    std::vector<Token> tokens;
    for(size_t pos=0;pos<current.size();){
        while(pos<current.size()&&std::isspace(static_cast<unsigned char>(current[pos])))++pos;
        if(pos==current.size())break;const size_t begin=pos;
        while(pos<current.size()&&!std::isspace(static_cast<unsigned char>(current[pos])))++pos;
        tokens.push_back({current.substr(begin,pos-begin),begin,pos});
    }
    const auto body=std::find_if(tokens.begin(),tokens.end(),[](const Token& value){return value.value=="BODY_3";});
    if(body==tokens.end())throw std::runtime_error("fixture missing BODY_3 section");
    const size_t at=size_t(body-tokens.begin()),count=std::stoull(tokens.at(at+1).value);
    const size_t rows=at+2,end=rows+11*count;
    if(end>tokens.size())throw std::runtime_error("fixture truncated BODY_3 rows");
    std::vector<std::pair<size_t,size_t>> removals;size_t convertedCount=count;
    for(size_t row=0;row<count;++row){
        const size_t start=rows+11*row;
        const bool empty= tokens.at(start+1).value=="0"&&tokens.at(start+2).value=="0"&&
            tokens.at(start+3).value=="0"&&tokens.at(start+7).value=="0"&&
            tokens.at(start+8).value=="0"&&tokens.at(start+9).value=="-1"&&
            tokens.at(start+10).value=="0";
        const size_t first=empty?start:start+4,last=empty?start+10:start+6;
        size_t finish=tokens.at(last).end;
        if(finish<current.size()&&std::isspace(static_cast<unsigned char>(current[finish])))++finish;
        removals.emplace_back(tokens.at(first).begin,finish);
        if(empty)--convertedCount;
    }
    std::string result=current;
    std::sort(removals.rbegin(),removals.rend());
    for(const auto&[begin,finish]:removals)result.erase(begin,finish-begin);
    result.replace(0,15,"POLYEC_REALM_29");
    result.replace(tokens.at(at).begin,std::string("BODY_3").size(),"BODY_2");
    result.replace(tokens.at(at+1).begin,tokens.at(at+1).value.size(),std::to_string(convertedCount));
    return result;
}
// Convert the current writer output to the immediately preceding save format
// for migration tests. SCHEDULE_5 has no effort-attention records; removing
// those records here keeps historical fixtures genuinely historical.
inline std::string save_as_v28(const std::string& source){
    const std::string v29=save_as_v29(source);
    if(v29.rfind("POLYEC_REALM_29 ",0)!=0)return v29;
    std::string result=v29;
    struct Token { std::string value;size_t begin=0,end=0; };
    auto tokenize=[](const std::string& text){
        std::vector<Token> result;
        for(size_t at=0;at<text.size();){
            while(at<text.size()&&std::isspace(static_cast<unsigned char>(text[at])))++at;
            if(at==text.size())break;
            const size_t begin=at;
            while(at<text.size()&&!std::isspace(static_cast<unsigned char>(text[at])))++at;
            result.push_back({text.substr(begin,at-begin),begin,at});
        }
        return result;
    };
    auto tokens=tokenize(result);
    auto tag=std::find_if(tokens.begin(),tokens.end(),[](const Token& token){return token.value=="SCHEDULE_6";});
    auto establish=std::find_if(tokens.begin(),tokens.end(),[](const Token& token){return token.value=="ESTABLISH_5";});
    if(tag==tokens.end()||establish==tokens.end())throw std::runtime_error("fixture missing schema 29 sections");
    const size_t at=size_t(tag-tokens.begin()),end=size_t(establish-tokens.begin());
    if(at+12>end)throw std::runtime_error("fixture truncated SCHEDULE_6 header");
    const size_t sessions=std::stoull(tokens.at(at+5).value),attendance=std::stoull(tokens.at(at+6).value);
    const size_t settled=std::stoull(tokens.at(at+7).value),regional=std::stoull(tokens.at(at+8).value);
    const size_t paid=std::stoull(tokens.at(at+9).value),site=std::stoull(tokens.at(at+10).value);
    const size_t effort=std::stoull(tokens.at(at+11).value);
    const size_t effortBegin=at+12+17*sessions+2*attendance+2*settled+3*regional+3*paid+3*site;
    const size_t effortEnd=effortBegin+3*effort;
    if(effortEnd>end)throw std::runtime_error("fixture truncated SCHEDULE_6 effort rows");
    if(effort){
        const size_t begin=tokens.at(effortBegin).begin;
        size_t finish=tokens.at(effortEnd-1).end;
        if(finish<result.size()&&std::isspace(static_cast<unsigned char>(result[finish])))++finish;
        result.erase(begin,finish-begin);
    }
    tokens=tokenize(result);
    tag=std::find_if(tokens.begin(),tokens.end(),[](const Token& token){return token.value=="SCHEDULE_6";});
    if(tag==tokens.end())throw std::runtime_error("fixture lost SCHEDULE_6 tag");
    const size_t headerAt=size_t(tag-tokens.begin());
    const size_t effortCountToken=headerAt+11;
    size_t countEnd=tokens.at(effortCountToken).end;
    if(countEnd<result.size()&&std::isspace(static_cast<unsigned char>(result[countEnd])))++countEnd;
    result.erase(tokens.at(effortCountToken).begin,countEnd-tokens.at(effortCountToken).begin);
    const size_t newTag=result.find("SCHEDULE_6");
    result.replace(newTag,std::string("SCHEDULE_6").size(),"SCHEDULE_5");
    result.replace(0,15,"POLYEC_REALM_28");
    const size_t establishmentTag=result.find(" ESTABLISH_5 ");
    if(establishmentTag>0&&std::isspace(static_cast<unsigned char>(result[establishmentTag-1]))&&
       std::isspace(static_cast<unsigned char>(result[establishmentTag-2])))
        result.erase(establishmentTag-1,1);
    return result;
}
// Explicit fixture setup through the same owner command used by players.
inline void authorize_work(Realm& world,uint64_t siteId,uint64_t worker,int64_t end=365000LL*64){
    const auto sites=world.establishments();
    const auto found=std::find_if(sites.begin(),sites.end(),[&](const auto& site){return site.id==siteId;});
    if(found==sites.end())throw std::runtime_error("fixture missing worksite");
    const auto& person=world.person(worker);
    if(!person.alive||person.destination!=-1||person.region!=found->region)return;
    const uint64_t owner=found->owner_kind==EstablishmentOwnerKind::Person?found->owner_id:
        (found->region==world.regions()?1:uint64_t(found->region+2));
    if(owner==worker)return;
    const auto existing=found->work_authorizations.find(worker);
    if(existing!=found->work_authorizations.end())for(const auto& window:existing->second)
        if(window.begin<=world.game_tick()&&window.end>=end)return;
    const auto result=world.execute(WorkAuthorizationCommand{owner,world.person(owner).decisions+1,siteId,worker,
        WorkAuthorizationOperation::Set,world.game_tick(),end});
    if(!result.success)throw std::runtime_error("fixture work authorization: "+result.reason);
}
// Test-only schema-25 conversion removes the market extension and writes the
// historical 11-value config section with its historical fingerprint.
inline std::string save_as_v25(const std::string& source){
    std::string result=save_as_v28(source);
    if(result.rfind("POLYEC_REALM_28 ",0)!=0&&result.rfind("POLYEC_REALM_27 ",0)!=0&&result.rfind("POLYEC_REALM_26 ",0)!=0)return result;
    if(result.rfind("POLYEC_REALM_28 ",0)==0){
        const auto shipments=result.find(" TRANSPORT_SHIPMENT_1 ");
        if(shipments==std::string::npos)throw std::runtime_error("fixture missing transport shipment section");
        result.erase(shipments);
        result.replace(0,15,"POLYEC_REALM_27");
    }
    if(result.rfind("POLYEC_REALM_27 ",0)==0){result.replace(0,15,"POLYEC_REALM_26");const auto route=result.find(" NATIONAL_TRANSPORT_1 ");if(route==std::string::npos)throw std::runtime_error("fixture missing national transport section");result.erase(route);}result.replace(0,15,"POLYEC_REALM_25");
    const auto market=result.find(" MARKET_1 ");
    if(market==std::string::npos)throw std::runtime_error("fixture missing market section");
    result.erase(market);
    const auto config=result.find(" CONFIG_2 ");
    const auto config_end=result.find(" WORKPAY_1 ",config);
    if(config==std::string::npos||config_end==std::string::npos)throw std::runtime_error("fixture missing config v2 section");
    std::istringstream input(result.substr(config));std::string tag;int version=0;
    input>>tag>>version;
    if(!input||tag!="CONFIG_2"||version!=2)throw std::runtime_error("fixture config v2 header invalid");
    std::array<int64_t,11> old_values{};
    for(auto&value:old_values)input>>value;
    if(!input)throw std::runtime_error("fixture config v2 values missing");
    uint64_t hash=1469598103934665603ull;std::string canonical="GAMEPLAY_1";
    for(auto value:old_values)canonical+=" "+std::to_string(value);
    for(unsigned char c:canonical){hash^=c;hash*=1099511628211ull;}
    std::ostringstream old_config;old_config<<" CONFIG_1 1";
    for(auto value:old_values)old_config<<' '<<value;
    old_config<<' '<<hash<<' ';
    result.replace(config,config_end-config,old_config.str());
    return result;
}
// Test-only historical grammar conversion; old reader never sees v16 fields.
inline std::string save_as_v16(const std::string& source){
    std::istringstream input(save_as_v25(source));std::vector<std::string> words;std::string word;
    while(input>>word)words.push_back(word);
    if(words.empty()||words.front()!="POLYEC_REALM_25")return source;
    words.front()="POLYEC_REALM_16";
    words.erase(std::find(words.begin(),words.end(),"WORKCREDIT_1"),words.end());
    const auto section=std::find(words.begin(),words.end(),"ESTABLISH_5");
    if(section==words.end())throw std::runtime_error("fixture missing payroll site section");
    size_t at=size_t(section-words.begin());words[at]="ESTABLISH_4";
    const size_t count=std::stoull(words.at(at+1));at+=2;
    for(size_t row=0;row<count;++row){
        at+=36+2*std::stoull(words.at(at+35));
        const size_t workers=std::stoull(words.at(at++));
        for(size_t n=0;n<workers;++n){const size_t ranges=std::stoull(words.at(at+1));at+=2+2*ranges;}
        const size_t first=at;
        for(int map=0;map<3;++map){const size_t entries=std::stoull(words.at(at));at+=1+2*entries;}
        words.erase(words.begin()+std::ptrdiff_t(first),words.begin()+std::ptrdiff_t(at));at=first;
    }
    std::ostringstream out;for(const auto& value:words)out<<value<<' ';return out.str();
}
inline std::string save_as_v15(const std::string& source){
    std::istringstream input(save_as_v16(source));std::vector<std::string> words;std::string word;
    while(input>>word)words.push_back(word);
    if(words.empty()||words.front()!="POLYEC_REALM_16")return source;
    words.front()="POLYEC_REALM_15";
    const auto section=std::find(words.begin(),words.end(),"ESTABLISH_4");
    if(section==words.end())throw std::runtime_error("fixture missing establishment section");
    size_t at=size_t(section-words.begin());words[at]="ESTABLISH_3";
    const size_t count=std::stoull(words.at(at+1));at+=2;
    for(size_t row=0;row<count;++row){
        at+=36+2*std::stoull(words.at(at+35));
        const size_t first=at,workers=std::stoull(words.at(at++));
        for(size_t n=0;n<workers;++n){const size_t ranges=std::stoull(words.at(at+1));at+=2+2*ranges;}
        words.erase(words.begin()+std::ptrdiff_t(first),words.begin()+std::ptrdiff_t(at));at=first;
    }
    std::ostringstream out;for(const auto& value:words)out<<value<<' ';return out.str();
}
}





