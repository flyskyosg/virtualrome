############################################################
#
# Adding Engines Subdirectories
#
############################################################

SUBDIRS(
	CoreOpenGL
	)

IF(OSG_FOUND)
	SUBDIRS(
		CoreBase
		LoadCore
		FunCore
	)
ENDIF(OSG_FOUND)

