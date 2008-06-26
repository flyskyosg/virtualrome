
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

def close_2_power(n):
    fit =1
    while n > 2 * fit : 
        fit = 2 * fit
    if n -fit > 2*fit -n :
        return 2*fit
    else :
        return fit

    
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
    dstate.setAttributeAndModes(osg.BlendFunc(),osg.StateAttribute.ON)
    alphaFunc = osg.AlphaFunc()
    alphaFunc.setFunction(osg.AlphaFunc.GEQUAL,0.05)
    dstate.setAttributeAndModes(alphaFunc,osg.StateAttribute.ON)
    #equivalent of , taken from Gl.h##  dstate.setMode(GL_LIGHTING,osg.StateAttribute.ON)
    dstate.setMode(0x0B50,osg.StateAttribute.OFF)
    dstate.setRenderingHint(osg.StateSet.TRANSPARENT_BIN)
    geoNode.setStateSet(dstate)
    geoNode.addDrawable( geom )
    for pointer in [dstate, geom, alphaFunc,geoNode,p,t,v]:
        pointer.thisown = False
    print "scrivo -->",outfile
    osgDB.writeNodeFile(geoNode,outfile)
    
  
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
    
def add_trasl_proxy(group,trasl,proxyfile):
    print "todo"
    
class TreeGeometry(): 
    """TreeGeometry -- A Class that accumulate trres geometry."""  
    def __init__(self,maxtrees): 
        self.maxtrees = maxtrees
        self.treecount=0
        self.count = 0
        self.v=osg.Vec3Array(8*maxtrees)
        self.t=osg.Vec2Array(8*maxtrees)

    def addtree(self, xx,yy,zz,width,hheight,rot,texbox):
        if(self.treecount < self.maxtrees):
            x = float(xx)
            y = float(yy)
            z = float(zz)
            height = float(hheight)
            #print "adding tree ",self.treecount," in ",x," ",y," z ",z
            sw = math.sin(math.radians(rot)) * width * 0.5
            cw = math.cos(math.radians(rot)) * width * 0.5
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
            self.treecount = self.treecount + 1
           
        self.count = self.count + 1 
        if (self.count % 100 == 0 ) :
            print "tree ", self.count
        
    def write(self, outfile):
        print "inizio di write -->",outfile
        gg = self.get_geom()
        print "scrivo -->",outfile
        osgDB.writeNodeFile(gg,outfile)
    
    def get_geom(self):
        p=osg.DrawArrays()
        p.setMode(osg.PrimitiveSet.QUADS)
        p.setFirst(0)
        p.setCount(8*self.treecount)
   
        geoNode = osg.Geode()
        geom = osg.Geometry()
        geom.setDataVariance(osg.Object.STATIC)
        geom.setVertexArray(self.v)
        geom.setTexCoordArray(0,self.t)
        geom.addPrimitiveSet(p)
    
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
        for pointer in [dstate, geom, alphaFunc,geoNode,p,self.t,self.v]:
            pointer.thisown = False
        return geoNode
    
  
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
                
                
def plant_on_tiles(log_file,tile_dir,out_dir):
    forest = TreeGeometry(30000)
    cur_tile = ""
    log_reader = csv.DictReader(open(log_file, "rb"))
    for row in log_reader:
        if("" == cur_tile):
            cur_tile = row['Nome_Tile']
            n = osgDB.readNodeFile(os.path.join(tile_dir,cur_tile))
        

class FindPage(): 
    """FindPage -- A Class that finds paged lod."""  
    def __init__(self): 
        self.result = dict()
    def walk(self,node,tag):
        if(node.asGroup()):
            if(node.className() == 'PagedLOD'):
                p = node.asLOD().asPagedLOD()
                for i in range(p.getNumFileNames()):
                    if(p.getFileName(i) != ''):
                        print "got paged -->",tag,"<-->",p.getFileName(i)
                    else:
                        self.walk(p.getChild(i),tag + "_p_" + str(i))
                        
            else:
                g = node.asGroup()
                for i in range(g.getNumChildren()):
                    s = g.getChild(i)
                    self.walk(s,tag + "_p_" + str(i))
        else:
            print "got -->",node.className()
            

#n = osgDB.readNodeFile(os.path.join("D:/vrome/web/terreni/prove_vegetazione/test4","romano.ive"))

base_data_dir = os.path.join(os.path.dirname(os.path.dirname(os.path.normpath(sys.argv[0]))) , "data")
print base_data_dir
base_texture_dir = os.path.join(base_data_dir ,"texture_vegetazione")
reader = csv.DictReader(open(os.path.join(base_data_dir , "piante_coords.txt"), "rb"))
tree_types = dict()
trees = dict()

tree_counter = 0
total_x_size=0
for row in reader:
    key = row['Imagename2']
    ok = 1
    if tree_types.has_key(key) :
        tree_types[key]['num_instances'] = tree_types[key]['num_instances'] + 1
        if tree_types[key]['Id'] != int(row['Id']) :
            print "ERROR at line ", tree_counter, "Id mismatch ",  tree_types[key]['Id'], " != ", row['Id']
            ok = 0
    else : 
        texture_file = os.path.join(base_texture_dir,key)
        if os.path.exists(texture_file) :
            im = Image.open(texture_file)
            tree_types[key] = dict()
            tree_types[key]['num_id'] = tree_types.__len__()
            tree_types[key]['num_instances'] = 0
            tree_types[key]['Id'] = int(row['Id'])
            tree_types[key]['Imagename'] = row['Imagename']
            tree_types[key]['Imagename2'] = key
            tree_types[key]['img_orig_size'] = im.size
            new_size = fit_y_texture(im,4,128)
            total_x_size = total_x_size + new_size[1]
            tree_types[key]['img_new_size'] = new_size
        else :
            print "line ",tree_counter," missing ",key," -->", os.path.join(base_texture_dir,key)
            ok = 0
    #print tree_counter," ",key," -->",tree_types[key]['num_id'],"<->",row['Height']
    if ok :
        trees[tree_counter] =dict()
        trees[tree_counter]['tree_id'] = tree_types[key]['num_id']
        trees[tree_counter]['Z'] = row['Z']
        trees[tree_counter]['X-coord'] = row['X-coord']
        trees[tree_counter]['Y-coord'] = row['Y-coord']
        trees[tree_counter]['Height'] = row['Height']
    tree_counter = tree_counter + 1

print "texture x size ",total_x_size
######   reordering list of type trees
tree_types_rev = dict()
for t in tree_types.keys():
    tree_types_rev[tree_types[t]['num_id']] = t
tree_types_list = tree_types_rev.keys()
tree_types_list.sort()

x_tex=0
y_tex=0
x_size=0
y_size=0
max_texture_x = 1024
for tr in tree_types_list:
    ns = tree_types[tree_types_rev[tr]]['img_new_size']
    if x_tex + ns[0] > max_texture_x:
        print x_tex + ns[0]
        x_tex=0
        y_tex =y_size
    else:
        if x_size < x_tex + ns[0] : x_size = x_tex + ns[0]
    if y_size < y_tex + ns[1] : y_size = y_tex + ns[1]
    tree_types[tree_types_rev[tr]]['tex_pix_pos'] = (x_tex,y_tex)
    x_tex = x_tex + ns[0]

print x_size," ",y_size
msize = (x_size,y_size)
mosaic = Image.new('RGBA', msize, (0,0,0,0))

for tr in tree_types_list:
    pos = tree_types[tree_types_rev[tr]]['tex_pix_pos']
    dim = tree_types[tree_types_rev[tr]]['img_new_size']
    tex_box = (pos[0] / float(msize[0]),pos[1] / float(msize[1]),(pos[0] + dim[0]) /  float(msize[0]),(pos[1] + dim[1]) /  float(msize[1]))
    tree_types[tree_types_rev[tr]]['tex_box'] = tex_box
    texture_file = os.path.join(base_texture_dir,tree_types_rev[tr])
    if os.path.exists(texture_file) :
        print "image ",tr," in ",pos,"  -->",texture_file
        im = Image.open(texture_file)
        mosaic.paste(im.resize(dim),(pos[0],pos[1],pos[0] + dim[0],pos[1] + dim[1]))
        
os.chdir("D:/vrome/web/terreni/prove_vegetazione/m_test3/")
mosaic.save("tmp.png")
write_top_group_texture("tmp.png","romano.ive","tmp.osg")
os.system("osgconv --compressed tmp.osg tmp.ive")
#print tree_types_list
#print tree_types_rev

type_writer = csv.DictWriter(open("piante_coords_tipi.csv", "wb"),['num_id','Imagename2','Id','Imagename','img_orig_size','img_new_size','tex_pix_pos','tex_box','num_instances'])
tree_writer = csv.DictWriter(open("piante_coords_indici.csv", "wb"),['tree_id','X-coord','Y-coord','Z','Height'])

for tr in tree_types_list:
    type_writer.writerow(tree_types[tree_types_rev[tr]])
#    createCrossQuadTree(10.0,20.0,30.0,tree_types[tree_types_rev[tr]]['tex_box'],os.path.join(base_texture_dir,"osg",tree_types_rev[tr].rsplit('.',1)[0]) + ".ive")
    createCrossQuadTree(10.0,20.0,30.0,tree_types[tree_types_rev[tr]]['tex_box'],tree_types_rev[tr].rsplit('.',1)[0] + ".ive")
    print tr," ",tree_types_rev[tr]," -->", tree_types[tree_types_rev[tr]]


    

forest = TreeGeometry(30000)
for p in trees.keys():
    tree_writer.writerow(trees[p])
    forest.addtree(trees[p]['X-coord'],trees[p]['Y-coord'],trees[p]['Z'],10.0,trees[p]['Height'],30.0,tree_types[tree_types_rev[trees[p]['tree_id']]]['tex_box'])

forest.write("foresta.osg")
os.system("osgconv --compressed foresta.osg foresta.ive")
write_top_group_texture("tmp.png","foresta.ive","tmp1.osg")
os.system("osgconv --compressed tmp1.osg tmp1.ive")

#print tree_types
#    print row['Height']




#import pick_module
#
#viewer = osgViewer.Viewer()
#viewer.setThreadingModel(osgViewer.Viewer.SingleThreaded)
#
#    #set the scene data
#    #n = osgDB.readNodeFile('cow.osg')   
#root = osg.Group()
##n = osgDB.readNodeFile("D:/models/demo_virtrome/piante/frassino.ive")
##n = osgDB.readNodeFile("Q:/virtualrome/web/modelli/romano/villa/prove_stanze/tmp/prove_stanze_mod.osg")
#n = osgDB.readNodeFile("D:/devel/prove_models/schiavo_marco/prova2.osg")
##n = osgDB.readNodeFile("Q:/vrome/web/modelli/attuale/generati/Malborghetto_attuale_osg/osg_exp_tiny_full.ive")
#root.addChild(n)
#viewer.setSceneData(root.__disown__())
#
#pickhandler = pick_module.PickHandler()
#
#viewer.addEventHandler(pickhandler.__disown__());
#
#wx_osg_console.OpenConsole(viewer)

