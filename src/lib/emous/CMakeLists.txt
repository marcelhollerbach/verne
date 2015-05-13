include_directories(
  ${EINA_INCLUDE_DIRS}
  ${EIO_INCLUDE_DIRS}
  ${EFREET_MIME_INCLUDE_DIRS}
  ${ECORE_FILE_INCLUDE_DIRS}
  /usr/include/libmount/
  ./
)

set (EMOUS_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/lib/emous/")

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

endforeach()

LIST(APPEND EMOUS_FILES
   Emous.h
   emous_priv.h
   mount.c
   emous_mm_linux.c
   emous_mp.c
   main.c
   emous_manager.eo.x
   emous_manager.eo.h
   emous_device.eo.x
   emous_device.eo.h
   emous_device_class.eo.x
   emous_device_class.eo.h
   )

add_library(emous SHARED ${EMOUS_FILES})

target_link_libraries(emous
  ${EINA_LIRBRARIES}
  ${EIO_LIBRARIES}
  ${ECORE_LIBRARIES}
  ${ECORE_FILE_LIBRARIES}
  ${EFREET_MIME_LIBRARIES}
  -lmount
  -lpthread
)

INSTALL(TARGETS emous
  LIBRARY DESTINATION lib
)

INSTALL(FILES Emous.h emous_device.eo.h emous_device_class.eo.h emous_manager.eo.h
  DESTINATION "include/emous-1/"
)

set_property(TARGET emous
             PROPERTY COMPILE_DEFINITIONS EMOUS_MODULE_PATH="${EMOUS_MODULE_PATH}")

LIST(APPEND EMOUS_UDISK_FILES
   ./modules/udisk/udisk.c
   ./modules/udisk/udisk_dbus.c
   )

add_library(emous_udisk MODULE ${EMOUS_UDISK_FILES})

INSTALL(TARGETS emous_udisk
  DESTINATION ${EMOUS_MODULE_PATH}
)