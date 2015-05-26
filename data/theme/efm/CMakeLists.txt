add_custom_command(
     OUTPUT efm.edc.edj
     COMMAND edje_cc -id $(CMAKE_SOURCE_DIR)/data/img $(CMAKE_SOURCE_DIR)/data/theme/efm/efm.edc $(CMAKE_SOURCE_DIR)/data/theme/efm/efm.edc.edj
     DEPENDS efm.edc
)

add_custom_target(efm_theme ALL DEPENDS efm.edc.edj)

INSTALL(FILES efm.edc.edj
  DESTINATION share/jesus/)