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
		FunCore
	)
	IF(USE_OPENGL_LOADER)
		SUBDIRS( LoaderOpenGL )
	ELSE(USE_OPENGL_LOADER)
		SUBDIRS( Loader )
	ENDIF(USE_OPENGL_LOADER)
ENDIF(OSG_FOUND)

