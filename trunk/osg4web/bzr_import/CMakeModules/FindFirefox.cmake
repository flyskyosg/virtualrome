# The module defines the following variables:
#   Firefox_EXECUTABLE - path to cvs command line client
#   Firefox_FOUND - true if the command line client was found
# Example usage:
#   find_package(Firefox)
#   if(Firefox_FOUND)
#     message("Firefox found: ${Firefox_EXECUTABLE}")
#   endif(Firefox_FOUND)

get_filename_component(
  Firefox_User_DefaultIcon
  "[HKEY_CURRENT_USER\\Software\\Classes\\FirefoxHTML\\DefaultIcon]" 
  PATH)
get_filename_component(
  Firefox_LocMachine_DefaultIcon
  "[HKEY_LOCAL_MACHINE\\Software\\Classes\\FirefoxHTML\\DefaultIcon]" 
  PATH)
get_filename_component(
  Firefox_Root_DefaultIcon
  "[HKEY_CLASSES_ROOT\\FirefoxURL\\DefaultIcon]" 
  PATH)


find_program(Firefox_EXECUTABLE Firefox
  ${Firefox_User_DefaultIcon}
  ${Firefox_LocMachine_DefaultIcon}
  ${Firefox_Root_DefaultIcon}
  "[HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\App Paths;Path]"
  DOC "Firefox executable"
  )
mark_as_advanced(Firefox_EXECUTABLE)

# Handle the QUIETLY and REQUIRED arguments and set CVS_FOUND to TRUE if 
# all listed variables are TRUE

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Firefox DEFAULT_MSG Firefox_EXECUTABLE)
#FIND_PROGRAM(
#		FIREFOX_EXE
#		NAME
#		Firefox
#		PATHS
#		"[HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\App Paths;Path]"
#		"[HKEY_LOCAL_MACHINE\\Software\\Classes\\Applications\\firefox.exe\\shell\\open\\command]"
#		"[HKEY_CURRENT_USER\\Software\\Classes\\http\\shell\\open\\command]"
#		"[HKEY_CURRENT_USER\\Software\\Classes\\http\\DefaultIcon]"
#		"[HKEY_CURRENT_USER\\Software\\Classes\\FirefoxHTML\\shell\\open\\command]"
#		"[HKEY_CURRENT_USER\\Software\\Classes\\FirefoxHTML\DefaultIcon]"
#		"[HKEY_CLASSES_ROOT\\Applications\\Firefox.exe\\shell\\open\\command]"
#		"[HKEY_CLASSES_ROOT\\FirefoxURL\DefaultIcon]"
#    ${OSG_DIR}/bin
#)