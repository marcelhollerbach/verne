include_directories(
  ${EINA_INCLUDE_DIRS}
  ${EIO_INCLUDE_DIRS}
  ${EEZE_INCLUDE_DIRS}
  ${EFREET_MIME_INCLUDE_DIRS}
  ${ECORE_FILE_INCLUDE_DIRS}
)

set (EMOUS_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/lib/emous/")

string(REPLACE "\n" "" EOLIAN_EO_DIR_WITHOUT_NEWLINE
"${EOLIAN_EO_DIR}")

file(GLOB files "*.eo")

foreach(filename ${files})
add_custom_command(
   OUTPUT ${filename}.x
   COMMAND eolian_gen -I ${EOLIAN_EO_DIR_WITHOUT_NEWLINE} --gc --eo --legacy -o ${filename}.x ${filename}
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

LIST(APPEND FM_MONITOR
   fm_monitor.c
   efm_monitor.c
   efm_monitor.eo.x
   efm_monitor.eo.h
   efm_monitor.eo.legacy.h
   efm_file.c
   efm_file.eo.x
   efm_file.eo.h
   efm_file.eo.legacy.h
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
  ${EEZE_LIBRARIES}
  ${ECORE_FILE_LIBRARIES}
  ${EFREET_MIME_LIBRARIES}
  ${LibArchive_LIBRARIES}
)

set_property(TARGET efm_lib
   PROPERTY COMPILE_DEFINITIONS ${RESOLVER_DIR_DEFINITION})

INSTALL(TARGETS efm_lib
  LIBRARY DESTINATION lib
)
INSTALL(FILES Efm.h efm_file.eo.h efm_file.eo.legacy.h efm_monitor.eo.h efm_monitor.eo.legacy.h
  DESTINATION include/efm-1/)