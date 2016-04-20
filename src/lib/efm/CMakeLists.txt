include_directories(
  ${EINA_INCLUDE_DIRS}
  ${EIO_INCLUDE_DIRS}
  ${EEZE_INCLUDE_DIRS}
  ${EFREET_MIME_INCLUDE_DIRS}
  ${ECORE_FILE_INCLUDE_DIRS}
)

LIST(APPEND FM_MONITOR
   efm.eo
   efm.eo.x
   efm.eo.h
   main.c
   efm.eot
   efm.eot.h
   fm_monitor.c
   efm_fs_monitor.c
   efm_fs_monitor.eo
   efm_fs_monitor.eo.x
   efm_fs_monitor.eo.h
   efm_archive_monitor.c
   efm_archive_monitor.eo
   efm_archive_monitor.eo.x
   efm_archive_monitor.eo.h
   efm_monitor.c
   efm_monitor.eo
   efm_monitor.eo.x
   efm_monitor.eo.h
   efm_archive_file.c
   efm_archive_file.eo
   efm_archive_file.eo.x
   efm_archive_file.eo.h
   efm_fs_file.c
   efm_fs_file.eo
   efm_fs_file.eo.x
   efm_fs_file.eo.h
   efm_file.c
   efm_file.eo
   efm_file.eo.x
   efm_file.eo.h
   efm_filter.c
   efm_filter.eo
   efm_filter.eo.x
   efm_filter.eo.h

   Efm.h
   efm_priv.h
   archive_libarchive.c
   archive_access.h
   )

list(APPEND FM_DEPS
  ./
)

eo_rule_create("${FM_MONITOR}" "${FM_DEPS}")

add_library(efm SHARED ${FM_MONITOR})

target_link_libraries(efm
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

configure_file(config.h.cmake ${CMAKE_SOURCE_DIR}/src/lib/efm/config.h)

INSTALL(TARGETS efm
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
INSTALL(FILES Efm.h efm_file.eo.h efm_monitor.eo.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/efm-1/)