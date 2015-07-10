include_directories(
  ${EINA_INCLUDE_DIRS}
  ${EO_INCLUDE_DIRS}
)

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

LIST(APPEND GRAPH
    Graph.h
    efl_tree_base.c
    efl_tree_base.eo.x
    efl_tree_base.eo.h
    efl_compare.c
    efl_compare.eo.x
    efl_compare.eo.h
   )

add_library(tree_lib SHARED ${GRAPH})

target_link_libraries(tree_lib
  ${EO_LIBRARIES}
)