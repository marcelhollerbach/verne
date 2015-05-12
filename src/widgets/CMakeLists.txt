LIST(APPEND WIDGET_LIBRARY
   elm_file_icon.eo
   elm_file_icon.eo.x
   elm_file_icon.eo.h
   elm_file_icon.eo.legacy.h
   elm_file_icon.c
   elm_file_display.c
   elm_file_display.h
   elm_file_display_config.c
   elm_file_display_log.c
   elm_file_display_bookmark.c
   elm_file_display_preview.c
   elm_file_display_helper.c
   elm_file_display.eo
   elm_file_display.eo.x
   elm_file_display.eo.h
   elm_file_display.eo.legacy.h
   #views
   elm_file_display_debug.c
   elm_file_display_grid.c
   elm_file_display_tree.c
   elm_file_display_list.c
   elm_tab_pane.c
   elm_tab_pane.eo
   elm_tab_pane.eo.x
   elm_tab_pane.eo.h
   elm_tab_pane.eo.legacy.h
   )

string(REPLACE "\n" "" EOLIAN_EO_DIR_WITHOUT_NEWLINE
"${EOLIAN_EO_DIR}")

file(GLOB files "*.eo")

foreach(filename ${files})
add_custom_command(
   OUTPUT ${filename}.x
   COMMAND eolian_gen -I ${EOLIAN_EO_DIR_WITHOUT_NEWLINE} --gc --eo -o ${filename}.x ${filename}
   DEPENDS ${filename}
)

add_custom_command(
   OUTPUT ${filename}.h
   COMMAND eolian_gen -I ${EOLIAN_EO_DIR_WITHOUT_NEWLINE} --gh --eo -o ${filename}.h ${filename}
   DEPENDS ${filename}
)

add_custom_command(
   OUTPUT ${filename}.legacy.h
   COMMAND eolian_gen -I ${EOLIAN_EO_DIR_WITHOUT_NEWLINE} --gh --legacy -o ${filename}.legacy.h ${filename}
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
