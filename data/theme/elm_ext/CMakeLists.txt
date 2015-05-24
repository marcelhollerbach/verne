add_custom_command(
     OUTPUT elm_ext.edc.edj
     COMMAND edje_cc -id $(CMAKE_SOURCE_DIR)/data/img data/elm_ext.edc $(CMAKE_SOURCE_DIR)/data/elm_ext.edc.edj
     DEPENDS elm_ext.edc
)

add_custom_target(elm_ext_theme DEPENDS elm_ext.edc.edj)