# Locate Lib cURL
# This module defines
# CURL_LIBRARY
# CURL_FOUND, if false, do not try to link to Gecko SDK 
# CURL_INCLUDE_DIR, where to find the headers
#
# $CURL_DIR is an environment variable that would
# correspond to the ./configure --prefix=$CURL_DIR


FIND_PATH(CURL_INCLUDE_DIR curl/curl.h	
		${CURL_ROOT}/include
    $ENV{CURL_DIR}/include
    $ENV{CURL_DIR}
    $ENV{CURLDIR}/include
    $ENVCURL_DIR}
    $ENV{CURL_ROOT}/include
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/include
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;CURL_ROOT]/include
    /usr/freeware/include
)

SET(CURL_FIND_PATHS 
				${CURL_ROOT}/lib
        $ENV{CURL_DIR}/lib
        $ENV{CURLK_DIR}
        $ENV{CURLDIR}/lib
        $ENV{CURLDIR}
        $ENV{CURL_ROOT}/lib
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/lib
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;CURL_ROOT]/lib
        /usr/freeware/lib64
)
FIND_LIBRARY(CURL_LIBRARY
	NAMES cmcurl libcurl
        PATHS ${CURL_FIND_PATHS}
)
IF(MSVC)
    IF( CURL_LIBRARY)
        SET(CURL_LIBRARY "${CURL_LIBRARY};winmm.lib;ws2_32.lib")
    ENDIF( CURL_LIBRARY)
ENDIF(MSVC)

FIND_LIBRARY(CURL_LIBRARY_DEBUG
	NAMES cmcurld libcurld
        PATHS ${CURL_FIND_PATHS}
)

IF( NOT CURL_LIBRARY_DEBUG)
	IF(CURL_LIBRARY)
    		SET(CURL_LIBRARY_DEBUG ${CURL_LIBRARY})
     	ENDIF(CURL_LIBRARY)
		ELSE( NOT CURL_LIBRARY_DEBUG)

	IF(MSVC)
        	SET(CURL_LIBRARY_DEBUG "${CURL_LIBRARY};winmm.lib;ws2_32.lib")
	ENDIF(MSVC)
ENDIF( NOT CURL_LIBRARY_DEBUG)
    
    
MARK_AS_ADVANCED( CURL_LIBRARY_DEBUG CURL_LIBRARY CURL_INCLUDE_DIR )

SET(CURL_FOUND "NO")
IF(CURL_INCLUDE_DIR AND CURL_LIBRARY)
    SET(CURL_FOUND "YES")
ENDIF(CURL_INCLUDE_DIR AND CURL_LIBRARY)



