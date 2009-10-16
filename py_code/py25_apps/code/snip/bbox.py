
import math

class bounder(object):
    def __init__(self,node):
        self.node=node
    def bbox(self):
        if(self.node):
            b=self.node.getBound()
            return (b._center.x(),b._center.y(),b._center.z(),b.radius())
    
    def boxgeode(self):
        g=osg.Geode()
        pmode=osg.PolygonMode()
        pmode.setMode(osg.PolygonMode.FRONT_AND_BACK,osg.PolygonMode.LINE)
        st=osg.StateSet()
        st.setAttributeAndModes(pmode,osg.StateAttribute.OVERRIDE | osg.StateAttribute.ON)
        g.setStateSet(st)
        if(self.node):
            bx=osg.Box(self.node.getBound()._center,self.node.getBound().radius()*math.sqrt(2))
        else:
            bx=osg.Box()
        dr=osg.ShapeDrawable(bx)
        g.addDrawable(dr)
        return g
        
        
    