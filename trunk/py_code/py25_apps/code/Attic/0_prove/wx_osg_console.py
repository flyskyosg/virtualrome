
import wx
import wx.py 
import wx.glcanvas

import osg_setup
import osg
import osgDB
import osgGA
import osgViewer

class TestFrame(wx.Frame):
    def __init__(self, parent=None, ID=-1, title='pyOSG' ):
        wx.Frame.__init__(self, parent, ID, title)
        #-- shell---------------------------------------------
        self.SetSize(wx.Size(800,600))
        self.shell = wx.py.shell.Shell(self, -1, introText="",style =wx.NO_BORDER)
        
        # --- Timer ---------------------------------
        self.timer = wx.Timer(self,-1)
        self.timer.Start(1) 

        # --- Bindings ---------------------------------
        self.Bind(wx.EVT_CLOSE, self.OnClose)
        self.Bind(wx.EVT_TIMER, self.OnTimer, id=self.timer.GetId() )
        
    def SetViewer(self,viewer):
        self.viewer = viewer
    
    def OnClose(self, event):
        self.close()
    def close(self):
        self.viewer.setSceneData(None)
        self.Destroy()
        
    def OnTimer(self,e):
        #global viewer    
        if self.viewer.done(): self.close()
        if not self.viewer.isRealized(): self.viewer.realize()
        self.viewer.frame()

#---------------------------------------------------------------------------



class TestApp(wx.App):
    def OnInit(self):
        self.Frame = TestFrame()
        self.SetTopWindow(self.Frame)
        self.Frame.Show() 
        return True
    
    def SetViewer(self,viewer):
        self.Frame.SetViewer(viewer)
   

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
    