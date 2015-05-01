cmake_minimum_required(VERSION 2.81)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(efm)

enable_testing()

set(CMAKE_BUILD_TYPE DEBUG)
add_definitions ("-Wall -Wextra")
find_package(Eina REQUIRED)
find_package(Eo REQUIRED)
find_package(Eolian REQUIRED)
find_package(Eet REQUIRED)
find_package(Efl REQUIRED)
find_package(Efreet COMPONENTS mime trash)
find_package(Eio REQUIRED)
find_package(Evas REQUIRED)
find_package(Edje REQUIRED)
find_package(Ecore COMPONENTS file)
find_package(Elementary 1.13.0 REQUIRED)


add_subdirectory(data)
add_subdirectory(src/lib/emous)
add_subdirectory(src/lib/efm)
add_subdirectory(src/widgets)
add_subdirectory(src/test)
add_subdirectory(src/bin)
