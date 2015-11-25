set(THEME_INSTALL_PATH "${CMAKE_INSTALL_FULL_DATADIR}/jesus/")

add_custom_command(
     OUTPUT ${CMAKE_BINARY_DIR}/data/theme/jesus.edc.edj
     COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/data/img ${CMAKE_SOURCE_DIR}/data/theme/efm/jesus.edc ${CMAKE_BINARY_DIR}/data/theme/jesus.edc.edj
     DEPENDS ${CMAKE_SOURCE_DIR}/data/theme/efm/jesus.edc
)

LIST(APPEND JESUS_FILES
  jesus.c
  titlebar.c
  ui.c
  config.c
  exec_ui.c
  exec.c
  hooks.c
  history.c
  clipboard.c
  fs_operation.c
  shortcuts.c
  ${CMAKE_BINARY_DIR}/data/theme/jesus.edc.edj
)

add_executable(jesus ${JESUS_FILES})

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

target_link_libraries (jesus
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
   efm_lib)

set_property(TARGET jesus PROPERTY COMPILE_DEFINITIONS THEME_PATH="${THEME_INSTALL_PATH}"
PACKAGE_DATA_DIR="${CMAKE_INSTALL_DATADIR}/efm/"
PACKAGE_LIB_DIR="${CMAKE_INSTALL_LIBDIR}"
PACKAGE_BIN_DIR="${CMAKE_INSTALL_BINDIR}"
)

INSTALL(FILES ${CMAKE_BINARY_DIR}/data/theme/jesus.edc.edj
  DESTINATION ${CMAKE_INSTALL_DATADIR}/jesus/)

INSTALL(TARGETS jesus DESTINATION ${CMAKE_INSTALL_BINDIR})
