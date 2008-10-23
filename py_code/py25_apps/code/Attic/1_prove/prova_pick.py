#import sys
#import os
#print os.path.realpath(sys.argv[0])

#os.environ['PATH'] = 'D:\\SE5\\msvc\\install\\bin'
#sys.path.append('D:/SE5/msvc/build/osgswig_svn/lib/python')



import osg_setup
import osg
import osgDB
import osgGA
import osgViewer
import wx_osg_console
import pick_module

viewer = osgViewer.Viewer()
viewer.setThreadingModel(osgViewer.Viewer.SingleThreaded)

    #set the scene data
    #n = osgDB.readNodeFile('cow.osg')   
root = osg.Group()
#n = osgDB.readNodeFile("D:/models/demo_virtrome/piante/frassino.ive")
#n = osgDB.readNodeFile("Q:/virtualrome/web/modelli/romano/villa/prove_stanze/tmp/prove_stanze_mod.osg")
n = osgDB.readNodeFile("../../../testData/teapot.ive")
#n = osgDB.readNodeFile("Q:/vrome/web/modelli/attuale/generati/Malborghetto_attuale_osg/osg_exp_tiny_full.ive")
root.addChild(n)
viewer.setSceneData(root.__disown__())

pickhandler = pick_module.PickHandler()

viewer.addEventHandler(pickhandler.__disown__());

wx_osg_console.OpenConsole(viewer)

