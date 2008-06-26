'''
'''
import os
import sys
import wx
import wx.py 
import wx.glcanvas
import wx.aui
import wxosgviewer

import osg
import osgDB
import osgGA
import osgViewer
import osgText

import url_chooser

import pick_module
import glob

#--------------------------------------------------------------------------
class osgscene():
    def __init__(self,viewer):
        self.viewer=viewer
        self.root_collection=osgGroup()
        self.root = self.root_collection.grpnode
        self.viewer.setSceneData(self.root.__disown__())
        self.colls=[self.root_collection]
        self.curr_coll=self.root_collection
        
        
    def write(self,filename,optionstring="noTexturesInIVEFile useOriginalExternalReferences"):
        osgDB.writeNodeFile_s(self.root,filename,optionstring)
        
    def LoadStuff(self,url):
        return(self.curr_coll.addUrl(url))
##        if(self.switcher):
##            return (self.switcher.addUrl(url),True)
##        else:
##            n = osgDB.readNodeFile(url)
##            if(n):
##                self.root.addChild(n)
##                return (url,False)
##            else:
##                return ("",False)
        
    def addcollection(self,coll):
        self.root.addChild(coll.grpnode)
        self.colls.append(coll)
        self.curr_coll=coll
        
        
##    def toggle(self,name):
##        if(self.switcher):
##            self.switcher.set(name,-1)

class osgCollection():    
    def __init__(self):
        self.components=dict()
    def setname(self,name):
        self.grpnode.setName(name)
    def addUrl(self,url,name=""):
        print "addurl ",url
        if(".py"==os.path.splitext(url)[1]):
            print "processing python file -->",url
            d={}
            d['__file__']=os.path.abspath(url)
            try:
                execfile(d['__file__'],globals(),d)
            except:
                print "some errors while executing -->",url
            if(d.has_key('loaded_node')):
                n=d['loaded_node']
            else:
                print "file -->",url,"<-- has not set the variable >>loaded_node<<"
                n=None
        else:
            n = osgDB.readNodeFile(url)
        if(n):
            self.grpnode.addChild(n,True)
            ind=self.grpnode.getChildIndex(n)
            if(name==""): 
                if(n.getName()):
                    key=n.getName()
                    count = 0
                    while(self.components.has_key(key)):
                        key=key + "_" + str(count)
                        count = count +1                        
                else:
                    key=url
            else : key=name
            self.components[key]=(n,ind,url)
            print "added key-->",key
            return key
        else:
            return ""
        
class osgGroup(osgCollection):
    def __init__(self):
        osgCollection.__init__(self)
        self.grpnode=osg.Group()
        
class osgSwitcher(osgCollection):
    def __init__(self):
        osgCollection.__init__(self)
        self.grpnode=osg.Switch()
    def set(self,key,status=-1):
        if(self.components.has_key(key)):
            (node,ind,url) = self.components[key]
            if( status== -1 ):
                curr= not self.grpnode.getChildValue(node)
            else : curr=bool(status)
            self.grpnode.setChildValue(node,curr)
        else: print "key ->",key," not found"
    
    def setsingle(self,key):
        if(self.components.has_key(key)):
            self.grpnode.setSingleChildOn(self.components[key][1])
        else: 
            print "key ->",key," not found"
    
class SwitchWidget():
    def __init__(self):
        self.curr_switch=None
        self.widget=None
    def LoadStuff(self,url):
        if(self.curr_switch):
            name=self.curr_switch.addUrl(url)
            if(name):
                self.add(name)
        
class SwitchCheck(SwitchWidget):
    def __init__(self,parent):
        self.widget = wx.CheckListBox(parent, -1, (0, 0), wx.DefaultSize, [])
        parent.Bind(wx.EVT_CHECKLISTBOX, self.handle, self.widget)
    def add(self,label):
        self.widget.AppendAndEnsureVisible(label)
    def handle(self, event):
        index = event.GetSelection()
        if(self.curr_switch):
            self.curr_switch.set(self.widget.GetString(index),self.widget.IsChecked(index))
        self.widget.SetSelection(index)
        
class SwitchRadio(SwitchWidget):
    def __init__(self,parent):
            self.widget = wx.Choice(parent, -1, (100, 50), choices =[])
            parent.Bind(wx.EVT_CHOICE, self.handle, self.widget)
    def add(self,label):
        self.widget.Append(label)
    def handle(self, event):
        if(self.curr_switch): 
            self.curr_switch.setsingle(event.GetString())
    
#--------------------------------------------------------------------------
class Frame(wx.Frame):
    def __init__(self, parent=None, ID=-1, title='pyOSG' ):
        wx.Frame.__init__(self, parent, ID, title)

        #-- Menu ---------------------------------------------
        menuBar = wx.MenuBar()
        menu1 = wx.Menu()
        
        item = menu1.Append(-1, "Open URL", "base url")
        self.Bind(wx.EVT_MENU, self.FileOpenUrl, id=item.GetId())

        item = menu1.Append(-1, "Open File", "file to load")
        self.Bind(wx.EVT_MENU, self.FileOpen, id=item.GetId())
        
        item = menu1.Append(-1, "Exit", "")
        self.Bind(wx.EVT_MENU, self.FileExit, id=item.GetId())

        menu2 = wx.Menu()
        item = menu2.Append(-1, "check", "select default check")
        self.Bind(wx.EVT_MENU, self.modecheck, id=item.GetId())
        item = menu2.Append(-1, "radio", "select default radio")
        self.Bind(wx.EVT_MENU, self.moderadio, id=item.GetId())

        menuBar.Append(menu1, "File")
        menuBar.Append(menu2, "Loadmode")
        self.SetMenuBar(menuBar)
        self.CreateStatusBar()
    
        #-- shell---------------------------------------------
        self.SetSize(wx.Size(800,600))
        self.shell = wx.py.shell.Shell(self, -1, introText="",style =wx.NO_BORDER)
        
        # --- Canvas ---------------------------------
        self.canvas = wxosgviewer.wxOsgViewer(self,-1)
        
        self.scene = osgscene(self.canvas.viewer)
        self.mainsw=osgSwitcher()
        self.radiosw=osgSwitcher()
        self.radiosw.setname("SWT_1")
        self.scene.addcollection(self.radiosw)
        self.scene.addcollection(self.mainsw)

        
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
        
        #empty fake panel
        ##self.lb = wx.CheckListBox(self, -1, (0, 0), wx.DefaultSize, [])

        ##self.Bind(wx.EVT_CHECKLISTBOX, self.EvtCheckListBox, self.lb)
        self.lb=SwitchCheck(self)
        self.lb.curr_switch = self.mainsw
        self._mgr.AddPane(self.lb.widget, wx.aui.AuiPaneInfo().CaptionVisible(True).
                        Name("fake panel").Caption("fake panel").Right().
                        BestSize(wx.Size(200,200)).MinSize(wx.Size(20,10)))

##        dummylist=[]
##        for i in range(0,9):
##            dummylist.append(str(i))
##        self.rb = wx.RadioBox(
##                self, -1, "wx.RadioBox", wx.DefaultPosition, wx.DefaultSize,
##                dummylist, 1, wx.RA_SPECIFY_COLS
##                )
##        for i in range(0,9):
##            self.rb.ShowItem(i,False)
        self.rb=SwitchRadio(self)
        self.rb.curr_switch = self.radiosw
        self._mgr.AddPane(self.rb.widget, wx.aui.AuiPaneInfo().CaptionVisible(True).
                        Name("radio panel").Caption("radio panel").Right().
                        BestSize(wx.Size(200,200)).MinSize(wx.Size(20,10)))
        ##self.Bind(wx.EVT_CHECKLISTBOX, self.EvtCheckListBox, self.rb)
        
        self.moderadio("  ")
        #self.scene.curr_coll.setname("SWT_1")
        self.modecheck("  ")
        #self.scene.curr_coll.setname("SWT_2")
        self._mgr.Update()

        # --- Bindings ---------------------------------
        #self.Bind(wx.EVT_CLOSE,             self.OnClose)
        #self.Bind(wx.EVT_MENU, self.FileOpen, id=101)
        #self.Bind(wx.EVT_MENU, self.FileExit, id=102)
        
##    def EvtCheckListBox(self, event):
##        index = event.GetSelection()
##        self.mainsw.set(self.lb.GetString(index),self.lb.IsChecked(index))
##        label = self.lb.GetString(index)
##        status = 'un'
##        if self.lb.IsChecked(index):
##            status = ''
##        print('Box %s is %schecked \n' % (label, status))
##        self.lb.SetSelection(index)    # so that (un)checking also selects (moves the highlight)
##
##    def EvtRadioBox(self, event):
##        index=event.GetInt()
##        print self.rb.GetItemLabel(index)
##        #self.radiosw.setsingle(self.rb.getLabel())
    def modecheck(self, event):
        self.active_switch=self.lb
    def moderadio(self, event):
        self.active_switch=self.rb
    def OnClose(self, event):
        self.close()

    def close(self):
        self.Destroy()

    def FileExit(self,evt):
        self.Destroy()

    def FileOpenUrl(self, event):
        dlg = url_chooser.Dlg()
        dlg.LoadHistory( "test_app", "url_history" )
        ret = dlg.ShowModal()
        if ret == wx.OK:
            print 'exited with OK'
            print dlg.GetUrl()
            self.active_switch.LoadStuff(dlg.GetUrl())
##            name=self.scene.LoadStuff(dlg.GetUrl())
##            if(name):
##                self.lb.AppendAndEnsureVisible(name)
            
            dlg.SaveHistory( "test_app", "url_history" )
        else:
            print 'exited with Cancel'
            
    def FileOpen(self, event):
        print "file open"
       
        wildcard = "OSG files (*.osg)|*.osg|"     \
           "OSG binary  (*.ive)|*.ive|" \
           "All files (*.*)|*.*"
    
        dlg = wx.FileDialog(
            self, message="Choose a file",
            defaultDir=os.getcwd(), 
            defaultFile="",
            wildcard=wildcard,
            style=wx.OPEN | wx.MULTIPLE | wx.CHANGE_DIR
            )

        if dlg.ShowModal() == wx.ID_OK:
            for path in dlg.GetPaths():
                self.active_switch.LoadStuff(path)
##                name=self.scene.LoadStuff(path)
##                if(name):
##                   self.lb.AppendAndEnsureVisible(name)
                print "loaded-->" + path
                    
        dlg.Destroy()

#--------------------------------------------------------------------------
class App(wx.App):
    def OnInit(self):
        self.frame = Frame()
        self.SetTopWindow(self.frame)
        self.frame.Show(True)
        return True

#--------------------------------------------------------------------------
if __name__ == "__main__":
    #app.frame.scene.curr_coll.setname("SWT_1")
    #app.frame.scene.write("H:/UrbanCenter/terrain/2007/prova.osg")
    #app.frame.scene.write("H:/UrbanCenter/terrain/2007/prova3.osg")
    app = App(0)  # importante: creare APP passando 0 
                  # se si crea la app con 'App()'
                  # lei si redireziona gli stdin/out 
                  # e non arriva piu niente allo stany.
    f = app.frame
    v = app.frame.canvas.viewer
    w = app.frame.canvas.gw
    
    pickhandler = pick_module.PickHandler()

    ##app.frame.canvas.viewer.addEventHandler(pickhandler.__disown__());
    app.MainLoop()

