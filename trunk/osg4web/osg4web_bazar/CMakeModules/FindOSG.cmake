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

FIND_PROGRAM(OSG_VERSION_APP
				NAMES osgversion osgversiond
        PATHS
        ${OSG_DIR}/bin
        $ENV{OSG_DIR}/bin
        $ENV{OSG_DIR}
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
IF(OSG_VERSION_APP)
	#define binary folder
	GET_FILENAME_COMPONENT(OSG_BINARY_DIR ${OSG_VERSION_APP} PATH)

	#get version numbers
	EXEC_PROGRAM(${OSG_VERSION_APP} ARGS  --version-number OUTPUT_VARIABLE OSG_VERSION_NUMBER)
	EXEC_PROGRAM(${OSG_VERSION_APP} ARGS  --so-number OUTPUT_VARIABLE OSG_SO_NUMBER) 
  EXEC_PROGRAM(${OSG_VERSION_APP} ARGS  --openthreads-soversion-number OUTPUT_VARIABLE OSG_OT_SO_NUMBER)
	# Automatically detected build options
	EXEC_PROGRAM(${OSG_VERSION_APP} ARGS Matrix::value_type OUTPUT_VARIABLE OSG_USE_FLOAT_MATRIX)
	IF(OSG_USE_FLOAT_MATRIX MATCHES "float")
    LIST(APPEND OSG_DEFINITIONS -DOSG_USE_FLOAT_MATRIX)
	ENDIF(OSG_USE_FLOAT_MATRIX MATCHES "float")

	EXEC_PROGRAM(${OSG_VERSION_APP} ARGS Plane::value_type OUTPUT_VARIABLE OSG_USE_FLOAT_PLANE)
	IF(OSG_USE_FLOAT_PLANE MATCHES "float")
    LIST(APPEND OSG_DEFINITIONS -DOSG_USE_FLOAT_PLANE)
	ENDIF(OSG_USE_FLOAT_PLANE MATCHES "float")

	EXEC_PROGRAM(${OSG_VERSION_APP} ARGS BoundingSphere::value_type  OUTPUT_VARIABLE OSG_USE_FLOAT_BOUNDINGSPHERE)
	IF(OSG_USE_FLOAT_BOUNDINGSPHERE MATCHES "double")
    LIST(APPEND OSG_DEFINITIONS -DOSG_USE_DOUBLE_BOUNDINGSPHERE)
	ENDIF(OSG_USE_FLOAT_BOUNDINGSPHERE MATCHES "double")

	EXEC_PROGRAM(${OSG_VERSION_APP} ARGS BoundingBox::value_type OUTPUT_VARIABLE OSG_USE_FLOAT_BOUNDINGBOX)
	IF(OSG_USE_FLOAT_BOUNDINGBOX MATCHES "double")
    LIST(APPEND OSG_DEFINITIONS -DOSG_USE_DOUBLE_BOUNDINGBOX)
	ENDIF(OSG_USE_FLOAT_BOUNDINGBOX MATCHES "double")

	MACRO(OSG_FIND_MODULE MYVAR MYNAME)
		IF(MYNAME MATCHES OpenThreads)
			SET(MYFILE "${OSG_BINARY_DIR}/ot${OSG_OT_SO_NUMBER}-${MYNAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
		ELSE(MYNAME MATCHES OpenThreads)
			SET(MYFILE "${OSG_BINARY_DIR}/osg${OSG_SO_NUMBER}-${MYNAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
		ENDIF(MYNAME MATCHES OpenThreads)
#		message("lloking for -->${MYFILE}<--")
		IF(EXISTS ${MYFILE})
#			message("found for -->${MYFILE}<--")
			SET(${MYVAR} ${MYFILE})
		ELSE(EXISTS ${MYFILE})
			SET(${MYVAR} "")
		ENDIF(EXISTS ${MYFILE})			
	ENDMACRO(OSG_FIND_MODULE MYVAR MYNAME)

	MACRO(OSG_FIND_PLUGIN MYVAR MYNAME)
		SET(MYFILE "${OSG_BINARY_DIR}/osgPlugins-${OSG_VERSION_NUMBER}/osgdb_${MYNAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
#		message("lloking for -->${MYFILE}<--")
		IF(EXISTS ${MYFILE})
#			message("found for -->${MYFILE}<--")
			SET(${MYVAR} ${MYFILE})
		ELSE(EXISTS ${MYFILE})
			SET(${MYVAR} "")
		ENDIF(EXISTS ${MYFILE})			
	ENDMACRO(OSG_FIND_PLUGIN MYVAR MYNAME)

	MACRO(OSG_COPY_MODULE_CUSTOM_COMMAND MYNAMES MYDIR MYDEP)
		SET(mod_list "")
		foreach(MYNAME ${MYNAMES})
			OSG_FIND_MODULE(mytmp ${MYNAME})
			if(MYNAME)
				GET_FILENAME_COMPONENT(myfile ${mytmp} NAME)
    		LIST(APPEND ${MYDEP} "${MYDIR}/${myfile}")
    		add_custom_command(
    			OUTPUT "${MYDIR}/${myfile}"
    			COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${mytmp} "${MYDIR}/${myfile}"
    			COMMAND ${CMAKE_COMMAND} ARGS -E remove "${MYDIR}/${myfile}.rule"
    		)
			endif(MYNAME)
		endforeach(MYNAME)
    #SET(${MYDEP} ${mod_list}) 			
	ENDMACRO(OSG_COPY_MODULE_CUSTOM_COMMAND MYNAMES MYDIR MYDEP)

	MACRO(OSG_COPY_PLUGIN_CUSTOM_COMMAND MYNAMES MYDIR MYDEP)
		SET(mod_list "")
		foreach(MYNAME ${MYNAMES})
			OSG_FIND_PLUGIN(mytmp ${MYNAME})
			if(MYNAME)
				GET_FILENAME_COMPONENT(mytmp1 ${mytmp} NAME)
				GET_FILENAME_COMPONENT(mytmp2 ${mytmp} PATH)
				GET_FILENAME_COMPONENT(mytmp2 ${mytmp2} NAME)
    		LIST(APPEND ${MYDEP} "${MYDIR}/${mytmp2}/${mytmp1}")
    		add_custom_command(
    			OUTPUT "${MYDIR}/${mytmp2}/${mytmp1}"
    			COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${mytmp} "${MYDIR}/${mytmp2}/${mytmp1}"
    			#COMMAND ${CMAKE_COMMAND} ARGS -E remove "${MYDIR}/${mytmp2}/${mytmp1}.rule"
    		)
			endif(MYNAME)
		endforeach(MYNAME)
    #SET(${MYDEP} ${mod_list}) 			
	ENDMACRO(OSG_COPY_PLUGIN_CUSTOM_COMMAND MYNAMES MYDIR MYDEP)
ENDIF(OSG_VERSION_APP)

	
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



MARK_AS_ADVANCED(OSG_BINARY_DIR OSG_INCLUDE_DIR)

SET(OSG_FOUND "NO")
IF(OSG_LIBRARY AND OSG_INCLUDE_DIR AND OSG_BINARY_DIR)
    SET(OSG_FOUND "YES")
ENDIF(OSG_LIBRARY AND OSG_INCLUDE_DIR AND OSG_BINARY_DIR)
