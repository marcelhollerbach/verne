set(THEME_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/share/elm_ext/")

set(THEME_PATH_DEFINITION THEME_PATH="${THEME_INSTALL_PATH}")

LIST(APPEND WIDGET_LIBRARY
   Elementary_Ext.h
   elementary_ext.c
   #itemspackage

   #filepackage
   file/elm_file_mimetype_cache.c
   file/elm_file_mimetype_cache.eo
   file/elm_file_mimetype_cache.eo.x
   file/elm_file_mimetype_cache.eo.h
   file/elm_file_icon.eo
   file/elm_file_icon.eo.x
   file/elm_file_icon.eo.h
   file/elm_file_icon.eo.legacy.h
   file/elm_file_icon.c
   file/elm_file_display.c
   file/elm_file_display.h
   file/elm_file_display_config.c
   file/elm_file_display_bookmark.c
   file/elm_file_display_preview.c
   file/elm_file_display_helper.c
   file/elm_file_display.eo
   file/elm_file_display.eo.x
   file/elm_file_display.eo.h
   file/elm_file_display.eo.legacy.h
   #views
   file/elm_file_display_view.c
   file/elm_file_display_view.eo.h
   file/elm_file_display_view.eo.x
   file/views/elm_file_display_view_debug.c
   file/views/elm_file_display_view_debug.eo.x
   file/views/elm_file_display_view_debug.eo.h
   file/views/elm_file_display_view_grid.c
   file/views/elm_file_display_view_grid.eo.x
   file/views/elm_file_display_view_grid.eo.h
   file/views/elm_file_display_view_list.c
   file/views/elm_file_display_view_list.eo.x
   file/views/elm_file_display_view_list.eo.h
   )

string(REPLACE "\n" "" EOLIAN_EO_DIR_WITHOUT_NEWLINE
"${EOLIAN_EO_DIR}")

file(GLOB_RECURSE files "*.eo")

foreach(filename ${files})
add_custom_command(
   OUTPUT ${filename}.x
   COMMAND eolian_gen -I ${EOLIAN_EO_DIR_WITHOUT_NEWLINE} -I ${CMAKE_SOURCE_DIR}/src/widgets -I ../lib/efm/ -I ../lib/efm/ --gc --eo -o ${filename}.x ${filename}
   DEPENDS ${filename}
)

add_custom_command(
   OUTPUT ${filename}.h
   COMMAND eolian_gen -I ${EOLIAN_EO_DIR_WITHOUT_NEWLINE} -I ${CMAKE_SOURCE_DIR}/src/ -I ../lib/efm/ --gh --eo -o ${filename}.h ${filename}
   DEPENDS ${filename}
)

add_custom_command(
   OUTPUT ${filename}.legacy.h
   COMMAND eolian_gen -I ${EOLIAN_EO_DIR_WITHOUT_NEWLINE} -I ${CMAKE_SOURCE_DIR}/src/widgets -I ../lib/efm/ --gh --legacy -o ${filename}.legacy.h ${filename}
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
