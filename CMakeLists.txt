cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(hps-eve VERSION 0.0.1)

if(NOT DEFINED ENV{ROOTSYS})
    message(FATAL_ERROR "ROOTSYS was not defined in the environment.")
endif()
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

find_package(ROOT REQUIRED COMPONENTS Core Rint Geom Gui Eve)

include(${ROOT_USE_FILE})

root_generate_dictionary(EventDisplayDic ${PROJECT_SOURCE_DIR}/include/EventDisplayDef.h
    LINKDEF ${PROJECT_SOURCE_DIR}/include/EventDisplayLinkDef.h)

# Install ROOT pcm file
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libEventDisplayDic_rdict.pcm DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${ROOT_INCLUDE_DIRS})

add_definitions(${ROOT_CXX_FLAGS})

file(GLOB_RECURSE library_sources ${PROJECT_SOURCE_DIR}/src/*.cxx)

add_library(EventDisplay SHARED EventDisplayDic.cxx ${library_sources})

# Install the libraries and headers
install(TARGETS EventDisplay LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

add_executable(hps-eve ${library_sources} ${PROJECT_SOURCE_DIR}/hps_eve.cxx)

target_link_libraries(hps-eve
    EventDisplay
    ROOT::Core
    ROOT::Rint
    ROOT::Geom
    ROOT::Gui
    ROOT::Eve)

message(STATUS "hps-eve will be installed to: ${CMAKE_INSTALL_PREFIX}")

install(TARGETS hps-eve hps-eve DESTINATION bin)
