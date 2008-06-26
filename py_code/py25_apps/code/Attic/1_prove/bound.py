import osg_setup
import osg
import osgDB
import osgGA
import osgViewer
import osgUtil

def mybound(n):
    bb = osg.BoundingBox()
    bb.expandBy(n.getBound())
    cx = bb._max.x() + bb._min.x() / 2.0
    cy = bb._max.y() + bb._min.y() / 2.0
    cz = bb._min.z()
    print cx," ",cy," ",cz
    
mybound(osgDB.readNodeFile("Q:/vrome/web/modelli/attuale/generati/TETTOIA_VILLA/osg_exp_tiny_full.ive"))

