import osg
import osgDB
import osgGA
import osgViewer
import osgUtil

#--------------------------------------------------------------------------

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

#--------------------------------------------------------------------------
def wireboxed(node):
    g=osg.Group()
    g.addChild(node)
    g.addChild(createWireBox(node))
    g.thisown=False
    return g

#--------------------------------------------------------------------------
def downsize(node):
    g=osg.MatrixTransform()
    m=osg.Matrixd()
    m.setTrans(0.0,0.0,10.0)
    g.addChild(node)
    g.setMatrix(m)
    g.thisown=False
    m.thisown=False
    return g

#--------------------------------------------------------------------------
def replace_nodes((oldnode,newnode,parents)):
##    print "in replace nodes, parentslist-->",parents.__len__()
##    print "oldnode-->",oldnode
##    print "newnode-->",newnode
    for i in parents:
##        print i
        i.addChild(newnode)
        i.removeChild(oldnode)
        
#--------------------------------------------------------------------------        
def getparents(node):
    parents=[]
##    print "getparents-->",node.getNumParents()
    for i in range(0,node.getNumParents()):
##        print i
        parents.append(node.getParent(i))
    return parents
 
#--------------------------------------------------------------------------
def makeLOD(low_node,hi_node,boundscale=20.0):
    bs=low_node.getBound()
    switch_dist=bs._radius * boundscale
    print "bound: ",bs.center().x(),bs.center().y(),bs.center().z(),bs._radius
    ##osgDB.writeNodeFile(g,filename)
    lod=osg.LOD()
    lod.setCenter(bs.center())
    lod.addChild(low_node)
    lod.addChild(hi_node)
    lod.setRange(0,switch_dist,100000000.0)
    lod.setRange(1,0.0,switch_dist)
    lod.thisown=False
    return lod

def replace_with_LOD(nlow,nhi,boundscale=20.0):
    print "replacing node>"+nlow.this.__hex__()+"< with lod"
    parents=getparents(nlow)
    lod=makeLOD(nlow,nhi)
    replace_nodes((nlow,lod,parents))
    
def make_group(t):
    g=osg.Group()
    if(t.asGroup()):
        for i in range(0,t.getNumChildren()):
            g.addChild(t.getChild(i))
    else:
        print "Warning cannot make group out of node type",t.className()
    g.thisown=False
    return g
    
