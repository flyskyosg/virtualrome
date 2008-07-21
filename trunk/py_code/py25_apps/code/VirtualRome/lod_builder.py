import osg
import re
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
            m.postMult(osg.Matrixd.inverse(mt.getMatrix()))
    m.thisown = False
    #return osg.Matrixd.inverse(m)
    return m
            
    

        

def LocalizeTrasf(root,child):
    #find the child starting from root, invert all the trasf from child up to root, add the inverted trsf to all the root childs
    print "here in LocalizeTrasf"
    if(child and root):
        v = FindNodeVisitor(child)
        root.accept(v)
        if(len(v.parentlist)):
            inv=invertTrasf(v.parentlist)
            mt=osg.MatrixTransform(inv)
            mt.addChild(root)
            mt.thisown=False
            return mt
        else:
            print "root not found in child parents"
            return None
##        go_on=True
##        p=child
##        while(go_on):
##            #test if p == root
##            if(p.this.__hex__() == root.this.__hex__()):
##                go_on=False
##            else:
##                if(p.getNumParents()):
##                    #TODO we do not test the case of multiple instancing with different transforms.....
##                    #TODO look if there are different parents with differnt transform path
##                    g.getParent(0)

def matchmodelledLOD(filename):
    root=osgDB.readNodeFile(filename)
    next=root
    go_on=true
    while go_on:
        grp=next.asGroup()
        if(grp):
            num_child=grp.getNumChildren()
            if(num_child ==1):
                next=grp.getChild(0)
                go_on=True
            else:
                if(num_child>1):
                    print "good structure, further search for lod levels"
                else:
                    print "BAD structure, no multiple lods"
                    return None
                go_on=False
        else:
            print "BAD structure, no group found..."
            return None
            go_on=False
    for i in range(0,num_child):
        level=grp.getChild(i)
        name=level.getName()
        if(name):
            mm=re.match('(.*)(_low?|_mid?|_hig?h?)',name,re.IGNORECASE)
         
    
if __name__ == "__main__":


    import osgDB
    import sys
    import os
    from utility import *
    from lod_visitor import FindNamePattern

    # locate the DataDir
    dir = os.getenv('DATADIR')
    if not dir:
        print 'env-var "DATADIR" not found, exiting'
        sys.exit()
    # there are good and bad data :-)
    dir = dir + 'bad\\'

    # open the test file
    filename = dir + 'f_pace_col_b.osg'
    node = osgDB.readNodeFile(filename)
    v = FindNamePattern('(.*low-GEODE)$')
    node.accept(v)

    # print results
    myroot=osg.Group()
    # open the ref file
    filename = dir + 'f_pace_col_b.obj'
    node_obj = osgDB.readNodeFile(filename)
    #myroot.addChild(node_obj)
    
    for n in v.NodesHash.keys():
        print ">",n,"<-->",v.NodesHash[n][0],"<--"
    count=0
    for name in v.names.keys():
        print "searching for -->"+name+"<--"
        for nlow in v.names[name]:
            localized=LocalizeTrasf(node,nlow)
            myroot.addChild(localized)
            ref_filename = dir + 'test_'+name+"_"+str(count)+".osg"
            loc_filename = dir + 'test_'+name+"_"+str(count)+"_loc.osg"
            osgDB.writeNodeFile_s(nlow, ref_filename,"")
            osgDB.writeNodeFile_s(localized, loc_filename,"")
            count=count + 1
            nref=osgDB.readNodeFile(ref_filename)
            myroot.addChild(nref)
    test_wx=False
    if(test_wx):
        import wxosgviewer 
        
        app = wxosgviewer.App(0)  # importante: creare APP passando 0 
        viewer = app.getViewer()
    else:
    # create a viewer
        viewer = osgViewer.Viewer()

    # configure
        viewer.setThreadingModel(osgViewer.Viewer.SingleThreaded)
        viewer.addEventHandler(osgViewer.WindowSizeHandler())
        viewer.addEventHandler(osgViewer.StatsHandler())
    
    viewer.setSceneData(myroot)
##    hlt=HiLight(highlighter(wireboxed))
##    pickhandler1 = PickHandlerBase(hlt)
##    
##    viewer.addEventHandler(pickhandler1.__disown__());
   
    if(test_wx):
        app.MainLoop()
    else:
        # loop until done
        viewer.run()
