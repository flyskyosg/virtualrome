'''

'''
import wx
import osg
from visitorbase import Visitor
from tree import TreeBase

#-----------------------------------------------------------------------------
class CustomTree( TreeBase ) :
    '''
    L'idea e' che in questo Tree puoi aggiungere tre tipi di oggetti:
    - settings-page / files / nodi-osg
    il Tree crea da solo i folder contenitori di questi nodi,
    e quando aggiungi figli, l'albero si capisce da solo parent e icona.
    
    API:
    AddSettings( oggetto, label) 
    AddFile( oggetto, label )
    AddOsgNode( nodo, parent ) -- il parent me lo devi passare perche le istanze hanno n-parent
    RemoveNode(oggetto)        -- vale per tutti
    ClearSceneGraph()          -- svuota la parte relativa allo scenegraph 
    '''
    def __init__(self, parent=None, id=-1, observer=None ):
        """ chiama OnSelect sull'oggetto Observer """
        TreeBase.__init__(self, parent,id, observer)
        
        # carico le icone - e  relativi ID
        self.il = il   = wx.ImageList(15,15)
        self.ifolder   = il.Add( wx.Bitmap('pic/tree_folder.png'    ) )
        self.ifile     = il.Add( wx.Bitmap('pic/tree_file.png'      ) )
        self.isettings = il.Add( wx.Bitmap('pic/tree_settings4.png'  ) )
        iN             = il.Add( wx.Bitmap('pic/tree_node.png'      ) )
        iT             = il.Add( wx.Bitmap('pic/tree_transform.png' ) )
        iG             = il.Add( wx.Bitmap('pic/tree_geode.png'     ) )
        iGr            = il.Add( wx.Bitmap('pic/tree_group.png'     ) )
        iD             = il.Add( wx.Bitmap('pic/tree_drawable.png'  ) )
        iP             = il.Add( wx.Bitmap('pic/tree_property.png'  ) )
        iS             = il.Add( wx.Bitmap('pic/tree_stateset.png'  ) )
        self.iObject   = il.Add( wx.Bitmap('pic/tree_object.png'    ) )
        self.SetImageList(il, 11)
        
        self.osgIcon = {}
        self.osgIcon['Node']            = iN
        self.osgIcon['MatrixTransform'] = iT
        self.osgIcon['Geode']           = iG
        self.osgIcon['Group']           = iGr
        self.osgIcon['Drawable']        = iD
        self.osgIcon['Geometry']        = iD
        self.osgIcon['Property']        = iP
        self.osgIcon['StateSet']        = iS

        # ID delle Root
        self.root           = 'tree_root' # una stringa e' un ID unico come un altro
        self.settingsRoot   = 'settings_root'
        self.filesRoot      = 'files_root'
        self.scenegraphRoot = 'scenegraph_root'
        # Creo le Root
        self.AddRoot(self.root, 'root')
        self.AddNode(self.root, self.settingsRoot,   self.ifolder,  'settings')
        self.AddNode(self.root, self.filesRoot,      self.ifolder,  'loaded files')
        self.AddNode(self.root, self.scenegraphRoot, self.ifolder,  'scenegraph')
        
        # puo succedere che due wrapper-di-oggetto-osg (wo)
        # siano diversi anche se puntano allo stesso oggetto osg (o).
        # SE mi hai gia passato un wo1 e poi mi passi un wo2 che punta allo stesso o
        # io internamente me ne accorgo e uso wo1.
        
        # Cioe' - faccio questa cosa almeno per i parent
        # va fatta anche alla Select
        
        # dizionario ID-NODO-OSG -> oggetto-wrapper-python-di-nodo-osg
        # ID-NODO-OSG == il puntatore all'oggetto C++ senza la classe
        self.osgNodes = {}

    def AddSettings(self, obj, label=""  ):
        TreeBase.AddNode(self, self.settingsRoot, obj, self.isettings, label )

    def AddFile(self, obj, label=""  ):
        TreeBase.AddNode(self, self.filesRoot, obj, self.ifile, label )

    def AddOsgNode(self, node, parent ):
        
        # registro il this di questo nodo
        self.osgNodes[self.getNodePointer(node)] = node
    
        if parent is None or parent == 0:
            parent = self.scenegraphRoot
        else:
            # il nodo parent devo averlo gia inserito
            # quindi devo conoscere il suo this --
            # uso il this per ritrovare il wrapper usato quando l'hai inserito
            parentid = self.getNodePointer(parent)
            if self.osgNodes.has_key(parentid):
                parent = self.osgNodes[parentid]
            else:
                print 'mah'
                parent = self.scenegraphRoot
        
        # compongo la label
        cls = node.className()
        label = cls +' '+ node.getName()
        # trovo l'icona
        icon = self.iObject
        if self.osgIcon.has_key(cls):
            icon = self.osgIcon[cls]
        
        # inserisco
        TreeBase.AddNode(self, parent, node, icon, label )
##        if(parent):
##            if(parent != self.scenegraphRoot):
##                if((parent.getName()!="") and (parent != self.scenegraphRoot)):
##                    print "collapsing parent of -->",node.getName()
##                    self._tree.CollapseAllChildren(self.nodeToItem[parent])

    def  CollapseAllChildren(self,node):
        self._tree.CollapseAllChildren(self.nodeToItem[node])
        
    def getNodePointer(self,node):
        ''' str(node.this) non va bene 
            a volte nei nodePath trovi lo stesso nodo 
            this, ma con tipo diverso.
            Allora come ID tengo solo il puntatore ''' 
        nodeid = str(node.this)
        return nodeid.split('_')[1]

    def Select(self,node):
        if node is None:
            return
        nodeid = self.getNodePointer(node)
        if self.osgNodes.has_key(nodeid):
                node = self.osgNodes[nodeid]
        TreeBase.Select(self,node)

    def ReadSceneGraph(self,root):
        self.ClearSceneGraph()
        v = FillTreeVisitor(self)
        root.accept(v)

    def ClearSceneGraph(self):
        self.RemoveNode(self.scenegraphRoot)
        self.osgNodes = {}
        self.AddNode(self.root, self.scenegraphRoot, self.ifolder,  'scenegraph')

#--------------------------------------------------------------------------
class FillTreeVisitor(Visitor): 
    '''
    visitor che riempie l'albero.
    In effetti questo veniva meglio rifatto da zero.
    
    Nota che ho aggiunto un flag VisitAllInstances in VisitorBase
    che per default e' a False
    '''
    def __init__(self,tree):
        Visitor.__init__(self,True)
        self.tree = tree
        ##self.lst = [] --- la lista serve per il debug
   
    def visitMatrixTransform(self, node):
        self.addNode(node)
        return True
        
    def visitGeode(self, node):
        self.addNode(node)
        return True

    def visitNode(self, node):
        self.addNode(node)
        return True

    def visitDrawable(self,drawable,geode):
        ''' parent noto '''
        self.tree.AddOsgNode(drawable, geode )
        ##self.lst.append((drawable,geode))
        pass

    def visitStateSet(self,ss, parent):
        ''' parent noto '''
        self.tree.AddOsgNode(ss, parent )
        ##self.lst.append((ss,parent))
        pass

    def addNode(self,node):
        ''' recuperare il parent '''
        # non so esattamente quali operatori sono definiti sul nodepath
        # ma so che posso passarlo in una lista
        pylist = []
        for n in self.getNodePath():
            pylist.append(n)
        # il parent e' il penultimo nodo nel nodepath
        num = len(pylist)
        if num <=1:
            parent = None
        else:
            parent = pylist[num-2]
        # inserisco nel tree
        self.tree.AddOsgNode(node, parent)
        ##self.lst.append((node,parent))
        pass

#--------------------------------------------------------------------------
class App(wx.App):
    def OnInit(self):        
        self.frame = wx.Frame(None,-1,'TreeTest')
        self.tree  = CustomTree(self.frame,-1)
        self.SetTopWindow(self.frame)
        self.frame.Show(True)
        return True

#--------------------------------------------------------------------------
if __name__ == "__main__":

    import osgDB
    import sys
    import os

    app = App(0)


    # locate the DataDir
    dir = os.getenv('DATADIR')
    if not dir:
        print 'env-var "DATADIR" not found, exiting'
        sys.exit()
    # there are good and bad data :-)
    dir = dir + 'bad\\'

    # open the test file
    filename = dir + 'f_pace.osg'
    node = osgDB.readNodeFile(filename)
    if not node : 
        print 'error loading', filename
        sys.exit()
    

    app.tree.AddSettings(1,'config1')
    app.tree.AddSettings(2,'config2')
    app.tree.AddFile(3,'file1')
    app.tree.AddFile(4,'file2')
    app.tree.ReadSceneGraph(node)
    
    app.MainLoop()
    
