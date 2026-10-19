#include "polyec/campaign.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <set>
#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace polyec {
namespace {
uint64_t hash(const std::string&s){uint64_t n=1469598103934665603ull;for(unsigned char c:s){n^=c;n*=1099511628211ull;}return n;}
uint64_t noise(uint64_t n){n+=0x9e3779b97f4a7c15ull;n=(n^(n>>30))*0xbf58476d1ce4e5b9ull;n=(n^(n>>27))*0x94d049bb133111ebull;return n^(n>>31);}
constexpr const char* bundle="m4-controller-v1";
}
const std::vector<MapRegion>& Campaign::geography(){
    static const std::vector<MapRegion> map={
        {"Thanh Điền","Đồng bằng · lưu vực Thanh",{470,365},{{340,260},{580,260},{620,420},{520,500},{320,450}},180},
        {"Hắc Sơn","Núi sắt · đèo thu phí",{180,190},{{60,90},{260,50},{340,160},{340,260},{190,350},{50,290}},35},
        {"Bạch Khâu","Cao nguyên len · xưởng dệt",{440,160},{{260,50},{550,50},{600,140},{580,260},{340,260},{340,160}},70},
        {"Thiết Lĩnh","Đồi quặng · cửa bắc",{720,180},{{550,50},{790,80},{880,220},{790,320},{580,260},{600,140}},60},
        {"Lâm Giang","Rừng · cầu sông Thanh",{730,395},{{580,260},{790,320},{880,220},{940,430},{820,520},{620,420}},140},
        {"Vân Hải","Duyên hải · cửa sông",{800,600},{{620,420},{820,520},{940,430},{910,640},{780,720},{620,680},{520,500}},90},
        {"Đất triều đình","Ruộng hoàng gia · kho dự trữ",{360,590},{{190,350},{340,260},{320,450},{520,500},{620,680},{380,720},{190,640},{100,490}},120}
    };return map;
}
Campaign::Campaign(uint64_t seed,int population):world_(seed,6,population),seed_(seed){
    routes_={{1,2,5,80,"Đèo Hắc Sơn",true,true},{2,0,3,200,"Đường cao nguyên"},{0,4,2,240,"Cầu sông Thanh"},{4,5,2,300,"Sông ra cảng"},{0,6,2,300,"Đường kho hoàng gia"},{2,3,4,140,"Đèo phía bắc"},{3,4,4,140,"Đường rừng"},{5,6,4,240,"Đường duyên hải"},{1,6,18,60,"Đường vòng miền tây"}};
    for(auto&r:world_.routes_)r.fill(false);for(auto&r:routes_)world_.routes_[r.from][r.to]=world_.routes_[r.to][r.from]=true;
    for(int r=0;r<7;++r){auto&p=world_.provinces_[r];int days=r==6?70:(r==1?6+int(seed%4):20+int(noise(seed+r)%15));int64_t food=std::max<int64_t>(100,int64_t(p.residents)*days/7);world_.opening_[0]+=food-p.stock[0];p.stock[0]=food;p.food_capacity=food;}
    // Finite, explicit royal opening reserve for the M4 intervention workload.
    world_.opening_money_+=60000-world_.provinces_[6].treasury;world_.provinces_[6].treasury=60000;
    // Opening debt is a funded initial condition, not a scheduled story event.
    auto result=world_.execute({3,world_.person(3).decisions+1,{Intent::Borrow,1,0,200,30}});
    if(!result.success)throw std::logic_error("M4 opening loan");
    for(int r=0;r<7;++r)record(217,r,world_.provinces_[r].stock[0]);
    record(218,1,1);record(204,1,200);world_.prices();update_cases();
    if(!invariant())throw std::logic_error("M4 opening invariant");
}
void Campaign::record(int kind,int region,int64_t amount,uint64_t parent){
    journal_.push_back({next_record_++,parent,1,0,world_.day(),region,kind,amount,false});
}
uint64_t Campaign::latest(int region,int kind)const{for(auto it=journal_.rbegin();it!=journal_.rend();++it)if(it->region==region&&it->kind==kind)return it->id;return 0;}
RoutePreview Campaign::preview(int from,int to,int64_t quantity)const{
    RoutePreview out;if(from<0||from>6||to<0||to>6||quantity<1||quantity>5000)return out;
    std::array<int,7>d,prev,starts{},ends{};d.fill(1000000);prev.fill(-1);d[from]=world_.day();
    std::array<bool,7>done{};
    for(int n=0;n<7;++n){int a=-1;for(int i=0;i<7;++i)if(!done[i]&&(a<0||d[i]<d[a]))a=i;if(a<0||d[a]==1000000)break;done[a]=true;
        for(auto&r:routes_)if(r.open&&(r.from==a||r.to==a)){int b=r.from==a?r.to:r.from;int batches=int((quantity+r.capacity-1)/r.capacity);int start=d[a];
            // Each edge has a single shared daily loading capacity in both
            // directions. Reserve whole slots; final partial slots stay bounded.
            bool moved=true;while(moved){moved=false;for(auto&cargo:cargo_)if(!cargo.delivered)for(size_t j=0;j+1<cargo.path.size();++j){bool same=(cargo.path[j]==a&&cargo.path[j+1]==b)||(cargo.path[j]==b&&cargo.path[j+1]==a);if(same&&start<=cargo.leg_ends[j]&&start+batches-1>=cargo.leg_starts[j]){start=cargo.leg_ends[j]+1;moved=true;}}}
            int arrival=start+batches-1+r.days+(r.claimed?4:0);if(arrival<d[b]){d[b]=arrival;prev[b]=a;starts[b]=start;ends[b]=start+batches-1;}}
    }
    if(d[to]==1000000)return out;out.reachable=true;out.days=d[to]-world_.day();out.fee=quantity*(1+out.days)/10+5;
    for(int a=to;a>=0;a=prev[a]){out.regions.push_back(a);if(a==from)break;out.leg_starts.push_back(starts[a]);out.leg_ends.push_back(ends[a]);out.leg_arrivals.push_back(d[a]);}std::reverse(out.regions.begin(),out.regions.end());for(auto*p:{&out.leg_starts,&out.leg_ends,&out.leg_arrivals})std::reverse(p->begin(),p->end());return out;
}
CampaignResult Campaign::execute(const CampaignCommand&c){
    if(c.actor!=1||!world_.person(1).alive)return {false,"Không có thẩm quyền hoàng gia"};
    if(c.sequence!=sequence_)return {false,"Lệnh trùng hoặc hồ sơ đã thay đổi"};
    if(c.target<0||c.target>6)return {false,"Lãnh địa không tồn tại"};
    auto&crown=world_.provinces_[6];auto&target=world_.provinces_[c.target];auto&pass=routes_[0];
    auto reject=[](const char*s){return CampaignResult{false,s};};
    // All funds, materials, rights and capacity are checked before any mutation.
    switch(c.action){
    case CampaignAction::Relief:{
        if(c.target==6||c.quantity<1||c.quantity>5000)return reject("Số lương thực ngoài 1–5.000 hoặc cùng kho nguồn");
        auto path=preview(6,c.target,c.quantity);int64_t purchase=c.quantity*crown.price[0];
        if(!path.reachable||crown.stock[0]<c.quantity||crown.treasury<purchase+path.fee)return reject("Thiếu hàng, ngân sách hoặc tuyến khả dụng");
        world_.transfer(crown.treasury,crown.firm,purchase);world_.transfer(crown.treasury,world_.carrier_,path.fee);crown.stock[0]-=c.quantity;
        Cargo cargo{next_record_++,0,6,c.target,0,world_.day(),world_.day()+path.days,c.quantity,purchase+path.fee,false,path.regions,path.leg_starts,path.leg_ends,path.leg_arrivals};record(200,c.target,c.quantity,cargo.id);cargo.cause=journal_.back().id;cargo_.push_back(cargo);break;}
    case CampaignAction::Road:
        if(c.target!=1||pass.works_due||pass.days<=2||crown.treasury<300||crown.stock[3]<4||crown.stock[5]<2)return reject("Đường đang thi công, đã nâng cấp hoặc thiếu 300 đ / 4 sắt / 2 công cụ");
        world_.transfer(crown.treasury,world_.provinces_[1].firm,300);crown.stock[3]-=4;crown.stock[5]-=2;world_.consumed_[3]+=4;world_.consumed_[5]+=2;pass.works_due=world_.day()+12;record(205,1,300);break;
    case CampaignAction::Negotiate:
        if(c.target!=1||!pass.claimed||crown.treasury<120)return reject("Không có quyền thu phí cần thỏa thuận hoặc thiếu 120 đ");
        world_.transfer(crown.treasury,world_.provinces_[1].treasury,120);pass.claimed=false;record(206,1,120,latest(1,218));break;
    case CampaignAction::Arbitrate:{
        if(c.target!=1||!pass.claimed||!surveyed_)return reject("Cần hồ sơ điều tra còn hiệu lực để phân xử quyền đèo");
        pass.claimed=false;world_.provinces_[1].unrest=std::min(100,world_.provinces_[1].unrest+10);record(207,1,10,latest(1,213));break;}
    case CampaignAction::Survey:
        if(c.target!=1||inspecting_until_||crown.treasury<30)return reject("Điều tra đang chạy hoặc thiếu 30 đ");
        world_.transfer(crown.treasury,world_.provinces_[1].firm,30);inspecting_until_=world_.day()+3;record(208,1,30);break;
    case CampaignAction::Refinance:{
        int64_t debt=0;for(auto&l:world_.loans_)if(l.debtor==3)debt+=l.principal+l.interest-l.paid;
        if(c.target!=1||debt<=0||crown.treasury<debt)return reject("Không có nợ cần xử lý hoặc ngân sách không đủ");
        for(auto&l:world_.loans_)if(l.debtor==3){auto due=l.principal+l.interest-l.paid;world_.transfer(crown.treasury,world_.provinces_[l.creditor].firm,due);l.paid+=due;}record(209,1,debt,latest(1,204));break;}
    case CampaignAction::Weave:{
        std::vector<uint64_t> recruits;for(auto&p:world_.people_)if(recruits.size()<6&&p.region==2&&p.alive&&p.family==Family::Civilian&&p.age_days>=18*365&&p.busy_until<=world_.day())recruits.push_back(p.id);
        if(c.target!=2||!workers_.empty()||recruits.empty()||crown.treasury<200||target.stock[5]<2)return reject("Thiếu lao động, công cụ, 200 đ hoặc xưởng đã có vốn");
        world_.transfer(crown.treasury,world_.organization_funds_[27],200);target.stock[5]-=2;world_.consumed_[5]+=2;
        workers_=recruits;for(auto id:workers_){auto&p=world_.mutable_person(id);p.job=-1;p.busy_until=world_.day()+30;}record(210,2,int64_t(workers_.size()));break;}
    default:return reject("Lệnh không tồn tại");
    }
    ++sequence_;++revision_;world_.prices();update_cases();return {true,"Đã ghi sổ; hệ quả theo ngày game"};
}
void Campaign::controller(){
    // Explicit M4 test controller: no claim that these strategies were learned.
    // Calls the same Realm validator/command pipeline as the existing UI and RL.
    for(auto&p:world_.people_)if(p.alive&&p.family==Family::Civilian&&p.age_days>=14*365&&p.busy_until<=world_.day()&&p.id%30==uint64_t(world_.day()%30)&&std::find(workers_.begin(),workers_.end(),p.id)==workers_.end()){
        world_.execute({p.id,p.decisions+1,{Intent::Work,p.region,0,0,30}});
    }
}
void Campaign::pause(std::optional<uint64_t> expected_pause_revision){
    RealmClockCommand cmd;cmd.operation=RealmClockCommand::Operation::Pause;cmd.expected_pause_revision=expected_pause_revision;
    auto result=world_.execute(cmd);if(!result.success)throw std::runtime_error(result.reason);
}
void Campaign::resume(std::optional<uint64_t> expected_pause_revision){
    RealmClockCommand cmd;cmd.operation=RealmClockCommand::Operation::Resume;cmd.expected_pause_revision=expected_pause_revision;
    auto result=world_.execute(cmd);if(!result.success)throw std::runtime_error(result.reason);
}
void Campaign::step_one_day(std::optional<uint64_t> expected_pause_revision){
    if(expected_pause_revision&&*expected_pause_revision!=world_.pause_revision())throw std::runtime_error("Campaign clock revision changed");
    const int64_t maxTick=365000LL*kSubdayUnitsPerDay;
    if(world_.game_tick()>maxTick||kSubdayUnitsPerDay>maxTick-world_.game_tick())throw std::invalid_argument("Campaign day exceeds the world horizon");
    controller();
    RealmClockCommand cmd;cmd.operation=RealmClockCommand::Operation::StepOneDay;cmd.expected_pause_revision=expected_pause_revision;
    auto result=world_.execute(cmd);if(!result.success)throw std::runtime_error(result.reason);
    settle_day();
}
void Campaign::advance(int days){
    if(days<1||days>30)throw std::invalid_argument("M4 day chunk 1..30");
    if(world_.paused())throw std::runtime_error("Campaign is paused");
    const int64_t maxTick=365000LL*kSubdayUnitsPerDay,requested=int64_t(days)*kSubdayUnitsPerDay;
    if(world_.game_tick()>maxTick||requested>maxTick-world_.game_tick())throw std::invalid_argument("Campaign advance exceeds the world horizon");
    for(int i=0;i<days;++i){controller();world_.advance();settle_day();}
}
void Campaign::settle_day(){
    int day=world_.day();
        if(day%90==0)for(int r=0;r<7;++r){auto&v=world_.provinces_[r];int weather=45+int(noise(seed_^uint64_t(day/90)*7919^uint64_t(r/3))%76);int alive=0;for(auto&p:world_.people_)alive+=p.alive&&p.region==r;
            // Both baseline and terrain/weather variant use the one Realm rule.
            int64_t original=world_.harvest_quantity(r,0,alive);
            int64_t actual=world_.harvest_quantity(r,0,alive,weather,geography()[r].harvest);v.stock[0]+=actual-original;world_.produced_[0]+=actual-original;if(weather<70)record(201,r,actual);
        }
        for(auto&c:cargo_)if(!c.delivered&&c.arrives<=day){world_.provinces_[c.to].stock[c.good]+=c.quantity;c.delivered=true;record(202,c.to,c.quantity,c.cause);}
        auto&pass=routes_[0];if(pass.works_due&&pass.works_due<=day){if(pass.claimed){pass.works_due=day+1;if(std::none_of(journal_.begin(),journal_.end(),[](auto&f){return f.kind==211;}))record(211,1,0,latest(1,205));}else {pass.days=2;pass.capacity=300;pass.works_due=0;world_.provinces_[1].road=1;record(212,1,0,latest(1,205));}}
        if(inspecting_until_&&inspecting_until_<=day){surveyed_=true;for(auto&k:cases_)if(k.region==1&&k.closed<0)k.evidence=true;inspecting_until_=0;record(213,1,0,latest(1,208));}
        for(auto&l:world_.loans_)if(l.due==day&&l.paid<l.principal+l.interest)record(219,world_.person(l.debtor).region,l.principal+l.interest-l.paid,latest(1,204));
        if(day%7==0&&!workers_.empty()){
            auto&v=world_.provinces_[2];int n=0;for(auto id:workers_)n+=world_.person(id).alive&&world_.person(id).region==2;
            auto&account=world_.organization_funds_[27];int64_t cost=n*2+v.price[1]*2+v.price[4];
            if(n&&account>=cost&&v.stock[1]>=2&&v.stock[4]>=1){world_.transfer(account,v.firm,v.price[1]*2+v.price[4]);for(auto id:workers_)if(world_.person(id).alive&&world_.person(id).region==2)world_.transfer(account,world_.mutable_person(id).cash,2);
                v.stock[1]-=2;--v.stock[4];world_.consumed_[1]+=2;++world_.consumed_[4];v.stock[2]+=n;world_.produced_[2]+=n;workshop_output_+=n;
                // Sell output to the funded local market; the workshop cannot mint income.
                int64_t sale=std::min<int64_t>(v.firm,n*v.price[2]);world_.transfer(v.firm,account,sale);record(214,2,n,latest(2,210));
            }
        }
        world_.prices();update_cases();++revision_;if(!invariant())throw std::logic_error("M4 accounting invariant at day "+std::to_string(day)+", raw realm="+std::to_string(world_.invariant()));
}
void Campaign::update_cases(){
    for(int r=0;r<6;++r){auto&v=world_.provinces_[r];bool overdue=false;for(auto&l:world_.loans_)overdue|=world_.person(l.debtor).region==r&&l.due<=world_.day()&&l.paid<l.principal+l.interest;
        int alive=0;for(auto&p:world_.people_)alive+=p.alive&&p.region==r;
        int cover=int(v.stock[0]*7/std::max(1,alive));bool blocked=r==1&&routes_[0].claimed&&routes_[0].works_due>0;
        auto it=std::find_if(cases_.begin(),cases_.end(),[&](auto&k){return k.region==r&&k.closed<0;});
        if(it==cases_.end()&&(cover<14||overdue||blocked)){
            uint64_t source=blocked?latest(r,205):(overdue?latest(r,219):latest(r,201));if(!source)source=latest(r,217);
            record(215,r,cover,source);uint64_t cause=journal_.back().id;
            cases_.push_back({next_record_++,cause,r,world_.day(),world_.day()+2,-1,false,"Nguồn sống và nghĩa vụ tại "+geography()[r].name});
        }else if(it!=cases_.end()&&cover>=21&&!overdue&&!blocked){it->closed=world_.day();record(216,r,cover,it->cause);}
    }
}
CampaignView Campaign::view()const{
    CampaignView v;v.realm=world_.view();v.regions=geography();v.routes=routes_;v.revision=revision_;v.cargo=cargo_;v.loans=world_.loans_;v.crown=world_.provinces_[6].treasury;v.workshop_cash=world_.organization_funds_[27];v.workshop_workers=int(workers_.size());v.cloth=workshop_output_;v.inspecting_until=inspecting_until_;v.player_sequence=sequence_;v.invariant=invariant();v.paused=world_.paused();v.pause_revision=world_.pause_revision();
    for(auto&p:v.realm.provinces)p.residents=0;for(auto&p:world_.people_)if(p.alive)++v.realm.provinces[p.region].residents;
    for(auto&k:cases_)if(k.reported<=world_.day())v.cases.push_back(k);
    for(auto&f:journal_)if(f.day+2<=world_.day())v.journal.push_back(f);return v;
}
bool Campaign::invariant()const{
    std::array<std::vector<std::pair<int,int>>,9> reservations;
    std::array<int64_t,7> bonded{};for(auto&c:cargo_){if(c.good<0||c.good>6||c.quantity<=0||c.quantity>5000||c.from<0||c.from>6||c.to<0||c.to>6||c.arrives<c.depart||c.path.size()<2||c.path.front()!=c.from||c.path.back()!=c.to||c.leg_starts.size()+1!=c.path.size()||c.leg_ends.size()!=c.leg_starts.size()||c.leg_arrivals.size()!=c.leg_starts.size())return false;if(!c.delivered)bonded[c.good]+=c.quantity;
        int previous=c.depart;for(size_t j=0;j+1<c.path.size();++j){if(c.leg_starts[j]<previous||c.leg_ends[j]<c.leg_starts[j]||c.leg_arrivals[j]<=c.leg_ends[j])return false;bool found=false;for(size_t r=0;r<routes_.size();++r){auto&e=routes_[r];if((e.from==c.path[j]&&e.to==c.path[j+1])||(e.to==c.path[j]&&e.from==c.path[j+1])){reservations[r].push_back({c.leg_starts[j],c.leg_ends[j]});found=true;break;}}if(!found)return false;previous=c.leg_arrivals[j];}if(c.arrives!=previous||c.delivered!=(c.arrives<=world_.day()))return false;
    }
    for(auto&slots:reservations){std::sort(slots.begin(),slots.end());for(size_t i=1;i<slots.size();++i)if(slots[i-1].second>=slots[i].first)return false;}
    if(!world_.invariant(bonded)||world_.regions()!=6||routes_.size()!=9||sequence_<1||inspecting_until_<0||workshop_output_<0)return false;
    std::set<uint64_t> ids;uint64_t highest=0;auto unique=[&](uint64_t id){highest=std::max(highest,id);return id>0&&ids.insert(id).second;};
    for(auto&c:cargo_)if(!unique(c.id))return false;
    for(auto&f:journal_)if(!unique(f.id)||f.day<0||f.day>world_.day()||f.region<0||f.region>6)return false;
    for(auto&k:cases_)if(!unique(k.id)||k.region<0||k.region>5||k.reported<k.opened||k.closed>=0&&k.closed<k.opened)return false;
    if(next_record_<=highest)return false;
    std::set<uint64_t> staff;for(auto id:workers_)if(!id||id>uint64_t(world_.count())||!staff.insert(id).second)return false;
    for(auto&r:routes_)if(r.from<0||r.from>6||r.to<0||r.to>6||r.from==r.to||r.days<1||r.days>30||r.capacity<1||r.capacity>10000||r.works_due<0)return false;return true;
}
std::string Campaign::state()const{
    std::ostringstream s;s<<"POLYEC_M4_1 "<<seed_<<' '<<revision_<<' '<<sequence_<<' '<<next_record_<<' '<<inspecting_until_<<' '<<surveyed_<<' '<<workshop_output_<<' ';
    s<<routes_.size()<<' ';for(auto&r:routes_)s<<r.from<<' '<<r.to<<' '<<r.days<<' '<<r.capacity<<' '<<std::quoted(r.terrain)<<' '<<r.open<<' '<<r.claimed<<' '<<r.works_due<<' ';
    s<<cargo_.size()<<' ';for(auto&c:cargo_){s<<c.id<<' '<<c.cause<<' '<<c.from<<' '<<c.to<<' '<<c.good<<' '<<c.depart<<' '<<c.arrives<<' '<<c.quantity<<' '<<c.cost<<' '<<c.delivered<<' '<<c.path.size()<<' ';for(int r:c.path)s<<r<<' ';for(auto*p:{&c.leg_starts,&c.leg_ends,&c.leg_arrivals})for(int t:*p)s<<t<<' ';}
    s<<cases_.size()<<' ';for(auto&k:cases_)s<<k.id<<' '<<k.cause<<' '<<k.region<<' '<<k.opened<<' '<<k.reported<<' '<<k.closed<<' '<<k.evidence<<' '<<std::quoted(k.title)<<' ';
    s<<journal_.size()<<' ';for(auto&f:journal_)s<<f.id<<' '<<f.parent<<' '<<f.day<<' '<<f.region<<' '<<f.kind<<' '<<f.amount<<' ';
    s<<workers_.size()<<' ';for(auto id:workers_)s<<id<<' ';s<<std::quoted(world_.serialize(bundle));return s.str();
}
std::string Campaign::serialize()const{return state();}
uint64_t Campaign::fingerprint()const{return hash(state());}
bool Campaign::deserialize(const std::string&data,std::string&error,bool interactive){try{
    if(data.size()>256*1024*1024)throw std::runtime_error("M4 save exceeds size limit");
    Campaign restored(1,128);std::istringstream s(data);std::string magic;s>>magic;if(magic!="POLYEC_M4_1")throw std::runtime_error("M4 schema mismatch");
    s>>restored.seed_>>restored.revision_>>restored.sequence_>>restored.next_record_>>restored.inspecting_until_>>restored.surveyed_>>restored.workshop_output_;
    auto size=[&](size_t maximum){size_t n=0;s>>n;if(!s||n>maximum)throw std::runtime_error("M4 save record bound");return n;};
    restored.routes_.resize(size(9));for(auto&r:restored.routes_)s>>r.from>>r.to>>r.days>>r.capacity>>std::quoted(r.terrain)>>r.open>>r.claimed>>r.works_due;
    restored.cargo_.resize(size(100000));std::array<int64_t,7> bonded{};for(auto&c:restored.cargo_){s>>c.id>>c.cause>>c.from>>c.to>>c.good>>c.depart>>c.arrives>>c.quantity>>c.cost>>c.delivered;c.path.resize(size(7));for(int&r:c.path)s>>r;if(c.path.size()<2)throw std::runtime_error("M4 path missing");for(auto*p:{&c.leg_starts,&c.leg_ends,&c.leg_arrivals}){p->resize(c.path.size()-1);for(int&t:*p)s>>t;}if(c.good<0||c.good>6||c.quantity<1||c.quantity>5000)throw std::runtime_error("M4 manifest invalid");if(!c.delivered)bonded[c.good]+=c.quantity;}
    restored.cases_.resize(size(100000));for(auto&k:restored.cases_)s>>k.id>>k.cause>>k.region>>k.opened>>k.reported>>k.closed>>k.evidence>>std::quoted(k.title);
    restored.journal_.resize(size(1000000));for(auto&f:restored.journal_)s>>f.id>>f.parent>>f.day>>f.region>>f.kind>>f.amount;
    restored.workers_.resize(size(6));for(auto&id:restored.workers_)s>>id;std::string realm;s>>std::quoted(realm);if(!s)throw std::runtime_error("M4 save incomplete");s>>std::ws;if(!s.eof())throw std::runtime_error("M4 save trailing data");
    if(!restored.world_.deserialize(realm,bundle,error,bonded))throw std::runtime_error("M4 world/accounting invalid: "+error);
    if(interactive)restored.pause();
    if(!restored.invariant())throw std::runtime_error("M4 world/accounting invalid: "+error);
    *this=std::move(restored);error.clear();return true;
}catch(const std::exception&e){error=e.what();return false;}}
bool Campaign::save(const std::string&path,std::string&error)const{try{
    auto dest=std::filesystem::u8path(path),temp=dest;temp+=".tmp";auto data=serialize();{std::ofstream f(temp,std::ios::binary|std::ios::trunc);f<<hash(data)<<'\n'<<data;f.flush();if(!f)throw std::runtime_error("M4 save write failed");}
    auto backup=dest;backup+=".bak";if(std::filesystem::exists(dest))std::filesystem::copy_file(dest,backup,std::filesystem::copy_options::overwrite_existing);
    // Never remove the current save before replacing it. A failed replacement
    // must leave both the live file and the recoverable backup available.
#ifdef _WIN32
    if(!MoveFileExW(temp.c_str(),dest.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))throw std::runtime_error("M4 save replace failed: "+std::to_string(GetLastError()));
#else
    std::filesystem::rename(temp,dest);
#endif
    error.clear();return true;
}catch(const std::exception&e){error=e.what();return false;}}
bool Campaign::load(const std::string&path,std::string&error,bool interactive){try{
    auto p=std::filesystem::u8path(path);if(std::filesystem::file_size(p)>256*1024*1024)throw std::runtime_error("M4 save exceeds size limit");std::ifstream f(p,std::ios::binary);uint64_t expected=0;f>>expected;f.get();std::string data((std::istreambuf_iterator<char>(f)),{});if(!f||expected!=hash(data))throw std::runtime_error("M4 save checksum failed");return deserialize(data,error,interactive);
}catch(const std::exception&e){error=e.what();return false;}}
}
