import wx
import config

            

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

        self.LoadHistory()

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

    def LoadHistory(self):
        c = config.Get()
        val = c.Read('urlHistory')
        lst = []
        try:
            lst = eval(val)
        except:
            pass
        for i in lst:
            self.lb.Append( i )

    def SaveHistory(self):
        values = []
        num = self.lb.Count
        for i in range(0,num):
            values.append( self.lb.GetString(i) )
        c = config.Get()
        c.Write( 'urlHistory', str(values) )
        c.Flush()

#--------------------------------------------------------------------------
if __name__ == "__main__":

    #---------------------------------
    class testFrame(wx.Frame):
        def __init__(self, parent=None, ID=-1, title='openURL test' ):
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
            ret = dlg.ShowModal()
            if ret == wx.OK:
                print 'exited with OK'
                print dlg.GetUrl()

                theUrlWasGood = True
                
                if theUrlWasGood:
                    dlg.SaveHistory()
            else:
                print 'exited with Cancel'

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

