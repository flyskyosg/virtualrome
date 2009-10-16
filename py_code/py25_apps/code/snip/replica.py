import osg
import osgDB

def replica(path,rx,ry,rz,scale=1.0):
    root=osg.Group()
    prox=osg.ProxyNode()
    prox.setFileName(0,path)
    tz=0.0
    while tz < rz[1]:
        ty=0.0
        while ty < ry[1]:
            tx=0.0
            while tx < rx[1]:
                t=osg.PositionAttitudeTransform()  
                t.setScale(osg.Vec3d(scale,scale,scale))  
                t.setPosition(osg.Vec3d(tx,ty,tz))
                t.addChild(prox)
                root.addChild(t)
                tx += rx[0]
            ty += ry[0]
        tz += rz[0]
    return root

osgDB.writeNodeFile(replica('pesante.osg',(0.2,1.0),(0.2,1.0),(0.2,1.0),0.1),'N:\\scuola\\models\\example_occlusion\\pesanti.osg')