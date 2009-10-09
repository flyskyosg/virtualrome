############################################################
#
# Utilities Sub Project
#
# Static library generation
#
############################################################

PROJECT(Utilities)


############################################################
#
# Adding Definition
#
############################################################

ADD_DEFINITIONS(-DOSG4WEB_UTILITIES_LIBRARY_STATIC)



############################################################
#
# Find Packages
#
############################################################

FIND_PACKAGE(JPEG)


############################################################
#
# Grabbing src and include files
#
############################################################

FOREACH(myvar include src)
	SET("${PROJECT_NAME}_${myvar}" ${OSG4WEB_ROOT}/${myvar}/${PROJECT_NAME})
	GRAB_FILES("${PROJECT_NAME}_${myvar}")
ENDFOREACH(myvar)


###########################################################
#
# Adding ext libraries include directories
#
############################################################

INCLUDE_DIRECTORIES( ${JPEG_INCLUDE_DIR} )


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
# Adding Library Dependancies
#
############################################################

TARGET_LINK_LIBRARIES(${PROJECT_NAME} "${JPEG_LIBRARY}" )


############################################################
#
# Changing target name
#
############################################################

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES PROJECT_LABEL "Library_${PROJECT_NAME}")




