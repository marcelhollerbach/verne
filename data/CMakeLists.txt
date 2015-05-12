LIST(APPEND WIDGET_THEME
   file_display.edc
   file_icon.edc
   efm.edc
   tab.edc
   )

foreach(filename ${WIDGET_THEME})
  add_custom_command(
     OUTPUT ${filename}.edj
     COMMAND edje_cc -id $(CMAKE_SOURCE_DIR)/data/img $(CMAKE_SOURCE_DIR)/data/${filename} $(CMAKE_BINARY_DIR)/data/${filename}.edj
     DEPENDS ${filename}
  )
  LIST(APPEND WIDGET_OUTPUTS ${filename}.edj)
endforeach()

add_custom_target(widget_theme ALL DEPENDS ${WIDGET_OUTPUTS})