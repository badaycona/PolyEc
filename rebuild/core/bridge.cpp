#include "model.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
using namespace pe;
std::filesystem::path utfpath(const std::string&s){return std::filesystem::path(std::u8string(reinterpret_cast<const char8_t*>(s.data()),s.size()));}
std::string read(const std::string&path){std::ifstream f(utfpath(path),std::ios::binary);require(bool(f),"Cannot open save file");f.seekg(0,std::ios::end);require(f.tellg()<=16000000,"Save file too large");f.seekg(0);return {std::istreambuf_iterator<char>(f),{}};}
void write(const std::string&path,const std::string&text){auto tmp=path+".tmp";auto dest=utfpath(path),temp=utfpath(tmp);{std::ofstream f(temp,std::ios::binary|std::ios::trunc);require(bool(f),"Cannot write save temporary");f<<text;f.flush();require(bool(f),"Save write failed");}
#ifdef _WIN32
 require(MoveFileExW(temp.c_str(),dest.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0,"Atomic save replacement failed");
#else
 std::filesystem::rename(temp,dest);
#endif
}
int run(int argc,char**argv){try{require(argc>=5,"Usage: core state operation viewer debug [args]");std::string path=argv[1],op=argv[2];Id viewer=std::stoi(argv[3]);bool debug=std::string(argv[4])=="1";World w;Result r{true,"Ready"};
 if(op!="reset"){r=w.load(read(path));require(r.ok,r.message);}else w=World(argc>5?std::stoul(argv[5]):251025,argc>6?std::string(argv[6])=="2":true);
 person(w.state(),viewer);
 if(op=="step"){require(argc>5,"Step count missing");r=w.advance(std::stoi(argv[5]));}
 else if(op=="command"){require(argc>=12,"Command arguments missing");Command c;c.actor=std::stoi(argv[5]);require(debug||c.actor==viewer,"Player cannot control another NPC; debug required");c.verb=argv[6];c.target=std::stoi(argv[7]);c.amount=std::stoi(argv[8]);c.option=std::stoi(argv[9]);c.revision=std::stoull(argv[10]);c.text=std::string(argv[11])=="__EMPTY__"?"":argv[11];for(int j=12;j<argc;++j){std::string token=argv[j];auto eq=token.find('=');require(eq!=std::string::npos&&eq>0,"Expected named parameter key=value");std::string name=token.substr(0,eq),number=token.substr(eq+1);size_t used=0;int value=std::stoi(number,&used);require(used==number.size(),"Invalid numeric parameter");require(c.params.emplace(name,value).second,"Duplicate named parameter");}r=w.submit(c);}
 else if(op=="load"){require(argc>5,"Load path missing");r=w.load(read(argv[5]));}
 else if(op=="save"){require(argc>5,"Save path missing");write(argv[5],w.save());r={true,"Saved native core state"};}
 else require(op=="view"||op=="reset","Unknown bridge operation");
 if(r.ok&&op!="view"&&op!="save")write(path,w.save());
 std::string view=w.view(viewer,debug);std::cout<<"{\"ok\":"<<(r.ok?"true":"false")<<",\"message\":"<<std::quoted(r.message)<<",\"world\":"<<view<<"}\n";return 0;
 }catch(const std::exception&e){std::cout<<"{\"ok\":false,\"message\":"<<std::quoted(e.what())<<"}\n";return 1;}}

#ifdef _WIN32
int wmain(int argc,wchar_t**args){std::vector<std::string> storage;std::vector<char*> argv;for(int i=0;i<argc;++i){int n=WideCharToMultiByte(CP_UTF8,0,args[i],-1,nullptr,0,nullptr,nullptr);std::string a(n,0);WideCharToMultiByte(CP_UTF8,0,args[i],-1,a.data(),n,nullptr,nullptr);a.pop_back();storage.push_back(std::move(a));}for(auto&a:storage)argv.push_back(a.data());return run(argc,argv.data());}
#else
int main(int argc,char**argv){return run(argc,argv);}
#endif

