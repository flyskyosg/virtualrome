
MACRO(GRAB_FILES_FOLDERS VARNAME DIRNAMES)
FOREACH(dir ${DIRNAMES})
	GET_FILENAME_COMPONENT(last_path ${${VARNAME}} PATH)
	GET_FILENAME_COMPONENT(last_path ${last_path} NAME)
	
	SET(PATTERN "*") 
	IF(${last_path} STREQUAL "src")
		SET(PATTERN "*.cpp") 
	ELSEIF(${last_path} STREQUAL "include")
		SET(PATTERN "*.h") 
	ENDIF(${last_path} STREQUAL "src")

	
	#	MESSAGE("FILE(GLOB ${VARNAME}_FILES ${${VARNAME}}/${dir}/${PATTERN})")
	FILE(GLOB TEMP ${${VARNAME}}/${dir}/${PATTERN})
	LIST(APPEND "${VARNAME}_FILES" ${TEMP})
ENDFOREACH(dir)
ENDMACRO(GRAB_FILES_FOLDERS VARNAME)


MACRO(GRAB_FILES VARNAME)
	IF(${ARGN})
		SET(PATTERN $ARGV0)
	ELSE(${ARGN})
		GET_FILENAME_COMPONENT(last_path ${${VARNAME}} PATH)
		GET_FILENAME_COMPONENT(last_path ${last_path} NAME)
		SET(PATTERN "*") 
		IF(${last_path} STREQUAL "src")
			SET(PATTERN "*.cpp") 
		ELSEIF(${last_path} STREQUAL "include")
			SET(PATTERN "*.h") 
		ELSEIF(${last_path} STREQUAL "res")
			SET(PATTERN "*.rc") 
		ENDIF(${last_path} STREQUAL "src")
	ENDIF(${ARGN})
#	MESSAGE("FILE(GLOB ${VARNAME}_FILES ${${VARNAME}}/${PATTERN})")
	FILE(GLOB "${VARNAME}_FILES" ${${VARNAME}}/${PATTERN})
	IF(${last_path} STREQUAL "res")
		FILE(GLOB tmp ${${VARNAME}}/*.def)
		SET(${VARNAME}_FILES ${${VARNAME}_FILES} ${tmp})
	ENDIF(${last_path} STREQUAL "res")
ENDMACRO(GRAB_FILES VARNAME)

#######################################################################################################
#  macro for linking libraries that come from Findxxxx commands, so there is a variable that contains the
#  full path of the library name. in order to differentiate release and debug, this macro get the
#  NAME of the variables, so the macro gets as arguments the target name and the following list of parameters
#  is intended as a list of variable names each one containing  the path of the libraries to link to
#  The existance of a varibale name with _DEBUG appended is tested and, in case it' s value is used
#  for linking to when in debug mode 
#  the content of this library for linking when in debugging
#######################################################################################################


MACRO(LINK_WITH_VARIABLES TRGTNAME)
    FOREACH(varname ${ARGN})
        IF(${varname}_DEBUG)
            TARGET_LINK_LIBRARIES(${TRGTNAME} optimized "${${varname}}" debug "${${varname}_DEBUG}")
        ELSE(${varname}_DEBUG)
            TARGET_LINK_LIBRARIES(${TRGTNAME} "${${varname}}" )
        ENDIF(${varname}_DEBUG)
    ENDFOREACH(varname)
ENDMACRO(LINK_WITH_VARIABLES TRGTNAME)

MACRO(LINK_INTERNAL TRGTNAME)
    FOREACH(LINKLIB ${ARGN})
        TARGET_LINK_LIBRARIES(${TRGTNAME} optimized "${LINKLIB}" debug "${LINKLIB}${CMAKE_DEBUG_POSTFIX}")
    ENDFOREACH(LINKLIB)
ENDMACRO(LINK_INTERNAL TRGTNAME)

MACRO(LINK_EXTERNAL TRGTNAME)
    FOREACH(LINKLIB ${ARGN})
        TARGET_LINK_LIBRARIES(${TRGTNAME} "${LINKLIB}" )
    ENDFOREACH(LINKLIB)
ENDMACRO(LINK_EXTERNAL TRGTNAME)
