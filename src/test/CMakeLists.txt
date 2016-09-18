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

add_subdirectory(./emous_test)
add_subdirectory(./efm_test)