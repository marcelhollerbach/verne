set(THEME_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/share/elm_ext/")

LIST(APPEND WIDGET_LIBRARY
   Elementary_Ext.h
   elementary_ext.c
   #itemspackage

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
   file/elm_file_preview.c
   file/elm_file_preview.eo
   file/elm_file_preview.eo.x
   file/elm_file_preview.eo.h
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
  ../lib/efm/
)
eo_rule_create("${WIDGET_LIBRARY}" "${WIDGET_INCLUDE}")

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
  ${EVAS_LIBRARIES}
  ${EFL_LIBRARIES}
  ${EET_LIBRARIES}
  ${ECORE_FILE_LIBRARIES}
  ${ELEMENTARY_LIBRARIES}
  ${EFREET_TRASH_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  efm_lib
  emous
)

list(APPEND definitions
            THEME_PATH="${THEME_INSTALL_PATH}"
    )
if (DEBUG)
list(APPEND definitions
            DEBUG
    )
endif(DEBUG)


set_property(TARGET widgets
   PROPERTY COMPILE_DEFINITIONS
   ${definitions}
)
INSTALL(FILES Elementary_Ext.h DESTINATION include/elm_ext-1/)
INSTALL(TARGETS widgets
  LIBRARY DESTINATION lib
)
