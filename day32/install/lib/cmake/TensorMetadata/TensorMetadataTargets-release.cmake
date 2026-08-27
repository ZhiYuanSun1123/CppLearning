#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "TensorMetadata::tensor_metadata" for configuration "Release"
set_property(TARGET TensorMetadata::tensor_metadata APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(TensorMetadata::tensor_metadata PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libtensor_metadata.a"
  )

list(APPEND _cmake_import_check_targets TensorMetadata::tensor_metadata )
list(APPEND _cmake_import_check_files_for_TensorMetadata::tensor_metadata "${_IMPORT_PREFIX}/lib/libtensor_metadata.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
