#!/usr/bin/env python
#dynamicosg.py

__author__  = ["Rene Molenaar"]
__url__     = ("http://code.google.com/p/osgswig/")
__version__ = "2.0.0"
__doc__     = """ This OpenSceneGraph in Python example shows creating a\
                 rotation callback for a transform node \
                 ____Rene Molenaar 2008 """
                
                
import os,sys,math,random
my_base_dir=os.path.split(os.path.split(os.path.split(os.path.abspath(os.path.realpath( __file__ )))[0])[0])[0]
print my_base_dir
os.environ['OSG_FILE_PATH']=os.path.join(my_base_dir,'models','OpenSceneGraph-Data')



#import the needed modules
import osg,  osgDB, osgGA, osgViewer

castdict = {"MatrixTransform": osg.NodeToMatrixTransform}
            
def objectUpcast (pObject):
    """
    Helper function for Callbacks that route through C++
     Normally the C++ function downcasts the objects (e.g. osg.Node ).
     We need this to upcast again, e.g. to the original Python overloaded (director) class
    """
    lClassname = pObject.className()
    castfunc = None
    #print "objectUpcast:",pObject,"(",lClassname,")"
    if lClassname:
        # currently, abuse the className function to transport an instancename 
        # if there is one, evaluate its name to get the correct Python instance
        # this is ugly, so other options might be used (hints are welcome!) 
        if lClassname.startswith("instance_"):
            return eval(lClassname[len("instance_"):])
        try:
            castfunc = castdict[lClassname]
        except KeyError:
            castfunc = None
    
    if castfunc:
        return castfunc(pObject)
    else:
        return pObject
    
#Create a callback function for a transform node
class RotateCB(osg.NodeCallback):
    """Simple Rotate UpdateCallback for Transform Nodes """
    base_class = osg.NodeCallback
    def __init__(self, axis = osg.Vec3( 0.0, 0.0, 1.0 ), startangle = 0.0):
        self._angle = startangle
        self._axis = axis
        self._axis.normalize()
        self._quat = osg.Quat()
        osg.NodeCallback.__init__(self)
        
    def __call__(self, node, nv):
        """casts the transform node and rotate and increment angle"""
        self._quat.makeRotate(self._angle, self._axis) 

        #cast the node to the appropriate class type
        node = objectUpcast(node)
        
        #determine which main class is needed here
        if isinstance(node,osg.MatrixTransform):
            node.setMatrix(osg.Matrixd_rotate(self._quat))
        elif isinstance(node,osg.PositionAttitudeTransform):           
            node.setAttitude(self._quat)
        
##        #special feature for special classes
##        if isinstance(node,myPose):
##            self._angle += node.customCallback()

        # Increment the angle.        
        self._angle += 0.01;
        # call traverse
        self.traverse(node,nv)


#load the model
loadedModel = osgDB.readNodeFile("cow.osg")
root=osg.Group()
st=osg.PositionAttitudeTransform()
st.setPosition(osg.Vec3d(20.0,0.0,0.0))
st.addChild(loadedModel)
#root.addChild(st)


#create a dynamic transformation node, we can use a MatrixTransform 
dynamicTransform = osg.MatrixTransform()
#or we can use a PositionAttitudeTransform: dynamic = osg.PositionAttitudeTransform()


#add the loaded model to the transform node
dynamicTransform.addChild(loadedModel)


#to prevent direct destruction you can create a variable for the callback
rotcb = RotateCB()
dynamicTransform.setUpdateCallback(rotcb.__disown__())
#or call the disown function: dynamicTransform.setUpdateCallback(RotateCB().__disown__())







#create the viewer, set the scene and run
viewer = osgViewer.Viewer()
viewer.setThreadingModel(osgViewer.Viewer.SingleThreaded)

#set the scene data
#viewer.setSceneData(dynamicTransform.__disown__())
viewer.setSceneData(root)

####################  EMPYRICAL FINDING  Luigi Calori: 
####################  if you put this intruction before, it stack overflow.....
#################### on winXP with osgswig 
#root.addChild(dynamicTransform)    

for i in range(30):
    for j in range(30):
        st=osg.PositionAttitudeTransform()
        st.setPosition(osg.Vec3d(10.0*i,10.0*j,0.0))
        dynamicTransform = osg.MatrixTransform()
        dynamicTransform.addChild(loadedModel)
        rotcb = RotateCB(osg.Vec3d(random.random(),random.random(),random.random()))
        dynamicTransform.setUpdateCallback(rotcb.__disown__())
        st.addChild(dynamicTransform)

        root.addChild(st)
   
#add the stats event handler
viewer.addEventHandler(osgViewer.StatsHandler());

#run the viewer
#viewer.run()
#or call the following functions
##viewer.realize()
##viewer.setCameraManipulator(osgGA.TrackballManipulator())
##for i in range(100):
###while not viewer.done():
##    viewer.frame()
viewer.run()          
#set an empty node for clean-up step
viewer.setSceneData(None)

