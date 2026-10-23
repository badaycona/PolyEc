import argparse,subprocess,json,time
import numpy as np
import polyec_env as core
catalog=core.CAPABILITY_CATALOG
assert [row["id"] for row in catalog]==list(range(7))
assert len({row["key"] for row in catalog})==7
assert all(row["owner"]=="Person" and row["minimum"]==0 and row["maximum"]==100 for row in catalog)
assert all(row["scenario_configurable"] and row["disabled_value"]==50 for row in catalog[:6])
assert catalog[6]=={"id":6,"key":"labor_skill","unit":"labor skill point","minimum":0,"maximum":100,
                    "owner":"Person","scenario_configurable":False,"always_active":True,"disabled_value":-1}
capability_person=core.Realm(19,1,128).person(1)
capability_names=("strategy","administration","economic_planning","diplomacy","investigation","military")
assert tuple(capability_person["legacy_capabilities"])==capability_names
assert tuple(capability_person["legacy_capabilities"][name] for name in capability_names)==tuple(capability_person["stats"])
assert capability_person["capabilities"]["labor_skill"]==capability_person["skill"]
assert "labor_skill" in capability_person["capability_schema"]
capability_world=core.Realm(21,1,128,capability_mask=0b101001)
capability_actor=capability_world.person(1)
assert capability_world.view(1)["knowledge"]==[]
assert capability_world.capability_mask==0b101001
assert tuple(capability_actor["active_capabilities"])==("strategy","diplomacy","military")
assert tuple(capability_actor["capability_schema"])==("strategy","diplomacy","military","labor_skill")
assert capability_world.model_contract.endswith("-caps-41")
assert capability_world.observe(1)[11]==0.5 and capability_world.observe(1)[14]==0.5
scenario_world=core.Realm.scenario(21,129,capability_mask=0b101001)
assert scenario_world.capability_mask==capability_world.capability_mask
p=argparse.ArgumentParser();p.add_argument('--replay',required=True);a=p.parse_args()
rows=subprocess.check_output([a.replay],text=True).splitlines();w=core.Realm(37,6,256)
checks=0
for day,row in enumerate(rows):
    for family in (0,1,2):
        for actor in w.due(family):
            mask=w.mask(actor);action=(actor+3*day)%core.REALM_ACTIONS
            if not mask[action]:action=0
            assert w.step(actor,action)[0]
    w.advance();d,fp,money=map(int,row.split());v=w.view()
    assert v['day']==d and w.fingerprint()==fp and v['money']==money and w.invariant();checks+=4
assert all(len(province['stock'])==7 and len(province['prices'])==7 for province in v['provinces']);checks+=1
batch=core.RealmBatch(32,37,2,6,64);before,_=batch.observe();invalid=np.zeros(32,dtype='int64');invalid[-1]=core.REALM_ACTIONS
try:batch.step(invalid);raise AssertionError('Invalid action accepted')
except ValueError:pass
after,_=batch.observe();assert np.array_equal(before,after);checks+=1
state=w.serialize('binding-check');fingerprint=w.fingerprint();assert not w.deserialize(state,'wrong')[0] and w.fingerprint()==fingerprint;checks+=1
view=w.view();regions=len(view['provinces'])-1;titles=w.land_holdings();assert titles and all(t['id']>view['persons'] and t['area_units']>0 and 0<=t['region']<=regions and 1<=t['title_holder']<=view['persons'] for t in titles);checks+=1
titled=w.serialize('land-binding-check');land_copy=core.Realm(1,1,128);assert land_copy.deserialize(titled,'land-binding-check')[0] and land_copy.land_holdings()==titles;checks+=1
transfer_world=core.Realm(91,2,128); transfer_titles=transfer_world.land_holdings()
adult=next(p for p in range(1,129) if transfer_world.person(p)['alive'] and transfer_world.person(p)['age_days']>=18*365)
minor=next(p for p in range(1,129) if transfer_world.person(p)['alive'] and transfer_world.person(p)['age_days']<18*365)
owner_title=next(t for t in transfer_titles if t['title_holder']==adult)
money=transfer_world.view()['money']; before=transfer_world.fingerprint()
ok,reason,fact=transfer_world.land_title_transfer(adult,transfer_world.person(adult)['decisions']+1,owner_title['id'],minor)
assert ok,(reason,fact); updated=next(t for t in transfer_world.land_holdings() if t['id']==owner_title['id'])
assert updated['title_holder']==minor and updated['region']==owner_title['region'] and updated['area_units']==owner_title['area_units']
assert transfer_world.view()['money']==money and transfer_world.fingerprint()!=before and transfer_world.invariant(); checks+=1
rejected_before=transfer_world.fingerprint(); other=next(p for p in range(1,129) if p not in (minor,adult) and transfer_world.person(p)['alive'] and transfer_world.person(p)['age_days']>=18*365)
ok,reason,_=transfer_world.land_title_transfer(other,transfer_world.person(other)['decisions']+1,owner_title['id'],adult)
assert not ok and reason=='Actor is not title holder' and transfer_world.fingerprint()==rejected_before; checks+=1
use_world=core.Realm(92,2,128); use_titles=use_world.land_holdings()
use_adult=next(p for p in range(1,129) if use_world.person(p)['alive'] and use_world.person(p)['age_days']>=18*365)
use_minor=next(p for p in range(1,129) if use_world.person(p)['alive'] and use_world.person(p)['age_days']<18*365)
use_title=next(t for t in use_titles if t['title_holder']==use_adult)
ok,reason,fact=use_world.land_use_grant(use_adult,use_world.person(use_adult)['decisions']+1,use_title['id'],use_minor)
rights=use_world.land_use_rights(); assert ok and len(rights)==1 and rights[0]['holding_id']==use_title['id'] and rights[0]['grantee']==use_minor and rights[0]['grantor']==use_adult and fact>0; checks+=1
use_before=use_world.fingerprint(); ok,reason,_=use_world.land_use_grant(use_adult,use_world.person(use_adult)['decisions'],use_title['id'],use_minor)
assert not ok and use_world.fingerprint()==use_before; checks+=1
use_save=use_world.serialize('land-use-binding'); use_restored=core.Realm(1,1,128)
assert use_restored.deserialize(use_save,'land-use-binding')[0] and use_restored.land_use_rights()==rights and use_restored.fingerprint()==use_before; checks+=1
ok,reason,_=use_world.land_title_transfer(use_adult,use_world.person(use_adult)['decisions']+1,use_title['id'],use_minor)
assert not ok and reason=='Revoke land use right before title transfer' and use_world.fingerprint()==use_before; checks+=1
ok,reason,fact=use_world.land_use_revoke(use_adult,use_world.person(use_adult)['decisions']+1,use_title['id'])
assert ok and fact>0 and not use_world.land_use_rights() and use_world.invariant(); checks+=1
# Legacy schema conversion is checked by the dedicated C++ land-title test,
# which owns the source-format fixture. This Python adapter check confirms that
# current bindings expose the persisted use-right record and its endpoints.
assert "POLYEC_REALM_52 " in use_save and " LANDUSE_1 1 " in use_save
assert use_restored.land_use_rights() == rights
checks+=1
guardian_world=core.Realm(93,2,128); guardian_case=None
for title in guardian_world.land_holdings():
    owner=title['title_holder']; owner_state=guardian_world.person(owner)
    if not owner_state['alive'] or owner_state['age_days']<18*365: continue
    household=guardian_world.household_of(owner)
    members=[candidate for candidate in range(1,129) if guardian_world.household_of(candidate)==household]
    children=[candidate for candidate in members if guardian_world.person(candidate)['alive'] and guardian_world.person(candidate)['age_days']<18*365]
    adults=[candidate for candidate in members if candidate!=owner and guardian_world.person(candidate)['alive'] and guardian_world.person(candidate)['age_days']>=18*365]
    if children and adults:
        child=children[0]
        guardian=max((candidate for candidate in members if candidate!=child and guardian_world.person(candidate)['alive'] and guardian_world.person(candidate)['age_days']>=18*365),key=lambda candidate:guardian_world.person(candidate)['age_days'])
        recipient=next(candidate for candidate in range(1,129) if candidate not in (child,guardian) and guardian_world.person(candidate)['alive'] and guardian_world.person(candidate)['age_days']>=18*365)
        guardian_case=(title,owner,child,guardian,recipient); break
assert guardian_case is not None,'fixture has an adult-owned household holding and a living minor dependent'
guardian_title,guardian_owner,dependent,guardian,recipient=guardian_case
ok,reason,_=guardian_world.land_title_transfer(guardian_owner,guardian_world.person(guardian_owner)['decisions']+1,guardian_title['id'],dependent)
assert ok,(reason,'dependent title setup')
guardian_money=guardian_world.view()['money']; guardian_before=guardian_world.fingerprint()
ok,reason,_=guardian_world.land_use_grant(guardian,guardian_world.person(guardian)['decisions']+1,guardian_title['id'],recipient)
guardian_rights=guardian_world.land_use_rights()
assert ok and len(guardian_rights)==1 and guardian_rights[0]['grantor']==dependent and guardian_world.view()['money']==guardian_money and guardian_world.invariant(); checks+=1
unauthorized=next(candidate for candidate in range(1,129) if candidate not in (dependent,guardian) and guardian_world.person(candidate)['alive'] and guardian_world.person(candidate)['age_days']>=18*365)
guardian_before=guardian_world.fingerprint(); ok,reason,_=guardian_world.land_use_revoke(unauthorized,guardian_world.person(unauthorized)['decisions']+1,guardian_title['id'])
assert not ok and reason=='Actor is not title holder or dependent guardian' and guardian_world.fingerprint()==guardian_before; checks+=1
guardian_save=guardian_world.serialize('guardian-land-use-binding'); guardian_copy=core.Realm(1,1,128)
assert guardian_copy.deserialize(guardian_save,'guardian-land-use-binding')[0] and guardian_copy.land_use_rights()==guardian_rights and guardian_copy.fingerprint()==guardian_world.fingerprint(); checks+=1
ok,reason,_=guardian_copy.land_use_revoke(guardian,guardian_copy.person(guardian)['decisions']+1,guardian_title['id'])
restored_title=next(title for title in guardian_copy.land_holdings() if title['id']==guardian_title['id'])
assert ok and not guardian_copy.land_use_rights() and restored_title['title_holder']==dependent and guardian_copy.invariant(); checks+=1
t=time.perf_counter()
for _ in range(64):batch.step(np.zeros(32,dtype='int64'))
elapsed=time.perf_counter()-t
print(json.dumps({'checks':checks,'observation':core.REALM_OBS,'actions':core.REALM_ACTIONS,'cpp_python_replay_days':64,'invalid_batch_atomic':True,'bundle_mismatch_atomic':True,'decisions':2048,'seconds':elapsed,'decisions_per_second':2048/elapsed,'schema':core.REALM_SCHEMA,'scope':'training binding world/partner steps, no neural inference'}))





