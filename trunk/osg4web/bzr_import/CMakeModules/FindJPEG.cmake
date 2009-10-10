# Locate Lib cURL
# This module defines
# UNRAR_LIBRARY
# UNRAR_FOUND, if false, do not try to link to Gecko SDK 
# UNRAR_INCLUDE_DIR, where to find the headers
#
# $UNRAR_DIR is an environment variable that would
# correspond to the ./configure --prefix=$UNRAR_DIR


FIND_PATH(JPEG_INCLUDE_DIR jpeglib.h	
	${JPEG_ROOT}/include
	$ENV{JPEG_DIR}/include
	$ENV{JPEG_DIR}
	$ENV{JPEGDIR}/include
	$ENVJPEG_DIR}
	$ENV{JPEG_ROOT}/include
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local/include
	/usr/include
	/sw/include # Fink
	/opt/local/include # DarwinPorts
	/opt/csw/include # Blastwave
	/opt/include
	[HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;JPEG_ROOT]/include
	/usr/freeware/include
)

MACRO(FIND_JPEG_LIBRARY MYLIBRARY MYLIBRARYNAME)

	FIND_LIBRARY( "${MYLIBRARY}_DEBUG"
        	NAMES "${MYLIBRARYNAME}d"
        	PATHS
		${JPEG_ROOT}/lib
        	$ENV{JPEG_DIR}/lib
        	$ENV{JPEG_DIR}
        	$ENV{JPEGDIR}/lib
        	$ENV{JPEGDIR}
        	$ENV{JPEG_ROOT}/lib
        	~/Library/Frameworks
        	/Library/Frameworks
		/usr/local/lib
		/usr/lib
		/sw/lib
		/opt/local/lib
		/opt/csw/lib
		/opt/lib
		[HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;JPEG_ROOT]/lib
		/usr/freeware/lib64
	)

	FIND_LIBRARY( ${MYLIBRARY}
        	NAMES ${MYLIBRARYNAME}
        	PATHS
		${JPEG_ROOT}/lib
        	$ENV{JPEG_DIR}/lib
        	$ENV{JPEG_DIR}
        	$ENV{JPEGDIR}/lib
        	$ENV{JPEGDIR}
        	$ENV{JPEG_ROOT}/lib
        	~/Library/Frameworks
        	/Library/Frameworks
		/usr/local/lib
		/usr/lib
		/sw/lib
		/opt/local/lib
		/opt/csw/lib
		/opt/lib
		[HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;JPEG_ROOT]/lib
		/usr/freeware/lib64
	)

	IF( NOT ${MYLIBRARY}_DEBUG)
		IF(${MYLIBRARY})
			SET(${MYLIBRARY}_DEBUG ${${MYLIBRARY}})
		ENDIF(${MYLIBRARY})
	ENDIF( NOT ${MYLIBRARY}_DEBUG)
	IF( NOT ${MYLIBRARY})
		IF(${MYLIBRARY}_DEBUG)
			SET(${MYLIBRARY} ${${MYLIBRARY}_DEBUG})
		ENDIF(${MYLIBRARY}_DEBUG)
	ENDIF( NOT ${MYLIBRARY})

	MARK_AS_ADVANCED(${MYLIBRARY} "${MYLIBRARY}_DEBUG")
   		
ENDMACRO(FIND_JPEG_LIBRARY LIBRARY LIBRARYNAME)

FIND_JPEG_LIBRARY(JPEG_LIBRARY libjpeg)

MARK_AS_ADVANCED(JPEG_INCLUDE_DIR JPEG_LIBRARY)

SET(JPEG_FOUND "NO")

IF(JPEG_INCLUDE_DIR AND JPEG_LIBRARY)
    SET(JPEG_FOUND "YES")
ENDIF(JPEG_INCLUDE_DIR AND JPEG_LIBRARY)

