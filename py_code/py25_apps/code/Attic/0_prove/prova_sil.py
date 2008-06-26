
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

def createCrossQuadTree(width,height,rot,texbox,outfile ) :
    v=osg.Vec3Array(8)
    sw = math.sin(math.radians(rot)) * width * 0.5
    cw = math.cos(math.radians(rot)) * width * 0.5
    v[0].set(-sw,-cw,0.0)
    v[1].set( sw, cw,0.0)
    v[2].set( sw, cw,height) 
    v[3].set(-sw,-cw,height)

    v[4].set(-cw,sw,0.0);
    v[5].set(cw,-sw,0.0);
    v[6].set(cw,-sw,height) 
    v[7].set(-cw,sw,height)
 

    v1=osg.Vec3Array(8)
    sw = math.sin(math.radians(rot +10)) * width * 0.3
    cw = math.cos(math.radians(rot+ 10)) * width * 0.3
    v1[0].set(-sw,-cw,0.0)
    v1[1].set( sw, cw,0.0)
    v1[2].set( sw, cw,height) 
    v1[3].set(-sw,-cw,height)

    v1[4].set(-cw,sw,0.0);
    v1[5].set(cw,-sw,0.0);
    v1[6].set(cw,-sw,height) 
    v1[7].set(-cw,sw,height)

   
    t=osg.Vec2Array(8)
    
    t[0].set(texbox[0],texbox[1])
    t[1].set(texbox[2],texbox[1])
    t[2].set(texbox[2],texbox[3])
    t[3].set(texbox[0],texbox[3])

    t[4].set(texbox[0],texbox[1])
    t[5].set(texbox[2],texbox[1])
    t[6].set(texbox[2],texbox[3])
    t[7].set(texbox[0],texbox[3])

    #p=osg.DrawArrays(osg.PrimitiveSet.QUADS,0,8)
    p=osg.DrawArrays()
    p.setMode(osg.PrimitiveSet.QUADS)
    p.setFirst(0)
    p.setCount(8)
   
    geoNode = osg.Geode()
    geom = osg.Geometry()
    geom.setDataVariance(osg.Object.STATIC)
    geom.setVertexArray(v)
    geom.setTexCoordArray(0,t)
    geom.addPrimitiveSet(p)
 

    geom1 = osg.Geometry()
    geom1.setDataVariance(osg.Object.STATIC)
    geom1.setVertexArray(v1)
    geom1.setTexCoordArray(0,t)
    geom1.addPrimitiveSet(p)
   
    dstate = osg.StateSet()
    dstate.setAttributeAndModes(osg.BlendFunc(),osg.StateAttribute.ON)
    alphaFunc = osg.AlphaFunc()
    alphaFunc.setFunction(osg.AlphaFunc.GEQUAL,0.05)
    dstate.setAttributeAndModes(alphaFunc,osg.StateAttribute.ON)
    #equivalent of , taken from Gl.h##  dstate.setMode(GL_LIGHTING,osg.StateAttribute.ON)
    dstate.setMode(0x0B50,osg.StateAttribute.OFF)
    dstate.setRenderingHint(osg.StateSet.TRANSPARENT_BIN)
    geoNode.setStateSet(dstate)
    geoNode.addDrawable( geom )
    geoNode.addDrawable( geom1 )
    for pointer in [dstate, geom, alphaFunc,geoNode,p,t,v]:
        pointer.thisown = False
    print "scrivo -->",outfile
    osgDB.writeNodeFile(geoNode,outfile)
    
 

tbox= (0.1,0,2,0.3,0.4)

createCrossQuadTree(10.0,20.0,30.0,tbox,"D:\\test\\sil.osg")