import wx
import wx.py 

            

class Dlg(wx.Dialog):
    def __init__(self, parent=None, ID=-1, title='choose URL' ):

        wx.Dialog.__init__(self, parent, ID, title, wx.DefaultPosition, wx.Size(100,300), wx.RESIZE_BORDER | wx.CAPTION )
        
        # --- Widgets ---------------------------------
        self.lab1 = wx.StaticText(self,-1,"url:")
        self.url  = wx.TextCtrl(self,-1,"type an Url or choose one from listbox below")
        self.lab2 = wx.StaticText(self,-1,"history:")
        self.lb   = wx.ListBox(self,-1)
        
        bok   = wx.Button( self, -1, "Ok" )
        bcancel = wx.Button( self, -1, "Cancel" )

        # --- Sizers ---------------------------------
        sz1 = wx.BoxSizer(wx.HORIZONTAL)
        sz1.Add(bok, 0 )
        sz1.Add(bcancel,  0)

        sz = wx.BoxSizer(wx.VERTICAL)
        sz.Add(self.lab1, 0, wx.EXPAND | wx.ALL, 3 )
        sz.Add(self.url,  0, wx.EXPAND | wx.ALL, 3 )
        sz.Add(self.lab2, 0, wx.EXPAND | wx.ALL, 3 )
        sz.Add(self.lb ,  1, wx.EXPAND | wx.ALL, 3 )
        sz.Add(sz1 ,      0, wx.CENTRE | wx.ALL, 3 )

        self.SetSizer(sz)
        self.Fit()
        self.SetSize( wx.Size(300,400) )

        # --- Bindings ---------------------------------
        self.Bind( wx.EVT_BUTTON,         self.onOk,     id=bok.GetId())  
        self.Bind( wx.EVT_BUTTON,         self.onCancel, id=bcancel.GetId())  
        self.Bind( wx.EVT_LISTBOX,        self.onSelect, id=self.lb.GetId())  
        self.Bind( wx.EVT_LISTBOX_DCLICK, self.onDClick, id=self.lb.GetId())  


    def onOk(self,evt):
        self.EndModal(wx.OK)
        
    def onCancel(self,evt):
        self.EndModal(wx.CANCEL)

    def onSelect(self,evt):
        val = self.lb.GetStringSelection()
        self.url.SetValue(val)
        
    def onDClick(self,evt):
        self.onSelect(evt) 
        self.onOk(evt) 

    def GetUrl(self):
        return self.url.GetValue()

    def LoadHistory(self, app_name, section):
        config = wx.FileConfig(app_name)
        key = section + '/len'
        len = config.Read(key, "0") 
        len = int(len)
        for i in range(0,len):
            key = section + '/value' + str(i)
            val = config.Read(key, "") 
            if val != "":
                self.lb.Append( val )

    def SaveHistory(self, app_name, section):
        num = self.lb.Count
        values = []
        for i in range(0,num):
            values.append( self.lb.GetString(i) )
        current = self.GetUrl()
        if current not in values:
            values.insert(0, current )
            num += 1
        config = wx.FileConfig(app_name)
        key = section + '/len'
        config.Write(key, str(num) ) 
        for i in range(0,num):
            val = values[i]
            key = section + '/value' + str(i)
            config.Write(key, val)

#--------------------------------------------------------------------------
if __name__ == "__main__":

    #---------------------------------
    class testFrame(wx.Frame):
        def __init__(self, parent=None, ID=-1, title='config test' ):
            wx.Frame.__init__(self, parent, ID, title)

            #-- Menu ---------------------------------------------
            menuBar = wx.MenuBar()
            menu1 = wx.Menu()
            menu1.Append(101, "Open Url", "open an URL")
            menuBar.Append(menu1, "File")
            self.SetMenuBar(menuBar)
            self.CreateStatusBar()
            
            self.Bind(wx.EVT_MENU, self.FileOpenUrl, id=101)

        def FileOpenUrl(self, event):
            dlg = Dlg();
            dlg.LoadHistory( "test_app", "terrain_url_history" )
            ret = dlg.ShowModal()
            if ret == wx.OK:
                print 'exited with OK'
                print dlg.GetUrl()
                dlg.SaveHistory( "test_app", "terrain_url_history" )
            else:
                print 'exited with Cancel'

                
##                url = dlg.GetUrl()
##                valid = osg.Open(url)
##                if Valid:
##                    dlg.StoreHistory()
##            dlg.delete
            
    #---------------------------------
    class testApp(wx.App):
        def OnInit(self):
            self.frame = testFrame()
            self.SetTopWindow(self.frame)
            self.frame.Show(True)
            return True

    #---------------------------------
    app = testApp(0)  # importante: creare APP passando 0 
                      # se si crea la app con 'App()'
                      # lei si redireziona gli stdin/out 
                      # e non arriva piu niente allo stany.
    app.MainLoop()

