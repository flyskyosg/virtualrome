############################################################
#
# OSG4Web Project
#
# Main CMakeFile
#
############################################################

PROJECT(OSG4Web)


############################################################
#
# Minimun CMake required Version
#
############################################################

IF(WIN32)
    CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0 FATAL_ERROR)
ELSE(WIN32)
    IF(APPLE)
        CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0 FATAL_ERROR)
        IF(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 6 AND ${CMAKE_PATCH_VERSION} LESS 0)
            MESSAGE("Warning: A critical CMake bug exists in 2.6.0 and below. Trying to build Universal Binaries will result in a compile error that seems unrelated. Either avoid building Universal Binaries by changing the CMAKE_OSX_ARCHITECTURES field to list only your architecture, or upgrade to the current CVS version of CMake or a newer stable version if it exists.")
        ENDIF(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 6 AND ${CMAKE_PATCH_VERSION} LESS 0)
    ELSE(APPLE)
        CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0 FATAL_ERROR)
    ENDIF(APPLE)
ENDIF(WIN32)


############################################################
#
# Adding CMakeModules Directory
#
############################################################

SET(USER_DEFINED_MODULE_PATH "" CACHE PATH "Location of additional configure_time modules folder, to be able to change macros at configure timme")
MARK_AS_ADVANCED(USER_DEFINED_MODULE_PATH)
IF(USER_DEFINED_MODULE_PATH)
	SET(CMAKE_MODULE_PATH "${USER_DEFINED_MODULE_PATH};${PROJECT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")
ELSE(USER_DEFINED_MODULE_PATH)
	SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")
ENDIF(USER_DEFINED_MODULE_PATH)


############################################################
#
# Additional Options
#
############################################################

OPTION(PLACE_IN_PLUGIN "If on place output files into Firefox/plugin" OFF)
OPTION(DEBUG_INFO_IN_FILE "If on redirect debug output in file" OFF)
OPTION(DISABLE_SHA1_CHECK "If on disable package sh1 testing" OFF)


############################################################
#
# Specific Compiler Properties
#
############################################################

IF(MSVC)
	OPTION(MSVC_DEBUG_INCREMENTAL_LINK "Set to OFF to build OSG4Web without incremental linking in debug" OFF)
	MARK_AS_ADVANCED(MSVC_DEBUG_INCREMENTAL_LINK)
	IF(NOT MSVC_DEBUG_INCREMENTAL_LINK)
		SET(CMAKE_MODULE_LINKER_FLAGS_DEBUG "/debug /INCREMENTAL:NO")
		SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/debug /INCREMENTAL:NO")
		SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "/debug /INCREMENTAL:NO")
	ENDIF(NOT MSVC_DEBUG_INCREMENTAL_LINK)
ENDIF(MSVC)

IF(MSVC80)
	OPTION(MSVC_GENERATE_MANIFESTS "Set to OFF to build OSG4Web without Visual Studio 8 manifests (Don't turn off if you intend to compile ieOSG4Web)" ON)
	MARK_AS_ADVANCED(MSVC_GENERATE_MANIFESTS)
	IF(NOT MSVC_GENERATE_MANIFESTS)
	       	SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /MANIFEST:NO")
	ENDIF(NOT MSVC_GENERATE_MANIFESTS)
ENDIF(MSVC80)


############################################################
#
# Configuring VSStart from CTest
#
############################################################

IF(WIN32)
	CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/CMakeIn/run_msvc_debug.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/run_msvc_debug.cmake @ONLY)
	ADD_TEST(open_project_in_debug ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/run_msvc_debug.cmake )
ENDIF(WIN32)



############################################################
#
# Include Files
#
############################################################

INCLUDE(GrabFiles)
INCLUDE(SetupCoreRuntime)
INCLUDE(FindOpenGL)


############################################################
#
# Find Packages
#
############################################################

FIND_PACKAGE(OSG)
FIND_PACKAGE(CURL QUIET)
FIND_PACKAGE(OpenGL)


############################################################
#
# Sets variables
#
############################################################

SET(OSG4WEB_ROOT ${PROJECT_SOURCE_DIR})
SET(FIREFOX_DIR "" CACHE PATH "set to Firefox install path")
SET(IEXPLORE_DIR "" CACHE PATH "set to Internet Explorer install path")
SET(OSG_DIR "" CACHE PATH "set to base osg install path")
SET(CURL_ROOT ${OSG4WEB_ROOT}/var/dependancies/cmcurl CACHE PATH "base dir for cURL Library")
SET(SOLUTION_TYPE Debug Release)

#FIXME: soluzione poco pulita
FOREACH(BTYPE ${SOLUTION_TYPE})
	 SET(FIREFOX_STUFF_LIST_${BTYPE} "")
	 SET(IEXPLORE_STUFF_LIST_${BTYPE} "") 
ENDFOREACH(BTYPE Debug Release)


############################################################
#
# Find Programs
#
############################################################

FIND_PROGRAM(FIREFOX_EXE Firefox ${FIREFOX_DIR})
FIND_PROGRAM(IEXPLORE_EXE iexplore ${IEXPLORE_DIR})

MARK_AS_ADVANCED(FIREFOX_EXE IEXPLORE_EXE)

############################################################
#
# Check 
#
############################################################

IF(FIREFOX_EXE)
	GET_FILENAME_COMPONENT(FIREFOX_DIR ${FIREFOX_EXE} PATH)
	SET(FIREFOX_PLUGIN_DIR ${FIREFOX_DIR}/plugins)
	ENABLE_TESTING()

	SET(FFOX_PROFILE_DIR ${FIREFOX_DIR}/osg4web_test_profile)
	
	FILE(TO_NATIVE_PATH ${FFOX_PROFILE_DIR} FFOX_PROFILE_DIR)

	IF(WIN32)
		EXEC_PROGRAM(taskkill ARGS "/im firefox.exe")
	ELSE(WIN32)
		#TODO: other OS
	ENDIF(WIN32)

	EXEC_PROGRAM(${FIREFOX_EXE} ARGS "-CreateProfile \"osg4web_test_profile ${FFOX_PROFILE_DIR}\"")
ENDIF(FIREFOX_EXE)

IF(IEXPLORE_EXE)
	GET_FILENAME_COMPONENT(IEXPLORE_DIR ${IEXPLORE_EXE} PATH)
	ENABLE_TESTING()
ENDIF(IEXPLORE_EXE)

IF(NOT CURL_FOUND)
	#try to search curl with custom module
	SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR};${CMAKE_MODULE_PATH}")
	FIND_PACKAGE(CURL)
ENDIF(NOT CURL_FOUND)


############################################################
#
# Adding Include Directories
#
############################################################

INCLUDE_DIRECTORIES(
  ${OSG4WEB_ROOT}/include
)


############################################################
#
# Adding Sub Directories
#
############################################################

ADD_SUBDIRECTORY(src/Utilities)

IF(OSG_FOUND)
	ADD_SUBDIRECTORY(src/CommonCore)
ELSE(OSG_FOUND)
	MESSAGE("OSG NOT FOUND: specify an OSG_DIR in order to compile OSG CommonCore Library")
ENDIF(OSG_FOUND)

ADD_SUBDIRECTORY(src/Engines)
ADD_SUBDIRECTORY(src/Examples)

IF(FIREFOX_PLUGIN_DIR)
	ADD_SUBDIRECTORY(src/npOSG4Web)

	IF(MSVC)
		ADD_SUBDIRECTORY(src/ieOSG4Web) # it needs firefox for npsr library
	ELSE(MSVC)
		IF(WIN32)
			MESSAGE("Internet Explorer NOT FOUND: specify an IE Directory in order to compile OSG4Web IE ActiveX")
		ENDIF(WIN32)
	ENDIF(MSVC)
ELSE(FIREFOX_PLUGIN_DIR)
	MESSAGE("Firefox NOT FOUND: specify a Firefox Directory in order to compile OSG4Web Firefox Plugin")
ENDIF(FIREFOX_PLUGIN_DIR)

MARK_AS_ADVANCED(EXECUTABLE_OUTPUT_PATH)


