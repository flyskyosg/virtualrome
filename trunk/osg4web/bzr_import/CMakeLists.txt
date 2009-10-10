IF(WIN32)
    CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6 FATAL_ERROR)
ELSE(WIN32)
    IF(APPLE)
        CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6 FATAL_ERROR)
        IF(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4 AND ${CMAKE_PATCH_VERSION} LESS 7)
            MESSAGE("Warning: A critical CMake bug exists in 2.4.6 and below. Trying to build Universal Binaries will result in a compile error that seems unrelated. Either avoid building Universal Binaries by changing the CMAKE_OSX_ARCHITECTURES field to list only your architecture, or upgrade to the current CVS version of CMake or a newer stable version if it exists.")
        ENDIF(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4 AND ${CMAKE_PATCH_VERSION} LESS 7)
    ELSE(APPLE)
        CMAKE_MINIMUM_REQUIRED(VERSION 2.4.0 FATAL_ERROR)
    ENDIF(APPLE)
ENDIF(WIN32)



PROJECT(OSG4Web)


# We have some custom .cmake scripts not in the official distribution.
# Maybe this can be used override existing behavior if needed?
SET(USER_DEFINED_MODULE_PATH "" CACHE PATH "Location of additional configure_time modules folder, to be able to change macros at configure timme")
MARK_AS_ADVANCED(USER_DEFINED_MODULE_PATH)
IF(USER_DEFINED_MODULE_PATH)
	SET(CMAKE_MODULE_PATH "${USER_DEFINED_MODULE_PATH};${PROJECT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")
ELSE(USER_DEFINED_MODULE_PATH)
	SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")
ENDIF(USER_DEFINED_MODULE_PATH)

SET(OSG4WEB_ROOT ${PROJECT_SOURCE_DIR})
INCLUDE(GrabFiles)

INCLUDE_DIRECTORIES(
  ${OSG4WEB_ROOT}/include
)


SET(FIREFOX_DIR "" CACHE PATH "set to Firefox install path")
FIND_PROGRAM(FIREFOX_EXE Firefox ${FIREFOX_DIR})
IF(FIREFOX_EXE)
	GET_FILENAME_COMPONENT(FIREFOX_DIR ${FIREFOX_EXE} PATH)
	SET(FIREFOX_PLUGIN_DIR ${FIREFOX_DIR}/plugins)
	ENABLE_TESTING()

ENDIF(FIREFOX_EXE)

# Common to all platforms:
SET(OSG_DIR "" CACHE PATH "set to base osg install path")
FIND_PACKAGE(OSG)

FOREACH(BTYPE Debug Release)
#	 SET(FIREFOX_STUFF_LIST_${BTYPE} "this_value_should_not_appear" CACHE INTERNAL "should accumulate values from subdirs") 
	 SET(FIREFOX_STUFF_LIST_${BTYPE} "") 
ENDFOREACH(BTYPE Debug Release)


ADD_SUBDIRECTORY(src/Utilities)
ADD_SUBDIRECTORY(src/CommonCore)
ADD_SUBDIRECTORY(src/Engines)
ADD_SUBDIRECTORY(src/Examples)
ADD_SUBDIRECTORY(src/npOSG4Web)

IF(WIN32)
ADD_SUBDIRECTORY(src/ieOSG4Web)
ENDIF(WIN32)



FOREACH(varname FIREFOX_STUFF_LIST_Debug FIREFOX_STUFF_LIST_Release)
	GET_DIRECTORY_PROPERTY(${varname} DIRECTORY src/npOSG4Web DEFINITION ${varname})
ENDFOREACH(varname FIREFOX_STUFF_LIST_Debug FIREFOX_STUFF_LIST_Release)

IF(OSG_FOUND)
#	ADD_SUBDIRECTORY(src/Engines/LoadCore/src)
#	ADD_SUBDIRECTORY(src/Engines/CoreBase/src)
#	FOREACH(varname FIREFOX_STUFF_LIST_Debug FIREFOX_STUFF_LIST_Release)
#		GET_DIRECTORY_PROPERTY(${varname} DIRECTORY src/Engines/LoadCore/src DEFINITION ${varname})
#		GET_DIRECTORY_PROPERTY(${varname} DIRECTORY src/Engines/CoreBase/src DEFINITION ${varname})
#	ENDFOREACH(varname FIREFOX_STUFF_LIST_Debug FIREFOX_STUFF_LIST_Release)
ELSE(OSG_FOUND)
	MESSAGE("OSG NOT FOUND Please specify OSG_DIR")
ENDIF(OSG_FOUND)


IF(FIREFOX_EXE AND OSG_FOUND)
	FOREACH(BTYPE Debug Release)  #TODO Release
		ADD_CUSTOM_TARGET(Shell_npOSG4Web_Prepare_${BTYPE} )
		ADD_DEPENDENCIES(Shell_npOSG4Web_Prepare_${BTYPE} Engine_LoadCore_Prepare_${BTYPE})	
		ADD_DEPENDENCIES(Shell_npOSG4Web_Prepare_${BTYPE} Shell_npOSG4Web_Prepare_XPT)	

		ADD_TEST(build_${BTYPE} ${CMAKE_CTEST_COMMAND}
			--build-and-test
			"${PROJECT_SOURCE_DIR}"
			"${PROJECT_BINARY_DIR}"
#			--build-two-config
			--build-nocmake
			--build-generator ${CMAKE_GENERATOR}
			--build-makeprogram ${CMAKE_MAKE_PROGRAM}
			--build-project ${PROJECT_NAME}
			--build-target Shell_npOSG4Web_Prepare_${BTYPE}
  			)
	ENDFOREACH(BTYPE)
	
	CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/run_debug.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/run_debug.cmake @ONLY)
	ADD_TEST(open_project_in_debug ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/run_debug.cmake )
ENDIF(FIREFOX_EXE AND OSG_FOUND)

