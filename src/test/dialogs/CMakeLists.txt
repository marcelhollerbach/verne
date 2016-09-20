include_directories(
  ${ELEMENTARY_INCLUDE_DIRS}
   ../widgets
)

add_executable(dialog main.c)

target_link_libraries (dialog
   ${EINA_LIBRARIES}
   ${EVAS_LIBRARIES}
   ${EFL_LIBRARIES}
   ${EO_LIBRARIES}
   ${ECORE_LIBRARIES}
   ${ELEMENTARY_LIBRARIES}
   elm_ext
   efm
)