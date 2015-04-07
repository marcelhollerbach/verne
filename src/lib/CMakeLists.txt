include_directories(
  ${EINA_INCLUDE_DIRS}
  ${EIO_INCLUDE_DIRS}
  ${EFREET_MIME_INCLUDE_DIRS}
  ${ECORE_FILE_INCLUDE_DIRS}
)

LIST(APPEND FM_MONITOR
   fm_monitor.c
   archive.c
   main.c
   Efm.h
   efm_priv.h
   )

add_library(efm_lib SHARED ${FM_MONITOR})

target_link_libraries(efm_lib
  ${EINA_LIRBRARIES}
  ${EIO_LIBRARIES}
  ${ECORE_LIBRARIES}
  ${ECORE_FILE_LIBRARIES}
  ${EFREET_MIME_LIBRARIES}
  archive
)

set_property(TARGET efm_lib
   PROPERTY COMPILE_DEFINITIONS ${RESOLVER_DIR_DEFINITION})

INSTALL(TARGETS efm_lib
  LIBRARY DESTINATION lib
)
