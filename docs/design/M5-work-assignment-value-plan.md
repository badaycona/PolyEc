# M5 WorkAssignment value composition — approved slice

Ngày review và thực hiện: 2026-10-20. Đây là lát cấu trúc chuyển tiếp theo core class relationship design; chưa phải EmploymentContract.

## Phạm vi

- Thêm `WorkAssignment { int job=-1; int job_kind=0; }` làm value member `Person::work_assignment`.
- Đưa mọi đọc/ghi hai trường hiện có qua value member. `Person::busy_until` vẫn là deadline bận dùng chung, giữ trực tiếp trên Person.
- EmploymentContract tương lai, ID hợp đồng/bên thuê, kỳ hạn riêng, loại trừ nhiều việc, điều khoản, establishment/worksite, pay/output và luật gameplay đều ngoài phạm vi.
- Cập nhật mô tả hiện trạng Person tại `core-class-reference.md`; §12.3 EmploymentContract vẫn là thiết kế đích riêng.

## Ownership, defaults và invariant

Person do Realm sở hữu như trước. WorkAssignment không có ID hay persistence độc lập; Realm tiếp tục serialize hai scalar qua chính các token cũ trong Person. Default `job=-1`, `job_kind=0`. Kiểm tra hiện có `job_kind ∈ [0,1]` giữ nguyên điều kiện, chỉ đổi đường member.

## Luồng và compatibility

- EmploymentManager, Realm, Campaign, court, Python binding, native benchmark và test đọc/ghi qua `work_assignment.job`/`work_assignment.job_kind`.
- Giữ `busy_until` trực tiếp và mọi thứ tự kiểm, cập nhật counter, ngày hết hạn, đường migration và outcome hiện có.
- `Realm::serialize`/`deserialize` giữ nguyên schema, thứ tự token, spacing và bytes: job ở vị trí cũ; busy_until ở vị trí cũ; job_kind ở vị trí cũ.
- Python `person()` giữ nguyên key set và giữ key `job_kind`, lấy giá trị từ `work_assignment.job_kind`.

## Files

Code: `core/include/polyec/realm.hpp`, `core/include/polyec/managers/employment_manager.hpp`, `core/src/realm.cpp`, `core/src/realm_court.cpp`, `core/src/campaign.cpp`, `core/tests/test_managers.cpp`, `core/tests/test_realm.cpp`, `core/tests/test_realm_court.cpp`, `core/tests/test_schedule.cpp`, `bindings/python/realm_binding.cpp`, `native/realm_benchmark.cpp`.

Documentation: `docs/design/core-class-reference.md` Person work-assignment/deadline rows only.

## Gates

- Release core build and full CTest using existing `build/m5-core`.
- Debug core build and targeted Realm, Schedule, Manager and clock replay tests using existing `build/m5-core`.
- Build `polyec_realm_benchmark` and `polyec_realm_contracts` in existing `build/m5-runtime`; run `runtime_contracts`.
- Build Python environment/replay targets and run clock binding replay in Release and Debug using existing `build/m5-python-ai`.
- `git diff --check` and verify no direct Person `.job`/`.job_kind` accesses remain.

## Acceptance boundary

No gameplay rule, command, action, observation, reward, model, save schema or API-key addition. The slice only changes where the two legacy Person values are grouped and accessed. M5 remains open for separately reviewed relationship/ownership contracts.
