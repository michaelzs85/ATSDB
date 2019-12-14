set(ATSDB_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
message("  Path: ${ATSDB_PATH}")

cmake_minimum_required(VERSION 3.1)
set ( CMAKE_BUILD_TYPE Debug )
set(CMAKE_CXX_FLAGS_MYREL "-O3")
#set ( CMAKE_BUILD_TYPE Release )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project( ATSDB )
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "4")
SET(CPACK_PACKAGE_VERSION_PATCH "2")

message("  System: ${CMAKE_SYSTEM}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

message("  Platform: Linux")
add_definitions ( -Wall -std=c++11)

include(${CMAKE_BINARY_DIR}/conan_paths.cmake)

#add_definitions(${Qt5Widgets_DEFINITIONS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CONAN_CXX_FLAGS} -fPIC")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH};${CMAKE_BINARY_DIR};${ATSDB_PATH}/cmake_modules)
message("  CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

#find_package(Qt5Widgets)
#find_package(Qt5Core)
#find_package(Qt5OpenGL)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

find_package (Qt5 5.13 COMPONENTS Core Widgets OpenGL REQUIRED)
find_package(qt)
message("  qt_INCLUDE_DIRS: ${qt_INCLUDE_DIRS}")
#find_package(Qt5 )


#find_package (boost COMPONENTS boost_program_options REQUIRED)
find_package (boost_any REQUIRED)
find_package (boost_program_options REQUIRED)
find_package (boost_date_time REQUIRED)
find_package (boost_assign REQUIRED)

find_package (Eigen3 3.3 REQUIRED)

find_package ( MySQLpp REQUIRED )
message("  MySQLpp_INCLUDE_DIR: ${MySQLpp_INCLUDE_DIR}")
message("  MySQLpp_LIBRARY: ${MySQLpp_LIBRARY}")

find_package ( MySQL REQUIRED )
message("  MYSQL_INCLUDE_DIR: ${MYSQL_INCLUDE_DIR}")
message("  MYSQL_LIBRARY: ${MYSQL_LIBRARY}")

find_package ( sqlite3 REQUIRED )

find_package ( LOG4CPP REQUIRED )
message("  LOG4CPP_INCLUDE_DIR: ${LOG4CPP_INCLUDE_DIR}")
message("  LOG4CPP_LIBRARY: ${LOG4CPP_LIBRARIES}")

find_package ( GDAL REQUIRED )

find_package ( tinyxml2 REQUIRED )

find_package ( LibArchive REQUIRED )

find_package(TBB REQUIRED)

find_package ( jASTERIX )
message("  jASTERIX found: ${jASTERIX_FOUND}")
message("  jASTERIX_INCLUDE_DIR: ${jASTERIX_INCLUDE_DIR}")
message("  jASTERIX_LIBRARIES: ${jASTERIX_LIBRARIES}")

find_package ( OpenSSL REQUIRED )

#find_package ( JSONCPP REQUIRED )
#message("  JSONCPP_INCLUDE_DIR: ${JSONCPP_INCLUDE_DIR}")
#message("  JSONCPP_LIBRARY: ${JSONCPP_LIBRARY}")

add_library(atsdb SHARED "")

message("Install path: ${CMAKE_INSTALL_PREFIX}")

include(src/CMakeLists.txt)

set (EXPERIMENTAL_SRC true)
message("Experimental source: ${EXPERIMENTAL_SRC}")

IF (EXPERIMENTAL_SRC)
    IF(IS_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/experimental_src")
        MESSAGE(STATUS "Experimental source found")
    ELSE()
        MESSAGE(STATUS "Experimental source not found found, deactivating")
        set (EXPERIMENTAL_SRC false)
    ENDIF()
ENDIF()

IF (EXPERIMENTAL_SRC)
    MESSAGE(STATUS "Including experimental source")
    include(experimental_src/CMakeLists.txt)
ENDIF()

configure_file(src/util/global.h.in include/global.h)

# includes and library linking

include_directories (
#    ${CMAKE_SOURCE_DIR}
#    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROJECT_BINARY_DIR}/include
#    ${Qt5Widgets_INCLUDE_DIRS}
#    ${Qt5OpenGL_INCLUDE_DIRS}
#    ${boost_INCLUDE_DIRS}
#    ${sqlite3_INCLUDE_DIRS}
    ${MYSQL_INCLUDE_DIR}
    ${MySQLpp_INCLUDE_DIR}
    ${LOG4CPP_INCLUDE_DIR}
#    ${GDAL_INCLUDE_DIRS}
#    ${OPENSCENEGRAPH_INCLUDE_DIRS}
#    ${LibArchive_INCLUDE_DIRS}
    ${CONAN_EIGEN3_ROOT}/include/eigen3
    ${boost_program_options_INCLUDE_DIRS}
    ${boost_any_INCLUDE_DIRS}
    ${boost_assign_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIR}
#    ${TBB_INCLUDE_DIRS}
#    ${tinyxml2_INCLUDE_DIRS}
    )

#link_directories(

 #   )

IF (jASTERIX_FOUND)
include_directories (
    ${jASTERIX_INCLUDE_DIR}
    )

link_directories(
    ${jASTERIX_LIBRARIES}
    )

target_link_libraries ( atsdb
    ${jASTERIX_LIBRARIES})
ENDIF()

target_link_libraries ( atsdb optimized
    boost_program_options::boost_program_options
    boost_date_time::boost_date_time
    LibArchive::LibArchive
    qt::qt
    TBB::TBB
    GDAL::GDAL
    tinyxml2::tinyxml2
    sqlite3::sqlite3
    OpenSSL::OpenSSL
    ${MySQLpp_LIBRARY}
    ${MYSQL_LIBRARY}
    ${LOG4CPP_LIBRARIES}
    )


# installation stuff

message("Installing using prefix: ${CMAKE_INSTALL_PREFIX}")
install(DIRECTORY "conf" DESTINATION atsdb)
install(DIRECTORY "data" DESTINATION atsdb)

install (TARGETS atsdb DESTINATION lib
    PUBLIC_HEADER DESTINATION include/atsdb)

install (TARGETS atsdb_client DESTINATION bin)

# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE
     "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
include (CPack)


# add a target to generate API documentation with Doxygen
#find_package(Doxygen)
#if(DOXYGEN_FOUND)
#    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in @ONLY)
#    add_custom_target(doc
#        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile
#        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc
#        COMMENT "Generating API documentation with Doxygen" VERBATIM
#        )
#endif(DOXYGEN_FOUND)

#file(GLOB DOT ".*")
#file(GLOB TILD "*~")
#file(GLOB TEMP "*.cmake")

