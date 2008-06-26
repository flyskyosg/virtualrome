#import sys
#import os
#print os.path.realpath(sys.argv[0])

#os.environ['PATH'] = 'D:\\SE5\\msvc\\install\\bin'
#sys.path.append('D:/SE5/msvc/build/osgswig_svn/lib/python')


#import osg_setup
from osg_setup import *
import os

print os.getcwd()
print "path--> ",os.environ['PATH']


import pyOsgFrame 
import pick_module






myviewer = pyOsgFrame.myviewer()

#viewer.setThreadingModel(osgViewer.Viewer.SingleThreaded)

    #set the scene data
    #n = osgDB.readNodeFile('cow.osg')   
#root = osg.Group()
#n = osgDB.readNodeFile("D:/models/demo_virtrome/piante/frassino.ive")
#n = osgDB.readNodeFile("Q:/virtualrome/web/modelli/romano/villa/prove_stanze/tmp/prove_stanze_mod.osg")
#n = osgDB.readNodeFile("D:/models/prove/bump_mapping_gaiani_e_marco/prove_bump_cleaned/schiavo_marco/prova2.osg")
#n = osgDB.readNodeFile("Q:/vrome/web/modelli/attuale/generati/Malborghetto_attuale_osg/osg_exp_tiny_full.ive")
#root.addChild(n)
#viewer.setSceneData(root.__disown__())

pickhandler = pick_module.PickHandler()

myviewer.getviewer().addEventHandler(pickhandler.__disown__());
myviewer.init()

app = pyOsgFrame.TestApp(0)
app.SetViewer(myviewer)
app.MainLoop()


#OpenConsole(viewer)

