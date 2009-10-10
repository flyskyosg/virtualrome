# Look for the header file.
FIND_PATH(CURL_INCLUDE_DIR NAMES curl/curl.h PATHS ${CURL_DIR}/include)
MARK_AS_ADVANCED(CURL_INCLUDE_DIR)

# Look for the Release library.
FIND_LIBRARY(CURL_LIBRARY_RELEASE NAMES cmcurl libcurl PATHS ${CURL_DIR}/lib)
MARK_AS_ADVANCED(CURL_LIBRARY_RELEASE)

# Look for the Release library.
FIND_LIBRARY(CURL_LIBRARY_DEBUG NAMES cmcurld libcurld PATHS ${CURL_DIR}/lib)
MARK_AS_ADVANCED(CURL_LIBRARY_DEBUG)

IF (CURL_LIBRARY_DEBUG AND CURL_LIBRARY_RELEASE)
  SET(CURL_LIBRARIES optimized ${CURL_LIBRARY_RELEASE}
                       debug ${CURL_LIBRARY_DEBUG})
ELSE (CURL_LIBRARY_DEBUG AND CURL_LIBRARY_RELEASE)
  IF (CURL_LIBRARY_DEBUG)
    SET (CURL_LIBRARIES ${CURL_LIBRARY_DEBUG})
  ENDIF (CURL_LIBRARY_DEBUG)
  IF (CURL_LIBRARY_RELEASE)
    SET (CURL_LIBRARIES ${CURL_LIBRARY_RELEASE})
  ENDIF (CURL_LIBRARY_RELEASE)
ENDIF (CURL_LIBRARY_DEBUG AND CURL_LIBRARY_RELEASE)


# handle the QUIETLY and REQUIRED arguments and set CURL_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CURL DEFAULT_MSG CURL_LIBRARIES CURL_INCLUDE_DIR)












#luigi## Locate Lib cURL
#luigi## This module defines
#luigi## CURL_LIBRARY
#luigi## CURL_FOUND, if false, do not try to link to Gecko SDK 
#luigi## CURL_INCLUDE_DIR, where to find the headers
#luigi##
#luigi## $CURL_DIR is an environment variable that would
#luigi## correspond to the ./configure --prefix=$CURL_DIR
#luigi#
#luigi#
#luigi#FIND_PATH(CURL_INCLUDE_DIR curl/curl.h	
#luigi#		${CURL_ROOT}/include
#luigi#    $ENV{CURL_DIR}/include
#luigi#    $ENV{CURL_DIR}
#luigi#    $ENV{CURLDIR}/include
#luigi#    $ENVCURL_DIR}
#luigi#    $ENV{CURL_ROOT}/include
#luigi#    ~/Library/Frameworks
#luigi#    /Library/Frameworks
#luigi#    /usr/local/include
#luigi#    /usr/include
#luigi#    /sw/include # Fink
#luigi#    /opt/local/include # DarwinPorts
#luigi#    /opt/csw/include # Blastwave
#luigi#    /opt/include
#luigi#    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;CURL_ROOT]/include
#luigi#    /usr/freeware/include
#luigi#)
#luigi#
#luigi#SET(CURL_FIND_PATHS 
#luigi#				${CURL_ROOT}/lib
#luigi#        $ENV{CURL_DIR}/lib
#luigi#        $ENV{CURLK_DIR}
#luigi#        $ENV{CURLDIR}/lib
#luigi#        $ENV{CURLDIR}
#luigi#        $ENV{CURL_ROOT}/lib
#luigi#        ~/Library/Frameworks
#luigi#        /Library/Frameworks
#luigi#        /usr/local/lib
#luigi#        /usr/lib
#luigi#        /sw/lib
#luigi#        /opt/local/lib
#luigi#        /opt/csw/lib
#luigi#        /opt/lib
#luigi#        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;CURL_ROOT]/lib
#luigi#        /usr/freeware/lib64
#luigi#)
#luigi#FIND_LIBRARY(CURL_LIBRARY
#luigi#	NAMES cmcurl libcurl
#luigi#        PATHS ${CURL_FIND_PATHS}
#luigi#)
#luigi#IF(MSVC)
#luigi#    IF( CURL_LIBRARY)
#luigi#        SET(CURL_LIBRARY "${CURL_LIBRARY};winmm.lib;ws2_32.lib")
#luigi#    ENDIF( CURL_LIBRARY)
#luigi#ENDIF(MSVC)
#luigi#
#luigi#FIND_LIBRARY(CURL_LIBRARY_DEBUG
#luigi#	NAMES cmcurld libcurld
#luigi#        PATHS ${CURL_FIND_PATHS}
#luigi#)
#luigi#
#luigi#IF( NOT CURL_LIBRARY_DEBUG)
#luigi#	IF(CURL_LIBRARY)
#luigi#    		SET(CURL_LIBRARY_DEBUG ${CURL_LIBRARY})
#luigi#     	ENDIF(CURL_LIBRARY)
#luigi#		ELSE( NOT CURL_LIBRARY_DEBUG)
#luigi#
#luigi#	IF(MSVC)
#luigi#        	SET(CURL_LIBRARY_DEBUG "${CURL_LIBRARY};winmm.lib;ws2_32.lib")
#luigi#	ENDIF(MSVC)
#luigi#ENDIF( NOT CURL_LIBRARY_DEBUG)
#luigi#    
#luigi#    
#luigi#MARK_AS_ADVANCED( CURL_LIBRARY_DEBUG CURL_LIBRARY CURL_INCLUDE_DIR )
#luigi#
#luigi#SET(CURL_FOUND "NO")
#luigi#IF(CURL_INCLUDE_DIR AND CURL_LIBRARY)
#luigi#    SET(CURL_FOUND "YES")
#luigi#ENDIF(CURL_INCLUDE_DIR AND CURL_LIBRARY)



