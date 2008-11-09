import os
import shelve
import visitorbase

import osg
import osgDB

class TerrainTilesCache():
    def __init__(self,top_tile):
        if(os.path.exists(top_tile)):
            self.terrain_dir=os.path.dirname(os.path.normpath(os.path.abspath(top_tile)))
            self.tiles_cache=shelve.open(os.path.normpath(os.path.abspath(top_tile))+'.cache',writeback=True)

        else:
            print "ERROR!!! invalid top tile-->"+top_tile+"<--"
            
            
    def apply_func(tile,func):
        if(self.tiles_cache.has_key(tile)):
            if(self.tiles_cache[tile].has_key(func)):
                return self.tiles_cache[tile][func]
        else:
            self.tiles_cache[tile]=dict()
        res=func(tile)
        self.tiles_cache[tile][func]=res
        return res
            
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
        
 
#--------------------------------------------------------------------------
if __name__ == "__main__":
    
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
    print ninfeo_extent.max()
    #mybox = osg.BoundingBox(789197.193,4655829.954,-1000,789674.057,4656366.800,1000)
    #mybox = osg.BoundingBox(789197.193,4655829.954,-1000,789198.193,4655830.954,1000)
    bx = BoxIntersectTileHandler(ninfeo_extent.box())
    terrwalk=ParseTerrain(9,bx)
    #terrwalk.walk(os.path.join("H:/vrome/web/terreni/attuale/ive_16_256_dds_l12_subtile","attuale.ive"),0)
    terrwalk.walk(os.path.join("H:/vrome/web/terreni/attuale/subtile_new","attuale.ive"),0)
    print bx.tiles  