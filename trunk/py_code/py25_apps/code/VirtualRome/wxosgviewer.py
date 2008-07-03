'''
wxOsgViewer: osgViewer on a wxPanel

this module also provide a simple wxOsgFrame / wxOsgApp
useful for testing
From Silvano Imboden --- CINECA 
'''

import wx
import wx.py 
import wx.glcanvas

import osg
import osgDB
import osgGA
import osgViewer

#----------------------------------------------------------------------
class Canvas(wx.glcanvas.GLCanvas):

    def __init__(self,parent,id):
        style = wx.WANTS_CHARS | wx.FULL_REPAINT_ON_RESIZE 
        wx.glcanvas.GLCanvas.__init__(self, parent, id, wx.DefaultPosition, wx.DefaultSize, style )

        # --- Timer ---------------------------------
        self.timer = wx.Timer(self,-1)
        self.timer.Start(1) 

        # --- Viewer ---------------------------------
        self.viewer = osgViewer.Viewer()
        self.viewer.addEventHandler(osgViewer.StatsHandler())

        self.gw = None
        self.gw = self.viewer.setUpViewerAsEmbeddedInWindow(0,0,800,600)
        #print self.gw

        self.viewer.setThreadingModel(osgViewer.ViewerBase.SingleThreaded)
        self.viewer.setCameraManipulator(osgGA.TrackballManipulator())

        ss = self.viewer.getCamera().getOrCreateStateSet()
        self.viewer.addEventHandler( osgGA.StateSetManipulator(ss) )

        loadedModel = osgDB.readNodeFile("cow.osg")
        if not loadedModel:
            print "could not find any cow --- please set the OSG_FILE_PATH env var"
        self.viewer.setSceneData(loadedModel)

        # --- Bindings ---------------------------------
        self.Bind(wx.EVT_TIMER,             self.onTimer, id=self.timer.GetId() )
        self.Bind(wx.EVT_ERASE_BACKGROUND,  self.onEraseBackground)
        self.Bind(wx.EVT_PAINT,             self.onPaint)
        self.Bind(wx.EVT_SIZE,              self.onSize)
        self.Bind(wx.EVT_KEY_DOWN,          self.onKeyDown)
        self.Bind(wx.EVT_KEY_UP,            self.onKeyUp)
        self.Bind(wx.EVT_MOUSE_EVENTS,      self.onMouse)

    def onTimer(self,e):
        # it inderectly calls  Paint
        self.Refresh()

    def onEraseBackground(self, evt):
        # no background clear
        pass

    def onPaint(self, evt):
        # this is mandatory in OnPaint
        dc = wx.PaintDC(self)

        # if  OpenGL contest is missing, something really bad happened
        if (self.GetContext() == 0 ) :
            print 'no context'
            return

        # OpenGL.MakeCurrent -- equivalent, we do for OSG
        self.SetCurrent()
        
        #  Realize must be done AFTER MakeCurrent -- after the first Show
        if not self.viewer.isRealized(): 
            self.viewer.addEventHandler(osgViewer.StatsHandler())
            #self.viewer.realize()
            print "viewer realized"
            
        # draw a Frame
        self.viewer.frame()
        
        # even this we do for  OSG
        self.SwapBuffers()

    def onSize(self, evt):
        w,h = self.GetClientSize()
        if self.GetParent().IsShown():
            self.SetCurrent()
        if self.gw.valid():
            self.gw.resized( 0, 0, w, h);
            self.gw.getEventQueue().windowResize(0, 0, w, h )
        evt.Skip() # let it be handled -- IMPORTANT

    def onKeyDown(self, evt):
        key = evt.GetKeyCode()
        # in wxWidgets, key is always an uppercase
        #if shift is not pressed convert to lowercase
        if key >=ord('A') and key <= ord('Z'):
            if not evt.ShiftDown():
                key += 32 
        # todo: handle CapsLock, Alt, Ctrl modifiers,   
        # todo: map Function keys, cursor keys, .... to the corresponding OSG-Representation

        #print "onKeyDown",key
        self.gw.getEventQueue().keyPress(key)
        evt.Skip()

    def onKeyUp(self, evt):
        key = evt.GetKeyCode()
        # in wxWidgets, key is always an uppercase
        #if shift is not pressed convert to lowercase
        if key >=ord('A') and key <= ord('Z'):
            if not evt.ShiftDown():
                key += 32 
        # todo: handle CapsLock, Alt, Ctrl modifiers,   
        # todo: map Function keys, cursor keys, .... to the corresponding OSG-Representation

        if key==27:
            self.viewer.setDone(True)
            self.timer.Stop() 
            self.GetParent().close()
            return
        self.gw.getEventQueue().keyRelease(key)
        evt.Skip()

    def onMouse(self, evt):
        x = evt.GetX()
        y = evt.GetY()
        if (evt.ButtonDown()):
            button = evt.GetButton()
            self.gw.getEventQueue().mouseButtonPress(x,y, button)
        elif (evt.ButtonUp()):
            button = evt.GetButton()
            self.gw.getEventQueue().mouseButtonRelease(x,y, button)
        elif (evt.Dragging()):
            self.gw.getEventQueue().mouseMotion(x,y)
            pass
        evt.Skip() # IMPORTANT -- otherwise  canvas does not get focus

#--------------------
class Frame(wx.Frame):
    def __init__(self, parent=None, ID=-1, title='pyOSG' ):
        wx.Frame.__init__(self, parent, ID, title, size=wx.Size(800,600))
        self.canvas = Canvas(self,-1)
    
    def close(self):
        self.Close()

#--------------------
class App(wx.App):
    def __init__(self):
        wx.App.__init__(self,0)
    def OnInit(self):
        self.frame = Frame()
        self.SetTopWindow(self.frame)
        self.frame.Show(True)
        return True
    def getFrame(self):
        return self.frame
    def getCanvas(self):
        return self.frame.canvas
    def getViewer(self):
        return self.frame.canvas.viewer

#--------------------------------------------------------------------------
if __name__ == "__main__":

    app = App()  
    viewer = app.getViewer()
    app.MainLoop()


