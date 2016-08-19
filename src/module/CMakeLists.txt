execute_process(COMMAND pkg-config --variable=modules enlightenment
   OUTPUT_VARIABLE _pkgconfig_invoke_result
   RESULT_VARIABLE _pkgconfig_failed)

if(_pkgconfig_failed)
   BUILD_WARNING ("Failed to find Eolian Eo directory.")
else()
   set (ENLIGHTENMENT_MOD_DIR ${_pkgconfig_invoke_result})
endif()

execute_process(COMMAND pkg-config --variable=release enlightenment
   OUTPUT_VARIABLE _pkgconfig_invoke_result
   RESULT_VARIABLE _pkgconfig_failed)

if(_pkgconfig_failed)
   BUILD_WARNING ("Failed to find Eolian Eo directory.")
else()
   set (ENLIGHTENMENT_VERSION ${_pkgconfig_invoke_result})
endif()

set(MOD_DESKTOP_INSTALL_DIR "${ENLIGHTENMENT_MOD_DIR}/kvasir/")
set(MOD_INSTALL_DIR "${MOD_DESKTOP_INSTALL_DIR}linux-gnu-${CMAKE_SYSTEM_PROCESSOR}-${ENLIGHTENMENT_VERSION}/")

string(REPLACE "\n" "" MOD_DESKTOP_INSTALL_DIR "${MOD_DESKTOP_INSTALL_DIR}")
string(REPLACE "\n" "" MOD_INSTALL_DIR "${MOD_INSTALL_DIR}")

message("Installing module to ${MOD_DESKTOP_INSTALL_DIR}")
message("Installing module binary to ${MOD_INSTALL_DIR}")

include_directories(
  ${Enlightenment_INCLUDE_DIRS}
  ${ELEMENTARY_INCLUDE_DIRS}
  ../lib/efm/
  ../widgets/
)

LIST(APPEND kvasir_files
   e_bg_widget.c
   e_bg_widget.eo
   e_bg_widget.eo.x
   e_bg_widget.eo.h
   e_mod_main.c
   )
list(APPEND bg_deps
  ../widgets/file/
  ../lib/efm
  )

eo_rule_create("${kvasir_files}" "${bg_deps}")

add_library(kvasir_module MODULE ${kvasir_files})

target_link_libraries (kvasir_module
   ${EINA_LIBRARIES}
   ${EVAS_LIBRARIES}
   ${EFL_LIBRARIES}
   ${EINA_LIBRARIES}
   ${EO_LIBRARIES}
   ${ELEMENTARY_LIBRARIES}
   efm
   elm_ext

)

set_target_properties(kvasir_module PROPERTIES OUTPUT_NAME module)
set_target_properties(kvasir_module PROPERTIES PREFIX "")

INSTALL(TARGETS kvasir_module
  DESTINATION  ${MOD_INSTALL_DIR}
  RENAME module.so
)
INSTALL(FILES module.desktop DESTINATION ${MOD_DESKTOP_INSTALL_DIR})
