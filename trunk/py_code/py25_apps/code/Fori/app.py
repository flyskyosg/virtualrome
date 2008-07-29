'''

'''


import os
import sys
import wx
import wx.py 
import wx.glcanvas
import wx.aui
import wx.lib.flatnotebook as fnb
import glob

import osg
import osgDB
import osgGA
import osgViewer
import osgText

import testdata
import console
import filehistory
import url_chooser
import wxosgviewer
import config
import custom_tree
import gui_support
import process_def

# ----------------------------------------------------
class appStdio:
    ''' usando questa, le print vanno nel LOG,
        ma allora non vanno nello Stany,
        puoi accenderlo o spegnerlo con redirect_stdio '''
    def __init__(self,TextCtrl):    
        self.out = TextCtrl
    
    def write(self,string):
        self.out.WriteText(string)

# mettilo a False per mandare le Print nello Stany
redirect_stdio = True
#--------------------------------------------------------------------------
class FileDropListener(wx.FileDropTarget):
    ''' Drag 'n Drop support
        puoi aprire un file/s droppandolo sulla finestra
    '''
    def __init__(self, win):
        wx.FileDropTarget.__init__(self)
        self.win = win

    def OnDropFiles(self, x, y, filenames):
        self.win.FileDrop(filenames)

#--------------------------------------------------------------------------
# questa e' la finestra principale
# dovresti metterci poco piu della creazione della GUI
# e i metodi per rispondere al menu --- tutto il resto dovrebbe stare in moduli
class Frame(wx.Frame):
    
    def AddMenu(self,label):
        menu = wx.Menu()
        self.GetMenuBar().Append( menu, label )
        return menu

    def AddMenuItem(self,menu,label,callback):
        item = menu.Append( -1, label, label)
        self.Bind(wx.EVT_MENU, callback, id=item.GetId())
        return item.GetId()
    
    def __init__(self, parent=None, ID=-1, title='Fori Processor and Viewer' ):
        wx.Frame.__init__(self, parent, ID, title)

        self.LastOpenedFile = ''
        #-- StatusBar,Menu,Toolbar and FileHistory---
        self.CreateStatusBar()

        menuBar = wx.MenuBar()
        self.SetMenuBar(menuBar)

        m = self.AddMenu('File')
        id_clear    = self.AddMenuItem( m,  "New       \t",     self.FileClear   )
        id_openfile = self.AddMenuItem( m,  "Open File \t",     self.FileOpen    )
        id_openurl  = self.AddMenuItem( m,  "Open URL  \t",     self.FileOpenUrl )
        id_reload   = self.AddMenuItem( m,  "Reload      ",     self.FileReload  )
        m.AppendSeparator()
        self.AddMenuItem( m, "Exit \tESC",  self.FileExit )

        self.fileHistory = filehistory.FileHistory(self,m)

        m = self.AddMenu('View')
        id_refreshTree = self.AddMenuItem( m, "refresh tree",                  self.refreshTree, )
        m.AppendSeparator()
        id_console = self.AddMenuItem( m, "toggle console/log panel",      self.toggleConsole, )
        id_tree    = self.AddMenuItem( m, "toggle tree panel",             self.toggleTree, )
        id_gui     = self.AddMenuItem( m, "toggle object property panel",  self.toggleGui, )

        # i bottoni della toolbar chiamano le stesse callback dei corrispondenti comandi di menu
        tb = self.CreateToolBar()
        tb.SetToolBitmapSize(wx.Size(16,16))
        tb.AddTool(id_clear,     wx.Bitmap('pic/new.png')      ,shortHelpString='drop all loaded files')
        tb.AddTool(id_openfile,  wx.Bitmap('pic/open.png')     ,shortHelpString='open file')
        tb.AddTool(id_openurl,   wx.Bitmap('pic/open_url.png') ,shortHelpString='open url')
        tb.AddSeparator()
        tb.AddTool(id_reload,    wx.Bitmap('pic/reload.png')   ,shortHelpString='reload all')
        tb.AddSeparator()
        tb.AddTool(id_console,   wx.Bitmap('pic/console.png')  ,shortHelpString='toggle console/log panel')
        tb.AddTool(id_tree,      wx.Bitmap('pic/tree.png')     ,shortHelpString='toggle tree panel')
        tb.AddTool(id_gui,       wx.Bitmap('pic/gui.png')      ,shortHelpString='toggle object property panel')
        tb.Realize()

        #-- Drag 'n Drop support---
        self.SetDropTarget( FileDropListener(self) )
        
        #-- shell e Log ------------------
        # nb1 == contenitore 'tabbed' 
        fnb_style = fnb.FNB_BOTTOM | fnb.FNB_NO_X_BUTTON | fnb.FNB_ALLOW_FOREIGN_DND
        self.nb1 = fnb.FlatNotebook(self, -1, style = fnb_style )
        
        self.shell = console.Console(self.nb1) 
        self.nb1.AddPage(self.shell,'shell')

        global redirect_stdio
        if redirect_stdio:
            self.log = wx.TextCtrl(self.nb1,-1, style=wx.NO_BORDER | wx.TE_MULTILINE)        
            self.ch  = appStdio( self.log )
            sys.stdout = self.ch 
            sys.stderr = self.ch 
            self.nb1.AddPage(self.log,  'log'  )

        # --- Canvas --------------
        self.canvas = wxosgviewer.Canvas(self,-1)
        self.root = self.canvas.root
        
        # --- tree e gui-----------
        self.tree = custom_tree.CustomTree(self,-1,self)
        self.gui = gui_support.GuiHolder(self)
        
        # --- Layout using AUI ----
        self.SetSize(wx.Size(600,600))
        self._mgr = wx.aui.AuiManager()
        self._mgr.SetManagedWindow(self)
        self._mgr.GetArtProvider().SetColor(wx.aui.AUI_DOCKART_INACTIVE_CAPTION_COLOUR, wx.Color(0,150,255)) 
        self._mgr.GetArtProvider().SetColor(wx.aui.AUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR, wx.Color(0,80,255)) 
        self._mgr.GetArtProvider().SetColor(wx.aui.AUI_DOCKART_INACTIVE_CAPTION_COLOUR, wx.Color(80,150,255)) 
        self._mgr.GetArtProvider().SetColor(wx.aui.AUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR, wx.Color(80,150,255)) 
                                              
        self._mgr.AddPane(self.canvas, wx.aui.AuiPaneInfo().Name("CenterPane").CenterPane())

        self._mgr.AddPane(self.tree, wx.aui.AuiPaneInfo().
                        Name("tree").Caption("Tree").Right().
                        BestSize(wx.Size(350,100)).MinSize(wx.Size(350,100)).Layer(1))

        self._mgr.AddPane(self.gui, wx.aui.AuiPaneInfo().
                        Name("gui").Caption("Selected Object Property").Right().
                        BestSize(wx.Size(100,100)).MinSize(wx.Size(100,100)).Layer(1))

        self._mgr.AddPane(self.nb1, wx.aui.AuiPaneInfo().CaptionVisible(True).
                        Name("shell").Caption("shell and log").Bottom().
                        BestSize(wx.Size(200,200)).MinSize(wx.Size(20,10)).Layer(0))
 
        self.SetSize(wx.Size(800,600))
        
        self.toggleTree(0)
        self.toggleGui(0)
        self._mgr.Update()


    def toggleConsole(self,event):
        pi = self._mgr.GetPane(self.nb1)
        if pi.IsOk():
            pi.Show( not pi.IsShown() )
            self._mgr.Update()

    def toggleTree(self,event):
        pi = self._mgr.GetPane(self.tree)
        if pi.IsOk():
            pi.Show( not pi.IsShown() )
            self._mgr.Update()

    def toggleGui(self,event):
        pi = self._mgr.GetPane(self.gui)
        if pi.IsOk():
            pi.Show( not pi.IsShown() )
            self._mgr.Update()

    def ResetCamera(self):
        self.canvas.ResetCamera()

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
        wildcard = "OSG files   (*.osg;*.ive)|*.osg;*.ive|"\
                   "OSG ascii   (*.osg)|*.osg|"\
                   "OSG binary  (*.ive)|*.ive|" \
                   "All files   (*.*)|*.*"
        
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

    def FileDrop(self,files):
        for f in files:
            if self.OpenFile(f): self.fileHistory.addFile(f)

    #  aggiungi il tuo codice a queste funzioni 
    #  nelle precedenti non dovresti avere bisogno di entrarci


    def OpenUrl(self,url):
        ''' devi definire questa funzione perche e' invocata anche dalla History 
            devi ritornare True se la url e' stata caricata con successo '''
        print 'OpenUrl',file
        return True

    def FileReload(self,event):
        ''' ricaricare tutto x vedere eventuali modifiche '''
        print 'FileReload'
        if self.LastOpenedFile:
            self.OpenFile(self.LastOpenedFile)

    def FileClear(self,event):
        ''' scaricare tutto '''
        print 'FileClear'
        self.root.removeChildren (0, self.root.getNumChildren() )

    def OnSelect(self,obj):
        ''' qualcosa e' stato sleezionato nel tree
            puo essere un file, un settings, una root, o un nodo '''

        #print 'OnSelect', obj
        # dai l'oggetto selezionato al GuiHolder, lui sa cosa fare :-)
        self.gui.SetObj(obj)
        # dopo self.gui.SetObj(obj) chiama sempre questo
        self._mgr.Update()

    def OpenFile(self,file):
        ''' devi definire questa funzione perche e' invocata anche dalla History 
            devi ritornare True se il file e' stato caricato con successo '''

        name = os.path.basename(file)
        ext  = name.split('.')[1]
        ext = ext.upper()

        print 'Loading:', file, ext

        if ext == "DEFINITION":
            p = process_def.ProcessDef(file)
            LoadedModel = osgDB.readNodeFile( p.GetIveRoot() )
        else:
            LoadedModel = osgDB.readNodeFile(file)

        if not LoadedModel:
            print 'Loading failed'
            return False

        self.root.removeChildren( 0 , self.root.getNumChildren() )
        self.root.addChild(LoadedModel)
        #self.tree.ReadSceneGraph(self.root)
        self.ResetCamera()
        self.LastOpenedFile = file
        print 'Loading terminated'
        return True

    def refreshTree(self, event):
        self.tree.ReadSceneGraph(self.root)

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
    
    
    
    # shortcut per l'uso nella console
    f = app.frame
    v = app.frame.canvas.viewer
    w = app.frame.canvas.gw
    
    # basta mucche
    teapot = testdata.dir + 'teapot.ive'
    f.OpenFile( teapot )

##    dir = testdata.dir + 'sil\\'
##    file = dir + 'istanze.osg'
##    root = osgDB.readNodeFile( file )
##    if not root :
##        print " *** failed to open : ", file
##        exit
##
##    import process_instances
##    from process_instances import ProcessInstances
##    process_instances.ModelsDir = dir
##    root = ProcessInstances( root )
##
##    f.canvas.viewer.setSceneData(root)

    app.MainLoop()



