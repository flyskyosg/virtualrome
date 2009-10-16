
import conv_utils

scene = f.canvas.root

def shrink(node,amount):
    if(node):
        pat=osg.PositionAttitudeTransform()
        pat.setPivotPoint(osg.Vec3d(node.getBound()._center))
        pat.setPosition(osg.Vec3d(node.getBound()._center))
        pat.setScale(osg.Vec3d(amount,amount,amount))
        pat.addChild(node)
        pat.thisown=False
        return pat
        


topg=scene.getChild(0).asGroup()
for i in range(topg.getNumChildren()):
    child=topg.getChild(i)
    print "shrinking ",child.getName(), " numpar",child.getNumParents()
    topg.setChild(i,shrink(child,0.9))
    
       
    

