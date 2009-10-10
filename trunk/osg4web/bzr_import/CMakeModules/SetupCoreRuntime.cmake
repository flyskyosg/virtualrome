############################################################
#
# Additional Options
#
############################################################

OPTION(PLACE_IN_PLUGIN "If on place output files into Firefox/plugin" OFF)


############################################################
#
# Coping Cores in Debug Directory
#
############################################################

MACRO(COPY_TARGET_POST_BUILD CORE_TYPE_PREFIX )
	# cmake string to copy only if exists
	SET( mycmakestring 
	"GET_FILENAME_COMPONENT(MY_FULL_PATH \"@in_file@\" ABSOLUTE)
	 GET_FILENAME_COMPONENT(MY_FULL_OUT_PATH \"@out_file@\" ABSOLUTE)
	IF(EXISTS \${MY_FULL_PATH})
		EXEC_PROGRAM(\${CMAKE_COMMAND} 
			ARGS -E copy \${MY_FULL_PATH} \\\"\${MY_FULL_OUT_PATH}\\\"
			OUTPUT_VARIABLE myout
			RETURN_VALUE myret
		)
		MESSAGE(\"OUTPUT_VARIABLE -->\${myout}\")
		MESSAGE(\"RETURN_VALUE -->\${myret}\")
	ENDIF(EXISTS \${MY_FULL_PATH})
	"
	)
	
	FOREACH(BTYPE Debug Release)
		GET_TARGET_PROPERTY(${CORE_TYPE_PREFIX}_${BTYPE}_LOCATION ${CORE_TYPE_PREFIX} ${BTYPE}_LOCATION)

		GET_FILENAME_COMPONENT( MYNAME_${BTYPE} ${${CORE_TYPE_PREFIX}_${BTYPE}_LOCATION} NAME)

		FILE(TO_CMAKE_PATH ${${CORE_TYPE_PREFIX}_${BTYPE}_LOCATION} in_file)
		FILE(TO_CMAKE_PATH "${OUTPUT_${BTYPE}_DIR}/${MYNAME_${BTYPE}}" out_file)
		STRING(CONFIGURE ${mycmakestring} mycmakestring_out @ONLY)
		FILE(WRITE "${CMAKE_BINARY_DIR}/script/copy_${CORE_TYPE_PREFIX}_${BTYPE}.cmake" "${mycmakestring_out}")
		ADD_CUSTOM_COMMAND(
			TARGET ${CORE_TYPE_PREFIX}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -P "${CMAKE_BINARY_DIR}/script/copy_${CORE_TYPE_PREFIX}_${BTYPE}.cmake"
		)
	ENDFOREACH(BTYPE Debug Release)
ENDMACRO(COPY_TARGET_POST_BUILD CORE_TYPE_PREFIX )


############################################################
#
# Setup Runtime Cores
#
############################################################

MACRO(SETUP_CORE_RUNTIME CORE_TYPE_PREFIX)
	IF(PLACE_IN_PLUGIN)
		SET(OUTPUT_Debug_DIR ${FIREFOX_PLUGIN_DIR}/${PROJECT_NAME})
		SET(OUTPUT_Release_DIR ${FIREFOX_PLUGIN_DIR}/${PROJECT_NAME})
	ELSE(PLACE_IN_PLUGIN)
		SET(OUTPUT_Debug_DIR $ENV{APPDATA}/osg4web/${PROJECT_NAME})
		SET(OUTPUT_Release_DIR $ENV{APPDATA}/osg4web/${PROJECT_NAME})
	ENDIF(PLACE_IN_PLUGIN)

	COPY_TARGET_POST_BUILD(	${PROJECT_NAME} )		
ENDMACRO(SETUP_CORE_RUNTIME CORE_TYPE_PREFIX)


############################################################
#
# Setup Runtime Loading Cores
#
############################################################

MACRO(SETUP_LOADCORE_RUNTIME CORE_TYPE_PREFIX)
	SET(OUTPUT_Debug_DIR ${FIREFOX_PLUGIN_DIR})
	SET(OUTPUT_Release_DIR ${FIREFOX_PLUGIN_DIR})

	COPY_TARGET_POST_BUILD( ${PROJECT_NAME} )		
ENDMACRO(SETUP_LOADCORE_RUNTIME CORE_TYPE_PREFIX)



############################################################
#
# Configuring IE and FFox HTML Tests
#
############################################################

MACRO(CONFIGURE_HTML_TEST HTML_TEST_DIR)
	CONFIGURE_FILE(${OSG4WEB_ROOT}/CMakeIN/run_firefox.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/run_firefox.cmake @ONLY)
	CONFIGURE_FILE(${OSG4WEB_ROOT}/CMakeIN/run_iexplore.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/run_iexplore.cmake @ONLY)

	FOREACH(HTML_CURRENT_DEFINITION ie np)
		IF(HTML_CURRENT_DEFINITION STREQUAL ie)
			SET(IE_CLASSID_DEBUG "classid=\"CLSID:559F0DCD-759E-48EE-B1F2-C917AA6FB7EA\"")
		ELSE(HTML_CURRENT_DEFINITION STREQUAL ie)
			SET(IE_CLASSID_DEBUG "")
		ENDIF(HTML_CURRENT_DEFINITION STREQUAL ie)
	
		CONFIGURE_FILE(${HTML_TEST_DIR}/${PROJECT_NAME}.html.in ${CMAKE_CURRENT_BINARY_DIR}/${HTML_CURRENT_DEFINITION}${PROJECT_NAME}.html @ONLY)
	ENDFOREACH(HTML_CURRENT_DEFINITION ie np)

	# Adding test
	ADD_TEST(RUN_np${PROJECT_NAME} ${CMAKE_COMMAND} -DURL=file:\"//${CMAKE_CURRENT_BINARY_DIR}/np${PROJECT_NAME}.html\" -P ${CMAKE_CURRENT_BINARY_DIR}/run_firefox.cmake )
	ADD_TEST(RUN_ie${PROJECT_NAME} ${CMAKE_COMMAND} -DURL=${CMAKE_CURRENT_BINARY_DIR}/ie${PROJECT_NAME}.html -P ${CMAKE_CURRENT_BINARY_DIR}/run_iexplore.cmake ) 
ENDMACRO(CONFIGURE_HTML_TEST)


############################################################
#
# TODO: DA RIFARE
#
# Copies extra files into Debug dirs
#
############################################################

MACRO(COPY_EXTRA_FILES DIR DATA)
	GET_FILENAME_COMPONENT(DATA_TGT ${DATA} NAME)
	ADD_CUSTOM_COMMAND(
		TARGET ${PROJECT_NAME}
		POST_BUILD
    		COMMAND ${CMAKE_COMMAND} -E copy ${DATA} ${DIR}
	    	)
ENDMACRO(COPY_EXTRA_FILES)


############################################################
#
# Setup Runtime Example Cores
#
############################################################

MACRO(SETUP_CORE_EXAMPLE)
	# CoreExample Sub Project
	GET_FILENAME_COMPONENT(THIS_PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
	PROJECT(${THIS_PROJECT_NAME})

	# Adding Definition
	ADD_DEFINITIONS(-DOSG4WEBCORE_LIBRARY)

	# Setting variables
	SET(CMAKE_DEBUG_POSTFIX  "d")
	SET("${PROJECT_NAME}_src" ${CMAKE_CURRENT_SOURCE_DIR}/src)

	# Grabbing src and include files
	GRAB_FILES( "${PROJECT_NAME}_src" )

	# Adding grabbed files to project
	ADD_LIBRARY(${PROJECT_NAME}
		SHARED
	    	${${PROJECT_NAME}_src_FILES}
	    	${OSG4WEB_ROOT}/src/Engines/CoreBase/src/CoreBase.cpp
	)

	# Adding ext libraries include directories
	INCLUDE_DIRECTORIES( ${${PROJECT_NAME}_src} ${OSG_INCLUDE_DIR} ${OSG4WEB_ROOT}/src/Engines/CoreBase/include  ${OSG4WEB_ROOT}/include)

	# Linking to ext libraries
	LINK_WITH_VARIABLES(${PROJECT_NAME} OSG_LIBRARY OSGUTIL_LIBRARY OSGDB_LIBRARY OSGGA_LIBRARY OSGFX_LIBRARY OSGTEXT_LIBRARY OSGVIEWER_LIBRARY OPENTHREADS_LIBRARY)

	# Adding Library Dependancies
	TARGET_LINK_LIBRARIES(${PROJECT_NAME} CommonCore)

	# Setting target properties
	SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES PROJECT_LABEL "Example_${PROJECT_NAME}")
	SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

	# Coping Target Result in Debug Directory
	SETUP_CORE_RUNTIME(Example ${PROJECT_NAME})

	# Configuring HTML Default Tests
	CONFIGURE_HTML_TEST(${CMAKE_CURRENT_SOURCE_DIR}/html)

	# Include ModuleInsall #TODO: forse da togliere...
	INCLUDE(ModuleInstall OPTIONAL)
ENDMACRO(SETUP_CORE_EXAMPLE)



