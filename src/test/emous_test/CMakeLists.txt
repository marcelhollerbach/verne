include_directories(
  ${EINA_INCLUDE_DIRS}
  ${EO_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/lib/emous
)

LIST(APPEND EMOUS_DEBUG
   emous_device_debug.eo
   emous_device_debug.eo.x
   emous_device_debug.eo.h
   emous_type_debug.eo
   emous_type_debug.eo.x
   emous_type_debug.eo.h
   main.c
   test_mount.c
   )

list(APPEND EMOUS_DEPS
  ../../lib/emous/
)

eo_rule_create("${EMOUS_DEBUG}" "${EMOUS_DEPS}")

add_executable(test_emous ${EMOUS_DEBUG})

target_link_libraries (test_emous
  ${EINA_LIBRARIES}
   ${ECORE_LIBRARIES}
   ${EO_LIBRARIES}
   ${Check_LIBRARIES}
   emous
   m
   rt
)

add_test(emous_test test_emous)