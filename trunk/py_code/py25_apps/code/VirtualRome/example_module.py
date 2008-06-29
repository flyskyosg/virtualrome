'''
Tu dovresti fare i tuoi moduli cosi:

cioe delle classi con un metodo MakeGui 
che ritorna una GUI
la Gui viene creata quando l'oggetto viene selezionato 


Da qualche parte (in app ?) devi poi 
1)- creare questo oggetto
2)- inserirlo nel Tree
'''

import wx
from gui_support import Gui, GuiHolder

#---------------------------------------------------------------------------
class TestObject(object):
    ''' esempio di oggetto con GUI 
        supponiamo che rappresenta la Camera
    '''

    def __init__(self):
        self.parallel = False
        self.fov = 30
        self.pos = (10,10,10)
        self.eye = (0,0,0)
        self.name = 'camera1'

    def SetPosX(self,val):
        x,y,z = self.pos
        x = val
        self.pos = (x,y,z)

    def SetPosY(self,val):
        x,y,z = self.pos
        y = val
        self.pos = (x,y,z)

    def SetPosZ(self,val):
        x,y,z = self.pos
        z = val
        self.pos = (x,y,z)

    def SetEyeX(self,val):
        x,y,z = self.eye
        x = val
        self.eye = (x,y,z)

    def SetEyeY(self,val):
        x,y,z = self.eye
        y = val
        self.eye = (x,y,z)

    def SetEyeZ(self,val):
        x,y,z = self.eye
        z = val
        self.eye = (x,y,z)

    def SetParallel(self,val):
        self.parallel = val

    def SetFov(self,val):
        self.fov = val

    def ResetCamera(self):
        self.pos = (10,10,10)
        self.eye = (0,0,0)
        # qui dovresti provocare una Select-Myself
        # cosi l'interfaccia viene rigenerata con i valori aggiornati

    def SetName(self, value):
        self.name = value

    def MakeGui(self, parent):
        ''' esempio di come si crea una GUI '''

        gui = Gui(parent)
        gui.Label   ( 'camera settings:')
        gui.Line()
        gui.Text    ( 'name',  self.name,     self.SetName)
        gui.CheckBox( 'parallel projection',  self.parallel, self.SetParallel )
        gui.Integer ( 'fov',    self.fov,     self.SetFov )    
        gui.Line()
        gui.Float   ( 'pos x',  self.pos[0],  self.SetPosX )
        gui.Float   ( 'pos y',  self.pos[1],  self.SetPosY )
        gui.Float   ( 'pos z',  self.pos[2],  self.SetPosZ )
        gui.Line()
        gui.Float   ( 'eye x',  self.eye[0],  self.SetEyeX )
        gui.Float   ( 'eye y',  self.eye[1],  self.SetEyeY )
        gui.Float   ( 'eye z',  self.eye[2],  self.SetEyeZ )
        gui.Line()
        gui.Button  ( 'reset camera',         self.ResetCamera )

        # chiamare sempre Finalize alla fine
        gui.Finalize()
        return gui

#---------------------------------------------------------------------------
class TestFileObject(object):
    ''' esempio di oggetto con GUI 
        supponiamo che rappresenta la Camera
    '''

    def __init__(self, file):
        self.file = file
        self.visible = True

    def Show(self,val):
        print 'Show', val
        self.visible  = val
        # trova lo switch corrispondente ed agisci

    def Discard(self):
        print 'Discard', val
        # di' alla scena di buttarlo
        # e togli anche il nodo dal Tree

    def MakeGui(self, parent):
        ''' esempio di come si crea una GUI '''
        gui = Gui(parent)
        gui.Label   ( 'file object:')
        gui.Line()
        gui.Label   ( 'name: ' + self.file)
        gui.CheckBox( 'visible',  self.visible, self.Show )
        gui.Button  ( 'discard',  self.Discard )
        gui.Finalize()
        return gui

#---------------------------------------------------------------------------

class App(wx.App):
    def OnInit(self):
        f = wx.Frame(None)
        
        o = TestObject()

        g = GuiHolder(f)
        g.SetObj(o)
        g.Fit()

        f.Show()
        self.SetTopWindow(f)
        return True
        
#---------------------------------------------------------------------------
if __name__ == '__main__':
    a = App(0)
    a.MainLoop()
    

