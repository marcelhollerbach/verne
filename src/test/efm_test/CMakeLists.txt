add_executable(efm_suite test_efm.c)

target_link_libraries (efm_suite
   ${EINA_LIBRARIES}
   ${ECORE_LIBRARIES}
   ${EO_LIBRARIES}
   ${CMAKE_THREAD_LIBS_INIT}
   ${Check_LIBRARIES}
   efm
)
set_property(TARGET efm_suite
   PROPERTY COMPILE_DEFINITIONS TEST_RESSOURCE="${CMAKE_SOURCE_DIR}/src/test/archive.tar")

add_test(efm_test efm_suite)
