
import wx
import wx.py 
import wx.glcanvas

from osg_setup import *
import osg
import osgDB
import osgGA
import osgViewer

class TestFrame(wx.Frame):
    def __init__(self, parent=None, ID=-1, title='pyOSG' ):
        wx.Frame.__init__(self, parent, ID, title)


        #-- Menu---------------------------------------------

        menuBar = wx.MenuBar()
        menu1 = wx.Menu()
        menu1.Append(101, "Open", "ciao bello")
        menu1.Append(102, "Exit", "")
        menuBar.Append(menu1, "File")
        self.SetMenuBar(menuBar)

        self.Bind(wx.EVT_MENU, self.FileOpen, id=101)
        self.Bind(wx.EVT_MENU, self.FileExit, id=102)

        #-- shell---------------------------------------------
        self.SetSize(wx.Size(800,600))
        self.shell = wx.py.shell.Shell(self, -1, introText="",style =wx.NO_BORDER)
        
        # --- Timer ---------------------------------
        self.timer = wx.Timer(self,-1)
        

        # --- Bindings ---------------------------------
        self.Bind(wx.EVT_CLOSE, self.OnClose)
        self.Bind(wx.EVT_TIMER, self.OnTimer, id=self.timer.GetId() )
        self.viewer = None
        
    def SetViewer(self,viewer):
        self.viewer = viewer
        self.timer.Start(1) 
        
    def OnClose(self, event):
        self.close()
    def close(self):
        self.viewer.getviewer().setSceneData(None)
        self.Destroy()
        
    def OnTimer(self,e):
        #global viewer    
        if self.viewer.getviewer() is None: return
        if self.viewer.getviewer().done(): self.close()
        if not self.viewer.getviewer().isRealized(): self.viewer.getviewer().realize()
        self.viewer.getviewer().frame()
        
    def FileOpen(self, event):
        print "file open"
       
        wildcard = "OSG ascii source (*.osg)|*.osg|"     \
           "OSG binary  (*.ive)|*.ive|" \
           "All files (*.*)|*.*"
    
        dlg = wx.FileDialog(
            self, message="Choose a file",
            defaultDir=os.getcwd(), 
            defaultFile="",
            wildcard=wildcard,
            style=wx.OPEN | wx.MULTIPLE | wx.CHANGE_DIR
            )

        if dlg.ShowModal() == wx.ID_OK:
            if self.viewer is None: 
                for path in dlg.GetPaths():
                    print "viewer none, skipping open -->" + path
            else:
                self.viewer.loadfiles(dlg.GetPaths())
                self.viewer.start()
                    
        dlg.Destroy()

    def FileExit(self, event):
        print "file exit"

#---------------------------------------------------------------------------



class TestApp(wx.App):
    def OnInit(self):
        self.Frame = TestFrame()
        self.SetTopWindow(self.Frame)
        self.Frame.Show() 
        return True
    
    def SetViewer(self,viewer):
        self.Frame.SetViewer(viewer)
   
class myviewer():
    def __init__(self):
        self._viewer  = osgViewer.Viewer()
        self._rootnode = osg.Group()
        self._viewer.setSceneData(self._rootnode.__disown__())
        self._viewer.setThreadingModel(osgViewer.Viewer.SingleThreaded)
        self._viewer.addEventHandler(osgViewer.StatsHandler())
        self._viewer.addEventHandler(osgViewer.WindowSizeHandler())
        self._viewer.setCameraManipulator(osgGA.TrackballManipulator())
        
    def init(self):
        self._viewer.init()
    def realize(self):
        self._viewer.realize()
    def frame(self):
        self._viewer.frame()
    def done(self):
        self._viewer.done()
        
    def getviewer(self):
        return self._viewer
    
    def loadfiles(self,list):
        for path in list:
            print "loading-->" + path
            n = osgDB.readNodeFile(path)
            self._rootnode.addChild(n)

def OpenConsole(viewer):
    viewer.addEventHandler(osgViewer.StatsHandler())
    
    s = osgViewer.WindowSizeHandler()
    viewer.addEventHandler(osgViewer.WindowSizeHandler())
    
    viewer.setCameraManipulator(osgGA.TrackballManipulator())
    viewer.init()
    viewer.realize()
    viewer.frame()

    app = TestApp(0)
    app.SetViewer(viewer)
    app.MainLoop()
    
   #--------------------------------------------------------------------------
if __name__ == "__main__":
