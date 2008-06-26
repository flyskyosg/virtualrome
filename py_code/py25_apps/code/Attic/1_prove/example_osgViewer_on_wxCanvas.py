'''

From the OSG: MailingList:

Hi Serge,

View::setUpViewerAsEmbeddedInWindow() creates a
osgViewer::GraphicsWindowEmbedded and attaches it to Viewer's master
Camera.  This GraphicsWindowEmbedded just does a non op for all the
operations like makeCurrent(), swap buffer etc.  It quite literally
fakes it, osgViewer::Viewer treats it like an ordinary window, but of
course it isn't - so there are real limitations to what you can do,
but if you use it in a restricted way it'll work just fine.

GraphicsWindowEmbedded is the trick used to allow
osgViewer::Viewer/CompositeViewer work in a way that their weren't
originally designed for. I never really expected to be able to cover
this type of embedded usage model with classes like powerful viewer
classes Viewer and CompositeViewer, but the trick allows us to get
away with reuse a great deal of the viewer functionality, and avoid
the need for extra viewer classes just to support one type of usage
model.

The restrictions associate with use GraphicsWindowEmbedded is that the
viewer must be run single threaded, and that there is only a single
GraphicsWindowEmbedded attached to the Viewer, and that the calling
application must create the graphics context itself, do make current
and swap buffers and cleanup and pass on actual window resizes.

The advantage that we gain is that you can use one viewer API to cover
the full range of viewer usage models - embedded, osgViewer inbuilt
GraphicsWindows, or API specific GraphicsWindow implementations, or
window inheritance.  The fact that its the same API means that your
app code will be easily portable between these different usage models.
 So one day you can stick your viewer in browser, the next you can do
it in cave, the next on normal desktop application, the next on
spherical display, the next on stereo monitor...

Robert.
'''

import wx
import wx.py 
import wx.glcanvas

import osg
import osgDB
import osgGA
import osgViewer

#import OpenGL.GL # - per glViewport

#long style=0, const wxString& name="GLCanvas", int* attribList = 0
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
        print self.gw

        self.viewer.setThreadingModel(osgViewer.ViewerBase.SingleThreaded)
        self.viewer.setCameraManipulator(osgGA.TrackballManipulator())

        ss = self.viewer.getCamera().getOrCreateStateSet()
        self.viewer.addEventHandler( osgGA.StateSetManipulator(ss) )

        loadedModel = osgDB.readNodeFile("cow.osg")
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
        # chiama indirettamente la Paint
        self.Refresh()

    def onEraseBackground(self, evt):
        # non cancelliamo lo sfondo
        pass

    def onPaint(self, evt):
        # questo e' obbligatorio nella OnPaint
        dc = wx.PaintDC(self)

        # se non abbimo il contesto OpenGL, qualcosa e' andato molto male
        if (self.GetContext() == 0 ) :
            print 'no context'
            return

        # OpenGL.MakeCurrent -- lo facciamo noi per OSG
        self.SetCurrent()
        
        # il Realize va posticipato dopo MakeCurrent -- che va fatto dopo il primo Show
        if not self.viewer.isRealized(): 
            self.viewer.addEventHandler(osgViewer.StatsHandler())
            #self.viewer.realize()
            print "viewer realized"
            
        # disegna un Frame
        self.viewer.frame()
        
        # anche questo lo facciamo noi per OSG
        self.SwapBuffers()

    def onSize(self, evt):
        w,h = self.GetClientSize()
        if self.GetParent().IsShown():
            self.SetCurrent()
        if self.gw.valid():
            self.gw.resized( 0, 0, w, h);
            self.gw.getEventQueue().windowResize(0, 0, w, h )
        evt.Skip() # lasciare che venga servito -- importante

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
        evt.Skip() # importante -- altrimenti il canvas non riceve il focus

#--------------------------------------------------------------------------
class Frame(wx.Frame):
    def __init__(self, parent=None, ID=-1, title='pyOSG' ):
        wx.Frame.__init__(self, parent, ID, title)

        #-- shell---------------------------------------------
        self.SetSize(wx.Size(800,600))
        self.shell = wx.py.shell.Shell(self, -1, introText="",style =wx.NO_BORDER)
        
        # --- Canvas ---------------------------------
        self.canvas = Canvas(self,-1)
        
        # --- Sizer ---------------------------------
        sz = wx.BoxSizer()
        sz.Add(self.canvas, 1, wx.EXPAND)
        sz.Add(self.shell,  1, wx.EXPAND)
        self.SetSizer(sz)
        self.Fit()        
        
        # --- Bindings ---------------------------------
        self.Bind(wx.EVT_CLOSE,             self.OnClose)
    
    def OnClose(self, event):
        self.close()

    def close(self):
        self.Destroy()

#--------------------------------------------------------------------------
class App(wx.App):
    def OnInit(self):
        self.frame = Frame()
        self.SetTopWindow(self.frame)
        self.frame.Show(True)
        return True

#--------------------------------------------------------------------------
if __name__ == "__main__":
    app = App(0)  # importante: creare APP passando 0 
                  # se si crea la app con 'App()'
                  # lei si redireziona gli stdin/out 
                  # e non arriva piu niente allo stany.
    f = app.frame
    v = app.frame.canvas.viewer
    w = app.frame.canvas.gw
    
    app.MainLoop()

