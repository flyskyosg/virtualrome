# Locate Gecko SDK
# This module defines
# GECKOSDK_LIBRARY
# GECKOSDK_FOUND, if false, do not try to link to Gecko SDK 
# GECKOSDK_INCLUDE_DIR, where to find the headers
#
# $GECKOSDK_DIR is an environment variable that would
# correspond to the ./configure --prefix=$GECKOSDK_DIR


FIND_PATH(GeckoSDK_INCLUDE_DIR nspr.h	
		${GeckoSDK_ROOT}/include
    $ENV{GECKOSDK_DIR}/include
    $ENV{GECKOSDK_DIR}
    $ENV{GECKOSDKDIR}/include
    $ENV{GECKOSDK_DIR}
    $ENV{GECKOSDK_ROOT}/include
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/include
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;GECKOSDK_ROOT]/include
    /usr/freeware/include
)

FIND_PATH(GeckoSDK_IDL_DIR nsISupports.idl
		${GeckoSDK_ROOT}/idl
    $ENV{GECKOSDK_DIR}/idl
    $ENV{GECKOSDK_DIR}
    $ENV{GECKOSDKDIR}/idl
    $ENV{GECKOSDK_DIR}
    $ENV{GECKOSDK_ROOT}/idl
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/idl
    /usr/idl
    /sw/idl # Fink
    /opt/local/idl # DarwinPorts
    /opt/csw/idl # Blastwave
    /opt/idl
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;GECKOSDK_ROOT]/idl
    /usr/freeware/idl
)

FIND_PROGRAM(GeckoSDK_XPIDL_EXECUTABLE 
	NAMES xpidl 
	PATHS
		${GeckoSDK_ROOT}/bin
    $ENV{GECKOSDK_DIR}/bin
    $ENV{GECKOSDK_DIR}
    $ENV{GECKOSDKDIR}/bin
    $ENV{GECKOSDK_DIR}
    $ENV{GECKOSDK_ROOT}/bin
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/bin
    /usr/bin
    /sw/bin # Fink
    /opt/local/bin # DarwinPorts
    /opt/csw/bin # Blastwave
    /opt/bin
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;GECKOSDK_ROOT]/bin
    /usr/freeware/bin
)

FIND_PROGRAM(GeckoSDK_XPT_LINK_EXECUTABLE 
	NAMES xpt_link 
	PATHS
		${GeckoSDK_ROOT}/bin
    $ENV{GECKOSDK_DIR}/bin
    $ENV{GECKOSDK_DIR}
    $ENV{GECKOSDKDIR}/bin
    $ENV{GECKOSDK_DIR}
    $ENV{GECKOSDK_ROOT}/bin
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/bin
    /usr/bin
    /sw/bin # Fink
    /opt/local/bin # DarwinPorts
    /opt/csw/bin # Blastwave
    /opt/bin
    [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;GECKOSDK_ROOT]/bin
    /usr/freeware/bin
)

FIND_LIBRARY(GeckoSDK_XPCOM_LIBRARY
        NAMES xpcom
#        NAMES xpcomglue
        PATHS
				${GeckoSDK_ROOT}/lib
        $ENV{GECKOSDK_DIR}/lib
        $ENV{GECKOSDK_DIR}
        $ENV{GECKOSDKDIR}/lib
        $ENV{GECKOSDKDIR}
        $ENV{GECKOSDK_ROOT}/lib
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/lib
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;GECKOSDK_ROOT]/lib
        /usr/freeware/lib64
    )

FIND_LIBRARY(GeckoSDK_PLC4_LIBRARY
        NAMES plc4
        PATHS
				${GeckoSDK_ROOT}/lib
        $ENV{GECKOSDK_DIR}/lib
        $ENV{GECKOSDK_DIR}
        $ENV{GECKOSDKDIR}/lib
        $ENV{GECKOSDKDIR}
        $ENV{GECKOSDK_ROOT}/lib
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/lib
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;GECKOSDK_ROOT]/lib
        /usr/freeware/lib64
    )

FIND_LIBRARY(GeckoSDK_PLDS4_LIBRARY
        NAMES plds4
        PATHS
				${GeckoSDK_ROOT}/lib
        $ENV{GECKOSDK_DIR}/lib
        $ENV{GECKOSDK_DIR}
        $ENV{GECKOSDKDIR}/lib
        $ENV{GECKOSDKDIR}
        $ENV{GECKOSDK_ROOT}/lib
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/lib
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;GECKOSDK_ROOT]/lib
        /usr/freeware/lib64
    )

FIND_LIBRARY(GeckoSDK_NSPR4_LIBRARY
        NAMES nspr4
        PATHS
				${GeckoSDK_ROOT}/lib
        $ENV{GECKOSDK_DIR}/lib
        $ENV{GECKOSDK_DIR}
        $ENV{GECKOSDKDIR}/lib
        $ENV{GECKOSDKDIR}
        $ENV{GECKOSDK_ROOT}/lib
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/lib
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;GECKOSDK_ROOT]/lib
        /usr/freeware/lib64
    )

MARK_AS_ADVANCED(GeckoSDK_NSPR4_LIBRARY  GeckoSDK_PLDS4_LIBRARY GeckoSDK_PLC4_LIBRARY GeckoSDK_XPCOM_LIBRARY GeckoSDK_INCLUDE_DIR GeckoSDK_IDL_DIR )

SET(GeckoSDK_FOUND "NO")
IF(GeckoSDK_NSPR4_LIBRARY AND GeckoSDK_XPCOM_LIBRARY AND GeckoSDK_INCLUDE_DIR AND GeckoSDK_IDL_DIR AND GeckoSDK_XPIDL_EXECUTABLE)
    SET(GECKOSDK_FOUND "YES")
ENDIF(GeckoSDK_NSPR4_LIBRARY AND GeckoSDK_XPCOM_LIBRARY AND GeckoSDK_INCLUDE_DIR AND GeckoSDK_IDL_DIR AND GeckoSDK_XPIDL_EXECUTABLE)



