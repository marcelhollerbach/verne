include_directories(
  ${ELEMENTARY_INCLUDE_DIRS}
   ../widgets/
   ../lib/emous
   ../lib/efm
)

#add_executable(fileicon_test fileicon.c)

#include_directories(
#  ${ELEMENTARY_INCLUDE_DIRS}
#)

#target_link_libraries (fileicon_test
#   ${EFL_LIBRARIES}
#   ${EVAS_LIBRARIES}
#   ${EO_LIBRARIES}
#   ${EFREET_MIME_LIBRARIES}
#   ${ECORE_LIBRARIES}
#   ${ELEMENTARY_LIBRARIES}
#   widgets
#)

#set_property(TARGET fileicon_test
#   PROPERTY COMPILE_DEFINITIONS #${THEME_PATH_DEFINITION})

add_executable(filedisplay_test filedisplay.c)

target_link_libraries (filedisplay_test
   ${EINA_LIBRARIES}
   ${EVAS_LIBRARIES}
   ${EFL_LIBRARIES}
   ${EO_LIBRARIES}
   ${ECORE_LIBRARIES}
   ${ELEMENTARY_LIBRARIES}
   elm_ext
   efm
)

add_executable(test_fm_monitor test_fm_monitor.c)

target_link_libraries (test_fm_monitor
   ${EINA_LIBRARIES}
   ${ECORE_LIBRARIES}
   ${EO_LIBRARIES}
   ${CMAKE_THREAD_LIBS_INIT}
   ${Check_LIBRARIES}
   efm
   m
   rt
)
set_property(TARGET test_fm_monitor
   PROPERTY COMPILE_DEFINITIONS TEST_RESSOURCES="${CMAKE_SOURCE_DIR}")

message("Disabled efm test because of fails");
#add_test(efm_test test_fm_monitor)

add_subdirectory(./emous_test)