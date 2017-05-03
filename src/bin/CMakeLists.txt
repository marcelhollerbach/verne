set(THEME_INSTALL_PATH "${CMAKE_INSTALL_FULL_DATADIR}/${CMAKE_PROJECT_NAME}/")

add_custom_command(
     OUTPUT ${CMAKE_BINARY_DIR}/data/theme/default.edc.edj
     COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/data/img ${CMAKE_SOURCE_DIR}/data/theme/efm/default.edc ${CMAKE_BINARY_DIR}/data/theme/default.edc.edj
     DEPENDS ${CMAKE_SOURCE_DIR}/data/theme/efm/default.edc
)

LIST(APPEND JESUS_FILES
  main.c
  main.h
  titlebar.c
  #ui.c
  config.c
  config.h
  exec_ui.c
  exec.c
  hooks.c
  history.c
  clipboard.c
  fs_operation.c
  shortcuts.c
  archive.h
  archive.c
  about.c
  ${CMAKE_BINARY_DIR}/data/theme/default.edc.edj
)

add_executable(verne ${JESUS_FILES})

include_directories(
  ${EINA_INCLUDE_DIRS}
  ${EVAS_INCLUDE_DIRS}
  ${EFL_INCLUDE_DIRS}
  ${EIO_INCLUDE_DIRS}
  ${EET_INCLUDE_DIRS}
  ${EFREET_INCLUDE_DIRS}
  ${ECORE_INCLUDE_DIRS}
  ${ELEMENTARY_INCLUDE_DIRS}
  ../lib/efm
  ../widgets/
  )

target_link_libraries (verne
   ${EINA_LIBRARIES}
   ${EVAS_LIBRARIES}
   ${EFL_LIBRARIES}
   ${EIO_LIBRARIES}
   ${EO_LIBRARIES}
   ${EFREET_LIBRARIES}
   ${EFREET_TRASH_LIBRARIES}
   ${ECORE_LIBRARIES}
   ${EET_LIBRARIES}
   ${ELEMENTARY_LIBRARIES}
   elm_ext
   efm)

target_compile_options(verne
  PUBLIC
   ${EINA_DEFINITIONS}
   ${EVAS_DEFINITIONS}
   ${EFL_DEFINITIONS}
   ${EIO_DEFINITIONS}
   ${EO_DEFINITIONS}
   ${EFREET_DEFINITIONS}
   ${EFREET_TRASH_DEFINITIONS}
   ${ECORE_DEFINITIONS}
   ${EET_DEFINITIONS}
   ${ELEMENTARY_DEFINITIONS}
)

file(READ ${CMAKE_SOURCE_DIR}/AUTHORS AUTHORS)
string(REPLACE "\n" "<br>" AUTHORS "${AUTHORS}")

set(THEME_PATH \"${THEME_INSTALL_PATH}\")
set(PACKAGE_DATA_DIR \"${CMAKE_INSTALL_FULL_DATADIR}/efm/\")
set(PACKAGE_LIB_DIR \"${CMAKE_INSTALL_FULL_LIBDIR}\")
set(PACKAGE_BIN_DIR \"${CMAKE_INSTALL_FULL_BINDIR}\")

configure_file(${CMAKE_SOURCE_DIR}/src/bin/config.h.cmake ${CMAKE_SOURCE_DIR}/src/bin/config.h)

INSTALL(FILES ${CMAKE_BINARY_DIR}/data/theme/default.edc.edj
  DESTINATION ${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}/)

INSTALL(TARGETS verne DESTINATION ${CMAKE_INSTALL_BINDIR})
