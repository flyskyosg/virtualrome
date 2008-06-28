"""
NOTA:
 obj.__methods__ restituisce i metodi dell'oggetto
 dir(obj) restituisce anche i metodi ___????___

 class.__methods__ non restituisce i metodi ereditati
 dir(class) idem
"""

import wx
import  wx.lib.scrolledpanel as scrolled

from vtkProxy import *
import re
import os
import types

#---------------------------------------------------------
def Repr(n, decimals=3):
    if type(n) == types.IntType:
        return '%d' % n
    if type(n) == types.FloatType:
        return '%g' % n
    if type(n) == types.NoneType:
        return ' '
    if type(n) == types.TupleType:
        res = '( '
        for i in n: res += Repr(i) + ' , '
        res += ')'        
        res = res.replace( ', )', ')' )
        return res
    return repr(n)

#---------------------------------------------------------
DEBUG=0
def debug (msg):
    if DEBUG:
        print msg
        
#---------------------------------------------------------

BannedMethods = (
'AbortExecuteOff',
'AbortExecuteOn',
'AddObserver',
'BreakOnError',
'ComputeInputUpdateExtents',
'DebugOff',
'DebugOn',
'Delete',
'EnlargeOutputUpdateExtents',
'GetAbortExecute',
'GetAddressAsString',
'GetClassName',
'GetDebug',
'GetErrorCode',
'GetGlobalWarningDisplay',
'GetMTime',
'GetOutputIndex',
'GetProgress',
'GetProgressMaxValue',
'GetProgressText',
'GetReferenceCount',
'GetReleaseDataFlag',
'GlobalWarningDisplayOn',
'GlobalWarningDisplayOff',
'HasObserver',
'InRegisterLoop',
'InvokeEvent',
'IsA',
'IsTypeOf',
'New',
'NewInstance',
'PrintRevisions',
'PropagateUpdateExtent',
'Register',
'ReleaseDataFlagOff',
'ReleaseDataFlagOn',
'RemoveAllInputs',
'RemoveObserver',
'RemoveObservers',
'SetAbortExecute',
'SafeDownCast',
'SetDebug',
'SetEndMethod',
'SetGlobalWarningDisplay',
'SetProgress',
'SetProgress',
'SetProgressMethod',
'SetProgressText',
'SetReleaseDataFlag',
'SetReferenceCount',
'SetStartMethod',
'SqueezeInputArray',
'TriggerAsynchronousUpdate',
'UnRegister',
'UnRegisterAllOutputs',
'UpdateData',
'UpdateInformation',
'UpdateProgress',
'UpdateWholeExtent',

'GetUserTransformMatrixMTime',
'ReleaseGraphicsResources',
'RenderOpaqueGeometry',
'RenderTranslucentGeometry',

'ApplyProperties',
'ComputeMatrix',
'ShallowCopy',

'AddCellReference',
'AddConsumer',
'AddReferenceToCell',
'Allocate',
'BuildCells',
'BuildLinks',
'CheckAttributes',
'ComputeBounds',
'CopyCells',
'CopyInformation',
'CopyStructure',
'CopyTypeSpecificInformation',
'DataHasBeenGenerated',
'ExtentTranslator',
'GetDataObjectType',
'GetDataReleased',
'GetEstimatedMemorySize',
'GetExtentType',
'GetGhostLevel',
'GetGlobalReleaseDataFlag',
'GetLocality',
'GetMaximumNumberOfPieces',
'GetNetReferenceCount',
'GetPiece',
'GetPipelineMTime',
'GetRequestExactExtent',
'GetUpdateGhostLevel',
'GetUpdateNumberOfPieces',
'GetUpdatePiece',
'GetUpdateTime',
'GlobalReleaseDataFlagOff',
'GlobalReleaseDataFlagOn',
'IsConsumer',
'PrepareForNewData',
'ReleaseData',
'RemoveCellReference',
'RemoveConsumer',
'RemoveGhostCells',
'RemoveReferenceToCell',
'RequestExactExtentOff',
'RequestExactExtentOn',
'SetGlobalReleaseDataFlag',
'SetLocality',
'SetMaximumNumberOfPieces',
'SetPipelineMTime',
'SetRequestExactExtent',
'SetUpdateGhostLevel',
'SetUpdateNumberOfPieces',
'SetUpdatePiece',
)

#---------------------------------------------------------

def lm( vtkobj ):
    ao = appObj( vtkobj)
    ao.Print()

#---------------------------------------------------------
class appObj:
    def __init__(self,vtkObj):

        self.MET    = {} # mapping NomeMetodo -> numero argomenti
        self.BOOL   = {} # SetX,GetX,XOn,XOff 
        self.SETGET = {} # SetX,GetX
        self.SETTO  = {} # SetX,GetX,SetXToY1,SetXToY2,...
        self.CONN   = {} # SetGet che formano connessioni
        self.MCONN  = {} # metodi che formano connessioni multiple
        self.CMD    = {} # gli altri
        self.GotAsString = {}   # GetXAsString

        self.vtkObj = vtkObj
        self.Parse()
        
    def GetFullDoc(self,m):
        return getattr(self.vtkObj,m).__doc__

    def GetDoc(self,m):
        return getattr(self.vtkObj,m).__doc__.splitlines()[0].split('.')[1].strip()

    def GetSignature(self,m):
        return self.GetDoc(m)[len(m):]

    def GetArg(self,m):
        return self.GetDoc(m).split('(')[1].split(')')[0].strip()

    def GetRet(self,m):
        doc = self.GetDoc(m)
        n = doc.count('>')
        if n == 0:
            return ''
        elif n == 1:
            ret = doc.split('>')[1].strip()
            if ret[0:1] == '(':
                ret = ret[1:]
            if ret[-1:] == ')':
                ret = ret[:-1]
            return ret
        else:
            debug('invalid doc:' + doc)
            return ''

    def Parse(self):
        global HiddenMethods
        # carico MET [nome]->(arg,ret)
        debug('######################################')
        debug('parsing a:'+self.vtkObj.__class__.__name__  )
        debug('######################################')
        for m in self.vtkObj.__methods__:
            #tolgo i metodi da nascondere
            if m[:2] == '__':
                continue
            if m in BannedMethods:
                continue
            if m[-8:] == 'MinValue' or m[-8:] == 'MaxValue':
                continue
            #ottengo il comando
            cm = getattr(self.vtkObj,m)
            #prendo la prima linea del doc
            doc = cm.__doc__.splitlines()[0]
            #controllo che ci sia il 'V.' 
            if doc[:2] != 'V.':
                debug('no V. in doc:' + doc)
                continue
            #conto gli argomenti
            arg = doc.split('(')[1].split(')')[0].strip()
            if arg == '':
                argc = 0
            else:
                argc = 1 + arg.count(',')
             #inserisco nella tabella
            self.MET[m]= argc
            #print m,'--',argc

        debug('done loading MET')
        # separo i Boolean
        for m in self.MET.keys():
            if m[-2:] == 'On':
                m2 = m[:-2]+'Off'
                m3 = 'Set' + m[:-2]
                m4 = 'Get' + m[:-2]
                if self.MET.has_key(m2) and self.MET.has_key(m3) and self.MET.has_key(m4):
                    # non controllo gli argomenti - speriamo bene 
                    del self.MET[m]
                    del self.MET[m2]
                    del self.MET[m3]
                    del self.MET[m4]
                    self.BOOL[m[:-2]] = ''
                    #print m,m2,m3,m4,'-> Bool ',m[:-2]

        debug('done loading BOOL')
        # separo le SetGet - la Get deve avere 0 argomenti
        for m in self.MET.keys():
            if m[:3] == 'Set':
                m2 = 'Get'+ m[3:]
                if self.MET.has_key(m2):
                    if self.MET[m2] == 0:
                        del self.MET[m]
                        del self.MET[m2]
                        self.SETGET[m[3:]] = self.GetArg(m)
                        if self.GetArg(m) != self.GetRet(m2):
                            debug( m+' args = '+self.GetArg(m)+' DIFFERENT FROM '+m2+' ret = '+self.GetRet(m2))
                        #print m,m2,'-> SetGet ',m[3:]
        debug('done loading SETGET')
        # separo i radio
        # deve esserci SetXxxToYyy e ci deve essere anche la SetGet (TODO: puo'esserci la SetGet)
        # - ci deve essere almeno un SetXxxToyyy
        # puo' esserci la GetXxxAsString
        rex = re.compile('^Set(.*)To')
        for m in self.MET.keys():
            mat = rex.match(m)
            if mat != None and self.MET.has_key(m) and self.MET[m] == 0:
                xxx = mat.group(1)
                if self.SETGET.has_key(xxx):
                    
                    GotAsString = False
                    if self.MET.has_key('Get'+xxx+'AsString'):
                        GotAsString = True
                        
                    values = ()
                    prefix = 'Set'+xxx+'To'
                    prefix_len = len(prefix)
                    rex2 = re.compile('^'+prefix)
                    for met in self.MET.keys():
                        if rex2.match(met) != None and self.MET[met] == 0:
                            values += met[prefix_len:],
                    
                    if len(values) >1 :
                        del self.SETGET[xxx]
                        if GotAsString:
                            del self.MET['Get'+xxx+'AsString']                        
                        for v in values:
                            del self.MET['Set'+xxx+'To'+v ]
                        self.GotAsString[xxx] = GotAsString
                        self.SETTO[xxx] = values
                        #print xxx,values

        debug('done loading SETTO')

        #cerco i metodi che fanno connessioni
        rex = re.compile('vtk')
        for m in self.SETGET.keys():
            doc = self.GetDoc('Set'+ m)
            doc2 = self.GetDoc('Get'+ m)
            if rex.search(doc+doc2) != None:
                s_arg = self.GetArg('Set'+m)
                g_ret = self.GetRet('Get'+m)
                if s_arg != g_ret:
                    debug( 'Set'+m+' args = '+s_arg+' DIFFERENT FROM Get'+m+' ret = '+g_ret)
                del self.SETGET[m]
                self.CONN[m] = s_arg
        debug('done loading CONN')
        
        # connessioni multiple:
        # esiste una SETGET xxxNumberOfyyy ed esiste una coppia di metodi
        # Getyyy(int)->vtkObj, Setyyy(vtkobj,i)
        # solo che il Glyph non ha la Setyyy(vtkobj,i) - vfc

        # value multiple:
        # - vfc

        # i metodi che sono rimasti non classificati diventano CMD
        self.CmdArguments = {}
        self.CmdReturnValue = {}
        for m in self.MET.keys():
            self.CMD[m] = self.GetSignature(m)
            self.CmdArguments[m] = self.GetArg(m)
            self.CmdReturnValue[m] = self.GetRet(m)
            del self.MET[m]

        self.radioValues = self.SETTO

        self.bool = self.BOOL.keys()
        self.bool.sort()

        self.setget = self.SETGET.keys()
        self.setget.sort()
        
        self.radio = self.SETTO.keys()
        self.radio.sort()
        
        self.conn = self.CONN.keys()
        self.conn.sort()
        
        self.cmd = self.CMD.keys()
        self.cmd.sort()
        
    def Print(self):
        
        j = 26
        print '----------------------------------------------'
        print 'Property for ',self.vtkObj.__class__.__name__     
        print

        for m in self.bool: 
            print 'bool....' + m
        
        if len(self.setget): print
        for m in self.setget: 
            print 'setget..' + m.ljust(j,'.') + self.SETGET[m] 

        if len(self.radio): print
        for m in self.radio: 
            print 'radio...' + m.ljust(j,'.') + repr( self.SETTO[m] ).replace("'","").replace(", ", " ").replace("(", "").replace(")", "").replace("AsString", "*")                                  

        if len(self.conn): print
        for m in self.conn: 
            print 'conn....' + m.ljust(j,'.') + self.CONN[m]

        if len(self.cmd): print
        for m in self.cmd: 
            print 'cmd.....' + m.ljust(j,'.') + self.CMD[m]
        print
        
    def Print2(self):
        
        j = 26
        print '############################################'
        print 'Property for ',self.vtkObj.__class__.__name__     

        lst = self.BOOL.keys()
        lst.sort()
        if len(lst): print 'BOOL'
        for m in lst: 
            print  '...' +m
        
        lst = self.SETGET.keys()
        lst.sort()
        if len(lst): print 'SETGET'
        for m in lst: 
            print '...' +m.ljust(j,'.') + self.SETGET[m] 

        lst = self.SETTO.keys()
        lst.sort()
        if len(lst): print 'RADIO'        
        for m in lst: 
            print '...' +m.rjust(j,'.') + self.SETTO[m]                  

        lst = self.CONN.keys()
        lst.sort()
        if len(lst): print 'CONN'
        for m in lst: 
            print '...' +m.ljust(j,'.') + self.CONN[m]

        lst = self.CMD.keys()
        lst.sort()
        if len(lst): print 'CMD'
        for m in lst: 
            print '...' + m.ljust(j,'.') + self.CMD[m]


    def Antenati(cls):

        """ riceve una classe vtk, restituisce la lista degli antenati """
        c = cls
        l = []
        l.append(c.__name__)
        t = c.__bases__
        while len(t):
                c = t[0]
                l.append(c.__name__)
                t = c.__bases__
        return l

#---------------------------------------------------------------------------
class appGui(scrolled.ScrolledPanel):
    def __init__(self, parent, vtkObj):
        scrolled.ScrolledPanel.__init__(self, parent, -1)#, style=wx.SUNKEN_BORDER )
        p = self
        add_spacer = False
        #p.SetBackgroundColour( wx.WHITE )
        
        self.parent = parent
        self.vtkObj = vtkObj
        self.winfo = {}

        sz = self.sz = wx.GridBagSizer(2,2)
        sz.SetEmptyCellSize( (2,2) )
        sz.SetHGap(1)
        sz.SetVGap(1)
        #sz.AddGrowableCol(1)
        sz.AddGrowableCol(2)
        r =1
            
        ao = appObj( vtkObj )
        #ao.Print()

        if 'FileName' in ao.setget:
            ao.setget.remove('FileName')
            add_spacer = True
            lab = wx.StaticText(p,-1, 'FileName' )
            val = self.vtkObj.GetFileName()
            val = os.path.basename(val)            
            lab.SetToolTip( wx.ToolTip( ao.GetFullDoc('SetFileName') )  )
            sz.Add( lab, (r,1) ) 
            e = wx.TextCtrl(p,-1, val, size=(-1,18) , style=wx.NO_BORDER )
            sz.Add( e,   (r,2) , flag=wx.EXPAND)
            b = wx.Button(p,-1,"...", size=(30,18) )
            b.Bind(wx.EVT_BUTTON,  self.OnChooseFile)
            sz.Add( b, (r,3) ) 
            self.winfo[b.GetId()]  =  { 'kind':'file', 'w':e }
            r +=1

        if len(ao.conn):
            if add_spacer:
                sz.Add( wx.StaticLine(p),(r,1),(1,2) , flag=wx.EXPAND)            
            add_spacer = True
            r +=1
            for i in ao.conn:
                dic = {}
                # fill a list with the available connection candidates
                arg_type = eval( ao.GetArg('Set'+i) )            
                for (o,proxy) in vtkObjToProxy.items():
                    if isinstance( o, arg_type ):
                        dic[proxy._name] = proxy
                choices = dic.keys()
                choices.sort()
                lab = wx.StaticText(p,-1, i )
                cb  = wx.Choice(p, -1, choices=choices ) 
                # find the current value
                current = eval( 'self.vtkObj.Get' + i + "()" )
                current_name = ''
                if vtkObjToProxy.has_key(current):
                    current_name = vtkObjToProxy[current]._name
                if current_name in choices:
                    cb.SetStringSelection( current_name )
                sz.Add( lab, (r,1) ) 
                sz.Add( cb,  (r,2), flag=wx.EXPAND)
                self.winfo[cb.GetId()]  =  { 'kind':'conn', 'cmd':i, 'cb':cb, 'dic':dic }
                r += 1

        if len(ao.bool):
            if add_spacer:
                sz.Add( wx.StaticLine(p),(r,1),(1,2) , flag=wx.EXPAND)            
            add_spacer = True
            r +=1
            for i in ao.bool:
                val = eval( 'self.vtkObj.Get' + i + '()' )
                cb = wx.CheckBox(p,-1, i, size=(-1,18) )
                cb.SetValue( val )
                cb.SetToolTip( wx.ToolTip( ao.GetFullDoc('Set'+i) )  )
                sz.Add( cb, (r,1),(1,2) ) 
                # potrei mettere il metodo nell'info, invece del nome
                self.winfo[cb.GetId()] = { 'kind':'bool', 'cmd':i, 'w':cb }
                r+=1
        
        if len(ao.radio):
            if add_spacer:
                sz.Add( wx.StaticLine(p),(r,1),(1,2) , flag=wx.EXPAND)            
            add_spacer = True
            r +=1
            for i in ao.radio:
                lab = wx.StaticText(p,-1, i )
                cb  = wx.Choice(p,-1, choices=ao.radioValues[i] ) 
                if ao.GotAsString[i]:
                    val = eval( 'self.vtkObj.Get' + i + 'AsString()' )
                    cb.SetStringSelection( val )
                sz.Add( lab, (r,1) ) 
                sz.Add( cb,  (r,2), flag=wx.EXPAND)
                self.winfo[cb.GetId()]  =  { 'kind':'radio', 'cmd':i, 'w':cb }
                r+=1

        if len(ao.setget):

            if add_spacer:
                sz.Add( wx.StaticLine(p),(r,1),(1,2) , flag=wx.EXPAND)            
            add_spacer = True

            r +=1
            CmdWidlst = []
            for i in ao.setget:
                txt = ' '+i
                if len(txt) > 28: txt = txt[:28] + '..'
                lab = wx.StaticText(p,-1, txt )
                val = ' ' + Repr( eval( 'vtkObj.Get' + i + '()' ) )
                lab.SetToolTip( wx.ToolTip( ao.GetFullDoc('Set'+i) )  )
                sz.Add( lab, (r,1) ) 
                e = wx.TextCtrl(p,-1, val, size=(-1,18) , style=wx.NO_BORDER )
                sz.Add( e,   (r,2) , flag=wx.EXPAND)
                CmdWidlst.append( {'cmd':i, 'w':e } )
                if  i in ['AmbientColor','DiffuseColor','SpecularColor',
                          'EdgeColor','Background','Color']:
                    col = getattr(self.vtkObj, 'Get' + i )()
                    wxcol = wx.Colour( col[0]*255, col[1]*255, col[2]*255)
                    b = wx.Button(p,-1,"...", size=(30,18) )
                    b.Bind(wx.EVT_BUTTON,  self.OnColor)
                    b.SetBackgroundColour(wxcol)
                    sz.Add( b, (r,3) ) 
                    self.winfo[b.GetId()]  =  { 'kind':'color', 'cmd':i, 'w':e, 'b':b }
                r+=1

            butt1 = wx.Button(p,-1,"set", size=(30,21) )
            butt2 = wx.Button(p,-1,"get", size=(30,21) )
            sz2   = wx.BoxSizer(wx.HORIZONTAL)    
            sz2.Add( butt1,1,wx.EXPAND) 
            sz2.Add( butt2,1,wx.EXPAND) 
            sz.Add ( sz2,(r,2), flag=wx.EXPAND)
            self.winfo[butt1.GetId()] =  { 'kind':'setall', 'CmdWidlst':CmdWidlst }
            self.winfo[butt2.GetId()] =  { 'kind':'getall', 'CmdWidlst':CmdWidlst }
            r+=1

        if len(ao.cmd):

            if add_spacer:
                sz.Add( wx.StaticLine(p),(r,1),(1,2) , flag=wx.EXPAND)            
            add_spacer = True

            r +=1
            for i in ao.cmd:
                r+=1
                
                sz2 = wx.BoxSizer(wx.HORIZONTAL)    
                butt = wx.Button(p,-1,"call", size=(30,18) )
                butt.SetToolTip( wx.ToolTip( ao.GetFullDoc(i) )  )
                butt.Bind(wx.EVT_BUTTON,  self.OnCmd)
                sz2.Add( butt ) 

                txt = ' '+i
                if len(txt) > 20: txt = txt[:20] + '..'
                lab = wx.StaticText(p,-1, txt )
                sz2.Add( lab )
                sz.Add ( sz2,(r,1) )
            
                if ao.CmdArguments[i]:
                    e = wx.TextCtrl(p,-1, "", size=(-1,18) ,style=wx.NO_BORDER)
                    sz.Add ( e,  (r,2), flag=wx.EXPAND)
                    self.winfo[butt.GetId()]  =  { 'kind':'call', 'cmd':i, 'w':e }
                else:
                    self.winfo[butt.GetId()]  =  { 'kind':'call', 'cmd':i }
            
        r += 1
        lab = wx.StaticText(p,-1, "", size=(5,2) )
        sz.Add (lab, (r,3) )

        p.Fit()
        p.SetSizer(sz)
        p.SetScrollRate(0,1)
        p.SetupScrolling(scroll_x = False)

        lst = self.winfo.keys()
        lst.sort()
        min_id = lst[0]
        max_id = lst[-1]
        #print min_id,max_id
        self.Bind( wx.EVT_BUTTON,   self.OnCmd,   id=min_id, id2=max_id)
        self.Bind( wx.EVT_CHECKBOX, self.OnCheck, id=min_id, id2=max_id)
        self.Bind( wx.EVT_CHOICE,   self.OnChoice, id=min_id, id2=max_id)

    def OnCmd(self,e):
        info = self.winfo[e.GetId()]
                
        if info['kind'] == 'call':
            args = '()'
            if info.has_key('w'):
               args = '(' + info['w'].GetValue() + ')'
            result = eval( 'self.vtkObj.' + info['cmd'] + args ) 
            self.parent.resultCtrl.SetValue( repr(result) )
            wx.GetApp().Render()
            return

        elif info['kind'] == 'setall':
            for CmdWid in info['CmdWidlst']:
                cmd = CmdWid['cmd']
                w   = CmdWid['w']
                eval( 'self.vtkObj.Set' + cmd + "(" + w.GetValue() + ")" )
            wx.GetApp().Render()
            return
        
        elif info['kind'] == 'getall':
            for CmdWid in info['CmdWidlst']:
                cmd = CmdWid['cmd']
                w   = CmdWid['w']
                val = eval( 'self.vtkObj.Get' + cmd + '()' )
                w.SetValue( ' ' + Repr( val ) )
            wx.GetApp().Render()
            return
        
    def OnCheck(self,e):
        info = self.winfo[e.GetId()]
        val = info['w'].GetValue()
        if val:
            eval( 'self.vtkObj.' + info['cmd'] + 'On()' ) 
        else:
            eval( 'self.vtkObj.' + info['cmd'] + 'Off()' ) 
        wx.GetApp().Render()
        
    def OnChoice(self,e):
        info = self.winfo[e.GetId()]

        if info['kind'] == 'radio':
            val = info['w'].GetStringSelection()
            eval( 'self.vtkObj.Set' + info['cmd'] + 'To' + val + '()' ) 
            wx.GetApp().Render()

        elif info['kind'] == 'conn':
            cmd = info['cmd']
            choosed_name  = info['cb'].GetStringSelection()
            choosed_proxy = info['dic'][choosed_name]
            proxy = vtkObjToProxy[self.vtkObj] # side effect wanted
            getattr( proxy, 'Set'+cmd )( choosed_proxy ) 
            wx.GetApp().Render()
            
            current = eval( 'self.vtkObj.Get' + cmd + "()" )
            current_name = ''
            if vtkObjToProxy.has_key(current):
                current_name = vtkObjToProxy[current]._name
                info['cb'].SetStringSelection( current_name )
            else:
                print '???'
    
    def OnChooseFile(self,e):
        info = self.winfo[e.GetId()]
        w = info['w']
        
        current = self.vtkObj.GetFileName()
        if current != '':
            ddir =  os.path.dirname(current)
            dfile = os.path.basename(current)
        else:
            ddir  = os.getcwd()
            dfile = ''

        cls = self.vtkObj.__class__.__name__
        cls = cls.replace('vtk','')
        cls = cls.replace('Reader','')
        cls = cls.replace('Writer','')

        wc = "All files (*.*)|*.*"
        if ('PolyData'         in cls or
            'UnstructuredGrid' in cls or
            'StructuredGrid'   in cls or
            'StructuredPoints' in cls or
            'RectilinearGrid'  in cls or
            'DataSet'          in cls or
            'ImageData'        in cls or
            'DataSet'          in cls or
            'Image'            in cls or
            'DataObject'       in cls ):
            wc = 'vtk Dataset (*.vtk)|*.vtk|' + wc
        if ('BMP'    in cls or
            'BYU'    in cls or
            'CGM'    in cls or
            'DEM'    in cls or
            'IV'     in cls or
            'JPEG'   in cls or
            'OBJ'    in cls or
            'PDB'    in cls or
            'PLOT3D' in cls or
            'PLY'    in cls or
            'PNG'    in cls or
            'PNM'    in cls or
            'SLC'    in cls or
            'STL'    in cls or
            'TIFF'   in cls ):
            wc = cls +  ' File (*.' + cls + ')|*.' + cls + '|' + wc
            
        dlg = wx.FileDialog( self, 
            message="Choose a file to open", 
            defaultDir=ddir, 
            defaultFile=dfile, 
            wildcard=wc, 
            style=wx.OPEN | wx.CHANGE_DIR )
        if dlg.ShowModal() == wx.ID_OK:
            file = dlg.GetPath()
            # if side effect wanted, use the proxy instead
            self.vtkObj.SetFileName( file )
            current = self.vtkObj.GetFileName()
            current = os.path.basename(current)            
            w.SetValue( current )
            wx.GetApp().Render()
        dlg.Destroy()
        
    def OnColor(self,e):
        info = self.winfo[e.GetId()]
        cmd = info['cmd']
        col = getattr(self.vtkObj, 'Get' + cmd )()
        wxcol = wx.Colour( col[0]*255, col[1]*255, col[2]*255)
        dlg = wx.ColourDialog(self)
        dlg.GetColourData().SetChooseFull(True)
        dlg.GetColourData().SetColour(wxcol)
        if dlg.ShowModal() == wx.ID_OK:
            wxcol = dlg.GetColourData().GetColour()
            col = ( wxcol.Red()/255.0, wxcol.Green()/255.0, wxcol.Blue()/255.0 )
            col = getattr(self.vtkObj, 'Set' + cmd )(col)
            col = getattr(self.vtkObj, 'Get' + cmd )()
            wxcol = wx.Colour( col[0]*255, col[1]*255, col[2]*255)
            info['w'].SetValue( Repr(col))
            info['b'].SetBackgroundColour(wxcol)
            wx.GetApp().Render()
        dlg.Destroy()
        
    
#---------------------------------------------------------------------------

class appGuiHolder(wx.Panel):
    def __init__(self, parent):
        wx.Panel.__init__(self, parent, -1)

    def SetObj(self, vtkObj):
        wx.SafeYield()
        self.DestroyChildren()
        wx.SafeYield()

        if vtkObj is None: return

        self.sz = sz = wx.BoxSizer(wx.VERTICAL)
        self.SetSizerAndFit(sz)

        self.nameCtrl = wx.StaticText(self,-1," Selected Object:")
        sz.Add(self.nameCtrl)

        sz.Add( wx.StaticLine(self),0,wx.EXPAND)

        self.gui = appGui( self, vtkObj )
        sz.Add(self.gui,1,wx.EXPAND)
        
        sz.Add( wx.StaticLine(self),0,wx.EXPAND)

        lab = wx.StaticText(self,-1," last result: ")
        self.resultCtrl = wx.TextCtrl(self,-1,style=wx.NO_BORDER )
        self.resultCtrl.SetBackgroundColour( lab.GetBackgroundColour() )
        sz3 = self.sz = wx.BoxSizer(wx.HORIZONTAL)
        sz3.Add(lab,0,wx.EXPAND)
        sz3.Add(self.resultCtrl, 1,wx.EXPAND)
        sz.Add(sz3,0,wx.EXPAND)
        
        self.Fit()
        
#---------------------------------------------------------------------------

# crash in chiusura !!???!?!!?!

class appObj_TestApp(wx.App):
    def OnInit(self):
        f = wx.Frame(None)
        g = appGuiHolder(f)
        #g.SetObj( vtkActor() )
        g.SetObj( vtkGlyph3D() )
        f.Show()
        self.SetTopWindow(f)
        return True
        
#---------------------------------------------------------------------------
if __name__ == '__main__':
    a = appObj_TestApp(0)
    a.MainLoop()
    

