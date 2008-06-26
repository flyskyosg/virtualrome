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


from pyOsgFrame import *
import pick_module





class TestPanel(wx.Panel):
    def __init__(self, parent, log):
        self.log = log
        wx.Panel.__init__(self, parent, -1)

        b = wx.Button(self, -1, "Create and Show an OPEN FileDialog", (50,50))
        self.Bind(wx.EVT_BUTTON, self.OnButton, b)


    def OnButton(self, evt):
        wildcard = "OSG ascii source (*.osg)|*.osg|"     \
           "OSG binary  (*.ive)|*.ive|" \
           "All files (*.*)|*.*"

        self.log.WriteText("CWD: %s\n" % os.getcwd())

        # Create the dialog. In this case the current directory is forced as the starting
        # directory for the dialog, and no default file name is forced. This can easilly
        # be changed in your program. This is an 'open' dialog, and allows multitple
        # file selections as well.
        #
        # Finally, if the directory is changed in the process of getting files, this
        # dialog is set up to change the current working directory to the path chosen.
        dlg = wx.FileDialog(
            self, message="Choose a file",
            defaultDir=os.getcwd(), 
            defaultFile="",
            wildcard=wildcard,
            style=wx.OPEN | wx.MULTIPLE | wx.CHANGE_DIR
            )

        # Show the dialog and retrieve the user response. If it is the OK response, 
        # process the data.
        if dlg.ShowModal() == wx.ID_OK:
            # This returns a Python list of files that were selected.
            paths = dlg.GetPaths()

            self.log.WriteText('You selected %d files:' % len(paths))

            for path in paths:
                self.log.WriteText('           %s\n' % path)

        # Compare this with the debug above; did we change working dirs?
        self.log.WriteText("CWD: %s\n" % os.getcwd())

        # Destroy the dialog. Don't do this until you are done with it!
        # BAD things can happen otherwise!
        dlg.Destroy()




#---------------------------------------------------------------------------

    def OnButton(self, evt):
        wildcard = "OSG ascii source (*.osg)|*.osg|"     \
           "OSG binary  (*.ive)|*.ive|" \
           "All files (*.*)|*.*"

        #self.log.WriteText("CWD: %s\n" % os.getcwd())

        # Create the dialog. In this case the current directory is forced as the starting
        # directory for the dialog, and no default file name is forced. This can easilly
        # be changed in your program. This is an 'open' dialog, and allows multitple
        # file selections as well.
        #
        # Finally, if the directory is changed in the process of getting files, this
        # dialog is set up to change the current working directory to the path chosen.
        dlg = wx.FileDialog(
            self, message="Choose a file",
            defaultDir=os.getcwd(), 
            defaultFile="",
            wildcard=wildcard,
            style=wx.OPEN | wx.MULTIPLE | wx.CHANGE_DIR
            )

        # Show the dialog and retrieve the user response. If it is the OK response, 
        # process the data.
        if dlg.ShowModal() == wx.ID_OK:
            # This returns a Python list of files that were selected.
            paths = dlg.GetPaths()

            self.log.WriteText('You selected %d files:' % len(paths))

            for path in paths:
                self.log.WriteText('           %s\n' % path)

        # Compare this with the debug above; did we change working dirs?
        self.log.WriteText("CWD: %s\n" % os.getcwd())

        # Destroy the dialog. Don't do this until you are done with it!
        # BAD things can happen otherwise!
        dlg.Destroy()


viewer = osgViewer.Viewer()
viewer.setThreadingModel(osgViewer.Viewer.SingleThreaded)

    #set the scene data
    #n = osgDB.readNodeFile('cow.osg')   
root = osg.Group()
#n = osgDB.readNodeFile("D:/models/demo_virtrome/piante/frassino.ive")
#n = osgDB.readNodeFile("Q:/virtualrome/web/modelli/romano/villa/prove_stanze/tmp/prove_stanze_mod.osg")
n = osgDB.readNodeFile("D:/models/prove/bump_mapping_gaiani_e_marco/prove_bump_cleaned/schiavo_marco/prova2.osg")
#n = osgDB.readNodeFile("Q:/vrome/web/modelli/attuale/generati/Malborghetto_attuale_osg/osg_exp_tiny_full.ive")
root.addChild(n)
viewer.setSceneData(root.__disown__())

pickhandler = pick_module.PickHandler()

viewer.addEventHandler(pickhandler.__disown__());

OpenConsole(viewer)

