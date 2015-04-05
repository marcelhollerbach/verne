
set(THEME_PATH_DEFINITION THEME_PATH="$(CMAKE_BINARY_DIR)/data/")

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

include_directories(
  ${ELEMENTARY_INCLUDE_DIRS}
  ../lib/
)

target_link_libraries (filedisplay_test
   ${EINA_LIBRARIES}
   ${EVAS_LIBRARIES}
   ${EFL_LIBRARIES}
   ${EO_LIBRARIES}
   ${ECORE_LIBRARIES}
   ${ELEMENTARY_LIBRARIES}
   widgets
   efm_lib
)

set_property(TARGET filedisplay_test
   PROPERTY COMPILE_DEFINITIONS ${THEME_PATH_DEFINITION})

add_executable(test_fm_monitor test_fm_monitor.c)

include_directories(

  )

target_link_libraries (test_fm_monitor
   ${ECORE_LIBRARIES}
   efm_lib
)

add_executable(test_archive test_archive.c)

include_directories(

  )

target_link_libraries (test_archive
   efm_lib
)

add_executable(test_tab test_pane.c)

include_directories(
   ${ELEMENTARY_INCLUDE_DIRS}
   ../widgets/
)

target_link_libraries (test_tab
   ${EVAS_LIBRARIES}
   ${EO_LIBRARIES}
   ${ECORE_LIBRARIES}
   ${ELEMENTARY_LIBRARIES}
   widgets
)

set_property(TARGET test_tab
   PROPERTY COMPILE_DEFINITIONS ${THEME_PATH_DEFINITION})
