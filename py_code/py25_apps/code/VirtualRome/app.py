'''
Virtual Rome Main Window
'''
import os
import sys
import wx
import wx.py 
import wx.glcanvas
import wx.aui

import osg
import osgDB
import osgGA
import osgViewer
import osgText

import console
import filehistory
import url_chooser
import wxosgviewer
#import glob

#--------------------------------------------------------------------------
class Frame(wx.Frame):
    
    def AddMenu(self,label):
        menu = wx.Menu()
        self.GetMenuBar().Append( menu, label )
        return menu

    def AddMenuItem(self,menu,label,callback):
        item = menu.Append(-1, label, label)
        self.Bind(wx.EVT_MENU, callback, id=item.GetId())
    
    def __init__(self, parent=None, ID=-1, title='pyOSG' ):
        wx.Frame.__init__(self, parent, ID, title)

        #-- Menu and FileHistory------------------------------
        menuBar = wx.MenuBar()
        self.SetMenuBar(menuBar)

        m = self.AddMenu('File')
        self.AddMenuItem( m, "Open URL",  self.FileOpenUrl )
        self.AddMenuItem( m, "Open File", self.FileOpen    )
        m.AppendSeparator()
        self.AddMenuItem( m, "Exit",      self.FileExit    )
        self.fileHistory = filehistory.FileHistory(self,m)

        m = self.AddMenu('View')
        self.AddMenuItem( m, "Console",   self.toggleConsole )
        self.AddMenuItem( m, "SideBar",   self.toggleSideBar )

        #-- statusBar---------------------------------------------
        self.CreateStatusBar()
    
        #-- shell---------------------------------------------
        self.SetSize(wx.Size(800,600))
        self.shell = console.Console(self) 
        
        # --- Canvas ---------------------------------
        self.canvas = wxosgviewer.Canvas(self,-1)
        
        #self.scene = osgscene(self.canvas.viewer)
        #self.mainsw=osgSwitcher()
        #self.radiosw=osgSwitcher()
        #self.radiosw.setname("SWT_1")
        #self.scene.addcollection(self.radiosw)
        #self.scene.addcollection(self.mainsw)

        # --- sideBar ---------------------------------
        self.sideBar = wx.Panel(self,-1)
        
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
 
        self._mgr.AddPane(self.sideBar, wx.aui.AuiPaneInfo().CaptionVisible(True).
                        Name("sidebar").Caption("sidebar").Right().
                        BestSize(wx.Size(200,200)).MinSize(wx.Size(20,10)))
        
        self._mgr.Update()
 
    def OnClose(self, event):
        self.close()

    def close(self):
        self.Destroy()

    def FileExit(self,evt):
        self.Destroy()

    def FileOpenUrl(self, event):
        dlg = url_chooser.Dlg()
        ret = dlg.ShowModal()
        if ret == wx.OK:
            url = dlg.GetUrl()
            print url
            good = OpenUrl( url )
            if good:
                self.fileHistory.addFile(url)
        else:
            print 'canceled'
            
    def FileOpen(self, event):
        wildcard = "OSG files (*.osg)|*.osg|"\
                   "OSG binary  (*.ive)|*.ive|" \
                   "All files (*.*)|*.*"
        
        # ritrovo l'ultima directory che ho usato
        c = config.Get()
        dir = c.Read( 'fileOpenDir', os.getcwd() )

        dlg = wx.FileDialog(
            self, message="Choose a file",
            defaultDir= dir, 
            defaultFile="",
            wildcard=wildcard,
            style=wx.OPEN | wx.MULTIPLE | wx.CHANGE_DIR
            )

        if dlg.ShowModal() == wx.ID_OK:
            for path in dlg.GetPaths():
                good = self.OpenFile(path)
                if good:
                    # aggiungo il file alla history e salvo la directory
                    self.fileHistory.addFile(path)
                    dir = os.path.dirname(path)
                    c.Write('fileOpenDir',dir)
        dlg.Destroy()

    def toggleConsole(self,event):
        pass # questo lo faccio a casa
        
    def toggleSideBar(self,event):
        pass # questo lo faccio a casa


    # add Drag and Drop
    
    

    def OpenFile(file):
        ''' devi definire questa funzione perche e' invocata dalla History 
            devi ritornarte True se il file e' stato caricato con successo '''
        print 'OpenFile',file
        return True

    def OpenUrl(url):
        ''' devi definire questa funzione perche e' invocata dalla History 
            devi ritornarte True se il file e' stato caricato con successo '''
        print 'OpenUrl',file
        return True

#--------------------------------------------------------------------------
class App(wx.App):
    def OnInit(self):
        self.frame = Frame()
        self.SetTopWindow(self.frame)
        self.frame.Show(True)
        return True

#--------------------------------------------------------------------------
if __name__ == "__main__":

    app = App(0)  
    f = app.frame
    v = app.frame.canvas.viewer
    w = app.frame.canvas.gw
    app.MainLoop()

