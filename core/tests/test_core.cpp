#include "polyec/sim.hpp"
#include <iostream>
#include <limits>
#include <memory>
using namespace polyec;
int checks = 0, failures = 0;
void check(bool value, const char* expr, int line) {
    ++checks;
    if (!value) { ++failures; std::cerr << "FAIL line " << line << ": " << expr << '\n'; }
}
#define CHECK(x) check((x), #x, __LINE__)
std::shared_ptr<GameWorld> make_world() {
    auto w = std::make_shared<GameWorld>();
    w->add_territory({EntityId{7}, "Thanh Điền", 0,0,100,100});
    w->add_territory({EntityId{8}, "Hắc Sơn", 200,0,100,100});
    return w;
}
// Frame deltas stay below the sleep threshold; binary fractions are exact.
void frames(SimulationController& c, int n) { for(int i=0;i<n;++i) c.update(0.125); }
int script(bool fake) {
    auto w=make_world(); SimulationController c(w);
    c.set_fake_time(fake); CHECK(c.is_fake_time()==fake);
    c.set_paused(false); frames(c,20); return w->get_date().day;
}
int main() {
    auto w=make_world(); SimulationController c(w);
    CHECK(c.is_paused()); frames(c,20); CHECK(w->get_date().day==0);
    CHECK(!c.step_day().success);
    CHECK(w->execute({CommandType::SelectTerritory,EntityId{8}}).success);
    for(auto id : {EntityId{},EntityId{999}}) {
        CHECK(!w->execute({CommandType::SelectTerritory,id}).success);
        CHECK(w->get_selected()==EntityId{8}); CHECK(w->get_date().day==0);
    }
    CHECK(w->territories()[0].id==EntityId{7});
    CHECK(w->territories()[1].id==EntityId{8});
    CHECK(!w->add_territory({EntityId{7}, "duplicate", 0,0,1,1}));
    CHECK(!w->add_territory({EntityId{}, "invalid", 0,0,1,1}));
    auto view=w->player_view(); view.territories[0].name="changed copy";
    CHECK(w->territories()[0].name != "changed copy");
    w->execute({CommandType::Resume,{}}); CHECK(!c.is_paused());
    frames(c,7); CHECK(w->get_date().day==0); CHECK(c.pending_seconds()==0.875);
    frames(c,1); CHECK(w->get_date().day==1); CHECK(c.pending_seconds()==0.0);
    frames(c,4); c.set_paused(true); CHECK(w->is_paused()); CHECK(c.pending_seconds()==0.0);
    frames(c,40); c.set_paused(false); frames(c,7); CHECK(w->get_date().day==1);
    frames(c,1); CHECK(w->get_date().day==2);
    w->execute({CommandType::SetPause,{}}); CHECK(c.is_paused()); CHECK(!c.step_day().success);
    w->execute({CommandType::Resume,{}}); CHECK(c.step_day().success); CHECK(w->get_date().day==3);
    frames(c,3); const double pending=c.pending_seconds();
    const double nan=std::numeric_limits<double>::quiet_NaN(), inf=std::numeric_limits<double>::infinity();
    for(double bad : {nan,inf,-inf,-1.0}) {
        c.update(bad); CHECK(!c.is_paused()); CHECK(c.pending_seconds()==pending);
        CHECK(w->get_date().day==3); CHECK(!c.set_time_scale(bad)); CHECK(c.get_time_scale()==1.0);
    }
    c.update(0.0); CHECK(c.pending_seconds()==pending);
    c.update(30.0); CHECK(c.is_paused()); CHECK(c.pending_seconds()==0.0); CHECK(w->get_date().day==3);
    c.set_paused(false); c.update(SimulationController::kMaxRealDeltaSeconds); CHECK(!c.is_paused());
    c.update(SimulationController::kMaxRealDeltaSeconds+0.001); CHECK(c.is_paused()); CHECK(w->get_date().day==3);
    c.set_paused(false); CHECK(c.set_time_scale(4.0)); frames(c,2); CHECK(w->get_date().day==4);
    CHECK(c.set_time_scale(0.0)); frames(c,20); CHECK(w->get_date().day==4);
    CHECK(!w->execute({CommandType::None,{}}).success);
    CHECK(!c.set_time_scale(5.0));
    c.set_time_scale(1.0); c.set_paused(false); frames(c,4);
    CHECK(c.pending_seconds()==0.5);
    w->execute({CommandType::SetPause,{}});
    w->execute({CommandType::Resume,{}});
    frames(c,4); CHECK(w->get_date().day==4);
    CHECK(c.pending_seconds()==0.5);
    const int days=script(true); CHECK(days==2); CHECK(script(true)==days); CHECK(script(false)==days);
    std::cout << checks-failures << "/" << checks << " checks passed\n";
    return failures==0 ? 0 : 1;
}

