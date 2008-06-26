import osg_setup
import osg
import osgDB
import osgGA
import osgViewer
import osgUtil

tex=0

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
    
    stateset.setAttribute(selmat)

    #create a drawablw box with the right position and size
    lx = bb._max.x() - bb._min.x()
    ly = bb._max.y() - bb._min.y()
    lz = bb._max.z() - bb._min.z()
    box = osg.Box(center, lx, ly, lz)
    shape = osg.ShapeDrawable(box)
    #shape.setColor(osg.Vec4(1.0, 0.0, 0.0, 1.0))
    
    #add the drawable to the wirebox geode
    wbgeode.addDrawable(shape)

    for pointer in [stateset, box, polyModeObj, lw, shape]:
        pointer.thisown = False

    #return the wirebox geode
    return wbgeode


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

