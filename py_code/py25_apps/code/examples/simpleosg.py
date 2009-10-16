#!/usr/bin/env python
#simpleosg.py

__author__  = ["Rene Molenaar"]
__url__     = ("http://code.google.com/p/osgswig/")
__version__ = "2.0.0"
__doc__     = """ This example show the most basic use of OpenSceneGraph in Python ____Rene Molenaar 2008 """

import os,sys
my_base_dir=os.path.split(os.path.split(os.path.split(os.path.abspath(os.path.realpath( __file__ )))[0])[0])[0]
print my_base_dir
#os.environ['OSG_FILE_PATH']=os.path.join(my_base_dir,'models','OpenSceneGraph-Data')

import osg, osgDB, osgViewer

#loadedModel = osgDB.readNodeFile(os.path.join(my_base_dir,'models','OpenSceneGraph-Data',"cow.osg"))
loadedModel = osgDB.readNodeFile("cow.osg")
viewer = osgViewer.Viewer()
viewer.setSceneData(loadedModel)
viewer.addEventHandler(osgViewer.StatsHandler());
viewer.run()
