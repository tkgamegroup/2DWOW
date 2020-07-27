cmake_minimum_required(VERSION 3.16.4)

project(2DWOW)

add_definitions(-W0 -std:c++latest)

foreach(type ${CMAKE_CONFIGURATION_TYPES})
	string(TOUPPER ${type} type_u)
	string(TOLOWER ${type} type_l)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${type_u} "${CMAKE_SOURCE_DIR}/bin/${type_l}")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${type_u} "${CMAKE_SOURCE_DIR}/bin/${type_l}")
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${type_u} "${CMAKE_SOURCE_DIR}/bin/${type_l}")
endforeach()

set(FLAME_DIR "" CACHE PATH "Flame Engine Dir")

add_executable(2DWOW main.cpp)
target_include_directories(2DWOW PRIVATE "${FLAME_DIR}/include")
target_link_libraries(2DWOW "${FLAME_DIR}/bin/debug/flame_foundation.lib")
add_custom_command(TARGET 2DWOW POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FLAME_DIR}/bin/debug/flame_foundation.dll" "${CMAKE_SOURCE_DIR}/bin/debug")
