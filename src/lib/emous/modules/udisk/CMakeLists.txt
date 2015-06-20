include_directories(
  ${EINA_INCLUDE_DIRS}
  ${EO_INCLUDE_DIRS}
  ${ELDBUS_INCLUDE_DIRS}
  ../../
)

set (EMOUS_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/lib/emous/")

string(REPLACE "\n" "" EOLIAN_EO_DIR_WITHOUT_NEWLINE
"${EOLIAN_EO_DIR}")

file(GLOB files "*.eo")

foreach(filename ${files})
add_custom_command(
   OUTPUT ${filename}.x
   COMMAND eolian_gen -I ${EOLIAN_EO_DIR_WITHOUT_NEWLINE} -I ${CMAKE_SOURCE_DIR}/src/lib/emous --gc --eo -o ${filename}.x ${filename}
   DEPENDS ${filename}
)

add_custom_command(
   OUTPUT ${filename}.h
   COMMAND eolian_gen -I ${EOLIAN_EO_DIR_WITHOUT_NEWLINE} -I ${CMAKE_SOURCE_DIR}/src/lib/emous/ --gh --eo -o ${filename}.h ${filename}
   DEPENDS ${filename}
)

endforeach()

LIST(APPEND EMOUS_UDISK_FILES
   emous_device_udisks.eo.x
   emous_device_udisks.eo.h
   emous_type_udisks.eo.x
   emous_type_udisks.eo.h
   device.c
   type.c
   main.c
   )

add_library(emous_udisk MODULE ${EMOUS_UDISK_FILES})

INSTALL(TARGETS emous_udisk
  DESTINATION ${EMOUS_MODULE_PATH}
)