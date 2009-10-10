############################################################
#
# FunCore Sub Project
#
# Dinamic Core library generation
#
############################################################

PROJECT(FunCore)


############################################################
#
# Adding Definition
#
############################################################

ADD_DEFINITIONS(-DOSG4WEBCORE_LIBRARY)

IF(DEBUG_INFO_IN_FILE)
	ADD_DEFINITIONS(-DOSG4WEB_DEBUG_INFO_IN_FILE)
ENDIF(DEBUG_INFO_IN_FILE)


############################################################
#
# Setting variables
#
############################################################

#SET(${PROJECT_NAME}_BIN_PATH "${PROJECT_NAME}")
SET(CMAKE_DEBUG_POSTFIX  "d")
SET("${PROJECT_NAME}_src" ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET("${PROJECT_NAME}_include_dir" ${CMAKE_CURRENT_SOURCE_DIR}/include)
SET("${PROJECT_NAME}_include" ${CMAKE_CURRENT_SOURCE_DIR}/include/FunCore)


############################################################
#
# Grabbing src and include files
#
############################################################

GRAB_FILES( "${PROJECT_NAME}_src" )
GRAB_FILES( "${PROJECT_NAME}_include" )


############################################################
#
# Adding grabbed files to project
#
############################################################

ADD_LIBRARY(${PROJECT_NAME}
	SHARED
	${${PROJECT_NAME}_include_FILES}
    	${${PROJECT_NAME}_src_FILES}
    	${OSG4WEB_ROOT}/src/Engines/CoreBase/src/CoreBase.cpp
)


############################################################
#
# Adding ext libraries include directories
#
############################################################

INCLUDE_DIRECTORIES( ${${PROJECT_NAME}_include_dir} ${OSG_INCLUDE_DIR} ${OSG4WEB_ROOT}/src/Engines/CoreBase/include  ${OSG4WEB_ROOT}/include )


############################################################
#
# Linking to ext libraries
#
############################################################

LINK_WITH_VARIABLES(${PROJECT_NAME} OSG_LIBRARY OSGUTIL_LIBRARY OSGDB_LIBRARY OSGGA_LIBRARY OSGFX_LIBRARY OSGTEXT_LIBRARY OSGVIEWER_LIBRARY OPENTHREADS_LIBRARY)


############################################################
#
# Adding Library Dependancies
#
############################################################

TARGET_LINK_LIBRARIES(${PROJECT_NAME} CommonCore)


############################################################
#
# Setting target properties
#
############################################################

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES PROJECT_LABEL "Engine_${PROJECT_NAME}")
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})


############################################################
#
# Coping Target Result in Debug Directory
#
############################################################

SETUP_CORE_RUNTIME(Engine ${PROJECT_NAME})


############################################################
#
# Configuring HTML Tests
#
############################################################

CONFIGURE_HTML_TEST(${CMAKE_CURRENT_SOURCE_DIR}/html)


############################################################
#
# Include ModuleInsall #TODO: forse da togliere...
#
############################################################
INCLUDE(ModuleInstall OPTIONAL)


