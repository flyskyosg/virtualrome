import os
import sys
import wx

class FileHistory(object):
    ''' - non ho provato con il wxFileHistory perche' ha max 10 entry e non supporta le URL
        - ho provato con il Pickle -- ma non mi piace perche il file non e' leggibilie/editabile
        - ho provato a salvare come codice py --- ma tutto si incasina perche vorrei files e hystory in un file solo
        - se uso gli ini file posso storare tutti i settings dell'applicazione in un file solo 
        
        - la pippa degli ini-file e' che non si possono rimuovere le entrate
          allora fissiamo un numero massimo di entrate (20 files e 20 url)
          e quando salvo le sovrascrivo tutte
    '''
    
    def __init__(self, win, menu, menuEntryPoint):
        '''
            win  ------------ the window with the Menubar.
                              she is supposed to have two methods named 'OpenFile(file)' and 'OpenUrl(url)'
            menu ------------ the win file menu
            menuEntryPoint -- position in the menu where to insert the fileHistoryMenuItems '''

        self.maxid = 20
        self.files = {}
        self.urls = {}

        import __main__
        appname = __main__.__file__
        appdir = os.path.dirname(appname)
        self.configfile = appdir + '\\' + 'virtualrome.ini'
        self.configfile = self.configfile.replace('\\','/')
        #self.configfile = 'virtualrome'
        
        print self.configfile
        if not os.path.exists(self.configfile):
            pass
            #print 'forcing the creation of the config file'
            #f = open(self.configfile,'w')
            #f.close()
            #self.save()
        
        self.config = wx.FileConfig(localFilename=self.configfile, style=wx.CONFIG_USE_LOCAL_FILE)
        
        self.firstFileID = wx.ID_HIGHEST 
        self.firstUrlID  = wx.ID_HIGHEST + self.maxid
        
        self.win = win
        self.menuEntryPoint = menuEntryPoint
        self.menu = menu
        self.menuItems = []

        self.load()
        
        self.win.Bind(wx.EVT_MENU_RANGE, self.onFile, id=self.firstFileID, id2=self.firstFileID+self.maxid)
        self.win.Bind(wx.EVT_MENU_RANGE, self.onUrl,  id=self.firstUrlID,  id2=self.firstUrlID+self.maxid)

    def load(self):
        #config = wx.FileConfig.Get()
        #config = wx.FileConfig(self.configfile, style=wx.CONFIG_USE_LOCAL_FILE)
        self.files = []
        for i in range(0,self.maxid):
            key = 'file' + str(i)
            val = self.config.Read(key, "") 
            if val != "":
                if os.path.exists( val ):
                    self.files.append(val)
        self.urls = []
        for i in range(0,self.maxid):
            key = 'url' + str(i)
            val = self.config.Read(key, "") 
            if val != "":
                self.urls.append(val)

    def save(self):
        ''' devo scrivere tutte le maxid entrate 
            per essere certo di eliminare eventuali entrate non piu valide '''
        #config = wx.FileConfig(self.configfile, style=wx.CONFIG_USE_LOCAL_FILE)
        #config = wx.FileConfig.Get()
        for i in range(0,self.maxid):
            key = 'file' + str(i)
            val = ''
            if len(self.files) > i:
                val = self.files[i]
            self.config.Write(key, val ) 

        for i in range(0,self.maxid):
            key = 'urls' + str(i)
            val = ''
            if len(self.urls) > i:
                val = self.urls[i]
            self.config.Write(key, val ) 
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
            pos = self.menu.GetMenuItemCount() -2
            mi = self.menu.InsertSeparator(pos)
            self.menuItems.append(mi)
            
        #files
        c=0
        for f in self.files:
            pos = self.menu.GetMenuItemCount() -2
            filepath = f
            filename = os.path.basename(f)
            mi = self.menu.Insert(pos, self.firstFileID+c, filename, filepath )
            self.menuItems.append(mi)
            c+=1

        # separator
        pos = self.menuEntryPoint
        if len(self.urls):
            pos = self.menu.GetMenuItemCount() -2
            mi = self.menu.InsertSeparator(pos)
            self.menuItems.append(mi)
        #urls
        c=0
        for u in self.urls:
            pos = self.menu.GetMenuItemCount() -2
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
            self.file_history = FileHistory(self,menu,3)

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
    
    fh = app.frame.file_history
    
    import glob
    files = glob.glob('*.py')
    files.sort(reverse=True)
    for f in files:
        fh.addFile(f)
    fh.addUrl('www.abc.com')
    fh.addUrl('www.def.com')
    
    import console
    f = console.Console(app.frame)
    
    app.MainLoop()
