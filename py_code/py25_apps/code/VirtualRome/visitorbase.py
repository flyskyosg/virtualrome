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
        print node.className(), node.getName(),"num parent",node.getNumParents()
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
        
    def apply(self, node):

        if self.alreadyVisited( node ):
            return # cosi non visita neanche il sottoalbero

        proceed = True

        # MatrixTransform
        if node.className() == 'MatrixTransform':
            proceed = self.visitMatrixTransform(node)

        # visit Geodes
        elif node.className() == 'Geode':
            geode = osg.NodeToGeode(node)
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
        print node.className(), node.getName()
        return True
        
    def visitGeode(self, node):
        '''virtual func to be redefined -- return False to stop visiting this branch'''
        print node.className(), node.getName(),"num parent",node.getNumParents()
        return True

    def visitDrawable(self,drawable,geode):
        '''virtual func to be redefined'''
        print geode.className(), geode.getName(),"num parent",geode.getNumParents()

    def visitStateSet(self,ss,parent):
        '''virtual func to be redefined  --- node can be a node or a drawable '''
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

    # test VisitorBase
    print '------- testing VisitorBase --------'
    v = VisitorBase()
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
    
