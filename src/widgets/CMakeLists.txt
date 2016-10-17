set(THEME_INSTALL_PATH "${CMAKE_INSTALL_FULL_DATADIR}/elm_ext/")

LIST(APPEND WIDGET_LIBRARY
   Elementary_Ext.h
   elementary_ext.c
   #dialogs
   dialog/elm_dialog.c
   dialog/elm_dialog.eo
   dialog/elm_dialog.eo.x
   dialog/elm_dialog.eo.h
   dialog/elm_dialog_decision.c
   dialog/elm_dialog_decision.eo
   dialog/elm_dialog_decision.eo.x
   dialog/elm_dialog_decision.eo.h
   #filepackage
   file/elm_file_mimetype_cache.c
   file/elm_file_mimetype_cache.eo
   file/elm_file_mimetype_cache.eo.x
   file/elm_file_mimetype_cache.eo.h
   file/elm_file_icon.c
   file/elm_file_icon.eo
   file/elm_file_icon.eo.x
   file/elm_file_icon.eo.h
   file/elm_file_bookmarks.c
   file/elm_file_bookmarks.eo
   file/elm_file_bookmarks.eo.x
   file/elm_file_bookmarks.eo.h
   file/elm_file_detail.c
   file/elm_file_detail.eo
   file/elm_file_detail.eo.x
   file/elm_file_detail.eo.h
   file/elm_file_selector.c
   file/elm_file_selector.eo
   file/elm_file_selector.eo.x
   file/elm_file_selector.eo.h
   file/elm_file_config.c
   file/elm_file_config.h
   file/elm_file_display.c
   file/elm_file_display.eo
   file/elm_file_display.eo.x
   file/elm_file_display.eo.h
   #views
   file/elm_file_view.c
   file/elm_file_view.eo
   file/elm_file_view.eo.h
   file/elm_file_view.eo.x
   file/views/View.h
   file/views/sort.c
   file/views/view_common.c
   file/views/view_common.h
   file/views/elm_file_display_view_grid.c
   file/views/elm_file_display_view_grid.eo
   file/views/elm_file_display_view_grid.eo.x
   file/views/elm_file_display_view_grid.eo.h
   file/views/elm_file_display_view_list.c
   file/views/elm_file_display_view_list.eo
   file/views/elm_file_display_view_list.eo.x
   file/views/elm_file_display_view_list.eo.h
   )
if (DEBUG)
  list(APPEND WIDGET_LIBRARY
   file/views/elm_file_display_view_debug.c
   file/views/elm_file_display_view_debug.eo
   file/views/elm_file_display_view_debug.eo.x
   file/views/elm_file_display_view_debug.eo.h
  )
endif(DEBUG)


list(APPEND WIDGET_INCLUDE
  ./
  ./file/
  ./dialog/
  ../lib/efm/
)
eo_rule_create("${WIDGET_LIBRARY}" "${WIDGET_INCLUDE}")

add_library(elm_ext SHARED ${WIDGET_LIBRARY})

include_directories(
  ${EINA_INCLUDE_DIRS}
  ${EO_INCLUDE_DIRS}
  ${ELEMENTARY_INCLUDE_DIRS}
  ../lib/efm
  ../lib/emous
)

target_link_libraries(elm_ext
  ${EIO_LIBRARIES}
  ${EVAS_LIBRARIES}
  ${EFL_LIBRARIES}
  ${EET_LIBRARIES}
  ${ECORE_FILE_LIBRARIES}
  ${ELEMENTARY_LIBRARIES}
  ${EFREET_TRASH_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  efm
  emous
)

target_compile_options(elm_ext
  PRIVATE ${EIO_DEFINITIONS}
  ${EVAS_DEFINITIONS}
  ${EFL_DEFINITIONS}
  ${EET_DEFINITIONS}
  ${ECORE_FILE_DEFINITIONS}
  ${ELEMENTARY_DEFINITIONS}
  ${EFREET_TRASH_DEFINITIONS}
)
set(THEME_PATH \"${THEME_INSTALL_PATH}\")


configure_file(config.h.cmake ${CMAKE_SOURCE_DIR}/src/widgets/config.h)
set_property(TARGET elm_ext
   PROPERTY COMPILE_DEFINITIONS
   ${definitions}
)

INSTALL(FILES Elementary_Ext.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/elm_ext-1/)
INSTALL(TARGETS elm_ext
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
