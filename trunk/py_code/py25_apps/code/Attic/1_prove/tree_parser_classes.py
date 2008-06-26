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

def close_2_power(n):
    fit =1
    while n > 2 * fit : 
        fit = 2 * fit
    if n -fit > 2*fit -n :
        return 2*fit
    else :
        return fit

#    
def fit_y_texture(in_image,min,max) :
    if in_image.size[1] < max :
        print "image too small ", in_image.size
        return in_image
    else :
        scale = max / float(in_image.size[1])
        #print scale
        if in_image.size[0] * scale < min :
            print "image too tall ", in_image.size
            tgt_x = min
        else :
            tgt_x = close_2_power(in_image.size[0] * scale)
        newsize = tgt_x, max
        #return in_image.resize(newsize, Image.ANTIALIAS ) 
        return newsize
#    
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
    
    t=osg.Vec2Array(8)
    #TODO: finisci questo bene
    
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
    
    dstate = osg.StateSet()
    blendFunc = osg.BlendFunc()
    dstate.setAttributeAndModes(blendFunc,osg.StateAttribute.ON)
    alphaFunc = osg.AlphaFunc()
    alphaFunc.setFunction(osg.AlphaFunc.GEQUAL,0.99)
    dstate.setAttributeAndModes(alphaFunc,osg.StateAttribute.ON)
    #equivalent of , taken from Gl.h##  dstate.setMode(GL_LIGHTING,osg.StateAttribute.ON)
    dstate.setMode(0x0B50,osg.StateAttribute.OFF)
    dstate.setRenderingHint(osg.StateSet.TRANSPARENT_BIN)
    geoNode.setStateSet(dstate)
    geoNode.addDrawable( geom )
    for pointer in [dstate, geom, alphaFunc,blendFunc,geoNode,p,t,v]:
        pointer.thisown = False
    print "scrivo -->",outfile
    osgDB.writeNodeFile(geoNode,outfile)
    
#      
def write_top_group_texture(tex_file,root_file,outfile) :
    g = osg.Group()
    s = osg.StateSet()
    tex = osg.Texture2D() 
    tex.setWrap(osg.Texture2D.WRAP_S,osg.Texture2D.CLAMP)
    tex.setWrap(osg.Texture2D.WRAP_T,osg.Texture2D.CLAMP)
    image = osgDB.readImageFile(tex_file)
    tex.setImage(image)
    s.setTextureAttributeAndModes(0,tex,osg.StateAttribute.ON)
    tenv = osg.TexEnv()
    s.setTextureAttribute(0,tenv)
    g.setStateSet(s)
    p = osg.ProxyNode()
    p.setFileName(0,root_file)
    print "scrivo -->",outfile
    g.addChild(p)
    for pointer in [g,s,tex,image,tenv,p]:
        pointer.thisown = False
    osgDB.writeNodeFile(g,outfile)
#        
def add_trasl_proxy(group,trasl,proxyfile):
    print "todo"
#        
class TreeGeometry(): 
    """TreeGeometry -- A Class that accumulate trres geometry."""  
    def __init__(self,maxtrees): 
        self.maxtrees = maxtrees
        self.treecount=0
        self.count = 0
        self.v=osg.Vec3Array(8*maxtrees)
        self.t=osg.Vec2Array(8*maxtrees)
##        self.v=osg.Vec3Array()
##        self.t=osg.Vec2Array()
#    
    def addtree(self, xx,yy,zz,width,hheight,rot,texbox):
        if(self.treecount < self.maxtrees):
            x = float(xx)
            y = float(yy)
            z = float(zz)
            height = float(hheight)
            #print "adding tree ",self.treecount," in ",x," ",y," z ",z
            sw = math.sin(math.radians(rot)) * width * 0.5
            cw = math.cos(math.radians(rot)) * width * 0.5
##            self.v.resize((self.treecount * 8) + 8)
##            self.t.resize((self.treecount * 8) + 8)
            self.v[(self.treecount * 8)    ].set( x - sw,y - cw,z)
            self.v[(self.treecount * 8) + 1].set( x + sw,y + cw,z)
            self.v[(self.treecount * 8) + 2].set( x + sw,y + cw,z + height) 
            self.v[(self.treecount * 8) + 3].set( x - sw,y - cw,z + height)

            self.v[(self.treecount * 8) + 4].set( x - cw,y + sw,z);
            self.v[(self.treecount * 8) + 5].set( x + cw,y - sw,z);
            self.v[(self.treecount * 8) + 6].set( x + cw,y - sw,z + height) 
            self.v[(self.treecount * 8) + 7].set( x - cw,y + sw,z + height)

            self.t[(self.treecount * 8)    ].set(texbox[0],texbox[1])
            self.t[(self.treecount * 8) + 1].set(texbox[2],texbox[1])
            self.t[(self.treecount * 8) + 2].set(texbox[2],texbox[3])
            self.t[(self.treecount * 8) + 3].set(texbox[0],texbox[3])

            self.t[(self.treecount * 8) + 4].set(texbox[0],texbox[1])
            self.t[(self.treecount * 8) + 5].set(texbox[2],texbox[1])
            self.t[(self.treecount * 8) + 6].set(texbox[2],texbox[3])
            self.t[(self.treecount * 8) + 7].set(texbox[0],texbox[3])
            
            
##            self.v.assign((self.treecount * 8)    ,osg.Vec3f( x - sw,y - cw,z))
##            self.v.assign((self.treecount * 8) + 1,osg.Vec3f( x + sw,y + cw,z))
##            self.v.assign((self.treecount * 8) + 2,osg.Vec3f( x + sw,y + cw,z + height)) 
##            self.v.assign((self.treecount * 8) + 3,osg.Vec3f( x - sw,y - cw,z + height))
##
##            self.v.assign((self.treecount * 8) + 4,osg.Vec3f( x - cw,y + sw,z))
##            self.v.assign((self.treecount * 8) + 5,osg.Vec3f( x + cw,y - sw,z))
##            self.v.assign((self.treecount * 8) + 6,osg.Vec3f( x + cw,y - sw,z + height)) 
##            self.v.assign((self.treecount * 8) + 7,osg.Vec3f( x - cw,y + sw,z + height))
##
##            self.t.assign((self.treecount * 8)    ,osg.Vec2f(texbox[0],texbox[1]))
##            self.t.assign((self.treecount * 8) + 1,osg.Vec2f(texbox[2],texbox[1]))
##            self.t.assign((self.treecount * 8) + 2,osg.Vec2f(texbox[2],texbox[3]))
##            self.t.assign((self.treecount * 8) + 3,osg.Vec2f(texbox[0],texbox[3]))
##
##            self.t.assign((self.treecount * 8) + 4,osg.Vec2f(texbox[0],texbox[1]))
##            self.t.assign((self.treecount * 8) + 5,osg.Vec2f(texbox[2],texbox[1]))
##            self.t.assign((self.treecount * 8) + 6,osg.Vec2f(texbox[2],texbox[3]))
##            self.t.assign((self.treecount * 8) + 7,osg.Vec2f(texbox[0],texbox[3]))

            self.treecount = self.treecount + 1
           
        self.count = self.count + 1 
        if (self.count % 100 == 0 ) :
            print "tree ", self.count
#            
    def write(self, outfile):
        print "inizio di write -->",outfile
        gg = self.get_geom()
        print "scrivo -->",outfile
        osgDB.writeNodeFile(gg,outfile)
#        
    def get_geom(self):
        p=osg.DrawArrays()
        p.setMode(osg.PrimitiveSet.QUADS)
        p.setFirst(0)
        p.setCount(8*self.treecount)
   
        geoNode = osg.Geode()
        geom = osg.Geometry()
        geom.setDataVariance(osg.Object.STATIC)
        ##print "prima di trim  ",self.v.size()
        self.v.resize(8*self.treecount)
        self.t.resize(8*self.treecount)
        self.v.trim()
        self.t.trim()
        ##print "dopo di trim  ",self.v.size()

        geom.setVertexArray(self.v)
        geom.setTexCoordArray(0,self.t)
        geom.addPrimitiveSet(p)
    
        dstate = osg.StateSet()
        blendFunc = osg.BlendFunc()
        dstate.setAttributeAndModes(blendFunc,osg.StateAttribute.ON)
        alphaFunc = osg.AlphaFunc()
        alphaFunc.setFunction(osg.AlphaFunc.GEQUAL,0.7)
        dstate.setAttributeAndModes(alphaFunc,osg.StateAttribute.ON)
        #equivalent of , taken from Gl.h##  dstate.setMode(GL_LIGHTING,osg.StateAttribute.ON)
        dstate.setMode(0x0B50,osg.StateAttribute.OFF)
        dstate.setRenderingHint(osg.StateSet.TRANSPARENT_BIN)
        geoNode.setStateSet(dstate)
        geoNode.addDrawable( geom )
        for pointer in [dstate, geom, alphaFunc,blendFunc,geoNode,p,self.t,self.v]:
            pointer.thisown = False
        return geoNode
    
#      
def write_top_group_texture(tex_file,root_file,outfile) :
    g = osg.Group()
    s = osg.StateSet()
    tex = osg.Texture2D() 
    tex.setWrap(osg.Texture2D.WRAP_S,osg.Texture2D.CLAMP)
    tex.setWrap(osg.Texture2D.WRAP_T,osg.Texture2D.CLAMP)
    image = osgDB.readImageFile(tex_file)
    tex.setImage(image)
    s.setTextureAttributeAndModes(0,tex,osg.StateAttribute.ON)
    tenv = osg.TexEnv()
    s.setTextureAttribute(0,tenv)
    g.setStateSet(s)
    p = osg.ProxyNode()
    p.setFileName(0,root_file)
    print "scrivo -->",outfile
    g.addChild(p)
    for pointer in [g,s,tex,image,tenv,p]:
        pointer.thisown = False
    osgDB.writeNodeFile(g,outfile)
                
#                    
def plant_on_tiles(log_file,tile_dir,out_dir):
    forest = TreeGeometry(30000)
    cur_tile = ""
    log_reader = csv.DictReader(open(log_file, "rb"))
    for row in log_reader:
        if("" == cur_tile):
            cur_tile = row['Nome_Tile']
            n = osgDB.readNodeFile(os.path.join(tile_dir,cur_tile))
#
def box_include(b1,b2):
    res = True
    i = 0
    while(res and i < 7 ):
        res = res and b1.contains(b2.corner(i))
        i = i + 1
    return res
#
class BaseTileHandler(): 
    def process(self,node_tile,level): 
        tparse = TileParse()
        tparse.walk(node_tile," ")
        self.plod_list = tparse.get_paged()  
    def leaf(self,f,filename,level):
        print "foglia livello ",level," -->",filename
class ListTileHandler(BaseTileHandler): 
    def to_walk(self):
        r=[]
        for t in self.plod_list.keys():
            bs = self.plod_list[t]['trasf'].getBound()
            bb = self.plod_list[t]['bbox']
            r.append(self.plod_list[t]['filename'])
            print t, " -->",self.plod_list[t]['filename']," ",bb.corner(0).x()," ",bb.corner(0).y()," ",bb.corner(7).x()," ",bb.corner(7).y()
        return r
class BoxIntersectTileHandler(BaseTileHandler): 
    def __init__(self,box):
        self.box=box
        self.tiles=[]
    def add_item(self,filename):
        self.tiles.append(filename)
    def to_walk(self):
        r=[]
        for t in self.plod_list.keys():
            bs = self.plod_list[t]['trasf'].getBound()
            bb = self.plod_list[t]['bbox']
            if(bb.intersects(self.box)):
                if(box_include(self.box,bb)):
                    self.add_item(self.plod_list[t]['filename'])
                else:
                    r.append(self.plod_list[t]['filename'])
                    print t, " MATCH!-->",self.plod_list[t]['filename']," ",bb.corner(0).x()," ",bb.corner(0).y()," ",bb.corner(7).x()," ",bb.corner(7).y()
                    print t, " MATCH!-->",self.plod_list[t]['filename']," ",self.box.corner(0).x()," ",self.box.corner(0).y()," ",self.box.corner(7).x()," ",self.box.corner(7).y()
##        if(1 == r.__len__()):
##            self.maxtiles[self.count]=r[0]
##        if(1 < r.__len__()):
##            self.count=self.count + 1
        return r
##        else:
##            return []
    def leaf(self,filename,level):
        print "foglia livello ",level," -->",filename
        self.add_item(filename)
       
class ParseTerrain():
    """ParseTerrain -- A Class that walk tiled terrains, takes a function to handle tiles."""  
    def __init__(self,maxlevel,handler=""):   
        self.maxlevel=maxlevel
        if(handler == ""): 
            self.handler=ListTileHandler()
        else:
            self.handler=handler
#    
    def walk(self,filename,level):
        #print "walk ",filename
        if (level > self.maxlevel):
            print "MAX LEVEL ", level, " EXEEDED"
            self.handler.leaf(filename,level)
            return
        if(os.path.exists(filename)):
            tile_dir=os.path.normpath(os.path.abspath(os.path.dirname(filename)))
            if(os.path.normpath(os.path.abspath(os.path.curdir)) != tile_dir):
                print "warning: resetting tile_dir from ",os.path.normpath(os.path.abspath(os.path.curdir)), " to ",os.path.normpath(os.path.abspath(os.path.dirname(filename)))
                tile_dir=os.path.normpath(os.path.abspath(os.path.dirname(filename)))
                os.chdir(tile_dir)
            file_to_open = os.path.basename(filename)
            node_tile = osgDB.readNodeFile(file_to_open)
            ##tparse = TileParse()
            ##tparse.walk(node_tile," ")
            ##plod_list = tparse.get_paged()
            self.handler.process(node_tile,level)
            #devo fare unref, se no fa esplodere la memoria
            node_tile.unref()
            child_list = self.handler.to_walk()
            if(child_list.__len__()):
                for t in child_list:
                    self.walk(t,level + 1)
                ##for t in plod_list.keys():
                ##    bs = plod_list[t]['trasf'].getBound()
                ##    bb = plod_list[t]['bbox']
                ##    print t, " -->",plod_list[t]['filename']," ",bb.corner(0).x()," ",bb.corner(0).y()," ",bb.corner(7).x()," ",bb.corner(7).y()
                ##    self.walk(plod_list[t]['filename'],level + 1)
                    os.chdir(tile_dir)
                    #print gc.get_count()
            else:
                self.handler.leaf(filename,level)
            
        else:
            print "missed file ",filename, " in ", os.path.normpath(os.path.abspath(os.path.curdir))
            
#            
class ParsedTile():
    def __init__(self,file): 
        self.filename = file
        self.main_n = osgDB.readNodeFile(file)
        self.splitmode = 0 #bad split
    def parse(self):
        finder = TileParse()
        finder.walk(self.main_n," ")
        plod_list = finder.get_paged()
        ##print "got ",plod_list.keys().__len__()," plods"
        for t in plod_list.keys():
            bs = plod_list[t]['trasf'].getBound()
            bb = plod_list[t]['bbox']
            #minx = bs.center().x() -bs._radius
            #miny = bs.center().y() -bs._radius
            #maxx = bs.center().x() +bs._radius
            #maxy = bs.center().y() +bs._radius
            #print t, " -->",plod_list[t]['filename']," ",minx," ",miny," ",maxx," ",maxy
            print t, " -->",plod_list[t]['filename']," ",bb.corner(0).x()," ",bb.corner(0).y()," ",bb.corner(7).x()," ",bb.corner(7).y()
        return plod_list
        #note adadadasd   
class TileParse(): 
    """FindPage -- A Class that finds paged lod."""  
    def __init__(self): 
        self.result = dict()
    #def __del__(self):
    #    print "sto morendo.... %s" % (self )
    def walk(self,node,tag):
        if(node.asGroup()):
            if(node.className() == 'PagedLOD'):
                p = node.asLOD().asPagedLOD()
                self.result[tag]=dict()
                bb = osg.BoundingBox()
                for i in range(p.getNumFileNames()):
                    
                    if(p.getFileName(i) != ''):
                        ##print "got paged -->",tag,"<-->",p.getFileName(i)
                        self.result[tag]['filename']=p.getFileName(i)
                    else:
                        tt = p.getChild(i).asTransform()
                        tt = osg.NodeToMatrixTransform(p.getChild(i))
                        if(tt):
                            #print tt.className()
                            mm = tt.getMatrix()
                            gg = osg.NodeToGeode(tt.getChild(0))
                            if(gg):
                                #if we have a geode, expand by the drawables bounding box, else use the bounding sphere
                                #print "geode found"  
                                bb2 = osg.BoundingBox()
                                for i in range(gg.getNumDrawables()):
                                    dwb = gg.getDrawable(i)
                                    bb2.expandBy(dwb.getBound());
                                    
                                bb.expandBy(mm.preMult(bb2.corner(0)))
                                bb.expandBy(mm.preMult(bb2.corner(7)))
                                
                            else:
                                bb.expandBy(tt.getBound())                                          
                            self.result[tag]['trasf'] = tt
                            self.result[tag]['bbox'] = bb
                        bs = tt.getBound()
                        
                        #print tt.className()," ",bs.center()," ",bs._radius
                        self.walk(p.getChild(i),tag + "_p_" + str(i))
                        
            else:
                g = node.asGroup()
                for i in range(g.getNumChildren()):
                    s = g.getChild(i)
                    self.walk(s,tag + " " + str(i))
        #else:
        #    if(osg.NodeToGeode(node)):
        #        print "geode ",node.getBound()
            
    def get_paged(self):
        return self.result
    def clear(self):
        return self.result.clear()
        
    
class tree_csv_handler():
    def __init__(self): 
        self.tree_types = dict()
        self.trees = dict()

        self.tree_counter = 0
        self.total_x_size=0
        
        self.base_data_dir = os.path.join(os.path.dirname(os.path.dirname(os.path.normpath(sys.argv[0]))) , "data")
        self.base_texture_dir = os.path.join(self.base_data_dir ,"texture_vegetazione")
        print "setting default texture dir to -->",self.base_texture_dir
#        
    def parse_trees(self,filename):
        file_to_open = filename
        if(os.path.isabs(filename)):
            self.base_data_dir = os.path.dirname(os.path.normpath(filename))
            print "resetting base_data_dir to -->",self.base_data_dir
        else:
            file_to_open = os.path.join(self.base_data_dir,filename)
        if(os.path.exists(file_to_open)):
            print "opening ",file_to_open
            self.reader = csv.DictReader(open(file_to_open, "rb"))            
            for row in self.reader :
                key = row['Imagename2']
                ok = 1
                if self.tree_types.has_key(key) :
                    self.tree_types[key]['num_instances'] = self.tree_types[key]['num_instances'] + 1
                    if self.tree_types[key]['Id'] != int(row['Id']) :
                        print "ERROR at line ", self.tree_counter, "Id mismatch ",  self.tree_types[key]['Id'], " != ", row['Id']
                        ok = 0
                else : 
                    texture_file = os.path.join(self.base_texture_dir,key)
                    if os.path.exists(texture_file) :
                        im = Image.open(texture_file)
                        self.tree_types[key] = dict()
                        self.tree_types[key]['num_id'] = self.tree_types.__len__()
                        self.tree_types[key]['num_instances'] = 0
                        self.tree_types[key]['Id'] = int(row['Id'])
                        self.tree_types[key]['Imagename'] = row['Imagename']
                        self.tree_types[key]['Imagename2'] = key
                        self.tree_types[key]['img_orig_size'] = im.size
                        ##new_size = fit_y_texture(im,4,128)
                        ##self.total_x_size = self.total_x_size + new_size[1]
                        ##self.tree_types[key]['img_new_size'] = new_size
                    else :
                        print "line ",self.tree_counter," missing ",key," -->", os.path.join(base_texture_dir,key)
                        ok = 0
                        
                        #print self.tree_counter," ",key," -->",self.tree_types[key]['num_id'],"<->",row['Height']
                if ok :
                    self.trees[self.tree_counter] =dict()
                    self.trees[self.tree_counter]['tree_id'] = self.tree_types[key]['num_id']
                    self.trees[self.tree_counter]['Z'] = row['Z']
                    self.trees[self.tree_counter]['X-coord'] = row['X-coord']
                    self.trees[self.tree_counter]['Y-coord'] = row['Y-coord']
                    self.trees[self.tree_counter]['Height'] = row['Height']
                self.tree_counter = self.tree_counter + 1
        else:
            print "file -->",file_to_open," NOT FOUND"
#
    def build_texture_atlas(self):
        ######   compute texture atlas and tree texture coords
        ###### 
        self.tree_types_rev = dict()
        for t in self.tree_types.keys():
            self.tree_types_rev[self.tree_types[t]['num_id']] = t
        self.tree_types_list = self.tree_types_rev.keys()
        self.tree_types_list.sort()
        x_tex=0
        y_tex=0
        x_size=0
        y_size=0
        max_texture_x = 1024
        for tr in self.tree_types_list:
            key=self.tree_types_rev[tr]
            texture_file = os.path.join(self.base_texture_dir,key)
            im = Image.open(texture_file)
            ns = fit_y_texture(im,4,128)
            self.tree_types[key]['img_new_size'] = ns
            if x_tex + ns[0] > max_texture_x:
                print x_tex + ns[0]
                x_tex=0
                y_tex =y_size
            else:
                if x_size < x_tex + ns[0] : x_size = x_tex + ns[0]
            if y_size < y_tex + ns[1] : y_size = y_tex + ns[1]
            self.tree_types[self.tree_types_rev[tr]]['tex_pix_pos'] = (x_tex,y_tex)
            x_tex = x_tex + ns[0]
            
        print x_size," ",y_size
        msize = (x_size,y_size)
        self.mosaic = Image.new('RGBA', msize, (0,0,0,0))

        for tr in self.tree_types_list:
            pos = self.tree_types[self.tree_types_rev[tr]]['tex_pix_pos']
            dim = self.tree_types[self.tree_types_rev[tr]]['img_new_size']
            tex_box = (pos[0] / float(msize[0]),pos[1] / float(msize[1]),(pos[0] + dim[0]) /  float(msize[0]),(pos[1] + dim[1]) /  float(msize[1]))
            self.tree_types[self.tree_types_rev[tr]]['tex_box'] = tex_box
            texture_file = os.path.join(self.base_texture_dir,self.tree_types_rev[tr])
            if os.path.exists(texture_file) :
                print "image ",tr," in ",pos,"  -->",texture_file
                im = Image.open(texture_file)
                self.mosaic.paste(im.resize(dim),(pos[0],pos[1],pos[0] + dim[0],pos[1] + dim[1]))
                
        ##os.chdir("D:/vrome/web/terreni/prove_vegetazione/m_test3/")
        ##mosaic.save("tmp.png")
        ##write_top_group_texture("tmp.png","romano.ive","tmp.osg")
        ##os.system("osgconv --compressed tmp.osg tmp.ive")

#
    def build_vegetation_tiles(self,filename,terrain_dir="",out_dir=""):
        def write_tile(tile_name,node_tile,trasf,veg,out_dir,suffix):
            #write previous tile
            ##print "writing tile ",prev," num trees ",tile_tree_count
            for t in trasf.keys():
                ##trasf[t].removeChild(0)
                trasf[t].addChild(veg[t].get_geom())
            outfile = os.path.join(out_dir,os.path.splitext(tile_name)[0]+suffix)
            osgDB.writeNodeFile(node_tile,outfile)
            #devo fare unref, se no fa esplodere la memoria
            node_tile.unref()

        ######      parse Andrea log files and plant trees in tiles
        file_to_open = filename
        if(os.path.isabs(filename)):
            if(terrain_dir == ""):
                terrain_dir = os.path.dirname(os.path.normpath(filename))
            print "setting terrain_dir to -->",terrain_dir
        else:
            if(terrain_dir == ""):
                terrain_dir = self.base_data_dir
            file_to_open = os.path.join(terrain_dir,filename)
        if(out_dir == ""):
            out_dir = terrain_dir
        if(os.path.exists(file_to_open)):
            print "opening ",file_to_open
            os.chdir(out_dir)
            self.mosaic.save("tmp.png")
            write_top_group_texture("tmp.png","romano_L0_X0_Y0_subtile.ive","tmp.osg")
            os.system("osgconv --compressed tmp.osg tmp.ive")
            tree_log_reader = csv.DictReader(open(file_to_open, "rb"))
            prev = ""
            for row in tree_log_reader :
                tree_id = int(row['ID_Pianta'])
                tile_name = row['Nome_Tile']
                id_foglia = int(row['Numero_Tile_Figlia'])
                x = float(row['x'])
                y = float(row['y'])
                z = float(row['z'])
                if(prev != tile_name):
                    if(prev != ""):
                        #write previous tile
                        print "writing tile ",prev," num trees ",tile_tree_count
                        #write_tile(prev,node_tile,trasf,veg,out_dir,"_newveg.ive")
                        write_tile(prev,node_tile,trasf,veg,out_dir,".ive")
##                        for t in trasf.keys():
##                            trasf[t].removeChild(0)
##                            trasf[t].addChild(veg[t].get_geom())
##                        outfile = os.path.join(out_dir,os.path.splitext(prev)[0]+"_newveg.ive")
##                        osgDB.writeNodeFile(node_tile,outfile)
##                        #devo fare unref, se no fa esplodere la memoria
##                        node_tile.unref()

                    full_tile_name = os.path.join(terrain_dir,tile_name)
                    if(os.path.exists(full_tile_name)):
                        tile_tree_count = 0
                        veg = dict()
                        trasf = dict()
                        os.chdir(os.path.dirname(full_tile_name))
                        node_tile = osgDB.readNodeFile(os.path.basename(full_tile_name))
                        g = node_tile.asGroup()
                        for i in range(g.getNumChildren()):
                            ##print tile_name, "  ->",i
                            child = g.getChild(i)
                            if(child.className() == 'PagedLOD'):
                                p = child.asLOD().asPagedLOD()
                                for j in range(p.getNumFileNames()):
                                    if(p.getFileName(j) == ''):
                                        tt = osg.NodeToMatrixTransform(p.getChild(j))
                                    else:
                                        ##print "pagedlod -->",p.getFileName(j)
                                        #p.setFileName(j,os.path.splitext(p.getFileName(j))[0]+"_newveg.ive")
                                        p.setFileName(j,os.path.splitext(p.getFileName(j))[0]+".ive")
                            else:
                                tt = osg.NodeToMatrixTransform(child)
                            if(tt):
                                ##print tt.className()
                                gg = osg.NodeToGeode(tt.getChild(0))
                                if(gg):
                                    ##print "tile ",tile_name," matched geode ",i 
                                    veg[i] = TreeGeometry(5000)
                                    trasf[i] = tt

                    prev = tile_name

                tile_tree_count = tile_tree_count + 1
                ##print "insert tree ",tree_id," ",self.trees[int(tree_id)]['Height']
                if veg.has_key(id_foglia) :
                    #print "sono qui ",id_foglia
                    veg[id_foglia].addtree(x,y,z,10.0,self.trees[tree_id]['Height'],30.0,self.tree_types[self.tree_types_rev[self.trees[tree_id]['tree_id']]]['tex_box'])

            print "writing ultimo tile ",prev," num trees ",tile_tree_count
            #write_tile(prev,node_tile,trasf,veg,out_dir,"_newveg.ive")
            write_tile(prev,node_tile,trasf,veg,out_dir,".ive")

    
#    
def dump(aa):
    for k,v in aa.__dict__.iteritems() : print k,"=",v
    
#

if __name__ == '__main__':
##    import sys,os
##    import run
##    run.main(['', os.path.basename(sys.argv[0])] + sys.argv[1:])
    tree_handler = tree_csv_handler()
    tree_handler.parse_trees("piante_coords.txt")
    tree_handler.build_texture_atlas()
##    tree_handler.build_vegetation_tiles("D:/vrome/web/terreni/prove_vegetazione/test_singletext/log.txt")
    tree_handler.build_vegetation_tiles("out/log.txt","Q:/virtualrome/web/terreni/romano/ive_16_256_dds_nos_full","Q:/virtualrome/web/terreni/romano/ive_16_256_dds_nos_full/out")
##    base_data_dir = os.path.join(os.path.dirname(os.path.dirname(os.path.normpath(sys.argv[0]))) , "data")
##    print base_data_dir
##    base_texture_dir = os.path.join(base_data_dir ,"texture_vegetazione")
##    reader = csv.DictReader(open(os.path.join(base_data_dir , "piante_coords.txt"), "rb"))

