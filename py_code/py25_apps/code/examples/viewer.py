#!/usr/bin/env python


# define default data thru OSG_FILE_PATH env var
import os,sys
my_base_dir=os.path.split(os.path.split(os.path.split(os.path.abspath(os.path.realpath( __file__ )))[0])[0])[0]
print my_base_dir
os.environ['OSG_FILE_PATH']=os.path.join(my_base_dir,'models','OpenSceneGraph-Data')

# import all necessary stuff
import osg
import osgDB
import osgGA
import osgViewer

# create a root node
node = osg.Group()

# needed for python
filepath = osgDB.getLibraryFilePathList()
for item in sys.path: filepath.append(item)
osgDB.setLibraryFilePathList(filepath)

loadedmodel = osgDB.readNodeFile("cow.osg")

# open a file
node.addChild(loadedmodel)

# create a viewer
viewer = osgViewer.Viewer()

# configure default threading
viewer.setThreadingModel(osgViewer.Viewer.SingleThreaded)

# add handlers
viewer.addEventHandler(osgViewer.StatsHandler())
viewer.addEventHandler(osgViewer.WindowSizeHandler())
viewer.addEventHandler(osgViewer.ThreadingHandler())
viewer.addEventHandler(osgViewer.HelpHandler())
viewer.addEventHandler(osgViewer.HelpHandler())
#### questo manipolatore gestisce i tasti w e t cge abilitano wire frame e texture
ss = viewer.getCamera().getOrCreateStateSet()
viewer.addEventHandler( osgGA.StateSetManipulator(ss) )



# add to the scene
viewer.setSceneData(node)

# loop until done
viewer.run()
