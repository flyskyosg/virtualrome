import osg

#--------------------------------------------------------------------------
class VisitorBase(osg.NodeVisitor): 
    ''' visit each node in a osg-scenegraph.
        takes care to visit each node just one time (unless visitAllInstances is True).
        for each node found the virtual method visitNode is invoked
    '''

    def __init__(self, visitAllInstances=False):
        osg.NodeVisitor.__init__(self,osg.NodeVisitor.TRAVERSE_ALL_CHILDREN)
        self.visited = {}  
        self.visitAllInstances = visitAllInstances
        
    def alreadyVisited(self,node):
        if self.visitAllInstances:
            return False
        id = str(node.this)
        if self.visited.has_key(id):
            return True
        else:
            self.visited[id] = 1
            return False

    def apply(self, node):
        if self.alreadyVisited( node ):
            return # cosi non visita neanche il sottoalbero

        proceed = self.visitNode(node)
        if proceed:
            osg.NodeVisitor.traverse(self, node)
    
    def visitNode(self, node):
        '''virtual func to be redefined -- return False to stop visiting this branch'''
        print "BASE GENERIC NODE: ",node.className(), node.getName(),"num parent",node.getNumParents()
        return True

#--------------------------------------------------------------------------
class Visitor(VisitorBase): 
    ''' 
    Extend VisitorBase by visiting also the Drawables and the StateSet.
    For each MatrixTransform,Geode,Drawable,Stateset,Generic-Node(all the rest)
    a particular virtual method is invoked
    '''

    def __init__(self,visitAllInstances=False):
        VisitorBase.__init__(self,visitAllInstances)
        self.visited = {}  
        self.verbose=True
        
    def apply(self, node):

        if self.alreadyVisited( node ):
            return # cosi non visita neanche il sottoalbero

        proceed = True

        # MatrixTransform
        if node.className() == 'MatrixTransform':
            mt = osg.NodeToMatrixTransform(node)
            proceed = self.visitMatrixTransform(mt)
        elif node.className() == 'Group':
            gr = node.asGroup()
            proceed = self.visitGroup(gr)
        
        # visit Geodes
        elif node.className() == 'Geode':
            geode = osg.NodeToGeode(node)
            geode.thisown=False
            proceed = self.visitGeode( geode )
            
            
            # visit Drawables
            for j in range(0,geode.getNumDrawables()):
                drawable  = geode.getDrawable(j)
                if not self.alreadyVisited( drawable ):
                    self.visitDrawable(drawable,geode)
                    
                    # visit Geometry StateSet
                    ss = drawable.getStateSet()
                    if ss:
                        if not self.alreadyVisited( ss ):
                            self.visitStateSet(ss,drawable)
        elif node.className() == 'PagedLOD':
            plod=osg.NodeToPagedLOD(node)
            plod.thisown=False
            proceed = self.visitPagedLOD( plod )

        elif node.className() == 'ProxyNode':
            pnode=osg.ProxyNode(node)
            pnode.thisown=False
            ##pnode=node.asProxyNode()
            proceed = self.visitProxyNode( pnode )

        else:
            proceed = self.visitNode(node)
            
        
        # visit Node StateSet
        ss = node.getStateSet()
        if ss:
            if not self.alreadyVisited( ss ):
                self.visitStateSet(ss,node)
        
        if proceed:
            osg.NodeVisitor.traverse(self, node)
    
    def visitMatrixTransform(self, node):
        '''virtual func to be redefined -- return False to stop visiting this branch'''
        if(self.verbose):
            print node.className(), node.getName()
        return True
        
    def visitGroup(self, gr):
        '''virtual func to be redefined -- return False to stop visiting this branch'''
        if(self.verbose):
            print gr.className(), gr.getName(),"num parent",gr.getNumParents()
        return True

    def visitPagedLOD(self, plod):
        '''virtual func to be redefined -- return False to stop visiting this branch'''
        if(self.verbose):
            print plod.className(), plod.getName(),"num parent",plod.getNumParents()
        #not working#   print "   ",plod.getRangeList()
        filenames=[]
        for i in range(0,plod.getNumFileNames()):
            filenames.append(plod.getFileName(i))
        if(self.verbose):
            print "   ",filenames
        return True

    def visitProxyNode(self, pnode):
        '''virtual func to be redefined -- return False to stop visiting this branch'''
        if(self.verbose):
            print pnode.className(), pnode.getName(),"num parent",pnode.getNumParents()
        #not working#   print "   ",plod.getRangeList()
        filenames=[]
        for i in range(0,pnode.getNumFileNames()):
            filenames.append(pnode.getFileName(i))
        if(self.verbose):
            print "   ",filenames
        return True

    def visitGeode(self, node):
        '''virtual func to be redefined -- return False to stop visiting this branch'''
        if(self.verbose):
            print node.className(), node.getName(),"num parent",node.getNumParents()
        return True

    def visitDrawable(self,drawable,geode):
        '''virtual func to be redefined'''
        if(self.verbose):
            print drawable.className(), drawable.getName(),"num parent",geode.getNumParents()

    def visitStateSet(self,ss,parent):
        '''virtual func to be redefined  --- node can be a node or a drawable '''
        if(self.verbose):
            print ss.className(), ss.getName(),"num parent",ss.getNumParents()

#--------------------------------------------------------------------------
# VisitorBase Test and  subclassing Example 

if __name__ == "__main__":


    class FindRif(VisitorBase): 
        ''' trova tutte le Transform con il nome che finisce in _RIF.
            il sottoalbero dei '_RIF' non viene visitato'''

        def __init__(self):
            VisitorBase.__init__(self)
            self.RifNodes = []

        def visitMatrixTransform(self, node):
            name = node.getName().upper()
            if name[-4:] == '_RIF':
                self.RifNodes.append(node)
                print name, '(RIF_NODE)'
                return False
            else:
                print name
                return True

    #--------------------------------------------------------------------------
    import osgDB
    import sys
    import os

    # locate the DataDir
    datadir = os.getenv('DATADIR')
    if not datadir:
        print 'env-var "DATADIR" not found, exiting'
        sys.exit()

    # open the test file
    
    #filename = datadir + 'fori\\f_pace_low.osg'
    filename = datadir + 'gaiani\\blocco1.osg'


    node = osgDB.readNodeFile(filename)
    if not node : 
        print 'error loading', filename
        sys.exit()

    # test VisitorBase
    print '------- testing VisitorBase --------'
    v = VisitorBase()
    node.accept(v)

    # test Visitor
    print '------- testing Visitor --------'
    v = Visitor()
    node.accept(v)

    # test a Visitor Subclass
    print '------- testing FindRif --------'
    v = FindRif()
    node.accept(v)

    # print results
    for n in v.RifNodes:
        name = n.getName()[:-4]
        filename = '-------------->',dir + name + '.osg'
        print filename

    print '------- Done --------'
    
