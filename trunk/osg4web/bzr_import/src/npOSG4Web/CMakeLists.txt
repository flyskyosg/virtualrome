############################################################
#
# npOSG4Web Sub Project
#
# Firefox Plugin
#
############################################################

PROJECT(npOSG4Web)


############################################################
#
# Settaggio Variabili 
#
############################################################

# Getting gecko root
SET(GeckoSDK_ROOT ${OSG4WEB_ROOT}/var/dependancies/gecko-sdk CACHE PATH "base dir for Firefox Gecko SDK")

# Defining OSG4Web root if not presents
IF(NOT OSG4WEB_ROOT)
	GET_FILENAME_COMPONENT(OSG4WEB_ROOT "${PROJECT_SOURCE_DIR}/../.." ABSOLUTE CACHE)
ENDIF(NOT OSG4WEB_ROOT)

# Getting resources
IF(WIN32)
        SET(${PROJECT_NAME}_src_FILES ${${PROJECT_NAME}_src_FILES} ${${PROJECT_NAME}_res_FILES})
ENDIF(WIN32)

FOREACH(myvar include idl src res)
	SET("${PROJECT_NAME}_${myvar}" ${OSG4WEB_ROOT}/${myvar}/${PROJECT_NAME} CACHE PATH "${myvar} dir for ${PROJECT_NAME}")
	MARK_AS_ADVANCED("${PROJECT_NAME}_${myvar}")
ENDFOREACH(myvar)

FOREACH(myvar include src)
	SET("CommonShell_${myvar}" ${OSG4WEB_ROOT}/${myvar}/CommonShell)
ENDFOREACH(myvar)

# MSVC Flags
IF(MSVC)
#        SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT,MSVCRT,MSVCRTD")
#        SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT,MSVCRT,MSVCRTD")
	SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT,MSVCRT,MFCS,MSVCRTD,MFCSD")
	SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT,MSVCRT,MFCS,MSVCRTD,MFCSD")
	IF(MSVC71)
		SET(ADDITIONAL_LIBRARY "optimized;mfcs71.lib;debug;mfcs71d.lib")
	ELSE(MSVC71)
		IF(MSVC80)
			SET(ADDITIONAL_LIBRARY "optimized;mfcs80.lib;debug;mfcs80d.lib")
		ENDIF(MSVC80)
	ENDIF(MSVC71)

	SET(ADDITIONAL_LIBRARY "${ADDITIONAL_LIBRARY};optimized;msvcrt.lib;debug;msvcrtd.lib")
ENDIF(MSVC)

# Unrar root folder
SET(UNRAR_ROOT ${OSG4WEB_ROOT}/var/dependancies/unrarlib CACHE PATH "base dir for UnRAR Library")

#set up folder for generated include files
SET(${PROJECT_NAME}_GEN_INC_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)
SET(IDL_FILENAME npOSG4Web_nsIScriptablePlugin)


############################################################
#
# Making Sub-Directories
#
############################################################

FILE(MAKE_DIRECTORY ${${PROJECT_NAME}_GEN_INC_DIR}/${PROJECT_NAME})


############################################################
#
# Grabbing files
#
############################################################

FOREACH(myvar include idl src res)
	GRAB_FILES("${PROJECT_NAME}_${myvar}")
ENDFOREACH(myvar)

FOREACH(myvar include src)
	GRAB_FILES("CommonShell_${myvar}")
ENDFOREACH(myvar)


############################################################
#
# Find Packages
#
############################################################

FIND_PACKAGE(GeckoSDK QUIET)
IF(NOT GeckoSDK_FOUND)
	SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR};${CMAKE_MODULE_PATH}")
	FIND_PACKAGE(GeckoSDK)
ENDIF(NOT GeckoSDK_FOUND)

FIND_PACKAGE(UNRAR QUIET)
IF(NOT UNRAR_FOUND)
	SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR};${CMAKE_MODULE_PATH}")
	FIND_PACKAGE(UNRAR)
ENDIF(NOT UNRAR_FOUND)


############################################################
#
# Definizioni
#
############################################################

ADD_DEFINITIONS(-DOSG4WEB_UTILITIES_LIBRARY_STATIC)

IF(CURL_LIBRARY)
	ADD_DEFINITIONS(-DHAVE_CONFIG_H -DCURL_STATICLIB)
ELSE(CURL_LIBRARY)
	MESSAGE(FATAL_ERROR "Curl NOT FOUND")
ENDIF(CURL_LIBRARY)

IF(PLACE_IN_PLUGIN)
	ADD_DEFINITIONS(-DNPOSG4WEB_COREDIR_PLUGIN)
ENDIF(PLACE_IN_PLUGIN)

IF(DEBUG_INFO_IN_FILE)
	ADD_DEFINITIONS(-DOSG4WEB_DEBUG_INFO_IN_FILE)
ENDIF(DEBUG_INFO_IN_FILE)

IF(DISABLE_SHA1_CHECK)
	ADD_DEFINITIONS(-DOSG4WEB_DISABLE_SHA1_CHECK)
ENDIF(DISABLE_SHA1_CHECK)

IF(MSVC)
#originale, firefox 2	ADD_DEFINITIONS(-DMOZILLA_STRICT_API -DXPCOM_GLUE -DXP_WIN -D_X86_ -D_USRDLL -D_WIN32_WINNT=0x0400)
	ADD_DEFINITIONS(-DXP_WIN -D_X86_ -D_USRDLL -D_WIN32_WINNT=0x0400)
ENDIF(MSVC)


############################################################
#
# Directory Include
#
############################################################

INCLUDE_DIRECTORIES(
	${GeckoSDK_INCLUDE_DIR}
	${CURL_INCLUDE_DIR}
	${UNRAR_INCLUDE_DIR}
	${${PROJECT_NAME}_GEN_INC_DIR}
)


############################################################
#
# Impostazione sorgenti generati
#
############################################################

SET_SOURCE_FILES_PROPERTIES(${${PROJECT_NAME}_GEN_INC_DIR}/${PROJECT_NAME}/${IDL_FILENAME}.h PROPERTIES GENERATED ON)


############################################################
#
# Creazione libreria
#
############################################################

ADD_LIBRARY(
	${PROJECT_NAME}
	MODULE
	${CommonShell_include_FILES}
	${${PROJECT_NAME}_include_FILES}
	#${${PROJECT_NAME}_GEN_INC_DIR}/${IDL_FILENAME}.h
	${OSG4WEB_ROOT}/res/${PROJECT_NAME}/${PROJECT_NAME}_res.def
	${OSG4WEB_ROOT}/res/${PROJECT_NAME}/${PROJECT_NAME}_res.rc
	${CommonShell_src_FILES} 
	${${PROJECT_NAME}_src_FILES}
)


############################################################
#
# Linking Esterni
#
############################################################

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${GeckoSDK_NSPR4_LIBRARY} ${GeckoSDK_XPCOM_LIBRARY} ${GeckoSDK_PLC4_LIBRARY} ${GeckoSDK_PLDS4_LIBRARY} ${GeckoSDK_XPCOM_GLUE_S_LIBRARY} ${UNRAR_LIBRARY} )
TARGET_LINK_LIBRARIES(${PROJECT_NAME} optimized ${CURL_LIBRARY} debug ${CURL_LIBRARY_DEBUG} )
TARGET_LINK_LIBRARIES(${PROJECT_NAME} Utilities )
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${ADDITIONAL_LIBRARY})


############################################################
#
# Target Properties
#
############################################################

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES PROJECT_LABEL "Shell_${PROJECT_NAME}")


###############################################################
#
# Pre Build Actions
#
###############################################################

GET_TARGET_PROPERTY(${PROJECT_NAME}_LOCATION ${PROJECT_NAME} LOCATION)
GET_FILENAME_COMPONENT( MYNAME ${${PROJECT_NAME}_LOCATION} NAME)

ADD_CUSTOM_COMMAND(
	TARGET ${PROJECT_NAME}
	PRE_BUILD
	COMMAND ${GeckoSDK_XPIDL_EXECUTABLE} 
		ARGS -w -v -m header -I${GeckoSDK_IDL_DIR} ${${PROJECT_NAME}_idl}/${IDL_FILENAME}.idl
	COMMAND ${CMAKE_COMMAND}
   		ARGS -E copy ${CMAKE_CURRENT_BINARY_DIR}/${IDL_FILENAME}.h ${${PROJECT_NAME}_GEN_INC_DIR}/${PROJECT_NAME}
	COMMAND ${GeckoSDK_XPIDL_EXECUTABLE} 
		ARGS -w -v -m typelib -I${GeckoSDK_IDL_DIR} ${${PROJECT_NAME}_idl}/${IDL_FILENAME}.idl
	COMMAND ${GeckoSDK_XPT_LINK_EXECUTABLE} 
		ARGS ${FIREFOX_PLUGIN_DIR}/${PROJECT_NAME}.xpt ${IDL_FILENAME}.xpt 
)


###############################################################
#
# Post Build Actions
#
###############################################################

ADD_CUSTOM_COMMAND(
	TARGET ${PROJECT_NAME}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND}
    		ARGS -E copy ${${PROJECT_NAME}_LOCATION} ${FIREFOX_PLUGIN_DIR}
)


###############################################################
#
# Adding Test 
#
###############################################################

ADD_TEST(build_${PROJECT_NAME} ${CMAKE_CTEST_COMMAND}
	--build-and-test
	"${PROJECT_SOURCE_DIR}"
	"${PROJECT_BINARY_DIR}"
	--build-two-config
	--build-nocmake
	--build-generator ${CMAKE_GENERATOR}
	--build-makeprogram ${CMAKE_MAKE_PROGRAM}
	--build-project ${PROJECT_NAME}
	--build-target Shell_${PROJECT_NAME}
)


