# M5 deep refactor notes

This document documents the deeper Realm refactor that extracts domain subsystems
and migrates behavior into small manager helpers.

Summary of changes
- Replaced internal transfer implementation with FinanceManager::transfer.
- Extracted payroll-like behavior into FinanceManager::payroll (keeps tax rounding
  semantics and transfers to people' cash and province treasury).
- Replaced household reindexing with HouseholdIndex::build.
- Replaced inline active project counting with ProjectManager::active_projects_count.
- Added manager headers in core/include/polyec/managers.

Guidance
- The extraction is conservative: behaviour is intentionally preserved while
  moving code into small units so the next step is to migrate full implementations
  (start_project/finish_projects/loans/serialize) into managers and add unit tests.

