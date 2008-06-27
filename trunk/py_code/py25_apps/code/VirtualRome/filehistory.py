import os
import sys
import wx
import config

'''
File and Url history support:

See the example for the usage
'''

class FileHistory(object):
    ''' - non ho provato con il wxFileHistory perche' ha max 10 entry e non supporta le URL
        - ho provato con il Pickle -- ma non mi piace perche il file non e' leggibilie/editabile
        - ho provato a salvare come codice py --- ma tutto si incasina perche vorrei files e hystory in un file solo
        - se uso gli ini file posso storare tutti i settings dell'applicazione in un file solo 
        
        - la pippa degli ini-file e' che non si possono rimuovere le entrate
          allora fissiamo un numero massimo di entrate (20 files e 20 url)
          e quando salvo le sovrascrivo tutte (cosi azzero quelle non piu valide)

        - cambio ancora sistema:
          serializzo files e urls trasformandole ciascuna in una stringa

        - appena finito, vedo che il config mi serve anche dal url_dialog, -> 
          quindi sposto il config-object in un modulo globale all'applicazione
    '''
    
    def __init__(self, win, menu, menuEntryPoint=-2):
        '''
            win  ------------ the window with the Menubar.
                              she is supposed to have two methods named 'OpenFile(file)' and 'OpenUrl(url)'
            menu ------------ the win file menu
            menuEntryPoint -- position in the menu where to insert the fileHistoryMenuItems - counting from the last
                              -2 is appropriate when the menu 'File' ends with a separator + the Exit item '''

        self.maxid = 20
        self.files = []
        self.urls  = []

        self.config = config.Get()
        
        self.firstFileID = wx.ID_HIGHEST 
        self.firstUrlID  = wx.ID_HIGHEST + self.maxid
        
        self.win = win
        self.menuEntryPoint = menuEntryPoint
        self.menu = menu
        self.menuItems = []

        self.load()
        self.updateMenu()
        
        self.win.Bind(wx.EVT_MENU_RANGE, self.onFile, id=self.firstFileID, id2=self.firstFileID+self.maxid)
        self.win.Bind(wx.EVT_MENU_RANGE, self.onUrl,  id=self.firstUrlID,  id2=self.firstUrlID+self.maxid)

    def load(self):
        val = self.config.Read('fileHistory', "[]") 
        self.files = []
        try:
            self.files = eval(val)
        except:
            print 'error initializing filehistory'
            
        val = self.config.Read('urlHistory', "[]") 
        self.urls = []
        try:
            self.urls = eval(val)
        except:
            print 'error initializing urlhistory'

    def save(self):
        self.config.Write('fileHistory', str( self.files))
        self.config.Write('urlHistory',  str( self.urls))
        self.config.Flush()
    
    def addFile(self,filename):
        if filename in self.files:
            self.files.remove(filename)
        # inserisco in testa
        self.files.insert(0,filename)
        # taglio la lista se e' diventata troppo lunga
        self.files = self.files[:self.maxid]
        self.save()
        self.updateMenu()
    
    def addUrl(self,url):
        if url in self.urls:
            self.urls.remove(url)
        # inserisco in testa - i file usati piu di recente vanno per primi
        self.urls.insert(0,url)
        # taglio la lista se e' diventata troppo lunga
        self.urls = self.urls[:self.maxid]
        self.save()
        self.updateMenu()

    def removeFile(self,filename):
        if filename in self.files:
            self.files.remove(filename)
            self.save()
            self.updateMenu()

    def removeUrl(self,url):
        if url in self.urls:
            self.urls.remove(url)
            self.save()
            self.updateMenu()

    def updateMenu(self):
        for mi in self.menuItems:
            self.menu.Delete(mi.GetId())
        self.menuItems = []

        # inserisco sempre in menuEntryPoint --- 

        # separator
        if len(self.files):
            pos = self.menu.GetMenuItemCount() + self.menuEntryPoint
            mi = self.menu.InsertSeparator(pos)
            self.menuItems.append(mi)
            
        #files
        c=0
        for f in self.files:
            pos = self.menu.GetMenuItemCount() + self.menuEntryPoint
            filepath = f
            filename = os.path.basename(f)
            mi = self.menu.Insert(pos, self.firstFileID+c, filename, filepath )
            self.menuItems.append(mi)
            c+=1

        # separator
        pos = self.menuEntryPoint
        if len(self.urls):
            pos = self.menu.GetMenuItemCount() + self.menuEntryPoint
            mi = self.menu.InsertSeparator(pos)
            self.menuItems.append(mi)
        #urls
        c=0
        for u in self.urls:
            pos = self.menu.GetMenuItemCount() + self.menuEntryPoint
            mi = self.menu.Insert(pos, self.firstUrlID+c, u, u )
            self.menuItems.append(mi)
            c+=1
        
    def onFile(self,event):
        id = event.GetId() - self.firstFileID
        if id<0: return 
        if id>=len( self.files): return 
        file = self.files[id]
        print 'onFile',file
        try:
            self.win.OpenFile(file)
        except:
            pass
    
    def onUrl(self,event):
        id = event.GetId() - self.firstUrlID
        if id<0: return 
        if id>=len( self.urls): return 
        url = self.urls[id]
        print 'onUrl',url
        try:
            self.win.OpenUrl(url)
        except:
            pass
        
#------------------------------------------------------------------------------
if __name__ == "__main__":

    #--------------------
    class Frame(wx.Frame):
        def __init__(self, parent=None, ID=-1, title='test file History' ):
            wx.Frame.__init__(self, parent, ID, title, size=wx.Size(800,600))

            self.CreateStatusBar()

            menuBar = wx.MenuBar()
            self.SetMenuBar(menuBar)
            menu = wx.Menu()
            menuBar.Append( menu, 'File' )
            mi  = menu.Append(-1, "Open", "Open")
            mi  = menu.Append(-1, "Save", "Save")
            sep = menu.AppendSeparator()
            mi  = menu.Append(-1, "Quit", "Quit")
            
            # create the file history
            self.file_history = FileHistory(self,menu)

        def OpenFile(file):
            print 'OpenFile',file
            # on success do also :
            self.file_history.addFile(file)
    
        def OpenUrl(url):
            print 'OpenUrl',file
            # on success do also :
            self.file_history.addUrl(url)

    #--------------------
    class App(wx.App):
        def __init__(self):
            wx.App.__init__(self,0)
        def OnInit(self):
            self.frame = Frame()
            self.SetTopWindow(self.frame)
            self.frame.Show(True)
            return True
    
    app = App()
    
    print config.configfile
    
    fh = app.frame.file_history
    
    # Add some Values
##    import glob
##    files = glob.glob('*.py')
##    files.sort(reverse=True)
##    for f in files:
##        fh.addFile(f)
##    fh.addUrl('www.abc.com')
##    fh.addUrl('www.def.com')
    
    import console
    f = console.Console(app.frame)
    
    app.MainLoop()
