############################################################
#
# Adding Engines Subdirectories
#
############################################################

IF(OSG_FOUND)
	SUBDIRS(
		CoreBase
		LoadCore
		FunCore
	)
ENDIF(OSG_FOUND)

