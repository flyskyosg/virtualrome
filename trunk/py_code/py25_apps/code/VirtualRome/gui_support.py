'''

'''

import os
import wx
import  wx.lib.scrolledpanel as scrolled
        
#---------------------------------------------------------------------------
class Gui(scrolled.ScrolledPanel):
    ''' gui generica con layout a griglia.
        ogni riga ha una Label ed un Controllo.
        
        hai metodi per creare:
        linee,checkbutton,combobox,textbox,spinbox,float,bottoni, ...
        ci sarebbe ancora da fare: vettori (position, ...)
        
    '''
    def __init__(self, parent):
        scrolled.ScrolledPanel.__init__(self, parent, -1)
        self.parent = parent

        p = self
        add_spacer = False
        #p.SetBackgroundColour( wx.WHITE )
        
        # sizer dei controlli (packer)
        self.sz = self.sz = wx.GridBagSizer(2,2)
        self.sz.SetEmptyCellSize( (2,2) )
        self.sz.SetHGap(1)
        self.sz.SetVGap(1)
        self.sz.AddGrowableCol(2)

        # linea di inserimento 
        self.r =1
        
        # dizionario widget-id -> widget
        self.widgets = {}

        # dizionario widget-id -> callback
        self.callbacks = {}

    def Finalize(self):
        ' chiamarla alla fine della creazione'''
        self.Fit()
        self.SetSizer(self.sz)
        self.SetScrollRate(0,1)
        self.SetupScrolling(scroll_x = False)


    # risponditori di eventi interni 
    # chiamano le tue Callback
    def OnCheck(self,e):
        widget = self.widgets[e.GetId()]
        callback = self.callbacks[e.GetId()]
        value = widget.GetValue()
        if callback:
            callback(value)

    def OnText(self,e):
        widget = self.widgets[e.GetId()]
        callback = self.callbacks[e.GetId()]
        value = widget.GetValue()
        if callback:
            callback(value)

    def OnInteger(self,e):
        widget = self.widgets[e.GetId()]
        callback = self.callbacks[e.GetId()]
        value = widget.GetValue()
        if callback:
            callback(value)

    def OnFloat(self,e):
        widget = self.widgets[e.GetId()]
        callback = self.callbacks[e.GetId()]
        value = widget.GetValue()
        try:
            num=float(value)
        except: 
            print 'error: value '+value+' is not a float number'
            return
        if callback:
            callback(num)

    def OnButton(self,e):
        callback = self.callbacks[e.GetId()]
        if callback:
            callback()

    def OnChoice(self,e):
        widget = self.widgets[e.GetId()]
        callback = self.callbacks[e.GetId()]
        value = widget.GetStringSelection()
        if callback:
            callback(value)


    # metodi per creare Widgets
    def Line(self):
        widget = wx.StaticLine(self)
        self.sz.Add( widget,(self.r,1),(1,2), flag=wx.EXPAND)
        self.r += 1

    def Label(self, name):
        widget = wx.StaticText(self,-1, name, size=(-1,18) )
        self.sz.Add( widget, (self.r,1),(1,2), flag=wx.EXPAND)
        self.r += 1

    def CheckBox(self, name, value, callback=None ):
        widget = wx.CheckBox(self,-1, name, size=(-1,18) )
        widget.SetValue( value )
        self.sz.Add( widget, (self.r,1),(1,2) ) 
        self.Bind( wx.EVT_CHECKBOX, self.OnCheck, id=widget.GetId())

        self.widgets[widget.GetId()] = widget
        self.callbacks[widget.GetId()] = callback
        self.r += 1

    def Button(self, name, callback=None ):
        widget = wx.Button(self,-1,name, size=(-1,18) )
        self.sz.Add( widget, (self.r,2),(1,1))
        self.Bind( wx.EVT_BUTTON, self.OnButton, id=widget.GetId())

        self.widgets[widget.GetId()] = widget
        self.callbacks[widget.GetId()] = callback
        self.r += 1

    def Text(self, name, value, callback=None ):
        ''' l'evento viene mandato quando premi enter '''
        label  = wx.StaticText(self,-1, name, size=(-1,-1) )
        w_style = wx.NO_BORDER | wx.TE_PROCESS_ENTER
        widget = wx.TextCtrl(self,-1, value, size=(-1,18) , style=w_style)
        self.sz.Add( label,  (self.r,1),(1,1) ) 
        self.sz.Add( widget, (self.r,2),(1,1) ) 
        self.Bind( wx.EVT_TEXT_ENTER, self.OnText, id=widget.GetId())
        self.Bind( wx.EVT_KILL_FOCUS, self.OnText, id=widget.GetId()) #non funzia

        self.widgets[widget.GetId()] = widget
        self.callbacks[widget.GetId()] = callback
        self.r += 1

    def Integer(self, name, value, callback=None ):
        ''' l'evento viene mandato quando premi enter o pigi le freccie'''
        label  = wx.StaticText(self,-1, name, size=(-1,-1) )
        w_style = wx.NO_BORDER | wx.TE_PROCESS_ENTER
        widget = wx.SpinCtrl(self,-1, str(value), size=(-1,18) , style=w_style,
                             initial=value, min=-1000000, max=1000000)
        widget.SetValue(value)
        self.sz.Add( label,  (self.r,1),(1,1) ) 
        self.sz.Add( widget, (self.r,2),(1,1) ) 
        self.Bind( wx.EVT_SPINCTRL , self.OnInteger, id=widget.GetId())

        self.widgets[widget.GetId()] = widget
        self.callbacks[widget.GetId()] = callback
        self.r += 1

    def Float(self, name, value, callback=None ):
        '''  e' uguale ad un Text 
             se ci scrivi dentro qualcosa che non e' un numero si generera' un errore.
             Per adesso stai attento, poi ci pensero'
        '''
        label  = wx.StaticText(self,-1, name, size=(-1,-1) )
        w_style = wx.NO_BORDER | wx.TE_PROCESS_ENTER
        widget = wx.TextCtrl(self,-1, str(value), size=(-1,18) , style=w_style)
        self.sz.Add( label,  (self.r,1),(1,1) ) 
        self.sz.Add( widget, (self.r,2),(1,1) ) 
        self.Bind( wx.EVT_TEXT_ENTER, self.OnFloat, id=widget.GetId())

        self.widgets[widget.GetId()] = widget
        self.callbacks[widget.GetId()] = callback
        self.r += 1

    def Combo(self, name, value, values, callback=None ):
        '''
            values = lista di stringhe
            value  = una stringa in values
        '''
        label  = wx.StaticText(self,-1, name, size=(-1,-1) )
        w_style = wx.NO_BORDER
        widget = wx.Choice(self,-1, size=(-1,18), choices=values, style = w_style )
        widget.SetStringSelection( value )
        self.sz.Add( label,  (self.r,1),(1,1) ) 
        self.sz.Add( widget, (self.r,2),(1,1) ) 
        self.Bind( wx.EVT_CHOICE , self.OnChoice, id=widget.GetId())

        self.widgets[widget.GetId()] = widget
        self.callbacks[widget.GetId()] = callback
        self.r += 1

#---------------------------------------------------------------------------

class GuiHolder(wx.Panel):
    ''' contenitore per Gui '''
    def __init__(self, parent):
        wx.Panel.__init__(self, parent, -1)

    def SetObj(self, Obj):
        ''' passare l'oggetto selezionato
            (in risposta alla OnSelect) 
        '''
        #1) ammazza la GUI precedente
        wx.SafeYield()
        self.DestroyChildren()
        wx.SafeYield()

        if Obj is None: return

        #2) lascia perdere questo
        self.sz = sz = wx.BoxSizer(wx.VERTICAL)
        self.SetSizerAndFit(sz)

        #3) creazione della GUI per l'oggetto selezionato
        # se l'oggetto selezionato ha il metodo MakeGui
        # viene invocato QUI
        if hasattr(Obj,'MakeGui'):
            self.gui = Obj.MakeGui(self)
            sz.Add(self.gui,1,wx.EXPAND)
        else:
            pass
            # vedi se e' un oggetto OSG
            # e se si crei una GUI corrispondente 
            # --- 
            # dovresti avere una Classe con un metodo MakeGui 
            # per ogni tipo di nodo OSG
            # 
            # ma non ho tempo di farlo .... :-(
        
        #4) lascia perdere 
        self.Fit()
        self.gui.Fit()

#---------------------------------------------------------------------------

class TestObject(object):
    ''' esempio di oggetto con Gui '''
    def __init__(self):
        self.value = True

    def SetValue(self,val):
        print 'SetValue', val
        self.value = val
        
    def OnButton(self):
        print 'OnButton'

    def OnText(self,text):
        print 'OnText',text

    def OnInteger(self,num):
        print 'OnInteger',integer

    def OnFloat(self,num):
        print 'OnFloat',num

    def OnChoice(self,value):
        print 'OnChoice',value

    def MakeGui(self, parent):
        ''' esempio di come si crea una GUI '''

        gui = Gui(parent)
        gui.Label   ( 'label:')
        gui.Line()
        gui.Text    ( 'text',          'ciao',               self.OnText)
        gui.Integer ( 'integer',        13,                  self.OnInteger)
        gui.Float   ( 'float',          13.13,               self.OnFloat)
        gui.Button  ( 'button',                              self.OnButton )
        gui.CheckBox( 'boolean value',  self.value,          self.SetValue )
        gui.Combo   ( 'combo',   'qui', ('qui','quo','qua'), self.OnChoice )

        # chiamare sempre Finalize alla fine
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
    

