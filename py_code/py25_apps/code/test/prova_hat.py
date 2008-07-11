import sys
import os
import glob
sys.path.append('F:\\devel\\svn_projects\\vegetazione\\python_prove')
import osg_setup
import osg
import osgDB
import osgGA
import osgViewer
import osgText
import osgSim
import osgUtil

#los=osgSim.LineOfSight()
hat=osgSim.HeightAboveTerrain()
#hat.setDatabaseCacheReadCallback(los.getDatabaseCacheReadCallback())
#filename="D:/vrome/web/terreni/prove_vegetazione/ive_16_256_dds_reduced_orig/romano.osg"
filename="H:/UrbanCenter/terrain/2007/c2007c.ive"
scene = osgDB.readNodeFile(filename)
bs = scene.getBound()
s=bs.center() 
ra=bs._radius
pp=osg.Vec3d()
numRows = 40
numColumns = 40

for r in range(0,numRows):
    for c in range(0,numColumns):
        pp=osg.Vec3d(s + osg.Vec3(r * ra * 0.01,c * ra * 0.01,300.0))
        #print s + osg.Vec3(r * ra * 0.01,c * ra * 0.01,0.0)
        print pp,"-->",hat.computeHeightAboveTerrain(scene,pp)
        hat.addPoint(pp)
hat.computeIntersections(scene)
print hat.getNumPoints()
for i in range(0,hat.getNumPoints()):
    print i,hat.getPoint(i),hat.getHeightAboveTerrain(i)