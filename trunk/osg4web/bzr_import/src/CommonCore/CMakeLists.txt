############################################################
#
# CommonCore Sub Project
#
# Static library generation
#
############################################################

PROJECT(CommonCore)


############################################################
#
# Adding Definition
#
############################################################

#ADD_DEFINITIONS(-DOSG4WEB_UTILITIES_LIBRARY_STATIC)


############################################################
#
# Setting variables
#
############################################################

SET(DIRS Visitors Manipulators SceneHandlers)
SET("${PROJECT_NAME}_include_dir" ${CMAKE_CURRENT_SOURCE_DIR}/include)


############################################################
#
# Grabbing src and include files
#
############################################################

FOREACH(myvar include src)
	SET("${PROJECT_NAME}_${myvar}" ${OSG4WEB_ROOT}/${myvar}/${PROJECT_NAME})
	GRAB_FILES_FOLDERS("${PROJECT_NAME}_${myvar}" "${DIRS}")
ENDFOREACH(myvar)


############################################################
#
# Adding grabbed files to project
#
############################################################

ADD_LIBRARY(${PROJECT_NAME}
    STATIC
    ${${PROJECT_NAME}_include_FILES}
    ${${PROJECT_NAME}_src_FILES}
)


############################################################
#
# Adding ext libraries include directories
#
############################################################

INCLUDE_DIRECTORIES( ${OSG_INCLUDE_DIR} ${${PROJECT_NAME}_include_dir} )


############################################################
#
# Linking to ext libraries
#
############################################################

LINK_WITH_VARIABLES(${PROJECT_NAME} OSG_LIBRARY OSGUTIL_LIBRARY OSGDB_LIBRARY OSGGA_LIBRARY OSGFX_LIBRARY OSGTEXT_LIBRARY OSGMANIPULATOR_LIBRARY OSGVIEWER_LIBRARY OPENTHREADS_LIBRARY)


############################################################
#
# Changing target name
#
############################################################

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES PROJECT_LABEL "Library_${PROJECT_NAME}")


