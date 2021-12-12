# Adapted from https://eicweb.phy.anl.gov/EIC/NPDet/-/blob/master/cmake/modules/FindOCC.cmake

# - Try to find OpenCASCADE libraries
### Does not test what version has been found,though
### that could be done by parsing Standard_Version.hxx

# Once done, this will define
#  OCC_FOUND - true if OCC has been found
#  OCC_VERSION - the OCC version
#  OCC_INCLUDE_DIR - the OCC include dir
#  OCC_LIBRARY_DIR - the OCC library directory
#  OCC_LIBRARIES - full path of OCC libraries

if (NOT CASROOT)
  find_path(OCC_INCLUDE_DIR
            NAMES Standard_Version.hxx
            PATH_SUFFIXES opencascade
            NO_CACHE
            DOC "Specify the directory containing Standard_Version.hxx")
else()
  find_path(OCC_INCLUDE_DIR
    NAMES Standard_Version.hxx
    PATHS ${CASROOT} "${CASROOT}/include"
    PATH_SUFFIXES opencascade
    NO_DEFAULT_PATH
    NO_CACHE
    DOC "Specify the directory containing Standard_Version.hxx")
endif()


file(READ "${OCC_INCLUDE_DIR}/Standard_Version.hxx" _standard_ver)
string(REGEX MATCH "OCC_VERSION_MAJOR[ \t]+([0-9]+)" _ "${_standard_ver}")
set(OCC_VER_MAJOR ${CMAKE_MATCH_1})

string(REGEX MATCH "OCC_VERSION_MINOR[ \t]+([0-9]+)" _ "${_standard_ver}")
set(OCC_VER_MINOR ${CMAKE_MATCH_1})

string(REGEX MATCH "OCC_VERSION_MAINTENANCE[ \t]+([0-9]+)" _ "${_standard_ver}")
set(OCC_VER_MAINTENANCE ${CMAKE_MATCH_1})

set(OCC_VERSION "${OCC_VER_MAJOR}.${OCC_VER_MINOR}.${OCC_VER_MAINTENANCE}")
if (NOT OCC_VERSION MATCHES "^[0-9]+\.[0-9]+\.[0-9]+$")
  unset(OCC_VERSION)
endif()

set(OCC_LIBRARIES_FOUNDATION
  TKMath
  TKernel
)

set(OCC_LIB_PATH_SUFFIX "lib")
if (LINK_OCC_DEBUG)
  set(OCC_LIB_PATH_SUFFIX "lib/debug")
endif(LINK_OCC_DEBUG)

set(OCC_LIBRARIES "")
foreach (OCC_LIB ${OCC_LIBRARIES_FOUNDATION})
  if (NOT CASROOT)
    find_library(OCC_${OCC_LIB} NAMES ${OCC_LIB} PATHS} NO_CACHE)
  else()
    find_library(OCC_${OCC_LIB} NAMES ${OCC_LIB} PATHS ${CASROOT} PATH_SUFFIXES ${OCC_LIB_PATH_SUFFIX} NO_DEFAULT_PATH)
  endif()
  list(APPEND OCC_LIBRARIES "${OCC_${OCC_LIB}}")
  mark_as_advanced("${OCC_${OCC_LIB}}")
endforeach()

get_filename_component(OCC_LIBRARY_DIR "${OCC_TKernel}" DIRECTORY)

set(OCC_LIBRARIES_MODELING_DATA
  TKBRep
  TKG2d
  TKG3d
  TKGeomBase
)

foreach (OCC_LIB ${OCC_LIBRARIES_MODELING_DATA})
  find_library(OCC_${OCC_LIB} NAMES ${OCC_LIB} PATHS ${OCC_LIBRARY_DIR} NO_CACHE NO_DEFAULT_PATH)
  list(APPEND OCC_LIBRARIES "${OCC_${OCC_LIB}}")
  mark_as_advanced("${OCC_${OCC_LIB}}")
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
  find_library(OCC_${OCC_LIB} NAMES ${OCC_LIB} PATHS ${OCC_LIBRARY_DIR} NO_CACHE NO_DEFAULT_PATH)
  list(APPEND OCC_LIBRARIES "${OCC_${OCC_LIB}}")
  mark_as_advanced("${OCC_${OCC_LIB}}")
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
  find_library(OCC_${OCC_LIB} NAMES ${OCC_LIB} PATHS ${OCC_LIBRARY_DIR} NO_CACHE NO_DEFAULT_PATH)
  list(APPEND OCC_LIBRARIES "${OCC_${OCC_LIB}}")
  mark_as_advanced("${OCC_${OCC_LIB}}")
endforeach()

set(OCC_LIBRARIES_APP_FRAMEWORK
  TKBin
  TKBinL
  TKBinTObj
  TKCAF
  TKCDF
  TKLCAF
  TKStd
  TKStdL
  TKTObj
  TKVCAF
  TKXml
  TKXmlL
  TKXmlTObj
)

foreach (OCC_LIB ${OCC_LIBRARIES_APP_FRAMEWORK})
  find_library(OCC_${OCC_LIB} NAMES ${OCC_LIB} PATHS ${OCC_LIBRARY_DIR} NO_CACHE NO_DEFAULT_PATH)
  list(APPEND OCC_LIBRARIES "${OCC_${OCC_LIB}}")
  mark_as_advanced("${OCC_${OCC_LIB}}")
endforeach()

# handle the QUIETLY and REQUIRED arguments and set OCC_FOUND to TRUE if
# all listed variables are TRUE

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OCC DEFAULT_MSG OCC_VERSION OCC_INCLUDE_DIR OCC_LIBRARY_DIR)
mark_as_advanced(${OCC_LIBRARIES})
