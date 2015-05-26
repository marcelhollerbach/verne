add_custom_command(
     OUTPUT elm_ext.edc.edj
     COMMAND edje_cc -id $(CMAKE_SOURCE_DIR)/data/img -id $(CMAKE_SOURCE_DIR)/data/elementary/data/themes/img/ -sd $(CMAKE_SOURCE_DIR)/data/elementary/data/themes/snd/ $(CMAKE_SOURCE_DIR)/data/theme/elm_ext/elm_ext.edc $(CMAKE_SOURCE_DIR)/data/theme/elm_ext/elm_ext.edc.edj
     DEPENDS elm_ext.edc
)

add_custom_target(theme DEPENDS elm_ext.edc.edj)

INSTALL(FILES elm_ext.edc.edj
  DESTINATION share/elm_ext/)