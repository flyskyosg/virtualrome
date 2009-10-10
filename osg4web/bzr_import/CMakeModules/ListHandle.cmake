#miscellaneous macros

################### macros from http://www.cmake.org/Wiki/CMakeMacroListOperations
MACRO(CAR var)
  SET(${var} ${ARGV1})
ENDMACRO(CAR)

MACRO(CDR var junk)
  SET(${var} ${ARGN})
ENDMACRO(CDR)


MACRO(LIST_INDEX var index)
  SET(list . ${ARGN})
  FOREACH(i RANGE 1 ${index})
    CDR(list ${list})
  ENDFOREACH(i)
  CAR(${var} ${list})
ENDMACRO(LIST_INDEX)

######### LIST_CONTAINS usage
#SET(MYLIST hello world foo bar)

#LIST_CONTAINS(contains foo ${MYLIST})
#IF (contains)
#  MESSAGE("MYLIST contains foo")
#ENDIF (contains)

#LIST_CONTAINS(contains baz ${MYLIST})
#IF (NOT contains)
#  MESSAGE("MYLIST does not contain baz")
#ENDIF (NOT contains)


MACRO(LIST_CONTAINS var value)
  SET(${var})
  FOREACH (value2 ${ARGN})
    IF (${value} STREQUAL ${value2})
      SET(${var} TRUE)
    ENDIF (${value} STREQUAL ${value2})
  ENDFOREACH (value2)
ENDMACRO(LIST_CONTAINS)

############################################################
################### macros from http://www.cmake.org/Wiki/CMakeMacroParseArguments
MACRO(PARSE_ARGUMENTS prefix arg_names option_names)
  #MESSAGE("!!!! ${prefix} args-->${arg_names}<-- opt-->${option_names}<--")
  SET(DEFAULT_ARGS)
  FOREACH(arg_name ${arg_names})
    SET(${prefix}_${arg_name})
  ENDFOREACH(arg_name)
  FOREACH(option ${option_names})
    SET(${prefix}_${option} FALSE)
  ENDFOREACH(option)

  SET(current_arg_name DEFAULT_ARGS)
  SET(current_arg_list)
  FOREACH(arg ${ARGN})
    #debug#MESSAGE("---->${arg}<------")
    LIST_CONTAINS(is_arg_name ${arg} ${arg_names})
    IF (is_arg_name)
      SET(${prefix}_${current_arg_name} ${current_arg_list})
      SET(current_arg_name ${arg})
      SET(current_arg_list)
    ELSE (is_arg_name)
      LIST_CONTAINS(is_option ${arg} ${option_names})
      IF (is_option)
    SET(${prefix}_${arg} TRUE)
      ELSE (is_option)
    SET(current_arg_list ${current_arg_list} ${arg})
      ENDIF (is_option)
    ENDIF (is_arg_name)
  ENDFOREACH(arg)
  SET(${prefix}_${current_arg_name} ${current_arg_list})
ENDMACRO(PARSE_ARGUMENTS)

