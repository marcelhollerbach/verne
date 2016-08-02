include_directories(
  ${Enlightenment_INCLUDE_DIRS}
  ${ELEMENTARY_INCLUDE_DIRS}
  ../lib/efm/
  ../widgets/
)

LIST(APPEND jesus_files
   e_bg_widget.c
   e_bg_widget.eo
   e_bg_widget.eo.x
   e_bg_widget.eo.h
   e_mod_main.c
   )
list(APPEND bg_deps
  ../widgets/file/
  ../lib/efm
  )

eo_rule_create("${jesus_files}" "${bg_deps}")

add_library(jesus_module MODULE ${jesus_files})

target_link_libraries (jesus_module
   ${EINA_LIBRARIES}
   ${EVAS_LIBRARIES}
   ${EFL_LIBRARIES}
   ${EINA_LIBRARIES}
   ${EO_LIBRARIES}
   ${ELEMENTARY_LIBRARIES}
   efm
   elm_ext

)

set_target_properties(jesus_module PROPERTIES OUTPUT_NAME module)
set_target_properties(jesus_module PROPERTIES PREFIX "")

INSTALL(TARGETS jesus_module
  DESTINATION  /usr/local-e/lib/enlightenment/modules/jesus/linux-gnu-x86_64-ver-0.22/
  RENAME module.so
)
INSTALL(FILES module.desktop DESTINATION /usr/local-e/lib/enlightenment/modules/jesus/)
