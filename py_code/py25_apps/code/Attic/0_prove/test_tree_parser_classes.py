import osg_setup
import osg
import osgDB
import osgGA
import osgViewer
import wx_osg_console

import csv
import Image
import os
import sys
import math
import gc

import tree_parser_classes

##n = osgDB.readNodeFile(os.path.join("D:/vrome/web/terreni/prove_vegetazione/test4","romano.ive"))
##forest = tree_parser_classes.TreeGeometry(30000)
##ff = tree_parser_classes.TileParse()
##ff.walk(n,"pp")
##ff.walk(osgDB.readNodeFile(os.path.join("D:/vrome/web/terreni/prove_vegetazione/test4","romano_L1_X0_Y1_subtile.ive")),"qq")
##pt = tree_parser_classes.ParsedTile(os.path.join("D:/vrome/web/terreni/prove_vegetazione/test4","romano_L1_X0_Y1_subtile.ive"))
##pt.parse()
#pt = tree_parser_classes.ParsedTile(os.path.join("D:/vrome/web/terreni/prove_vegetazione/test4","romano_L2_X0_Y1_subtile.ive"))
##pt = tree_parser_classes.ParsedTile(os.path.join("D:/vrome/web/terreni/prove_vegetazione/test4","romano_L2_X0_Y1_subtile.ive"))
##pt.parse()
##terrwalk=tree_parser_classes.ParseTerrain(6)
##terrwalk.walk(os.path.join("Q:/vrome/web/terreni/attuale/ive_16_256_dds_l12_subtile","attuale.ive"),0)

class myextent():
    def __init__(self,x,y,dx,dy): 
        self.min_x=x
        self.min_y=y
        self.max_x=x + dx
        self.max_y=y + dy
    def max(self):
        return (self.max_x,self.max_y)
    def box(self):
        return osg.BoundingBox(self.min_x,self.min_y,-1000,self.max_x, self.max_y,1000)
#
ninfeo_extent = myextent(792366, 4640018.77, 340, 258)
##ninfeo_extent = myextent(792366, 4640018.77, 940, 958)
print ninfeo_extent.max()
#ninfeo_extent.max = (ninfeo_extent.min[0] + ninfeo_extent.dim[0],ninfeo_extent.min[1] + ninfeo_extent.dim[1])
#ninfeo_extent.box = osg.BoundingBox(ninfeo_extent.min[0],ninfeo_extent.min[1],-1000,ninfeo_extent.max[0],ninfeo_extent.max[1],1000)
mybox = osg.BoundingBox(789197.193,4655829.954,-1000,789674.057,4656366.800,1000)
#mybox = osg.BoundingBox(789197.193,4655829.954,-1000,789198.193,4655830.954,1000)
bx = tree_parser_classes.BoxIntersectTileHandler(ninfeo_extent.box())
terrwalk=tree_parser_classes.ParseTerrain(3,bx)
terrwalk.walk(os.path.join("Q:/vrome/web/terreni/attuale/ive_16_256_dds_l12_subtile","attuale.ive"),0)
print bx.tiles

##terrwalk.walk(os.path.join("D:/vrome/web/terreni/prove_vegetazione/test4","romano.ive"),0)
##terrwalk.walk(os.path.join("Q:\\vrome\\web\\terreni\\romano\\zenit2","romano.ive"),0)
##dir = "Q:\\vrome\\web\\terreni\\romano\\zenit2\\romano_root_L0_X0_Y0"
##os.chdir(dir)
##for f in os.listdir(dir):
##    n= osgDB.readNodeFile(f)
##    n.thisown = False
##    n.unref()
##    print gc.get_count()