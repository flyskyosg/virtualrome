############################################################
#
# LoadCore Sub Project
#
# Dinamic Core library generation
#
############################################################

PROJECT(LoadCore)


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
SET(IS_LOAD_CORE TRUE)
SET("${PROJECT_NAME}_src" ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET("${PROJECT_NAME}_include_dir" ${CMAKE_CURRENT_SOURCE_DIR}/include)
SET("${PROJECT_NAME}_include" ${CMAKE_CURRENT_SOURCE_DIR}/include/LoadCore)

#coping debug files
SET(OUTPUT_DIR $ENV{APPDATA}/osg4web/${PROJECT_NAME})
SET(FONT_DIR ${OSG4WEB_ROOT}/var/stuffs/fonts)
SET(MODEL_DIR ${OSG4WEB_ROOT}/var/stuffs/models)
SET(DATA_SRCS ${FONT_DIR}/arial.ttf ${MODEL_DIR}/osg4web_world.ive)


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

###########################################################
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
#SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})


############################################################
#
# Coping Target Result in Debug Directory
#
############################################################

SETUP_CORE_RUNTIME(Engine ${PROJECT_NAME})


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




#IF(FIREFOX_PLUGIN_DIR)
#	SET(OUTPUT_LIBDIR ${FIREFOX_PLUGIN_DIR})
#	MAKE_DIRECTORY(${OUTPUT_LIBDIR})

###################   data copying ###################
#	SET(DUMMY_SHA1 "${PROJECT_NAME}")
#	SET(OUTPUT_Debug_DIR $ENV{APPDATA}/osg4web/${DUMMY_SHA1})

#	GET_TARGET_PROPERTY(${PROJECT_NAME}_Debug_LOCATION ${PROJECT_NAME} DEBUG_LOCATION)
#	GET_TARGET_PROPERTY(${PROJECT_NAME}_Release_LOCATION ${PROJECT_NAME} RELEASE_LOCATION)

#	FOREACH(BTYPE Debug Release)
#		GET_FILENAME_COMPONENT( MYNAME_${BTYPE} ${${PROJECT_NAME}_${BTYPE}_LOCATION} NAME)
#		ADD_CUSTOM_COMMAND(
#			OUTPUT ${OUTPUT_LIBDIR}/${MYNAME_${BTYPE}}
#    			COMMAND ${CMAKE_COMMAND} -E copy ${${PROJECT_NAME}_${BTYPE}_LOCATION} ${OUTPUT_LIBDIR}
#    			DEPENDS ${PROJECT_NAME}
#    		)
#
#		ADD_CUSTOM_TARGET(Engine_${PROJECT_NAME}_Prepare_${BTYPE}
#			DEPENDS ${OUTPUT_LIBDIR}/${MYNAME_${BTYPE}} ${DATA_TARGET_FILES}
#			)
#
#   		SET(FIREFOX_STUFF_LIST_${BTYPE} "${FIREFOX_STUFF_LIST_${BTYPE}} ${OUTPUT_LIBDIR}/${MYNAME_${BTYPE}}") 
#	ENDFOREACH(BTYPE Debug Release)
#ENDIF(FIREFOX_PLUGIN_DIR)



