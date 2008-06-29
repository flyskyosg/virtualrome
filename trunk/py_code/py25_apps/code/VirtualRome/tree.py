"""

"""
import wx

#-----------------------------------------------------------------------------

class TreeBase( wx.Panel ) :
    ''' classe impestata da usare a scatola chiusa '''
    
    
    def __init__(self, parent=None, id=-1, observer=None ):
        """ chiama OnSelect sull'oggetto Observer """
        wx.Panel.__init__(self, parent,id, style=wx.WANTS_CHARS)
        
        self.observer = observer    
        self.sel = None #to be used in OnSelect only --
        self.dont_notify_selection = 0

        self.il     = wx.ImageList(15,15)
        self.i_root = self.il.Add( wx.Bitmap('pic/tree_folder.png'    ) )
        self.num_icons = 1

        # wx.TR_MULTIPLE | wx.TR_HIDE_ROOT | wx.TR_HAS_BUTTONS
        self._tree = wx.TreeCtrl(self, -1,style = wx.NO_BORDER | wx.TR_HAS_BUTTONS )
        self._tree.SetImageList(self.il) 
        self._tree.SetIndent(10)
        #self._tree.SetBackgroundColour(wx.RED)
        
        self.Clear()
        
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_TREE_SEL_CHANGED, self.OnSelect, self._tree)

    def Clear(self):
        self._tree.DeleteAllItems()
        self.nodeToItem = {}
        self._root = None
        
    def SetImageList(self, il, num_icons ):
        self.il = il
        self._tree.SetImageList(il) 
        self.num_icons = num_icons

    def OnSize(self, e ):
        w,h = self.GetClientSizeTuple()
        self._tree.SetDimensions(0, 0, w, h)

    def NodeExist(self,node):
        return self.nodeToItem.has_key(node)

    def AddRoot(self, node, name):
        self._root = self._tree.AddRoot(name, self.i_root, self.i_root)
        self._tree.SetPyData(self._root, node)    
        self.nodeToItem[node]=self._root
        self._tree.SelectItem(self._root)

##        self._tree.SetItemHasChildren(self._root,True)
##        self._tree.Expand(self._root)
##        self._tree.EnsureVisible(self._root)

    def AddNode(self, parent, node, icon=0, name=None ):

        if self.NodeExist(node): return
        if name == None : name = repr(node)
        
        if icon < 0: icon =0
        if icon > self.num_icons: icon = self.num_icons                  

        #parent =0 or None mean root
        if self.NodeExist(parent):
            p_item = self.nodeToItem[parent]
        else :
            if self._root is None: return
            p_item = self._root

        item = self._tree.AppendItem(p_item, name,icon,icon)
        self._tree.SetPyData(item,node)  
        self.nodeToItem[node] = item
        
        self._tree.SetItemHasChildren(p_item,True)
        self._tree.Expand(p_item)
        self._tree.EnsureVisible(item)
        
    def RemoveNode(self, node ): 
        if not self.NodeExist(node): return
        item = self.nodeToItem[node]
        if item == self._root : return

        p_item = self._tree.GetItemParent(item)
        nc = self._tree.GetChildrenCount(p_item,False)
        self._tree.SetItemHasChildren(p_item, nc>1)

        self.DeleteNodeRec(item)

    def DeleteNodeRec(self,item):
        c_item,cookie  = self._tree.GetFirstChild(item)
        while c_item.IsOk():
            self.DeleteNodeRec(c_item)    
            c_item,cookie = self._tree.GetFirstChild(item)
        
        node = self._tree.GetPyData(item)
        self._tree.Delete(item)
        del self.nodeToItem[node]
        # mandare eventi ??

    def RenameNode(self,node, new_name):
        if not self.NodeExist(node): return
        item = self.nodeToItem[node]
        self._tree.SetItemText(item,new_name)
        
    def ReparentNode(self, new_parent, node ):
        # if one of the items to be moved is the selected one
        # we must restore the selection later -- (item may change ID)
        self.dont_notify_selection = 1
        sel_item = self._tree.GetSelection() # dont use self.sel here !!
        selected_node = None
        if not sel_item is None:
            selected_node = self._tree.GetPyData(sel_item)
        
        if not self.NodeExist(node): return
        item = self.nodeToItem[node]
        if item == self._root : return
        if not self.NodeExist(new_parent): return
        new_parent_item = self.nodeToItem[new_parent] 

        # new_parent non deve essere nel sottoalbero di node
        p = new_parent_item         
        while p != self._root :
            if p == item: return
            p = self._tree.GetItemParent(p)

        #controllare se il vecchio padre ha altri figli
        p_item = self._tree.GetItemParent(item)
        nc = self._tree.GetChildrenCount(p_item,False)
        self._tree.SetItemHasChildren(p_item, nc>1)

        #CopyTreeRec
        self.CopyTreeRec(new_parent_item, item )

        #espando il nuovo padre e visualizzo il nodo spostato
        new_item = self.nodeToItem[node]       
        self._tree.SetItemHasChildren(new_parent_item,True)
        self._tree.Expand(new_parent_item)
        self._tree.EnsureVisible(new_item)

        if not selected_node is None:
            self.Select(selected_node)
        self.dont_notify_selection = 0

    def CopyTreeRec(self, new_parent_item, item ):
        
        # 1 - creo la mia copia
        # 2 - sposto i miei figli
        # 3 - mi cancello

        node   = self._tree.GetPyData(item)
        name   = self._tree.GetItemText(item)
        icon   = self._tree.GetItemImage(item)
        p_item = self._tree.GetItemParent(item)

        item2 = self._tree.AppendItem(new_parent_item, name, icon ,icon )
        self._tree.SetPyData(item2,node)
        self.nodeToItem[node] = item2
        self._tree.SetItemHasChildren(new_parent_item,True)
        self._tree.Expand(new_parent_item)

        c_item,cookie = self._tree.GetFirstChild(item)
        while c_item.IsOk():
            c_node = self._tree.GetPyData(c_item)
            self.CopyTreeRec( item2, c_item )
            c_item,cookie = self._tree.GetFirstChild(item)
        self._tree.Delete(item)
    
    def OnSelect(self,e):
        if self.dont_notify_selection == 1: return
        
        if e.GetItem() == self.sel: # strangely, OnSel event is received twice
            return
        self.sel = e.GetItem()
        if self.sel:
            try:
                self.observer.OnSelect( self._tree.GetPyData( self.sel))
            except AttributeError:
                pass

    def Select(self,node):
        if self.NodeExist(node): 
            item = self.nodeToItem[node]
            self._tree.SelectItem(item)


#-----------------------------------------------------------------------------
# non usare questa classe, vedi custom-tree
class Tree( TreeBase ) :
    def __init__(self, parent=None, id=-1, observer=None ):
        """ chiama OnSelect sull'oggetto Observer """
        TreeBase.__init__(self, parent,id, observer)
        
        # inizializzati la ImageList con le icone che ti servono
        # le icone saranno poi riferite per indice
        # non puoi cambiare/aggiungere icone a run-time
        
        self.il = il   = wx.ImageList(15,15)
        self.ifolder   = il.Add( wx.Bitmap('pic/tree_folder.png'    ) )
        self.ifile     = il.Add( wx.Bitmap('pic/tree_file.png'      ) )
        self.isettings = il.Add( wx.Bitmap('pic/tree_settings4.png'  ) )
        self.iN        = il.Add( wx.Bitmap('pic/tree_node.png'      ) )
        self.iT        = il.Add( wx.Bitmap('pic/tree_transform.png' ) )
        self.iG        = il.Add( wx.Bitmap('pic/tree_geode.png'     ) )
        self.iGr       = il.Add( wx.Bitmap('pic/tree_group.png'     ) )
        self.iD        = il.Add( wx.Bitmap('pic/tree_drawable.png'  ) )
        self.iP        = il.Add( wx.Bitmap('pic/tree_property.png'  ) )
        self.iS        = il.Add( wx.Bitmap('pic/tree_stateset.png'  ) )
        self.iO        = il.Add( wx.Bitmap('pic/tree_object.png'    ) )
        self.SetImageList(il, 11)

    def AddNode(self, parent, node, label="", icon=0  ):
        ''' ridefinendola puoi applicare automaticamente l'icona, scegliere nome e parent'''
        TreeBase.AddNode(self, parent, node, icon, label )
        
    def ReparentNode(self, new_parent, node ):
        ''' e' interessante ridefinirla se sai estrarre il nuovo parent implicitamente'''
        TreeBase.ReparentNode(self, new_parent, node )
    
    def RenameNode(self, node, new_label ):
        ''' e' interessante ridefinirla se sai estrarre il nuovo nome implicitamente'''
        TreeBase.RenameNode(self, node, new_label  )
        
    def TestIcons(self):
        self.Clear()
        self.AddRoot(1,'testing tree icons')
        self.AddNode(1,2, 'settings')
        self.AddNode(2,3, 'set1',         2)
        self.AddNode(2,4, 'set2',         2)

        self.AddNode(1,5, 'loaded files')
        self.AddNode(5,6, 'file',         1)
        self.AddNode(5,7, 'file',         1)

        self.AddNode(1,8, 'scenegraph')
        self.AddNode(8,9, 'transform',    4)
        self.AddNode(8,10,'geode',        5)
        self.AddNode(8,11,'geometry',     6)
        self.AddNode(8,12,'drawable',     7)
        self.AddNode(8,13,'property',     8)
        self.AddNode(8,14,'stateset',     9)
        self.AddNode(8,15,'object',      10)


#-----------------------------------------------------------------------------

class App(wx.App):
    def OnInit(self):        
        self.frame = wx.Frame(None,-1,'TreeTest')
        self.SetTopWindow(self.frame)
        return True

#---------------------------------------------------------------------------

if __name__ == '__main__':
    app = App(0)

    tree = Tree(app.frame,-1)
    tree.TestIcons()

    app.frame.Show() 
    app.MainLoop()

