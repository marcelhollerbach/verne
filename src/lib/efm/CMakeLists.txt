include_directories(
  ${EINA_INCLUDE_DIRS}
  ${EIO_INCLUDE_DIRS}
  ${EEZE_INCLUDE_DIRS}
  ${EFREET_MIME_INCLUDE_DIRS}
  ${ECORE_FILE_INCLUDE_DIRS}
)

LIST(APPEND FM_MONITOR
   efm.eot
   efm.eot.h
   fm_monitor.c
   efm_monitor.c
   efm_monitor.eo
   efm_monitor.eo.x
   efm_monitor.eo.h
   efm_file.c
   efm_file.eo
   efm_file.eo.x
   efm_file.eo.h
   efm_filter.c
   efm_filter.eo
   efm_filter.eo.x
   efm_filter.eo.h
   main.c
   Efm.h
   efm_priv.h
   )

list(APPEND FM_DEPS
  ./
)

eo_rule_create("${FM_MONITOR}" "${FM_DEPS}")

add_library(efm_lib SHARED ${FM_MONITOR})

target_link_libraries(efm_lib
  ${EINA_LIBRARIES}
  ${EO_LIBRARIES}
  ${EIO_LIBRARIES}
  ${ECORE_LIBRARIES}
  ${EEZE_LIBRARIES}
  ${ECORE_FILE_LIBRARIES}
  ${EFREET_LIBRARIES}
  ${EFREET_MIME_LIBRARIES}
  ${LibArchive_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
)

INSTALL(TARGETS efm_lib
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
INSTALL(FILES Efm.h efm_file.eo.h efm_monitor.eo.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/efm-1/)