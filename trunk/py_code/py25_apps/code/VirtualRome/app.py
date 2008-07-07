'''
Virtual Rome Main Window

TODO:
ok - filehistory
ok - toggle bars
ok - drag 'n drop
ok - toolbar
ok - reload
ok - LOG
ok - GUI (da finire)

REM
- puoi chiudere tutto premendo ESC nell'OSG viewer (+ veloce)
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

import console
import filehistory
import url_chooser
import wxosgviewer
import config
import custom_tree
import gui_support

import scene_manager

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
redirect_stdio = False

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
    
    def __init__(self, parent=None, ID=-1, title='Virtual Rome Project -- OSG Processor & Preview' ):
        wx.Frame.__init__(self, parent, ID, title)

        #-- StatusBar,Menu,Toolbar and FileHistory---
        self.CreateStatusBar()

        menuBar = wx.MenuBar()
        self.SetMenuBar(menuBar)

        m = self.AddMenu('File')
        id_clear    = self.AddMenuItem( m,  "New       \t(clear)",      self.FileClear   )
        id_openfile = self.AddMenuItem( m,  "Open File \t(append)",     self.FileOpen    )
        id_openurl  = self.AddMenuItem( m,  "Open URL  \t(append)",     self.FileOpenUrl )
        id_reload   = self.AddMenuItem( m,  "Reload all ",              self.FileReload  )
        m.AppendSeparator()
        self.AddMenuItem( m, "Exit \tESC",  self.FileExit )

        self.fileHistory = filehistory.FileHistory(self,m)

        m = self.AddMenu('View')
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


    def OpenFile(self,file):
        ''' devi definire questa funzione perche e' invocata anche dalla History 
            devi ritornare True se il file e' stato caricato con successo '''
        print 'OpenFile',file
        if(not dir(self).__contains__('osg_app')):
            self.osg_app=scene_manager.app(self.canvas.viewer)
        fileObj=self.osg_app.LoadStuff(file)    
        if(fileObj):
            print 'loaded->', file, "<-name->",fileObj.key
            f.tree.AddFile(fileObj,fileObj.key)
            f.tree.ReadSceneGraph(self.osg_app.root)
            f.tree.CollapseAllChildren(f.tree.scenegraphRoot)
        else:
            print 'error loading', file
            return False


        return True

    def OpenUrl(self,url):
        ''' devi definire questa funzione perche e' invocata anche dalla History 
            devi ritornare True se la url e' stata caricata con successo '''
        print 'OpenUrl',file
        return True

    def FileReload(self,event):
        ''' ricaricare tutto x vedere eventuali modifiche '''
        print 'FileReload'

    def FileClear(self,event):
        ''' scaricare tutto '''
        print 'FileClear'

    def OnSelect(self,obj):
        ''' qualcosa e' stato sleezionato nel tree
            puo essere un file, un settings, una root, o un nodo '''

        print 'OnSelect', obj
        # dai l'oggetto selezionato al GuiHolder, lui sa cosa fare :-)
        self.gui.SetObj(obj)
        # dopo self.gui.SetObj(obj) chiama sempre questo
        self._mgr.Update()


#--------------------------------------------------------------------------
class App(wx.App):
    def OnInit(self):
        self.frame = Frame()
        self.SetTopWindow(self.frame)
        self.frame.Show(True)
        return True

#--------------------------------------------------------------------------
def defaultFilename():
    # locate the DataDir
    dir = os.getenv('DATADIR')
    if not dir:
        print 'env-var "DATADIR" not found, exiting'
        sys.exit()
    # there are good and bad data :-)
    dir = dir + 'bad\\'

    # open the test file
    filename = dir + 'f_pace.osg'
    return filename

#--------------------------------------------------------------------------
if __name__ == "__main__":

    app = App(0)  
    
    
    # shortcut per l'uso nella console
    f = app.frame
    v = app.frame.canvas.viewer
    w = app.frame.canvas.gw

    text = '''Novita:
- nel Menu File hai la fileHistory, e la urlHistory"
- le History sono scritte in VirtualRome.ini, se lo cancelli si ricrea
- nella History dovrebbero andarci solo file e url 'buone' (sta a te)
- il Dialogo FileOpen si ricorda la directory dell'ultimo file aperto
- puoi aprire uno o piu file facendo Drag'n Drop

- ho pensato che potresti fare che OpenFile e OpenURL 'aggiungono' alla scena
- e con File/New resetti la scena .... 
  ti mancherebbe il supporto per scaricare singoli pezzi, ma e' gia qualcosa
- ed ho predisposto per una funzione 'reload' che aggiorna tutti i file caricati

- ho aggiunto la Toolbar
- ho aggiunto il LOG --- per mandare messaggi subliminali ai modellatori


- ho predisposto un Tree 
  che possa ospitare Files,Settings e lo SceneGraph

- ho fatto un Visitor che riempie l'albero


- ho fatto un primo supporto alle GUI -- vedi il modulo gui_support

- ho messo un primo supporto al display delle GUI in risposta alla Selezione

- faremo di meglio quando torno

'''
    #print text


##    filename = defaultFilename()
##    f.OpenFile( filename )
    
    # modulo di esempio
    #quando lo selezioni la sua interfaccia viene creata
    import example_module
    settings1 = example_module.TestObject()
    f.tree.AddSettings(settings1,'camera settings')

    # altro modulo di esempio
    settings2 = gui_support.TestObject()
    f.tree.AddSettings(settings2,'stupid settings')

    # altro modulo di esempio
##    fileObj = example_module.TestFileObject(filename)

    app.MainLoop()

#frame.tree.selectnode(obj)
