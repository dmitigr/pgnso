# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see file LICENSE.txt

cmake_minimum_required(VERSION 3.10)
project(pg_dmitigr_nso CXX)

set(pg_dmitigr_nso_version_major 0)
set(pg_dmitigr_nso_version_minor 1)

# ------------------------------------------------------------------------------

enable_language(CXX)
set(CMAKE_CXX_STANDARD 11)
set(CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------------------

if(NOT DEFINED PG_INCLUDE_DIR_SERVER)
  set(PG_INCLUDE_DIR_SERVER "" CACHE
    PATH "Path to header files for the PostgreSQL server")
endif()

if ("${PG_INCLUDE_DIR_SERVER}" STREQUAL "")
  execute_process(COMMAND "pg_config" "--includedir-server"
    RESULT_VARIABLE status
    OUTPUT_VARIABLE PG_INCLUDE_DIR_SERVER
    )
  if (${status} EQUAL 0)
    string(STRIP "${PG_INCLUDE_DIR_SERVER}" PG_INCLUDE_DIR_SERVER)
  else()
    message(FATAL_ERROR "Could not determine the location of header files for the PostgreSQL server")
  endif()
endif()
get_filename_component(PG_INCLUDE_DIR_SERVER "${PG_INCLUDE_DIR_SERVER}" ABSOLUTE)

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

message("Using \"${PG_INCLUDE_DIR_SERVER}\" as the location of header files for the PostgreSQL server")
message("Using \"${PG_PKGLIB_DIR}\" as the location of dynamically loadable modules of the PostgreSQL server")
message("Using \"${PG_SHARE_DIR}/extension\" as the location of the PostgreSQL server extensions")

# ------------------------------------------------------------------------------

include_directories(BEFORE ${PG_INCLUDE_DIR_SERVER})

add_library(pg_dmitigr_nso SHARED textnso.cpp)

#find_package(ICU COMPONENTS i18n uc data REQUIRED)
#target_compile_definitions(pg_dmitigr_nso PUBLIC -DUSE_ICU)
#target_link_libraries(pg_dmitigr_nso PUBLIC ICU::i18n ICU::uc ICU::data)

set_target_properties(pg_dmitigr_nso
  PROPERTIES
  PREFIX ""
  OUTPUT_NAME "dmitigr_nso"
  LINKER_LANGUAGE "CXX"
  POSITION_INDEPENDENT_CODE True
  DEBUG_POSTFIX "d")

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
