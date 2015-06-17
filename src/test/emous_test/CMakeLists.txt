include_directories(
  ${EINA_INCLUDE_DIRS}
  ${EO_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/lib/emous
)

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
   COMMAND eolian_gen -I ${EOLIAN_EO_DIR_WITHOUT_NEWLINE} -I ${CMAKE_SOURCE_DIR}/src/lib/emous --gh --eo -o ${filename}.h ${filename}
   DEPENDS ${filename}
)

endforeach()

LIST(APPEND EMOUS_DEBUG
   emous_device_debug.eo.x
   emous_device_debug.eo.h
   emous_type_debug.eo.x
   emous_type_debug.eo.h
   main.c
   test_mount.c
   )

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