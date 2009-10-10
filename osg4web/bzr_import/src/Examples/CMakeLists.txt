
############################################################
#
# Adding Example Engines Subdirectories
#
############################################################

IF(OSG_FOUND)
	SUBDIRS(
		CoreAnimateView
		CoreAsyncLoad
		CoreEdit
		CoreFly
		CoreJSCall
		CorePick
		CoreSceneModifier
		CoreScribe
		CoreTooltips
		CoreWalk
	)
ENDIF(OSG_FOUND)

