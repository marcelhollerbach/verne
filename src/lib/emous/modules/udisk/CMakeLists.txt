include_directories(
  ${EINA_INCLUDE_DIRS}
  ${EO_INCLUDE_DIRS}
  ${ELDBUS_INCLUDE_DIRS}
  ../../
)

set (EMOUS_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/lib/emous/")

LIST(APPEND EMOUS_UDISK_FILES
   emous_device_udisks.eo
   emous_device_udisks.eo.x
   emous_device_udisks.eo.h
   emous_type_udisks.eo
   emous_type_udisks.eo.x
   emous_type_udisks.eo.h
   device.c
   type.c
   main.c
   )

list(APPEND udisks_include
   ../../
)
eo_rule_create("${EMOUS_UDISK_FILES}" "${udisks_include}")

add_library(emous_udisk MODULE ${EMOUS_UDISK_FILES})

INSTALL(TARGETS emous_udisk
  DESTINATION ${EMOUS_MODULE_PATH}
)