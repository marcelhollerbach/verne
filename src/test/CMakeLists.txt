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
   efm_lib
)

add_executable(test_fm_monitor test_fm_monitor.c)

target_link_libraries (test_fm_monitor
   ${EINA_LIBRARIES}
   ${ECORE_LIBRARIES}
   ${EO_LIBRARIES}
   ${CMAKE_THREAD_LIBS_INIT}
   ${Check_LIBRARIES}
   efm_lib
   m
   rt
)

add_test(efm_test test_fm_monitor)

add_subdirectory(./emous_test)