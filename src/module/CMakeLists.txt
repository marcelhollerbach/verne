include_directories(
  ${Enlightenment_INCLUDE_DIRS}
  ${ELEMENTARY_INCLUDE_DIRS}
  ../lib/efm/
  ../widgets/
)

LIST(APPEND jesus_files
   e_mod_main.c
   )

add_library(jesus MODULE ${jesus_files})

target_link_libraries (jesus
   ${EINA_LIBRARIES}
   ${EVAS_LIBRARIES}
   ${EFL_LIBRARIES}
   ${EO_LIBRARIES}
   ${ELEMENTARY_LIBRARIES}
   widgets
   efm_lib
   ${Enlightenment_LIBRARIES}
)

set_target_properties(jesus PROPERTIES OUTPUT_NAME module)
set_target_properties(jesus PROPERTIES PREFIX "")

INSTALL(TARGETS jesus
  DESTINATION  /usr/local/lib/enlightenment/modules/jesus/linux-gnu-x86_64-ver-autocannoli-0.20/
  RENAME module.so
)
INSTALL(FILES module.desktop DESTINATION /usr/local/lib/enlightenment/modules/jesus/)