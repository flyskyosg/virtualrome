# Locate Lib cURL
# This module defines
# UNRAR_LIBRARY
# UNRAR_FOUND, if false, do not try to link to Gecko SDK 
# UNRAR_INCLUDE_DIR, where to find the headers
#
# $UNRAR_DIR is an environment variable that would
# correspond to the ./configure --prefix=$UNRAR_DIR


FIND_PATH(UNRAR_INCLUDE_DIR unrarlib.h	
		${UNRAR_ROOT}/include
    $ENV{UNRAR_DIR}/include
    $ENV{UNRAR_DIR}
    $ENV{UNRARDIR}/include
    $ENVUNRAR_DIR}
    $ENV{UNRAR_ROOT}/include
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/include
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;UNRAR_ROOT]/include
    /usr/freeware/include
)

FIND_LIBRARY(UNRAR_LIBRARY
	NAMES unrar
        PATHS
				${UNRAR_ROOT}/lib
        $ENV{UNRAR_DIR}/lib
        $ENV{UNRARK_DIR}
        $ENV{UNRARDIR}/lib
        $ENV{UNRARDIR}
        $ENV{UNRAR_ROOT}/lib
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/lib
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;UNRAR_ROOT]/lib
        /usr/freeware/lib64
    )

MARK_AS_ADVANCED(UNRAR_INCLUDE_DIR UNRAR_LIBRARY)

SET(UNRAR_FOUND "NO")
IF(UNRAR_INCLUDE_DIR AND UNRAR_LIBRARY)
    SET(UNRAR_FOUND "YES")
ENDIF(UNRAR_INCLUDE_DIR AND UNRAR_LIBRARY)



