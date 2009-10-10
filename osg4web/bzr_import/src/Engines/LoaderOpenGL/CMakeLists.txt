############################################################
#
# Loader OpenGL Sub Project
#
# Dinamic Core library generation
#
############################################################

PROJECT(LoaderOpenGL)


############################################################
#
# Adding Definition
#
############################################################

ADD_DEFINITIONS(-DOSG4WEBCORE_LIBRARY)


############################################################
#
# Setting variables
#
############################################################

SET(CMAKE_DEBUG_POSTFIX  "d")
SET("${PROJECT_NAME}_src" ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET("${PROJECT_NAME}_include_dir" ${CMAKE_CURRENT_SOURCE_DIR}/include)
SET("${PROJECT_NAME}_include" ${CMAKE_CURRENT_SOURCE_DIR}/include/LoaderOpenGL)

IF(WIN32)
    	SET(COREOPENGL_ADD_SRCS ${OSG4WEB_ROOT}/src/Engines/CoreOpenGL/src/WindowWin32.cpp)
ELSE(WIN32)
	#TODO: non windows
ENDIF(WIN32)


#coping debug files
SET(OUTPUT_DIR $ENV{APPDATA}/osg4web/${PROJECT_NAME})
#SET(FONT_DIR ${OSG4WEB_ROOT}/var/stuffs/fonts)
#SET(MODEL_DIR ${OSG4WEB_ROOT}/var/stuffs/models)
#SET(DATA_SRCS ${FONT_DIR}/arial.ttf ${MODEL_DIR}/osg4web_world.ive) #TODO


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
    	${OSG4WEB_ROOT}/src/Engines/CoreOpenGL/src/CoreOpenGL.cpp
	${COREOPENGL_ADD_SRCS}
)

###########################################################
#
# Adding ext libraries include directories
#
############################################################

INCLUDE_DIRECTORIES( ${${PROJECT_NAME}_include_dir} ${OSG4WEB_ROOT}/src/Engines/CoreOpenGL/include  ${OSG4WEB_ROOT}/include )


############################################################
#
# Linking to ext libraries
#
############################################################


LINK_WITH_VARIABLES(${PROJECT_NAME} )


############################################################
#
# Adding Library Dependancies
#
############################################################

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${OPENGL_LIBRARIES})


############################################################
#
# Setting target properties
#
############################################################

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES PROJECT_LABEL "Engine_${PROJECT_NAME}")


############################################################
#
# Coping Target Result in Debug Directory
#
############################################################

SETUP_LOADER_RUNTIME(${PROJECT_NAME})


############################################################
#
# Coping extra files in debug directory
#
############################################################

IF(FIREFOX_PLUGIN_DIR)
	MAKE_DIRECTORY(${FIREFOX_PLUGIN_DIR})
	FOREACH( DATA_SINGLE_SRC ${DATA_SRCS} )
		COPY_EXTRA_FILES(${FIREFOX_PLUGIN_DIR} ${DATA_SINGLE_SRC})
	ENDFOREACH( DATA_SINGLE_SRC )
ENDIF(FIREFOX_PLUGIN_DIR)


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


