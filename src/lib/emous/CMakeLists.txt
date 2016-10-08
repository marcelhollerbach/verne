include_directories(
  ${EINA_INCLUDE_DIRS}
  ${EIO_INCLUDE_DIRS}
  ${EFREET_MIME_INCLUDE_DIRS}
  ${ECORE_FILE_INCLUDE_DIRS}
  ${Mount_INCLUDE_DIRS}
  ./
)

set (EMOUS_MODULE_PATH "${CMAKE_INSTALL_FULL_LIBDIR}/emous-1/")

LIST(APPEND EMOUS_FILES
   Emous.h
   emous_priv.h
   emous_mp.c
   emous.c
   emous.eo
   emous.eo.x
   emous.eo.h
   emous_manager.c
   emous_manager.eo
   emous_manager.eo.x
   emous_manager.eo.h
   emous_device.c
   emous_device.eo
   emous_device.eo.x
   emous_device.eo.h
   emous_type.c
   emous_type.eo
   emous_type.eo.x
   emous_type.eo.h
   )

if (Mount_FOUND)
  LIST(APPEND EMOUS_FILES
      emous_mm_linux.c
     )
endif (Mount_FOUND)

list(APPEND include
   ./
)
eo_rule_create("${EMOUS_FILES}" "${include}")

add_library(emous SHARED ${EMOUS_FILES})

set(EMOUS_MODULE_PATH \"${EMOUS_MODULE_PATH}\")
configure_file(config.h.cmake ${CMAKE_SOURCE_DIR}/src/lib/emous/config.h)

target_link_libraries(emous
  ${EINA_LIBRARIES}
  ${EO_LIBRARIES}
  ${EIO_LIBRARIES}
  ${ECORE_LIBRARIES}
  ${ECORE_FILE_LIBRARIES}
  ${EFREET_MIME_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${Mount_LIBRARIES}
  m
)

INSTALL(TARGETS emous
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

INSTALL(FILES Emous.h emous_device.eo.h emous_type.eo.h emous_manager.eo.h emous.eo.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/emous-1/
)
