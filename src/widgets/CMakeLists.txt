set(THEME_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/share/elm_ext/")

set(THEME_PATH_DEFINITION THEME_PATH="${THEME_INSTALL_PATH}")

LIST(APPEND WIDGET_LIBRARY
   Elementary_Ext.h
   elementary_ext.c
   elm_file_icon.eo
   elm_file_icon.eo.x
   elm_file_icon.eo.h
   elm_file_icon.eo.legacy.h
   elm_file_icon.c
   elm_file_display.c
   elm_file_display.h
   elm_file_display_config.c
   elm_file_display_bookmark.c
   elm_file_display_preview.c
   elm_file_display_helper.c
   elm_file_display.eo
   elm_file_display.eo.x
   elm_file_display.eo.h
   elm_file_display.eo.legacy.h
   #views
   elm_file_display_view.c
   elm_file_display_view.eo.h
   elm_file_display_view.eo.x
   elm_file_display_view_debug.c
   elm_file_display_view_debug.eo.x
   elm_file_display_view_debug.eo.h
   elm_file_display_view_grid.c
   elm_file_display_view_grid.eo.x
   elm_file_display_view_grid.eo.h
   )

string(REPLACE "\n" "" EOLIAN_EO_DIR_WITHOUT_NEWLINE
"${EOLIAN_EO_DIR}")

file(GLOB files "*.eo")

foreach(filename ${files})
add_custom_command(
   OUTPUT ${filename}.x
   COMMAND eolian_gen -I ${EOLIAN_EO_DIR_WITHOUT_NEWLINE} -I ${CMAKE_SOURCE_DIR}/src/widgets --gc --eo -o ${filename}.x ${filename}
   DEPENDS ${filename}
)

add_custom_command(
   OUTPUT ${filename}.h
   COMMAND eolian_gen -I ${EOLIAN_EO_DIR_WITHOUT_NEWLINE} -I ${CMAKE_SOURCE_DIR}/src/widgets --gh --eo -o ${filename}.h ${filename}
   DEPENDS ${filename}
)

add_custom_command(
   OUTPUT ${filename}.legacy.h
   COMMAND eolian_gen -I ${EOLIAN_EO_DIR_WITHOUT_NEWLINE} -I ${CMAKE_SOURCE_DIR}/src/widgets --gh --legacy -o ${filename}.legacy.h ${filename}
   DEPENDS ${filename}
)

endforeach()

add_library(widgets SHARED ${WIDGET_LIBRARY})

include_directories(
  ${EINA_INCLUDE_DIRS}
  ${EO_INCLUDE_DIRS}
  ${ELEMENTARY_INCLUDE_DIRS}
  ../lib/efm
  ../lib/emous
)

target_link_libraries(widgets
  ${EIO_LIBRARIES}
  ${ECORE_FILE_LIBRARYS}
  ${ELEMENTARY_LIRBRARIES}
  ${EFREET_TRASH}
  efm_lib
  emous
)

set_property(TARGET widgets
   PROPERTY COMPILE_DEFINITIONS ${THEME_PATH_DEFINITION})

INSTALL(FILES Elementary_Ext.h DESTINATION include/elm_ext-1/)
INSTALL(TARGETS widgets
  LIBRARY DESTINATION lib
)