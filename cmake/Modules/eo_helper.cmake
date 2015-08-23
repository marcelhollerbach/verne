find_package(Eolian 1.15 REQUIRED)
# macro to create a eolian generated c source file
#
# macro adds a generate rule, which depends on the original file the rule will output file.x
#
# The passed include snippet will just be added to the command

macro(_rule_eox file include)
    add_custom_command(
       OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${file}.x
       COMMAND eolian_gen ${include} --gc --eo -o ${CMAKE_CURRENT_SOURCE_DIR}/${file}.x ${CMAKE_CURRENT_SOURCE_DIR}/${file}
       DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    )
endmacro()

# macro to create a eolian generated header file
#
# other details are like the eox rule
macro(_rule_eoh file include)
    add_custom_command(
       OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${file}.h
       COMMAND eolian_gen ${include} --gh --eo -o ${CMAKE_CURRENT_SOURCE_DIR}/${file}.h ${CMAKE_CURRENT_SOURCE_DIR}/${file}
       DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file}
    )
endmacro()

# Can be used to setup rules for eo module
#
# A eo module is a .eo or .eot file which will be used to generate a .x and .h file or a .h file
# build_files - A list of files
# include_dirs - A list of dirs to include
#
# Only .eo and .eot files in build_files will be used to setup a module

function(eo_rule_create build_files include_dirs)
   string(REPLACE "\n" "" EOLIAN_EO_DIR_WITHOUT_NEWLINE "${EOLIAN_EO_DIR}")
   set(includes "-I${EOLIAN_EO_DIR_WITHOUT_NEWLINE}")

   foreach(include_cmd ${include_dirs})
      string(CONCAT includes "${includes}" " -I${CMAKE_CURRENT_SOURCE_DIR}/${include_cmd}")
   endforeach()

   string(REPLACE " " ";" includes "${includes}")
   foreach(file ${build_files})
      get_filename_component(ext ${file} EXT)
      if (ext MATCHES "^\\.eo$")
         _rule_eoh("${file}" "${includes}")
         _rule_eox("${file}" "${includes}")
      endif()
      if (ext MATCHES "^\\.eot$")
         _rule_eoh("${file}" "${includes}")
      endif()
    endforeach()
endfunction()