from osg_scene_helpers import *

import wx
from gui_support import Gui, GuiHolder

class app(osgSceneCollection):
    def __init__(self,viewer):
        print "osg_app init"
        osgSceneCollection.__init__(self,viewer)
        self.mainsw=osgSwitcher()
        #activate the default optimization at load
        #self.mainsw.optimize()
        self.mainsw.setname("MainDataSwitch")
        self.addcollection(self.mainsw)

    def LoadStuff(self,Url):
        ret_str=self.mainsw.addUrl(Url)
        if(ret_str==""):
            print "Url >"+Url+"< loading failed"
        else:
            print "Url >"+Url+"< loading OK name >"+ret_str+"<"
            return SwitchFileObject(self.mainsw,Url,ret_str)
        

#---------------------------------------------------------------------------
class SwitchFileObject(object):
    ''' esempio di oggetto con GUI 
        supponiamo che rappresenta la Camera
    '''

    def __init__(self, switch,file,key):
        print "creating SwitchFileObject file->",file,"<- key ->",key,"<- switch ->",switch
        self.file = file
        self.visible = True
        self.switch=switch
        self.key=key
    def Show(self,val):
        print 'Show', val
        self.visible  = val
        # trova lo switch corrispondente ed agisci
        self.switch.set(self.key,val)
    def Discard(self):
        print 'Discard', val
        # di' alla scena di buttarlo
        # e togli anche il nodo dal Tree

    def MakeGui(self, parent):
        ''' esempio di come si crea una GUI '''
        gui = Gui(parent)
        gui.Label   ( 'file object:'+self.key)
        gui.Line()
        gui.Label   ( 'name: ' + self.file)
        gui.CheckBox( 'visible',  self.visible, self.Show )
        gui.Button  ( 'discard',  self.Discard )
        gui.Finalize()
        return gui
