#import osg_setup
import osg
import osgDB
import osgGA
import osgViewer
import osgUtil



def createWireBox(node):
    """create a bounding box for the node """

    #create an empty bounding box
    bb = osg.BoundingBox()

    #if we have a geode, expand by the drawables bounding box, else use the bounding sphere
    geode = osg.NodeToGeode(node)
    if geode:
        print "geode found"   
        for i in range(geode.getNumDrawables()):
            dwb = geode.getDrawable(0)
            bb.expandBy(dwb.getBound());
    else:
        bb.expandBy(node.getBound())            
    
    center = node.getBound().center()
    
    #create a geode for the wirebox
    wbgeode = osg.Geode()
    wbgeode.setName("ExtentsGeode")

    #create a stateset for the wirebox
    stateset = osg.StateSet()
    wbgeode.setStateSet(stateset)
#    stateset.thisown = 0   

    #create a polygonmode state attribute
    polyModeObj = osg.PolygonMode()
    polyModeObj.setMode(osg.PolygonMode.FRONT_AND_BACK, osg.PolygonMode.LINE)
    stateset.setAttribute(polyModeObj)
    
    #create a linewidth state attribute
    lw = osg.LineWidth()
    lw.setWidth(2.0)
    stateset.setAttribute(lw)
    
    #stateset.setAttribute(selmat)

    #create a drawablw box with the right position and size
    lx = bb._max.x() - bb._min.x()
    ly = bb._max.y() - bb._min.y()
    lz = bb._max.z() - bb._min.z()
    box = osg.Box(center, lx, ly, lz)
    shape = osg.ShapeDrawable(box)
    #shape.setColor(osg.Vec4(1.0, 0.0, 0.0, 1.0))
    
    #add the drawable to the wirebox geode
    wbgeode.addDrawable(shape)
    wbgeode.setName("Wirebox")

    for pointer in [stateset, box, polyModeObj, lw, shape]:
        pointer.thisown = False

    #return the wirebox geode
    return wbgeode

def wireboxed(node):
    g=osg.Group()
    g.addChild(node)
    g.addChild(createWireBox(node))
    g.thisown=False
    return g

def downsize(node):
    g=osg.MatrixTransform()
    m=osg.Matrixd()
    m.setTrans(0.0,0.0,10.0)
    g.addChild(node)
    g.setMatrix(m)
    g.thisown=False
    m.thisown=False
    return g

def replace_nodes((oldnode,newnode,parents)):
##    print "in replace nodes, parentslist-->",parents.__len__()
##    print "oldnode-->",oldnode
##    print "newnode-->",newnode
    for i in parents:
##        print i
        i.addChild(newnode)
        i.removeChild(oldnode)
        
def getparents(node):
    parents=[]
##    print "getparents-->",node.getNumParents()
    for i in range(0,node.getNumParents()):
##        print i
        parents.append(node.getParent(i))
    return parents
        
class highlighter():
    def __init__(self,hfun):
        self.selected=dict()
        self.hilight=hfun
        self.count=0
    def select(self,node):
##        print "in select-->",node,node.getNumParents()
        if(node):
            if(node.getNumParents()):
                if(self.selected.has_key(node.this.__hex__())):
                    print "de-select-->",self.selected[node.this.__hex__()][1]
                    replace_nodes(self.selected.pop(node.this.__hex__()))
                    self.count=self.count -1
                else:
                    print "select-->",str(node.this.__hex__())
                    parents=getparents(node)
                    hilight=self.hilight(node)
                    hilight.setName("selected_"+str(self.count))
                    self.selected[hilight.this.__hex__()]=(hilight,node,parents)
                    replace_nodes((node,hilight,parents))
                    print "inserting selection-->",hilight.this.__hex__(),"<-->",self.count
                    self.count=self.count +1

    def is_hilit(self,node):
        nodename=node.getName()
        if(nodename):
            nodename= node.this.__hex__() + "<-->" + nodename
        else:
            nodename= node.this.__hex__() 
        print "testing hilit node-->",nodename,"<-->",self.selected.has_key(node.this.__hex__())
        return self.selected.has_key(node.this.__hex__())

class PickAction():
    def __init__(self):
        print "PickAction. __init__ "
        self.start()
    def handle(self,intersection):
        self.start()
        mypoint = intersection.getWorldIntersectPoint()
        print "Intersection Point ->", mypoint.x(), mypoint.y(), mypoint.z()
        for node in intersection.nodePath:
            print node.className(), "<-->",node.getName()
            if(self.match(node)):
                if(self.do(node)):
                    return True
            else:
                print "node -->",node.this.__hex__()," does not match"
        return self.end()
    
    def match(self,node):
        print "PickAction match "
        if(not self.firstnode):
            self.firstnode=node
            print "setting firstnode-->",node

    def do(self,node):
        self.done=True
        print "PickAction do "
    def start(self):
        self.firstnode=None
        self.done=False

    def end(self):
        print "PickAction end "
        return False
    
class PrintTextures(PickAction):
    def __init__(self):
        PickAction.__init__(self)
        print "PrintTextures __init__ "
    def match(self,node):
        print "PrintTextures match "
        self.st = node.getStateSet()
        if self.st:
            self.st.thisown = False
            return True
        else:
            g = osg.NodeToGeode(node)
            if g:
                print "geode-->",g.getNumDrawables()
                for j in range(0,g.getNumDrawables()):
                    d  = g.getDrawable(j);
                    self.st = d.getStateSet()
                    if self.st:
                        self.st.thisown = False
                        return True
            return False
    
    def do(self,node):
        print "PrintTextures do "
        for i in [0,1,2,3]:
            tex = self.st.getTextureAttribute(i,osg.StateAttribute.TEXTURE)
            if tex:
                print "got texture channel", i, " -->",tex.className()
        return False
                

class HiLight(PickAction):
    def __init__(self,hl):
        PickAction.__init__(self)
        print "HiLight __init__ "
        self.hl=hl
    def match(self,node):
        PickAction.match(self,node)
        print "sono qui"
        if(self.hl.is_hilit(node)):
            return True
        else:
            g = osg.NodeToGeode(node)
            if g:
                self.firstnode=g
            return False

    
    def do(self,node):
        PickAction.do(self,node)
        print "HiLight do "
        self.hl.select(node)
        return True
 
    def end(self):
        print "HiLight end -->",self.done
        if(not self.done):
            self.hl.select(self.firstnode)
            return True
        else:
            return False
               


class PickHandlerBase(osgGA.GUIEventHandler): 
    """PickHandler -- A GUIEventHandler that implements picking."""
    base_class = osgGA.GUIEventHandler
    def __init__(self,action):
        #Store mouse xy location for button press and move events.
        self._mX = 0
        self._mY = 0   
        self.action=action
        osgGA.GUIEventHandler.__init__(self)
    def handle(self, ea, aa, obj, nv):        
        vwr = osgViewer.GUIActionAdapterToViewer(aa)
        if not vwr:
            return False
        eventtype = ea.getEventType()
        if eventtype == ea.PUSH or eventtype == ea.MOVE:
            self._mX = ea.getX()
            self._mY = ea.getY()
            return False
        if eventtype == ea.RELEASE:
            print "mouse coord-->",self._mX, self._mY
            #check if mouse didn't move (else let trackball manipulator handle it)
            if self._mX == ea.getX() and self._mY == ea.getY():
                if self.pick( ea.getXnormalized(), ea.getYnormalized(), vwr ):
                    return True
        return False
    def pick(self, x, y , viewer):

        print "Picking->",x,"<->",y 
        if not viewer.getSceneData():
            return False
        #create an intersector
        picker = osgUtil.LineSegmentIntersector(osgUtil.Intersector.PROJECTION, x, y )
        #create an intersectionVisitor
        iv = osgUtil.IntersectionVisitor( picker )
        #visit the sceneGraph
        viewer.getCamera().accept( iv )
        #check for intersections
        if picker.containsIntersections():
            intersection = picker.getFirstIntersection()
            self.action.handle(intersection)
            return False                            
        else:
            print "No Intersection Found"            
        return False       
        




class PickHandler(osgGA.GUIEventHandler): 
    """PickHandler -- A GUIEventHandler that implements picking."""
    base_class = osgGA.GUIEventHandler
    def __init__(self):
        #Store mouse xy location for button press and move events.
        self._mX = 0
        self._mY = 0   
        self._selectedNode = 0
        self.wb = 0
        osgGA.GUIEventHandler.__init__(self)
    def handle(self, ea, aa, obj, nv):        
        vwr = osgViewer.GUIActionAdapterToViewer(aa)
        if not vwr:
            return False
        eventtype = ea.getEventType()
        if eventtype == ea.PUSH or eventtype == ea.MOVE:
            self._mX = ea.getX()
            self._mY = ea.getY()
            return False
        if eventtype == ea.RELEASE:
            print "mouse coord-->",self._mX, self._mY
            #check if mouse didn't move (else let trackball manipulator handle it)
            if self._mX == ea.getX() and self._mY == ea.getY():
                if self.pick( ea.getXnormalized(), ea.getYnormalized(), vwr ):
                    return True
        return False
    def pick(self, x, y , viewer):
        global tex
        print "Picking->",x,"<->",y 
        if not viewer.getSceneData():
            return False
        #create an intersector
        picker = osgUtil.LineSegmentIntersector(osgUtil.Intersector.PROJECTION, x, y )
        #create an intersectionVisitor
        iv = osgUtil.IntersectionVisitor( picker )
        #visit the sceneGraph
        viewer.getCamera().accept( iv )
        #check for intersections
        if picker.containsIntersections():
            intersection = picker.getFirstIntersection()
            print "inters-->", intersection
            mypoint = intersection.getWorldIntersectPoint()
            print "Intersection Point ->", mypoint.x(), mypoint.y(), mypoint.z()
            #find the first Transform node and make it the selected node
            for node in intersection.nodePath:
                print node.className(), "<-->",node.getName()
                st = node.getStateSet()
                if st:
                    for i in [0,1,2,3]:
                        tex = st.getTextureAttribute(i,osg.StateAttribute.TEXTURE)
                        if tex:
                            print "got texture channel", i, " -->",tex.className()
##                            tx = osg.StateAttributeToTexture(tex)
##                            if tx:
##                                im = tx.getImage(0)
##                                print "filename->",im.getFileName(),"<-size->",im.s(),"<->",im.t()
                
#                mt = osg.NodeToMatrixTransform(node)        
#                if mt:
#                    print "found MatrixTransform"
#                    #if there is a previous selected node, 'deselect' it
#                    if self._selectedNode:
#                        if self.wb:
#                            self._selectedNode.removeChild(self.wb)
#                    self._selectedNode = mt
#                    #show that the node is selected
#                else:            
#                    pot = osg.NodeToPositionAttitudeTransform(node)
#                    if pot:           
#                        #if there is a previous selected node, 'deselect' it
#                        if self._selectedNode:
#                            self._selectedNode.setUpdateCallback(None)
#                            #remove the wirebox of the previous selection
#                            if self.wb:
#                                self._selectedNode.removeChild(self.wb)
#                        self._selectedNode = pot
#                        #show that the node is selected
#                        pot.setUpdateCallback(RotateCB().__disown__())
#                    else:
#                        geode = osg.NodeToGeode(node)
#                        if geode:
#                            #create a wirebox and safe it, the wire box 
#                            #will be attached to the parental transform node
#                            self.wb = createWireBox(geode)
#                            self._selectedNode.addChild(self.wb)
#                            self.wb.thisown = False
#                            return True
            return False                            
        else:
            print "No Intersection Found"            
        return False       

###-------------------------------------------------------------------
##if __name__ == "__main__":
##    # create a viewer
##    viewer = osgViewer.Viewer()
##
### configure
##    viewer.setThreadingModel(osgViewer.Viewer.SingleThreaded)
##
### add to the scene
##    root=osg.Group()
##    n=osgDB.readNodeFile("D:/prove/oppidum/oppidum_opt._fwslash.osg")
##    root.addChild(n)
##    root.addChild(downsize(n))
##    root.addChild(wireboxed(n))
##    root.addChild(wireboxed(downsize(n)))
##    viewer.setSceneData(root)
##
##    hlt=HiLight(highlighter(wireboxed))
##    pickhandler1 = PickHandlerBase(hlt)
##    
##    viewer.addEventHandler(pickhandler1.__disown__());
##
### loop until done
##    viewer.run()



#--------------------------------------------------------------------------
if __name__ == "__main__":
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
    
    viewer.setSceneData(node)
    hlt=HiLight(highlighter(wireboxed))
    pickhandler1 = PickHandlerBase(hlt)
    
    viewer.addEventHandler(pickhandler1.__disown__());

    
    
    
    if(test_wx):
        app.MainLoop()
    else:
        # loop until done
        viewer.run()