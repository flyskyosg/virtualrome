# Look for the header file.
FIND_PATH(UNRAR_INCLUDE_DIR NAMES unrarlib.h PATHS ${UNRAR_DIR}/include)
MARK_AS_ADVANCED(UNRAR_INCLUDE_DIR)

# Look for the Release library.
FIND_LIBRARY(UNRAR_LIBRARY_RELEASE NAMES unrar PATHS ${UNRAR_DIR}/lib)
MARK_AS_ADVANCED(UNRAR_LIBRARY_RELEASE)

# Look for the Release library.
FIND_LIBRARY(UNRAR_LIBRARY_DEBUG NAMES unrard unrarD unrar_D PATHS ${UNRAR_DIR}/lib)
MARK_AS_ADVANCED(UNRAR_LIBRARY_DEBUG)

IF (UNRAR_LIBRARY_DEBUG AND UNRAR_LIBRARY_RELEASE)
  SET(UNRAR_LIBRARIES optimized ${UNRAR_LIBRARY_RELEASE}
                       debug ${UNRAR_LIBRARY_DEBUG})
ELSE (UNRAR_LIBRARY_DEBUG AND UNRAR_LIBRARY_RELEASE)
  IF (UNRAR_LIBRARY_DEBUG)
    SET (UNRAR_LIBRARIES ${UNRAR_LIBRARY_DEBUG})
  ENDIF (UNRAR_LIBRARY_DEBUG)
  IF (UNRAR_LIBRARY_RELEASE)
    SET (UNRAR_LIBRARIES ${UNRAR_LIBRARY_RELEASE})
  ENDIF (UNRAR_LIBRARY_RELEASE)
ENDIF (UNRAR_LIBRARY_DEBUG AND UNRAR_LIBRARY_RELEASE)


# handle the QUIETLY and REQUIRED arguments and set UNRAR_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(UNRAR DEFAULT_MSG UNRAR_LIBRARIES UNRAR_INCLUDE_DIR)




