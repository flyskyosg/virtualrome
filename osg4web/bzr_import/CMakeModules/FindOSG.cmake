# Locate gdal
# This module defines
# OSG_LIBRARY
# OSG_FOUND, if false, do not try to link to gdal 
# OSG_INCLUDE_DIR, where to find the headers
#
# $OSG_DIR is an environment variable that would
# correspond to the ./configure --prefix=$OSG_DIR
#
# Created by Robert Osfield. 

FIND_PATH(OSG_INCLUDE_DIR osg/Node
    ${OSG_DIR}/include
    $ENV{OSG_DIR}/include
    $ENV{OSG_DIR}
    $ENV{OSGDIR}/include
    $ENV{OSGDIR}
    $ENV{OSG_ROOT}/include
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/include
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/include
    /usr/freeware/include
)

MACRO(FIND_OSG_LIBRARY MYLIBRARY MYLIBRARYNAME)

    FIND_LIBRARY("${MYLIBRARY}_DEBUG"
        NAMES "${MYLIBRARYNAME}d"
        PATHS
        ${OSG_DIR}/lib/Debug
        ${OSG_DIR}/lib
        $ENV{OSG_DIR}/lib/debug
        $ENV{OSG_DIR}/lib
        $ENV{OSG_DIR}
        $ENV{OSGDIR}/lib
        $ENV{OSGDIR}
        $ENV{OSG_ROOT}/lib
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/lib
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
        /usr/freeware/lib64
    )

    FIND_LIBRARY(${MYLIBRARY}
        NAMES ${MYLIBRARYNAME}
        PATHS
        ${OSG_DIR}/lib/Release
        ${OSG_DIR}/lib
        $ENV{OSG_DIR}/lib/Release
        $ENV{OSG_DIR}/lib
        $ENV{OSG_DIR}
        $ENV{OSGDIR}/lib
        $ENV{OSGDIR}
        $ENV{OSG_ROOT}/lib
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/lib
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
        /usr/freeware/lib64
    )
#    MESSAGE("-->${${MYLIBRARY}}<-->${${MYLIBRARY}_DEBUG}<--")
    IF( NOT ${MYLIBRARY}_DEBUG)
    	IF(${MYLIBRARY})
    		SET(${MYLIBRARY}_DEBUG ${${MYLIBRARY}})
     	ENDIF(${MYLIBRARY})
    ENDIF( NOT ${MYLIBRARY}_DEBUG)
    IF( NOT ${MYLIBRARY})
#    MESSAGE("d1-->${${MYLIBRARY}}<-->${${MYLIBRARY}_DEBUG}<--")
    	IF(${MYLIBRARY}_DEBUG)
#    MESSAGE("d2-->${${MYLIBRARY}}<-->${${MYLIBRARY}_DEBUG}<--")
    		SET(${MYLIBRARY} ${${MYLIBRARY}_DEBUG})
     	ENDIF(${MYLIBRARY}_DEBUG)
    ENDIF( NOT ${MYLIBRARY})
#    MESSAGE("d3-->${${MYLIBRARY}}<-->${${MYLIBRARY}_DEBUG}<--")
   		
ENDMACRO(FIND_OSG_LIBRARY LIBRARY LIBRARYNAME)

FIND_OSG_LIBRARY(OSG_LIBRARY osg)
FIND_OSG_LIBRARY(OSGUTIL_LIBRARY osgUtil)
FIND_OSG_LIBRARY(OSGDB_LIBRARY osgDB)
FIND_OSG_LIBRARY(OSGGA_LIBRARY osgGA)
FIND_OSG_LIBRARY(OSGTEXT_LIBRARY osgText)
FIND_OSG_LIBRARY(OSGTERRAIN_LIBRARY osgTerrain)
FIND_OSG_LIBRARY(OSGFX_LIBRARY osgFX)
FIND_OSG_LIBRARY(OSGSIM_LIBRARY osgSim)
FIND_OSG_LIBRARY(OSGMANIPULATOR_LIBRARY osgManipulator)
FIND_OSG_LIBRARY(OSGVIEWER_LIBRARY osgViewer)
FIND_OSG_LIBRARY(OPENTHREADS_LIBRARY OpenThreads)

#GET_FILENAME_COMPONENT(tmp ${OSG_LIBRARY} PATH)
#GET_FILENAME_COMPONENT(tmp ${tmp} PATH)

#IF(EXISTS ${tmp}/bin
FIND_PATH(OSG_BIN_DIR 
		NAMES 
#		"osg${CMAKE_SHARED_LIBRARY_SUFFIX}"
#		"osgd${CMAKE_SHARED_LIBRARY_SUFFIX}"
		"osgVersion${CMAKE_EXECUTABLE_SUFFIX}" 
		"osgVersiond${CMAKE_EXECUTABLE_SUFFIX}" 
		PATHS
    ${OSG_DIR}/bin
    $ENV{OSG_DIR}/bin
    $ENV{OSG_DIR}
    $ENV{OSGDIR}/bin
    $ENV{OSGDIR}
    $ENV{OSG_ROOT}/bin
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/bin
    /usr/bin
    /sw/bin # Fink
    /opt/local/bin # DarwinPorts
    /opt/csw/bin # Blastwave
    /opt/bin
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/bin
    /usr/freeware/bin
)
SET(OSG_FOUND "NO")
IF(OSG_LIBRARY AND OSG_INCLUDE_DIR AND OSG_BIN_DIR)
    SET(OSG_FOUND "YES")
ENDIF(OSG_LIBRARY AND OSG_INCLUDE_DIR AND OSG_BIN_DIR)
