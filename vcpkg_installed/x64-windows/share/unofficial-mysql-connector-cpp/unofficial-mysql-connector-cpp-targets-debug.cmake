#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "unofficial::mysql-connector-cpp::connector" for configuration "Debug"
set_property(TARGET unofficial::mysql-connector-cpp::connector APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(unofficial::mysql-connector-cpp::connector PROPERTIES
  IMPORTED_IMPLIB_DEBUG "${_IMPORT_PREFIX}/debug/lib/mysqlcppconn8.lib"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/bin/mysqlcppconn8-2-vs14.dll"
  )

list(APPEND _cmake_import_check_targets unofficial::mysql-connector-cpp::connector )
list(APPEND _cmake_import_check_files_for_unofficial::mysql-connector-cpp::connector "${_IMPORT_PREFIX}/debug/lib/mysqlcppconn8.lib" "${_IMPORT_PREFIX}/debug/bin/mysqlcppconn8-2-vs14.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
