# Adapted from https://eicweb.phy.anl.gov/EIC/NPDet/-/blob/master/cmake/modules/FindOCC.cmake

# - Try to find OpenCASCADE libraries
### Does not test what version has been found,though
### that could be done by parsing Standard_Version.hxx

# Once done, this will define
#  OCC_FOUND - true if OCC has been found
#  OCC_VERSION - the OCC version
#  OCC_INCLUDE_DIR - the OCC include dir
#  OCC_LIBRARY_DIR - the OCC library directory
#  OCC_LIBRARIES (not cached) - full path of OCC libraries

set(_occdirs ${CASROOT} ${CASS_DIR} $ENV{CASROOT})

find_path(OCC_INCLUDE_DIR
          NAMES Standard_Version.hxx
          HINTS ${_occdirs}
          PATH_SUFFIXES opencascade
          DOC "Specify the directory containing Standard_Version.hxx")

file(READ "${OCC_INCLUDE_DIR}/Standard_Version.hxx" _standard_ver)
string(REGEX MATCH "OCC_VERSION_MAJOR[ ]+([0-9]+)" _ "${_standard_ver}")
set(OCC_VER_MAJOR ${CMAKE_MATCH_1})

string(REGEX MATCH "OCC_VERSION_MINOR[ ]+([0-9]+)" _ "${_standard_ver}")
set(OCC_VER_MINOR ${CMAKE_MATCH_1})

string(REGEX MATCH "OCC_VERSION_MAINTENANCE[ ]+([0-9]+)" _ "${_standard_ver}")
set(OCC_VER_MAINTENANCE ${CMAKE_MATCH_1})

set(OCC_VERSION "${OCC_VER_MAJOR}.${OCC_VER_MINOR}.${OCC_VER_MAINTENANCE}")
if (NOT OCC_VERSION MATCHES "^[0-9]+\.[0-9]+\.[0-9]+$")
  unset(OCC_VERSION)
endif()

set(OCC_LIBRARIES_FOUNDATION
  TKMath
  TKernel
)

foreach (OCC_LIB ${OCC_LIBRARIES_FOUNDATION})
  if (NOT OCC_LIBRARY_DIR)
    find_library(OCC_${OCC_LIB} NAMES ${OCC_LIB} PATHS ${_occdirs})
  else()
    find_library(OCC_${OCC_LIB} NAMES ${OCC_LIB} PATHS ${OCC_LIBRARY_DIR} NO_DEFAULT_PATH)
  endif()
  list(APPEND OCC_LIBRARIES "${OCC_${OCC_LIB}}")
endforeach()

get_filename_component(OCC_LIBRARY_DIR "${OCC_TKernel}" DIRECTORY)

set(OCC_LIBRARIES_MODELING_DATA
  TKBRep
  TKG2d
  TKG3d
  TKGeomBase
)

foreach (OCC_LIB ${OCC_LIBRARIES_MODELING_DATA})
  find_library(OCC_${OCC_LIB} NAMES ${OCC_LIB} PATHS ${OCC_LIBRARY_DIR})
  list(APPEND OCC_LIBRARIES "${OCC_${OCC_LIB}}")
endforeach()

set(OCC_LIBRARIES_DATA_EXCHANGE
  TKBinXCAF
  TKIGES
  TKSTEP
  TKSTEP209
  TKSTEPAttr
  TKSTEPBase
  TKSTL
  TKVRML
  TKXCAF
# TKXCAFSchema # only used in OCE (OpenCascade 6.9.x)
  TKXDEIGES
  TKXDESTEP
  TKXSBase
  TKXmlXCAF
)

foreach (OCC_LIB ${OCC_LIBRARIES_DATA_EXCHANGE})
  find_library(OCC_${OCC_LIB} NAMES ${OCC_LIB} PATHS ${OCC_LIBRARY_DIR})
  list(APPEND OCC_LIBRARIES "${OCC_${OCC_LIB}}")
endforeach()

set(OCC_LIBRARIES_MODELING_ALGO
  TKBO
  TKBool
  TKFeat
  TKFillet
  TKGeomAlgo
  TKHLR
  TKMesh
  TKOffset
  TKPrim
  TKShHealing
  TKTopAlgo
  TKXMesh
)

foreach (OCC_LIB ${OCC_LIBRARIES_MODELING_ALGO})
  find_library(OCC_${OCC_LIB} NAMES ${OCC_LIB} PATHS ${OCC_LIBRARY_DIR})
  list(APPEND OCC_LIBRARIES "${OCC_${OCC_LIB}}")
endforeach()

# handle the QUIETLY and REQUIRED arguments and set OCC_FOUND to TRUE if
# all listed variables are TRUE

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OCC DEFAULT_MSG OCC_VERSION OCC_INCLUDE_DIR OCC_LIBRARY_DIR)
mark_as_advanced(${OCC_LIBRARIES})
