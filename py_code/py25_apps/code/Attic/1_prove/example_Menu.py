
import wx
import wx.py 

#--------------------------------------------------------------------------
if __name__ == "__main__":

    #---------------------------------
    class testFrame(wx.Frame):
        def __init__(self, parent=None, ID=-1, title='config test' ):
            wx.Frame.__init__(self, parent, ID, title)

##            #-- Menu MODO 1 ---------------------------------------------
##            
##            menuBar = wx.MenuBar()
##            menu1 = wx.Menu()
##
##            FileOpenUrlItem = menu1.Append(-1, "Open Url", "open an URL")
##            FileExitItem    = menu1.Append(-1, "Exit", "")
##
##            menuBar.Append(menu1, "File")
##            self.SetMenuBar(menuBar)
##            self.CreateStatusBar()
##            
##            self.Bind(wx.EVT_MENU, self.FileOpenUrl, id=FileOpenUrlItem.GetId() )
##            self.Bind(wx.EVT_MENU, self.FileExit,    id=FileExitItem.GetId() )

            #-- Menu MODO 2 ---------------------------------------------
            
            menuBar = wx.MenuBar()
            menu1 = wx.Menu()

            item = menu1.Append(-1, "Open Url", "open an URL")
            self.Bind(wx.EVT_MENU, self.FileOpenUrl, id=item.GetId() )

            item = menu1.Append(-1, "Exit", "")
            self.Bind(wx.EVT_MENU, self.FileExit, id=item.GetId() )

            menuBar.Append(menu1, "File")
            self.SetMenuBar(menuBar)
            self.CreateStatusBar()

        def FileOpenUrl(self, event):
            print  "OpenURL"
            
        def FileExit(self, event):
            print  "FileExit"

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

