cmake_minimum_required(VERSION 3.0.2)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(verne)

include(GNUInstallDirs)

option(Debug "Show the debug screen" OFF)

enable_testing()
set(CMAKE_BUILD_TYPE DEBUG)

add_definitions ("-Wall -Wextra -fvisibility=hidden -std=gnu99 -g -Wshadow")

find_package(Eina REQUIRED)
find_package(Eo 1.18 REQUIRED)
find_package(Eolian 1.18 REQUIRED)
find_package(Eet REQUIRED)
find_package(Efl REQUIRED)
find_package(Eldbus REQUIRED)
find_package(Efreet COMPONENTS mime trash)
find_package(Eio REQUIRED)
find_package(Evas 1.18 REQUIRED)
find_package(Edje REQUIRED)
find_package(Ecore COMPONENTS file)
find_package(Elementary 1.18.0 REQUIRED)
find_package(LibArchive REQUIRED)
find_package(Threads REQUIRED)

pkg_check_modules(Mount mount)
pkg_check_modules(Check REQUIRED check)
pkg_check_modules(Enlightenment REQUIRED enlightenment)

if (Mount_FOUND)
    message("Mount is found! YEAH")
elseif (NOT Mount_FOUND)
    message("Mount is not found! Mountpoint detection will not work!")
endif (Mount_FOUND)

if (${Eolian_VERSION} VERSION_LESS 1.16.99)
    message("Using legecy eo_helper")
    include(eo_helper)
endif (${Eolian_VERSION} VERSION_LESS 1.16.99)

add_subdirectory(data)
add_subdirectory(src/lib/emous)
add_subdirectory(src/lib/emous/modules/udisk)
add_subdirectory(src/lib/efm)
add_subdirectory(src/widgets)
add_subdirectory(src/test)
add_subdirectory(src/bin)
add_subdirectory(src/module)