cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(hps-eve VERSION 0.0.1)

if(DEFINED ENV{ROOTSYS})
    list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
endif()
#message(FATAL_ERROR "ROOTSYS was not defined in the environment.")

find_package(ROOT REQUIRED COMPONENTS Core Rint Geom Gui Eve)
find_package(LCIO REQUIRED)

find_package(CURL)
if(CURL_FOUND)
    message(STATUS "CURL found at: ${CURL_LIBRARIES}")
    add_definitions(-DHAVE_CURL)
    include_directories(${CURL_INCLUDE_DIRS})
else()
    message(STATUS "CURL not found")
endif()

find_package(LibXml2)
if(LIBXML2_FOUND)
    message(STATUS "LibXml2 found at: ${LIBXML2_LIBRARIES}")
    add_definitions(-DHAVE_LIBXML2)
    include_directories(${LIBXML2_INCLUDE_DIRS})
else()
    message(STATUS "LibXml2 not found")
endif()

include(${ROOT_USE_FILE})

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${ROOT_INCLUDE_DIRS})
include_directories(${LCIO_INCLUDE_DIRS})

add_definitions(${ROOT_CXX_FLAGS})

root_generate_dictionary(EventDisplayDic ${PROJECT_SOURCE_DIR}/include/EventDisplayDef.h
    LINKDEF ${PROJECT_SOURCE_DIR}/include/EventDisplayLinkDef.h)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libEventDisplayDic_rdict.pcm DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

file(GLOB_RECURSE library_sources ${PROJECT_SOURCE_DIR}/src/*.cxx)

add_library(EventDisplay SHARED EventDisplayDic.cxx ${library_sources})
install(TARGETS EventDisplay LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

set(exec_name hps-eve)

add_executable(hps-eve ${library_sources} ${PROJECT_SOURCE_DIR}/hps_eve.cxx)
target_link_libraries(${exec_name}
    ${LCIO_LCIO_LIBRARY}
    ${LCIO_SIO_LIBRARY}
    EventDisplay
    ROOT::Core
    ROOT::Rint
    ROOT::Geom
    ROOT::Gui
    ROOT::Eve)
if(CURL_FOUND)
    target_link_libraries(${exec_name} ${CURL_LIBRARIES})
endif()
if(LIBXML2_FOUND)
    target_link_libraries(${exec_name} ${LIBXML2_LIBRARIES})
endif()
install(TARGETS ${exec_name} ${exec_name} DESTINATION bin)

configure_file( ${PROJECT_SOURCE_DIR}/scripts/hps-eve-env.sh.in ${CMAKE_CURRENT_BINARY_DIR}/hps-eve-env.sh @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/hps-eve-env.sh DESTINATION bin
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/icons DESTINATION share)

message(STATUS "hps-eve will be installed to: ${CMAKE_INSTALL_PREFIX}")
