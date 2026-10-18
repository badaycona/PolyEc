> LƯU TRỮ 2026-10-18. Snapshot trước hợp nhất; không thực thi prompt hoặc dùng trạng thái trong file này làm chỉ đạo hiện tại. Đọc [nguồn chuẩn](../../README.md).

# M5.1a — Quỹ chung của hộ (household pooled account) — implementation plan

Ngày: 2026-10-15. **PLAN — chờ Codex review. Chưa sửa source/config/test, chưa train.**
Tiếp nối lát cắt đã nghiệm thu (membership index + giám hộ: CTest 5/5, `realm_tests` 25.298 check) — không khởi động lại, không viết lại.

## 1. Phạm vi

Làm: tài khoản quỹ chung **canonical** song song `Person::cash`; đóng góp tự nguyện (private → quỹ); chi tiêu được ủy quyền (quỹ → điểm đến đã có), tất cả qua `RealmCommand`/`Candidate` hiện tại.
Không làm: hôn nhân, thừa kế/di sản, quân sự, map/quốc gia, tổ chức mới, đổi `guardian`/`spend()`/`available_funds()` đang có, binding/train mới, migration save.

## 2. Ownership và invariant

- Canonical: `std::unordered_map<uint64_t,int64_t> household_funds_` (household label → số dư). Nguồn sự thật duy nhất của quỹ; vào `serialize`, `total_money`, `balances`, `invariant`, `fingerprint`.
- Derived (giữ nguyên): `households_` membership index, dựng lại từ `Person::household` sau load; không nhân bản, không tự sinh tiền.
- Invariant mới: mọi key là label có mặt trong `households_`; mọi giá trị `0 <= fund <= kHouseholdFundCap = 10^15`; `total_money()==opening_money_+endowment_` giữ nguyên nên không sinh/hủy tiền khi thêm hằng số tiền mới.
- **Không quyền tịch thu ngầm:** quỹ không bao giờ bị đẩy vào `Person::cash`, `estate_` hay treasury; thành viên chết không làm mất quỹ; hết người có quyền chỉ đóng băng chi tiêu (mục 6).

## 3. File / lớp / hàm

`core/include/polyec/realm.hpp`
- `realm_schema` → `"realm-v7"`; thêm `Intent::Contribute`, `Intent::PoolPay` **append cuối enum** (24,25) để giá trị fact cũ và thứ tự obs không đổi.
- `action_name`: thêm 2 tên `"Góp quỹ hộ"`, `"Chi quỹ hộ"`.
- Public: `int64_t household_fund(uint64_t household) const;`, `size_t household_fund_records() const;`, `balances()` → `std::array<int64_t,10>` (slot 9 = tổng quỹ).
- Private: `std::unordered_map<uint64_t,int64_t> household_funds_;`, `int64_t& fund_for(uint64_t)`, `bool household_spend_terms(uint64_t actor,const Candidate&,std::string&) const;`
`core/src/realm.cpp` (duy nhất file .cpp đổi; `realm_court.cpp` không đụng)
- `legal()`: hai nhánh mới đặt **trước** dòng `if(c.target>regions_)` (cùng khối `Intent::Support`) — `c.target` là household label/person ID, không phải region.
- `execute()`: `case Intent::Contribute` / `case Intent::PoolPay` gọi `transfer(...)`; guard lặp lại số dư quỹ trước mọi mutation.
- `candidates()` (Civilian): thêm `c[24]=Contribute`, `c[25]=PoolPay` chỉ khi world đủ điều kiện, không bao giờ throw khi `household_authority()==0`.
- `total_money()`, `balances()`, `invariant()` cộng/validate hằng số tiền mới; `serialize` ghi block label→fund **sort theo label** (fingerprint phải tất định); `deserialize` đọc block đó và validate label sau `reindex_households()`.
- `observation()`: `o[94]` đổi nghĩa thành "tiền actor được dùng" = `cash` + quỹ hộ (mục 7); **không đổi `realm_obs=512`**.

## 4. Ai được quyền, consent khác chi tiêu thế nào

- `Contribute` (target = household label, phải == `household_of(actor)`): mọi thành viên còn sống, kể cả chưa thành niên, góp **tiền riêng của mình**. Một chiều, tự nguyện, không cần chấp thuận của authority; không thể rút ngược bằng lệnh này.
- `PoolPay` (target = ID thành viên cùng hộ, `good=0` = học phí, `term` = kỳ học): chỉ `household_authority(hộ)` còn sống được phát; tính đủ điều kiện học của **target** mirror `Intent::Study` (tuổi 6–65, `skill<100`, `v.school_used<v.school`, `term` 1–30).
- Quỹ là tài sản chung nhưng **không** mở quyền vào tiền riêng: `PoolPay` chỉ đọc `household_funds_`, không đọc `Person::cash` của ai; authority không thể ra lệnh cho thành viên khác.
- Deadline/cooldown dùng game time (`day_`); không có gì chạy ngoài `execute`/`advance`.

## 5. Luồng thành công/reject (reason riêng, không gộp)

Thành công: góp hợp lệ → `transfer(p.cash, fund, amount)`; chi hợp lệ → `transfer(fund, provinces_[target.region].firm, amount)` + set `school_days`/`busy_until` cho target. Cả hai đều `event(actor, region, int(kind), amount, target)` cuối `execute`, nên vào facts/knowledge như mọi lệnh khác.
Reject (đều giữ nguyên fingerprint): `"Unknown actor"`; actor dead; `"Invalid terms"` (`amount<=0`, `amount>1000000`, `target<0`); `"Household account unavailable"` (label != hộ actor, hoặc ID0/ngoải dải người); `"Household funds insufficient"`; `"No household spending authority"` (authority==0); `"Not the household authority"`; `"Target outside household"`; `"Beneficiary deceased"`; `"Beneficiary cannot study"`; `"Household pool cap exceeded"`; sequence sai trả `"Stale or duplicate decision ID"` (kiểm đúng chuỗi reason như test M5 hiện có).
Xung đột: hai lệnh chạy tuần tự trong `execute`, lệnh sau nhìn số dư sau lệnh trước → góp rồi chi, hoặc chi hai lần vượt quỹ là reject của lệnh thứ hai; không có partial state.

## 6. Lifecycle

Chết: private cash vẫn vào `estate_` như cũ, **quỹ không đổi**; authority chuyển sang người lớn sống cao tuổi nhất tiếp theo (quy tắc đã có). Hết thành viên sống: quỹ vẫn canonical và được đếm trong `total_money`, chi bị reject, góp vẫn được; việc giải thể quỹ mồ côi thuộc lát thừa kế sau.

## 7. Tương thích save / binding / action / observation / reward / model

- Save: magic → `POLYEC_REALM_7` (bundle string không đổi), `realm_schema="realm-v7"`. Save `realm-v6` cũ **bị reject** bằng `"Save schema/model bundle mismatch"`, không migration. Lý do: bản chất sở hữu tiền đổi (thêm hằng số tiền có chủ riêng), không được giữ "tương thích hình dạng" realm-v6. Grep+ sửa mọi chỗ ghi literal (test fixture `patch_save`, `realm_transition_replay`, doc/README nhắc schema, nếu có).
- Binding/action: `realm_actions=32` giữ nguyên → `realm_obs=512` không đổi, không cần rebuild shape. Enum Intent append cuối nên fact cũ và thứ tự action slot cũ không đổi; chỉ thêm 2 slot mới.
- Observation: `o[94]` semantic change (mục 3) — báo rõ trong tài liệu, model pilot realm-v1..v6 không được tái dùng cho world mới; không có train trong tác vụ này.
- Reward: `utility()` không đổi trong lát này (quỹ chưa vào wealth) — ghi thành điểm cố ý, tránh đổi reward nửa vời.

## 8. Hiệu năng

`total_money`/`invariant` thêm O(số hộ có quỹ) ≤ O(người), không đổi bậc; `observation` O(1) lookup map. Không thêm vòng quét theo hộ trong `daily`. Phải giữ được assertion thời gian ở cuối block "profile A accounting" trong `test_realm.cpp` (re-run, không sửa ngưỡng).

## 9. Test (thêm vào `core/tests/test_realm.cpp`, dùng lại `patch_save`/`load_fixture`/`rejects`)

- "voluntary contribution moves private cash into the household pool" + "contribution conserves total money" (pool +5, cash -5).
- "contribution refused without funds" (`amount>cash`), "zero and negative terms rejected".
- "contribution to a foreign household label rejected atomically".
- "household spending pays the pool and never private cash" (authority cash, member cash không đổi; firm +; `school_days` set).
- "household spending without a living authority rejected" và "not-the-authority member cannot spend".
- "insufficient household pool rejected atomically"; "beneficiary outside household / deceased / ineligible rejected".
- "stale and duplicate decision ID rejected without state change" cho cả hai intent.
- "competing contribution then spend on one pool" (cả hai thứ tự; lệnh thứ hai vượt quỹ bị reject, tổng tiền bất biến).
- "household pool survives save/load with rebuilt membership index" (fingerprint bằng nhau) + "realm-v6 save text rejected with schema mismatch".
- "death of a member leaves the pool untouched and hands authority to the next oldest adult"; "orphaned pool stays counted and cannot be spent".
- Regression: `invariant()` với quỹ trong 8-seed 20 năm hiện có; `balances()` sum == `total_money()`.

## 10. Lệnh nghiệm thu (toolchain đã kiểm chứng, Windows)

```powershell
& build/tooling/cmake/data/bin/cmake.exe --build build/m5-core --parallel 4
& build/tooling/cmake/data/bin/ctest.exe --test-dir build/m5-core --output-on-failure
```

Kỳ vọng: CTest 5/5, `realm_tests` > 25.298 check (số check tăng), exit 0. Không chạy Linux/package/app/binding trong tác vụ này; không suy kết quả từ Windows.

## 11. Loại trừ rõ ràng và lựa chọn kiến trúc còn mở (đề xuất kèm theo)

- Không làm: hôn nhân/cam kết escrow (ví dụ D), thừa kế (ví dụ E), quân sự, 27 khu, profile B, controller/RL, đổi guardian `spend()`.
- `spend()` guardian lấy tiền riêng của guardian cho trẻ (behavior M5 slice 1): giữ nguyên trong lát này để không phá 25.298 check đã chạy; đường đi mới không bao giờ đụng tiền riêng. Follow-up: thay bằng consent + quỹ hộ.
- Cần thiết cho `PoolPay` là trả học phí cho thành viên; mọi mục đích khác (`good!=0`) reject, không mở rộng ngầm.
- Không hỏi lại người dùng: tất cả trên là quyết định kỹ thuật trong contract M5 đã duyệt; chỉ dừng nếu phải đổi luật hộ/thừa kế đã chốt.

**Dừng ở đây chờ Codex review plan trước khi viết code.**
