'''
Python Console:

- to embedd it in another window do:
    import console
    c = console.Console( your-window )
    
- to create it as a Frame do:
    import console
    f = console.Frame()
    c = f.console

- to have it as an wx.App:
    import console
    app = console.App()
    c = app.getConsole
'''

import wx
import wx.py 

#--------------------
class Console(wx.py.shell.Shell):
    def __init__(self,parent):
        wx.py.shell.Shell.__init__(self,parent, -1, introText="",style =wx.NO_BORDER)

#--------------------
class Frame(wx.Frame):
    def __init__(self, parent=None, ID=-1, title='pyOSG' ):
        wx.Frame.__init__(self, parent, ID, title, size=wx.Size(800,600))
        self.console = Console(self)

    def close(self):
        self.Close()

#--------------------
class App(wx.App):
    def __init__(self):
        wx.App.__init__(self,0)
    def OnInit(self):
        self.frame = Frame()
        self.SetTopWindow(self.frame)
        self.frame.Show(True)
        return True
    def getFrame(self):
        return self.frame
    def getConsole(self):
        return self.frame.console

#--------------------------------------------------------------------------
if __name__ == "__main__":

    # creation mode 1 (console as a whole app)
    app = App()  # importante: creare APP passando 0 
    console1 = app.getConsole()
    
    # creation mode2 (console as a Frame)
    f = Frame()
    f.Show(True)
    f.SetTitle('console2')
    console2 = f.console
    
    # creation mode3 (console embedded in your frame)
    f2 = wx.Frame(None,-1)
    f2.SetTitle('console3')
    console3 = Console(f2)
    f2.Show(True)
    
    app.MainLoop()


