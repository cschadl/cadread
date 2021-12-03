# Adapted from https://eicweb.phy.anl.gov/EIC/NPDet/-/blob/master/cmake/modules/FindOCC.cmake

# - Try to find OpenCASCADE libraries
### Does not test what version has been found,though
### that could be done by parsing Standard_Version.hxx

# Once done, this will define
#  OCC_FOUND - true if OCC has been found
#  OCC_INCLUDE_DIR - the OCC include dir
#  OCC_LIBRARIES (not cached) - full path of OCC libraries

set(_occdirs ${CASROOT} ${CASS_DIR} $ENV{CASROOT} /opt/occ)

find_path(OCC_INCLUDE_DIR
          NAMES Standard_Real.hxx
          HINTS ${_occdirs} /usr/include/opencascade /usr/include/oce
          PATH_SUFFIXES inc
          DOC "Specify the directory containing Standard_Real.hxx")

set(OCC_LIBRARIES_FOUNDATION
  TKMath
  TKernel
)

foreach (OCC_LIB ${OCC_LIBRARIES_FOUNDATION})
  find_library(OCC_${OCC_LIB} NAMES ${OCC_LIB} PATHS _occdirs /usr/lib)
  list(APPEND OCC_LIBRARIES "${OCC_${OCC_LIB}}")
endforeach()

set(OCC_LIBRARIES_MODELING_DATA
  TKBRep
  TKG2d
  TKG3d
  TKGeomBase
)

foreach (OCC_LIB ${OCC_LIBRARIES_MODELING_DATA})
  find_library(OCC_${OCC_LIB} NAMES ${OCC_LIB} PATHS _occdirs /usr/lib)
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
  find_library(OCC_${OCC_LIB} NAMES ${OCC_LIB} PATHS _occdirs /usr/lib)
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
  find_library(OCC_${OCC_LIB} NAMES ${OCC_LIB} PATHS _occdirs /usr/lib)
  list(APPEND OCC_LIBRARIES "${OCC_${OCC_LIB}}")
endforeach()

# handle the QUIETLY and REQUIRED arguments and set OCC_FOUND to TRUE if
# all listed variables are TRUE

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OCC DEFAULT_MSG OCC_INCLUDE_DIR)
mark_as_advanced(OCC_INCLUDE_DIR ${OCC_LIBRARIES})
