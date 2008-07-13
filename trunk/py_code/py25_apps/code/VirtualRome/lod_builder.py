import osg
from visitorbase import VisitorBase


class FindNodeVisitor(VisitorBase): 
    '''
    visitor che cerca un nodo 
    '''
    def __init__(self,searchnode):
        VisitorBase.__init__(self)
        self.searchnode=searchnode
        self.found=False
        self.parentlist=[]
        
    def visitNode(self, node):
        if(node.this.__hex__() == self.searchnode.this.__hex__()):
            #found node
            if(self.found):
                print "Warning: multiple instances found while searching for ->",self.searchnode.this.__hex__()
            else:
                #get node path to root
                for n in self.getNodePath():
                    self.parentlist.append(n)
                self.found=True
            return False
        else:
            return True
        
def invertTrasf(nodelist):
    m=osg.Matrixd_identity()
    for n in nodelist:
        mt=osg.NodeToMatrixTransform(n)
        if(mt):
            m.postMult(t.getMatrix())
    
            
    

        

def LocalizeTrasf(root,child):
    #find the child starting from root, invert all the trasf from child up to root, add the inverted trsf to all the root childs
    print "here in LocalizeTrasf"
    if(child and root):
        v = FindNodeVisitor(child)
        root.accept(v)

        go_on=True
        p=child
        while(go_on):
            #test if p == root
            if(p.this.__hex__() == root.this.__hex__()):
                go_on=False
            else:
                if(p.getNumParents()):
                    #TODO we do not test the case of multiple instancing with different transforms.....
                    #TODO look if there are different parents with differnt transform path
                    g.getParent(0)
    
if __name__ == "__main__":


    import osgDB
    import sys
    import os
    from utility import *

    # locate the DataDir
    dir = os.getenv('DATADIR')
    if not dir:
        print 'env-var "DATADIR" not found, exiting'
        sys.exit()
    # there are good and bad data :-)
    dir = dir + 'bad\\'

    # open the test file
    filename = dir + 'f_pace_mi_SIL.osg'
    node = osgDB.readNodeFile(filename)
