# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see file LICENSE.txt

cmake_minimum_required(VERSION 3.10)
project(pg_dmitigr_nso CXX)

set(pg_dmitigr_nso_version_major 0)
set(pg_dmitigr_nso_version_minor 2)

# ------------------------------------------------------------------------------

enable_language(CXX)
set(CMAKE_CXX_STANDARD 11)
set(CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------------------

if(NOT DEFINED CMAKE_BUILD_TYPE OR NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
    "Build type: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

if(NOT DEFINED PG_INCLUDE_DIR_SERVER)
  set(PG_INCLUDE_DIR_SERVER "" CACHE
    PATH "Path to header files of the PostgreSQL server")
endif()

if ("${PG_INCLUDE_DIR_SERVER}" STREQUAL "")
  execute_process(COMMAND "pg_config" "--includedir-server"
    RESULT_VARIABLE status
    OUTPUT_VARIABLE PG_INCLUDE_DIR_SERVER
    )
  if (${status} EQUAL 0)
    string(STRIP "${PG_INCLUDE_DIR_SERVER}" PG_INCLUDE_DIR_SERVER)
  else()
    message(FATAL_ERROR "Could not determine the location of header files of the PostgreSQL server")
  endif()
endif()
get_filename_component(PG_INCLUDE_DIR_SERVER "${PG_INCLUDE_DIR_SERVER}" ABSOLUTE)

if(NOT DEFINED PG_PKGLIB_DIR)
  set(PG_PKGLIB_DIR "" CACHE
    PATH "Path to dynamically loadable modules of the PostgreSQL server")
endif()

if ("${PG_LIB_DIR}" STREQUAL "")
  execute_process(COMMAND "pg_config" "--libdir"
    RESULT_VARIABLE status
    OUTPUT_VARIABLE PG_LIB_DIR
    )
  if (${status} EQUAL 0)
    string(STRIP "${PG_LIB_DIR}" PG_LIB_DIR)
  else()
    message(FATAL_ERROR "Could not determine the location of object code libraries of the PostgreSQL server")
  endif()
endif()
get_filename_component(PG_LIB_DIR "${PG_LIB_DIR}" ABSOLUTE)

if(NOT DEFINED PG_PKGLIB_DIR)
  set(PG_PKGLIB_DIR "" CACHE
    PATH "Path to dynamically loadable modules of the PostgreSQL server")
endif()

if ("${PG_PKGLIB_DIR}" STREQUAL "")
  execute_process(COMMAND "pg_config" "--pkglibdir"
    RESULT_VARIABLE status
    OUTPUT_VARIABLE PG_PKGLIB_DIR
    )
  if (${status} EQUAL 0)
    string(STRIP "${PG_PKGLIB_DIR}" PG_PKGLIB_DIR)
  else()
    message(FATAL_ERROR "Could not determine the location of dynamically loadable modules of the PostgreSQL server")
  endif()
endif()
get_filename_component(PG_PKGLIB_DIR "${PG_PKGLIB_DIR}" ABSOLUTE)

if(NOT DEFINED PG_SHARE_DIR)
  set(PG_SHARE_DIR "" CACHE
    PATH "Path to architecture-independent support files of the PostgreSQL server")
endif()

if ("${PG_SHARE_DIR}" STREQUAL "")
  execute_process(COMMAND "pg_config" "--sharedir"
    RESULT_VARIABLE status
    OUTPUT_VARIABLE PG_SHARE_DIR
    )
  if (${status} EQUAL 0)
    string(STRIP "${PG_SHARE_DIR}" PG_SHARE_DIR)
  else()
    message(FATAL_ERROR "Could not determine the location of architecture-independent support files of the PostgreSQL server")
  endif()
endif()
get_filename_component(PG_SHARE_DIR "${PG_SHARE_DIR}" ABSOLUTE)

set(DMITIGR_PGNSO_STRCMP_LOCALE False CACHE BOOL
  "Compare strings according to the locale (or lexicographically if not)?")

message("Using \"${PG_INCLUDE_DIR_SERVER}\" as the location of header files of the PostgreSQL server")
message("Using \"${PG_LIB_DIR}\" as the location of object code libraries of the PostgreSQL server")
message("Using \"${PG_PKGLIB_DIR}\" as the location of dynamically loadable modules of the PostgreSQL server")
message("Using \"${PG_SHARE_DIR}/extension\" as the location of the PostgreSQL server extensions")
message("Compare strings according to the locale: ${DMITIGR_PGNSO_STRCMP_LOCALE}")

# ------------------------------------------------------------------------------

include_directories(BEFORE ${PG_INCLUDE_DIR_SERVER})
if (WIN32)
  include_directories(BEFORE
    ${PG_INCLUDE_DIR_SERVER}/port/win32_msvc
    ${PG_INCLUDE_DIR_SERVER}/port/win32)
endif()

add_library(pg_dmitigr_nso SHARED textnso.cpp)

if (WIN32)
  target_link_libraries(pg_dmitigr_nso PRIVATE "${PG_LIB_DIR}/postgres.lib")
endif()

if (DMITIGR_PGNSO_STRCMP_LOCALE)
  target_compile_definitions(pg_dmitigr_nso PRIVATE DMITIGR_PGNSO_STRCMP_LOCALE)
endif()

#find_package(ICU COMPONENTS i18n uc data REQUIRED)
#target_compile_definitions(pg_dmitigr_nso PUBLIC -DUSE_ICU)
#target_link_libraries(pg_dmitigr_nso PUBLIC ICU::i18n ICU::uc ICU::data)

set_target_properties(pg_dmitigr_nso
  PROPERTIES
  PREFIX ""
  OUTPUT_NAME "dmitigr_nso"
  LINKER_LANGUAGE "CXX"
  POSITION_INDEPENDENT_CODE True)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(pg_dmitigr_nso PRIVATE
    -fno-exceptions
#    -pedantic
    -Wall
    -Wextra
    -Wswitch)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(pg_dmitigr_nso PRIVATE
    -fno-exceptions
    -pedantic
    -Weverything)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_compile_options(pg_dmitigr_nso PRIVATE
    /W4
    /wd4127
    /wd4200
    /Zc:referenceBinding
    /errorReport:none
    /nologo
    /utf-8)
endif()

# ------------------------------------------------------------------------------

install(TARGETS pg_dmitigr_nso
  LIBRARY DESTINATION "${PG_PKGLIB_DIR}"
  RUNTIME DESTINATION "${PG_PKGLIB_DIR}"
  )

install(FILES
  dmitigr_nso--${pg_dmitigr_nso_version_major}.${pg_dmitigr_nso_version_minor}.sql
  dmitigr_nso.control

  DESTINATION "${PG_SHARE_DIR}/extension"
  )
