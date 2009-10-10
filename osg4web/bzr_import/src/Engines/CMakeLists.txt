############################################################
#
# Adding Engines Subdirectories
#
############################################################

SUBDIRS(
	CoreOpenGL
	LoaderOpenGL
	)

IF(OSG_FOUND)
	SUBDIRS(
		CoreBase
		Loader
		FunCore
	)
ENDIF(OSG_FOUND)

