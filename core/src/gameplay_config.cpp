#include "polyec/gameplay_config.hpp"
#include <algorithm>
#include <charconv>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <locale>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>

namespace polyec {
namespace {
constexpr std::array<std::string_view,18> names{
    "work.ticks_per_unit", "work.wage_per_unit", "establishment.default_capacity",
    "recipes.cloth.fiber_input", "recipes.cloth.cloth_output",
    "recipes.tools.iron_input", "recipes.tools.fuel_input", "recipes.tools.tools_output",
    "recipes.military_supply.tools_input", "recipes.military_supply.cloth_input",
    "recipes.military_supply.military_supply_output", "market.system_version",
    "market.response_basis_points", "market.maximum_change_basis_points",
    "market.price_floor", "market.price_ceiling",
    "recipes.salt.fuel_input", "recipes.salt.salt_output"};
constexpr std::array<int64_t,18> minimum{1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1};
constexpr std::array<int64_t,18> maximum{64,1000000,1000,1000000,1000000,1000000,1000000,1000000,1000000,1000000,1000000,1,10000,10000,1000000,1000000,1000000,1000000};
constexpr GameplayConfig::Values historical_values{16,1,1,1,1,1,1,1,1,1,1,0,1000,1000,1,1000000,1,1};
constexpr GameplayConfig::Values current_defaults{16,1,1,1,1,1,1,1,1,1,1,1,1000,1000,1,1000000,1,1};
std::string_view trim(std::string_view s){
    const auto first=s.find_first_not_of(" \t\r");
    if(first==std::string_view::npos)return {};
    return s.substr(first,s.find_last_not_of(" \t\r")-first+1);
}
[[noreturn]] void fail(std::string_view path,std::string_view message){
    throw std::invalid_argument("Gameplay config "+std::string(path)+": "+std::string(message));
}
void validate_encoding(std::string_view text){
    for(size_t i=0;i<text.size();){
        const auto lead=static_cast<unsigned char>(text[i]);
        if(lead<0x80){
            if((lead<0x20&&lead!='\t'&&lead!='\r'&&lead!='\n')||lead==0x7f)
                fail("$encoding","forbidden control character");
            if(lead=='\r'&&(i+1==text.size()||text[i+1]!='\n'))fail("$encoding","expected LF or CRLF newline");
            ++i;continue;
        }
        const int width=lead>=0xc2&&lead<=0xdf?2:lead>=0xe0&&lead<=0xef?3:lead>=0xf0&&lead<=0xf4?4:0;
        if(width==0||text.size()-i<size_t(width))fail("$encoding","invalid UTF-8");
        uint32_t point=lead&((1u<<(7-width))-1u);
        for(int n=1;n<width;++n){
            const auto next=static_cast<unsigned char>(text[i+size_t(n)]);
            if((next&0xc0)!=0x80)fail("$encoding","invalid UTF-8 continuation");
            point=(point<<6)|(next&0x3f);
        }
        if(point<(width==2?0x80u:width==3?0x800u:0x10000u)||point>0x10ffffu||(point>=0xd800&&point<=0xdfff))
            fail("$encoding","invalid UTF-8 code point");
        i+=size_t(width);
    }
}
int64_t integer(std::string_view s,std::string_view path){
    s=trim(s);
    if(s.empty())fail(path,"expected decimal integer");
    // The supported TOML subset deliberately excludes integer separators,
    // hexadecimal notation and floats. Leading zeros are invalid TOML.
    const auto digits=(s[0]=='+'||s[0]=='-')?s.substr(1):s;
    if(digits.empty()||(digits.size()>1&&digits.front()=='0'))fail(path,"invalid decimal integer");
    for(char c:digits)if(c<'0'||c>'9')fail(path,"expected decimal integer");
    if(s.front()=='+')s.remove_prefix(1);
    int64_t value=0;
    const auto result=std::from_chars(s.data(),s.data()+s.size(),value);
    if(result.ec!=std::errc{}||result.ptr!=s.data()+s.size())fail(path,"integer outside int64 range");
    return value;
}
}
GameplayConfig GameplayConfig::from_values(const Values& values){
    for(size_t i=0;i<values.size();++i)
        if(values[i]<minimum[i]||values[i]>maximum[i])fail(names[i],"outside valid range "+std::to_string(minimum[i])+".."+std::to_string(maximum[i]));
    if(values[14]>values[15])fail("market.price_floor","must not exceed market.price_ceiling");
    GameplayConfig config;config.values_=values;return config;
}
GameplayConfig GameplayConfig::historical(){return from_values(historical_values);}
GameplayConfig GameplayConfig::parse(std::string_view text){
    if(text.size()>65536)fail("$","file exceeds 64 KiB");
    validate_encoding(text);
    Values values=current_defaults;std::set<std::string> seen,tables;
    std::string table;int version=0;
    std::istringstream input{std::string(text)};std::string line;
    size_t lineNumber=0;
    while(std::getline(input,line)){
        ++lineNumber;
        auto row=trim(line);const auto comment=row.find('#');if(comment!=row.npos)row=trim(row.substr(0,comment));
        if(row.empty())continue;
        if(row.front()=='['){
            if(row.size()<3||row.back()!=']')fail("line "+std::to_string(lineNumber),"invalid table");
            table=std::string(trim(row.substr(1,row.size()-2)));
            if(table!="work"&&table!="establishment"&&table!="recipes.cloth"&&table!="recipes.tools"&&table!="recipes.military_supply"&&table!="recipes.salt"&&table!="market")fail(table,"unknown table");
            if(!tables.insert(table).second)fail(table,"duplicate table");
            continue;
        }
        const auto equal=row.find('=');
        if(equal==row.npos)fail("line "+std::to_string(lineNumber),"expected key = integer");
        auto key=trim(row.substr(0,equal));
        if(key.empty())fail("line "+std::to_string(lineNumber),"empty key");
        for(char c:key)if(!((c>='a'&&c<='z')||c=='_'))fail(key,"unsupported key syntax");
        std::string path=table.empty()?std::string(key):table+"."+std::string(key);
        if(!seen.insert(path).second)fail(path,"duplicate key");
        if(path=="schema_version"){
            const auto parsed=integer(row.substr(equal+1),path);
            if(parsed<1||parsed>3)fail(path,"unsupported version");
            version=int(parsed);continue;
        }
        size_t index=0;while(index<names.size()&&names[index]!=path)++index;
        if(index==names.size())fail(path,"unknown key");
        values[index]=integer(row.substr(equal+1),path);
    }
    if(!version)fail("schema_version","missing key");
    for(size_t i=0;i<11;++i)if(!seen.count(std::string(names[i])))fail(names[i],"missing key");
    if(version>=2)for(size_t i=11;i<16;++i)if(!seen.count(std::string(names[i])))fail(names[i],"missing key");
    if(version>=3)for(size_t i=16;i<names.size();++i)if(!seen.count(std::string(names[i])))fail(names[i],"missing key");
    return from_values(values);
}
GameplayConfig GameplayConfig::load(const std::string& path){
    std::ifstream file(std::filesystem::u8path(path),std::ios::binary);
    if(!file)fail(path,"cannot open file");
    // Bound reading as well as parsing, including nonregular input paths.
    std::string text;char buffer[4096];
    while(file.read(buffer,sizeof buffer)||file.gcount()){
        text.append(buffer,static_cast<size_t>(file.gcount()));
        if(text.size()>65536)fail(path,"file exceeds 64 KiB");
    }
    if(!file.eof())fail(path,"read failed");
    return parse(text);
}
std::string GameplayConfig::canonical()const{
    std::ostringstream out;out.imbue(std::locale::classic());out<<"GAMEPLAY_3";for(auto value:values_)out<<' '<<value;return out.str();
}
uint64_t GameplayConfig::fingerprint()const{
    uint64_t hash=1469598103934665603ull;for(unsigned char c:canonical()){hash^=c;hash*=1099511628211ull;}return hash;
}
void write_gameplay_config(std::ostream& out,const GameplayConfig& config){
    out<<" CONFIG_3 3 ";for(auto value:config.values())out<<value<<' ';out<<config.fingerprint()<<' ';
}
GameplayConfig read_gameplay_config(std::istream& in){
    std::string tag;int version=0;uint64_t hash=0,expected_hash=0;in>>tag>>version;
    GameplayConfig config;
    if(tag=="CONFIG_1"&&version==1){
        GameplayConfig::Values values=historical_values;
        for(size_t i=0;i<11;++i)in>>values[i];
        config=GameplayConfig::from_values(values);
        std::ostringstream oldCanonical;oldCanonical.imbue(std::locale::classic());oldCanonical<<"GAMEPLAY_1";
        for(size_t i=0;i<11;++i)oldCanonical<<' '<<values[i];
        expected_hash=1469598103934665603ull;
        for(unsigned char c:oldCanonical.str()){expected_hash^=c;expected_hash*=1099511628211ull;}
    }else if(tag=="CONFIG_2"&&version==2){
        std::array<int64_t,16> old{};for(auto&value:old)in>>value;
        GameplayConfig::Values values{};std::copy(old.begin(),old.end(),values.begin());values[16]=values[17]=1;
        config=GameplayConfig::from_values(values);
        std::ostringstream oldCanonical;oldCanonical.imbue(std::locale::classic());oldCanonical<<"GAMEPLAY_2";
        for(auto value:old)oldCanonical<<' '<<value;
        expected_hash=1469598103934665603ull;
        for(unsigned char c:oldCanonical.str()){expected_hash^=c;expected_hash*=1099511628211ull;}
    }else if(tag=="CONFIG_3"&&version==3){
        GameplayConfig::Values values{};for(auto&value:values)in>>value;
        config=GameplayConfig::from_values(values);expected_hash=config.fingerprint();
    }else fail("$save","invalid config section");
    in>>hash;
    if(!in)fail("$save","invalid config section");
    if(expected_hash!=hash)fail("$save.fingerprint","mismatch");
    return config;
}
} // namespace polyec
