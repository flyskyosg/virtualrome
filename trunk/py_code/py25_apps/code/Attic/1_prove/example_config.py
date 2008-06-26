import wx
import wx.py 

'''
Example of using the Config-File-support of WxWidgets
Note also that:
    keys are strings
    keys can be plain values like "test"
    keys can be path like "test/subtest/value"
    
    values can be string or numbers
'''

def read_config( app_name, key_name, default_value):
    config = wx.FileConfig(app_name);
    value = config.Read(key_name, default_value) 
    return value
    
def write_config( app_name, key_name, value):
    config = wx.FileConfig(app_name);
    config.Write(key_name, value) 
    return value


#--------------------------------------------------------------------------
if __name__ == "__main__":

    #---------------------------------
    class testFrame(wx.Frame):
        def __init__(self, parent=None, ID=-1, title='config test' ):
            wx.Frame.__init__(self, parent, ID, title)


            self.b1 = wx.Button  (self, 100, "write")
            self.t1 = wx.TextCtrl(self, 101, "value to be saved")
            self.b2 = wx.Button  (self, 102, "read")
            self.t2 = wx.TextCtrl(self, 103, "retrieved value")

            sz = wx.BoxSizer(wx.VERTICAL)
##            sz.Add(self.b1, 1, wx.EXPAND)
##            sz.Add(self.t1, 1, wx.EXPAND)
##            sz.Add(self.b2, 1, wx.EXPAND)
##            sz.Add(self.t2, 1, wx.EXPAND)

            sz.Add(self.b1, 0, wx.EXPAND)
            sz.Add(self.t1, 0, wx.EXPAND)
            sz.Add(self.b2, 0, wx.EXPAND)
            sz.Add(self.t2, 0, wx.EXPAND)
            self.SetSizer(sz)
            self.Fit()        

            # --- Bindings ---------------------------------
            self.Bind( wx.EVT_BUTTON, self.onWrite, id=100)  
            self.Bind( wx.EVT_BUTTON, self.onRead,  id=102)  
            
            
            write_config("garbage_app", "section1/", "" )
            write_config("garbage_app", "section1/list_len", "2" )
            write_config("garbage_app", "section1/list0", "value1" )
            write_config("garbage_app", "section1/list1", "value1" )
            
            len = read_config("garbage_app", "section1/list_len", "0" )
            len = int(len)
            Alist = []
            for i in range(0,len):
                key = "section1/list" + str(i)
                Alist.append( read_config("garbage_app", key, "0" ))
            print Alist

        def onWrite(self, event):
            value = self.t1.GetValue()
            write_config("garbage_app", "garbage_key", value )

        def onRead(self,event):
            value = read_config("garbage_app", "garbage_key", "0" )
            self.t2.SetValue(value)


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

