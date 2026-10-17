*** Begin Patch
*** Update File: core/CMakeLists.txt
@@
 add_library(polyec_core STATIC
   src/sim.cpp
   src/trade.cpp
   src/realm.cpp
   src/realm_court.cpp
   src/campaign.cpp
+  src/household_index.cpp
+  src/finance_manager.cpp
+  src/project_manager.cpp
 )
*** End Patch
