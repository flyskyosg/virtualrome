'''
'''

import wx
import wx.py 
import wx.glcanvas
import wx.aui
import wxosgviewer

import osg
import osgDB
import osgGA
import osgViewer

#--------------------------------------------------------------------------
class Frame(wx.Frame):
    def __init__(self, parent=None, ID=-1, title='pyOSG' ):
        wx.Frame.__init__(self, parent, ID, title)

        #-- Menu ---------------------------------------------
        menuBar = wx.MenuBar()
        menu1 = wx.Menu()
        menu1.Append(101, "Open", "ciao bello")
        menu1.Append(102, "Exit", "")
        menuBar.Append(menu1, "File")
        self.SetMenuBar(menuBar)
        self.CreateStatusBar()
    
        #-- shell---------------------------------------------
        self.SetSize(wx.Size(800,600))
        self.shell = wx.py.shell.Shell(self, -1, introText="",style =wx.NO_BORDER)
        
        # --- Canvas ---------------------------------
        self.canvas = wxosgviewer.wxOsgViewer(self,-1)
        
        # --- sizing using AUI ---------------------------------
        self.SetSize(wx.Size(600,600))
        self._mgr = wx.aui.AuiManager()
        self._mgr.SetManagedWindow(self)
        self._mgr.GetArtProvider().SetColor(wx.aui.AUI_DOCKART_INACTIVE_CAPTION_COLOUR, wx.Color(0,150,255)) 
        self._mgr.GetArtProvider().SetColor(wx.aui.AUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR, wx.Color(0,80,255)) 
        self._mgr.GetArtProvider().SetColor(wx.aui.AUI_DOCKART_INACTIVE_CAPTION_COLOUR, wx.Color(80,150,255)) 
        self._mgr.GetArtProvider().SetColor(wx.aui.AUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR, wx.Color(80,150,255)) 
                                              
        self._mgr.AddPane(self.canvas, wx.aui.AuiPaneInfo().Name("CenterPane").CenterPane())

        self._mgr.AddPane(self.shell, wx.aui.AuiPaneInfo().CaptionVisible(True).
                        Name("shell").Caption("shell").Bottom().
                        BestSize(wx.Size(200,200)).MinSize(wx.Size(20,10)))
        
        #empty fake panel
        self.p = wx.Panel(self,-1)
        self._mgr.AddPane(self.p, wx.aui.AuiPaneInfo().CaptionVisible(True).
                        Name("fake panel").Caption("fake panel").Right().
                        BestSize(wx.Size(200,200)).MinSize(wx.Size(20,10)))

        self._mgr.Update()

        # --- Bindings ---------------------------------
        self.Bind(wx.EVT_CLOSE,             self.OnClose)
        #self.Bind(wx.EVT_MENU, self.FileOpen, id=101)
        #self.Bind(wx.EVT_MENU, self.FileExit, id=102)
    
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

