############################################################
#
# ieOSG4Web Sub Project
#
# Internet Explorer ActiveX
#
############################################################

PROJECT(ieOSG4Web)


############################################################
#
# Settaggio Variabili 
#
############################################################

# Gecko Root (Used for Threads implementation)
SET(GeckoSDK_ROOT ${OSG4WEB_ROOT}/var/dependancies/gecko-sdk CACHE PATH "base dir for Firefox Gecko SDK")

# cURL root directory
SET(CURL_ROOT ${OSG4WEB_ROOT}/var/dependancies/cmcurl CACHE PATH "base dir for cURL Library")

# UNRAR directory
SET(UNRAR_ROOT ${OSG4WEB_ROOT}/var/dependancies/unrarlib CACHE PATH "base dir for UnRAR Library")

# Defining OSG4Web root if not presents
IF(NOT OSG4WEB_ROOT)
	GET_FILENAME_COMPONENT(OSG4WEB_ROOT "${PROJECT_SOURCE_DIR}/../.." ABSOLUTE CACHE)
ENDIF(NOT OSG4WEB_ROOT)

# Microsoft Tools Directory
GET_FILENAME_COMPONENT(MSTOOLS_DIR ${CMAKE_MAKE_PROGRAM} PATH)
GET_FILENAME_COMPONENT(MSTOOLS_DIR ${MSTOOLS_DIR} PATH)
SET(MSTOOLS_DIR "${MSTOOLS_DIR}/Tools/Bin")

# Compiling Type
SET(CMAKE_MFC_FLAG 2)

# MSVC libraries
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

# Source code files
FOREACH(myvar include idl src)
	SET("${PROJECT_NAME}_${myvar}" ${OSG4WEB_ROOT}/${myvar}/${PROJECT_NAME} )
ENDFOREACH(myvar)

FOREACH(myvar include src)
	SET("CommonShell_${myvar}" ${OSG4WEB_ROOT}/${myvar}/CommonShell)
ENDFOREACH(myvar)

# Set up folder for generated include/resource files
SET(${PROJECT_NAME}_GEN_INC_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)
SET(${PROJECT_NAME}_GEN_RES_DIR ${CMAKE_CURRENT_BINARY_DIR}/res)
SET(IDL_FILENAME ieOSG4Web)

# Windows registry service
SET(WINDOWS_REGISTRY_SERVICE regsvr32)


############################################################
#
# Find Programs
#
############################################################

FIND_PROGRAM( MIDL_MS_COMPILER midl PATHS ${MSTOOLS_DIR} )

IF(NOT MIDL_MS_COMPILER)
	MESSAGE(FATAL_ERROR "midl compiler NOT FOUND")
ENDIF(NOT MIDL_MS_COMPILER)

# NSPR Library 
FIND_FILE(NSPR4_DLL nspr4.dll PATHS ${FIREFOX_DIR})

IF(NOT NSPR4_DLL)
	MESSAGE(FATAL_ERROR "nspr4.dll NOT FOUND")
ENDIF(NOT NSPR4_DLL)


############################################################
#
# Grabbing files
#
###########################################################

FOREACH(myvar include idl src)
	GRAB_FILES("${PROJECT_NAME}_${myvar}")
ENDFOREACH(myvar)

FOREACH(myvar include src)
	GRAB_FILES("CommonShell_${myvar}")
ENDFOREACH(myvar)


############################################################
#
# Find Packages
#
###########################################################

FIND_PACKAGE(CURL QUIET)

FIND_PACKAGE(UNRAR QUIET)
IF(NOT UNRAR_FOUND)
	SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR};${CMAKE_MODULE_PATH}")
	FIND_PACKAGE(UNRAR)
ENDIF(NOT UNRAR_FOUND)

FIND_PACKAGE(GeckoSDK QUIET)
IF(NOT GeckoSDK_FOUND)
	SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR};${CMAKE_MODULE_PATH}")
	FIND_PACKAGE(GeckoSDK)
ENDIF(NOT GeckoSDK_FOUND)


############################################################
#
# Definizioni
#
############################################################

IF(CURL_LIBRARY)
	ADD_DEFINITIONS(-DHAVE_CONFIG_H -DCURL_STATICLIB)
ELSE(CURL_LIBRARY)
	MESSAGE(FATAL_ERROR "Curl NOT FOUND")
ENDIF(CURL_LIBRARY)	

ADD_DEFINITIONS(-D_AFXDLL)

ADD_DEFINITIONS(-DOSG4WEB_UTILITIES_LIBRARY_STATIC)

ADD_DEFINITIONS(-DHAVE_CONFIG_H -DCURL_STATICLIB)

ADD_DEFINITIONS(-D_USRDLL -D_WINDOWS -D_MFCAPP)



############################################################
#
# Making Sub-Directories
#
############################################################

FILE(MAKE_DIRECTORY ${${PROJECT_NAME}_GEN_INC_DIR})
FILE(MAKE_DIRECTORY ${${PROJECT_NAME}_GEN_RES_DIR})


############################################################
#
# Configuring Resource
#
############################################################

# Generating Debug
FILE( TO_NATIVE_PATH ${${PROJECT_NAME}_GEN_INC_DIR}/${IDL_FILENAME}.tlb MY_TLB_FILE)
CONFIGURE_FILE(${OSG4WEB_ROOT}/res/${PROJECT_NAME}/${PROJECT_NAME}_res.rc.in ${${PROJECT_NAME}_GEN_RES_DIR}/${PROJECT_NAME}_res.rc @ONLY)


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
	${${PROJECT_NAME}_GEN_RES_DIR}
	${OSG4WEB_ROOT}/res/${PROJECT_NAME}
	)


############################################################
#
# Impostazione sorgenti generati
#
############################################################

SET_SOURCE_FILES_PROPERTIES(
	${${PROJECT_NAME}_GEN_INC_DIR}/${IDL_FILENAME}_gen.h 
	${${PROJECT_NAME}_GEN_RES_DIR}/${PROJECT_NAME}_res.rc
	PROPERTIES GENERATED ON
)


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
	${${PROJECT_NAME}_GEN_INC_DIR}/${IDL_FILENAME}_gen.h
	${CommonShell_src_FILES}
	${${PROJECT_NAME}_src_FILES}
	${OSG4WEB_ROOT}/res/${PROJECT_NAME}/${IDL_FILENAME}_res.def
	${${PROJECT_NAME}_GEN_RES_DIR}/${PROJECT_NAME}_res.rc
)

# custom vecchio

#ADD_CUSTOM_COMMAND(
#	OUTPUT  ${${PROJECT_NAME}_GEN_INC_DIR}/Debug/${IDL_FILENAME}.tlb ${${PROJECT_NAME}_GEN_INC_DIR}/Debug/${PROJECT_NAME}/${IDL_FILENAME}_gen.h
#		${${PROJECT_NAME}_GEN_INC_DIR}/Release/${IDL_FILENAME}.tlb ${${PROJECT_NAME}_GEN_INC_DIR}/Release/${PROJECT_NAME}/${IDL_FILENAME}_gen.h
#	COMMAND ${MIDL_MS_COMPILER} /D "_DEBUG" /nologo /char signed /env win32 /tlb ${${PROJECT_NAME}_GEN_INC_DIR}/Debug/${IDL_FILENAME}.tlb /h  ${${PROJECT_NAME}_GEN_INC_DIR}/Debug/${PROJECT_NAME}/${IDL_FILENAME}_gen.h ${${PROJECT_NAME}_idl}/${IDL_FILENAME}.idl #IN RELEASE DEVE ESSERE /D "NDEBUG" /mktyplib203 
#	COMMAND ${MIDL_MS_COMPILER} /D "NDEBUG" /mktyplib203 /nologo /char signed /env win32 /tlb ${${PROJECT_NAME}_GEN_INC_DIR}/Release/${IDL_FILENAME}.tlb /h  ${${PROJECT_NAME}_GEN_INC_DIR}/Release/${PROJECT_NAME}/${IDL_FILENAME}_gen.h ${${PROJECT_NAME}_idl}/${IDL_FILENAME}.idl #IN RELEASE DEVE ESSERE /D "NDEBUG" /mktyplib203 
#	WORKING_DIRECTORY ${${PROJECT_NAME}_GEN_INC_DIR}/Debug
#	DEPENDS ${${PROJECT_NAME}_idl}/${IDL_FILENAME}.idl
#	DEPENDS ${MIDL_MS_COMPILER}
#)


############################################################
#
# Linking Esterni
#
############################################################

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${GeckoSDK_NSPR4_LIBRARY} ${GeckoSDK_XPCOM_LIBRARY} ${GeckoSDK_PLC4_LIBRARY} ${GeckoSDK_PLDS4_LIBRARY} ${UNRAR_LIBRARY} Utilities)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${ADDITIONAL_LIBRARY})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} optimized ${CURL_LIBRARY} debug ${CURL_LIBRARY_DEBUG})


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

ADD_CUSTOM_COMMAND(
	TARGET ${PROJECT_NAME}
	PRE_BUILD
	COMMAND ${MIDL_MS_COMPILER} 
		ARGS /D "NDEBUG" /mktyplib203 /nologo /char signed /env win32 /tlb ${${PROJECT_NAME}_GEN_INC_DIR}/${IDL_FILENAME}.tlb /h  ${${PROJECT_NAME}_GEN_INC_DIR}/${IDL_FILENAME}_gen.h ${${PROJECT_NAME}_idl}/${IDL_FILENAME}.idl
)


###############################################################
#
# Post Build Actions
#
###############################################################

GET_TARGET_PROPERTY(${PROJECT_NAME}_LOCATION ${PROJECT_NAME} LOCATION)
GET_FILENAME_COMPONENT( MYNAME ${${PROJECT_NAME}_LOCATION} NAME)

SET(REG_COMPONENT ${FIREFOX_PLUGIN_DIR}/${MYNAME})
STRING(REPLACE "/" "\\" REG_COMPONENT ${REG_COMPONENT})

ADD_CUSTOM_COMMAND(
	TARGET ${PROJECT_NAME}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND}
    		ARGS -E copy ${NSPR4_DLL} ${FIREFOX_PLUGIN_DIR}/
	COMMAND ${CMAKE_COMMAND}
		#ARGS -E copy ${${PROJECT_NAME}_LOCATION} ${FIREFOX_PLUGIN_DIR}/${MYNAME}
		ARGS -E copy ${${PROJECT_NAME}_LOCATION} ${FIREFOX_PLUGIN_DIR}/
	COMMAND ${WINDOWS_REGISTRY_SERVICE}
		ARGS ${REG_COMPONENT}
)

