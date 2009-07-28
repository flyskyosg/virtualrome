import os
import glob
import Image
import re
import osg
import osgDB
import osgViewer
import osgUtil
from visitorbase import Visitor







#--------------------------------------------------------------------------
class BaseOsgFileTranslator(object): 
    ''' base class for Osg files transformation '''

    def __init__(self):
        self.basepath=""
        self.tmpdir = ""
        self.outdir = ""
        self.rel_imgdir = ""
        self.name = ""
        self.verbose=True
        for e in ['TMPDIR','TEMP']:
            if os.environ.has_key(e):
                self.tmpdir = os.environ[e]
##                if(self.verbose):
##                    print "in ",self.__class__," constructor tmpdir set to ",
##                    print self.tmpdir
                break

    def SetOsgFilePath(self,paths=[],Append=False):
        if not paths:
            paths=[os.path.join(self.basepath,self.rel_imgdir)]
        if(Append):
            ofp=os.environ['OSG_FILE_PATH']
        else:
            ofp=""
        for p in paths:
            if(ofp):
                ofp=ofp+os.pathsep+os.path.abspath(os.path.normpath(p))
            else:
                ofp=os.path.abspath(os.path.normpath(p))
        os.environ['OSG_FILE_PATH']=ofp
        print 'OSG_FILE_PATH='+os.environ['OSG_FILE_PATH']
        
    def FullExtFileName(self,relpath):
        if(os.path.exists(relpath)):
            return os.path.abspath(relpath)
        else:
            return os.path.abspath(os.path.join(self.basepath,relpath))
    
    def LoadFile(self,relpath):
        filename=self.FullExtFileName(relpath)
        if(os.path.exists(filename)):
            node = osgDB.readNodeFile(filename)
            if(node):
                (p,n)=os.path.split(filename)
                self.name=os.path.splitext(n)[0]
                if(self.basepath):
                    if(os.path.abspath(self.basepath) != p):
                        print "WARNING changing basepath from -->"+self.basepath+"<- to --->"+p+"<-"
                        self.basepath=p
                        #TODO controllare altri membri tipo rel_imgdir outdir
                else:
                        print "WARNING setting basepath to -->"+p+"<-"
                        self.basepath=p
                return node
            else:
                print "cannot load -->"+filename+'<--'
            
        
        
    

#--------------------------------------------------------------------------
class BaseHandler(object): 
    ''' base class for Osg node transformations '''
    def node_handler(self,node,id=""):
        print "handling ",node.className()," id:",id



#--------------------------------------------------------------------------
class ExternalVisitor(BaseOsgFileTranslator,Visitor): 
    def __init__(self,visitAllInstances=False):
        Visitor.__init__(self,visitAllInstances)
        BaseOsgFileTranslator.__init__(self)
        self.curr_fname=""
        self.curr_path=""
    
    def walk(self,fname):
        filename = os.path.join(self.basepath ,fname)
        if os.path.exists(fname):
            sel
        node = osgDB.readNodeFile(filename)
        if(node):
            node.accept(self)




#--------------------------------------------------------------------------
class GeomToProxy(BaseOsgFileTranslator,Visitor): 
    ''' find all geometry and make it a proxy '''

    def __init__(self):
        Visitor.__init__(self,False)
        BaseOsgFileTranslator.__init__(self)

        self.geodes=dict()
        self.stateset=dict()
        self.drawables=dict()
      

    def visitGeode(self, g):
        '''virtual func to be redefined -- return False to stop visiting this branch'''
        if g:
            if self.geodes.has_key(g.this.__hex__()) :
                print "Geode instance  FOUND !!!!"
            else:                        
                self.geodes[g.this.__hex__()]=g
        return True

    def visitDrawable(self,d,g):
        '''virtual func to be redefined'''
        if d:
            if self.drawables.has_key(d.this.__hex__()) :
                print "Drawable instance  FOUND !!!!"
            else:                        
                self.drawables[d.this.__hex__()]=(d,g)
        return False
    



#--------------------------------------------------------------------------
class FindTextures(Visitor): 
    ''' find all the textures and images '''

    def __init__(self):
        Visitor.__init__(self,True)
        self.textures=dict()
        self.images=dict()
        self.parents=dict()
        self.basepath=""
    def set_base_path(self,path):
        self.basepath=path
    
    def visitGeode(self,ge): 
        print "GEODE---->"   
        return Visitor.visitGeode(self,ge)
    def visitStateSet(self,ss, parent):
        ''' parent noto '''
        
        if ss:
            for i in [0,1,2,3]:
                tex = ss.getTextureAttribute(i,osg.StateAttribute.TEXTURE)
                if tex:
                    #print "got texture channel", i, " -->",tex.className()
                    tx = osg.StateAttributeToTexture(tex)
                    if tx:
                        if self.textures.has_key(tx.this.__hex__()) :
                            print "DUPLICATE TEX FOUND !!!!"
                        else:
                            im = tx.getImage(0)
                            if(im):
                                tex_file_name=im.getFileName()
                                print "filename->",tex_file_name,"<-size->",im.s(),"<->",im.t()
                        
                                self.textures[tx.this.__hex__()]=(tx,tex_file_name,ss,i)

                                if self.images.has_key(tex_file_name) :
                                    print "DDUPLICATE IMAGE FOUND !!!!"           
                                else:
                                    self.images[tex_file_name] = ((im.s(),im.t()),[],[])
                                    #print "filename->",im.getFileName(),"<-size->",im.s(),"<->",im.t()
                                self.images[tex_file_name][1].append(str(tx.this))

                            else:
                                print "texture missing image"
  
#--------------------------------------------------------------------------

def MultiLoad(basepath,patlist,useproxy=False,optimize=True,handler=None):
    opt=osgUtil.Optimizer()
    optimization_flags= osgUtil.Optimizer.SHARE_DUPLICATE_STATE | osgUtil.Optimizer.OPTIMIZE_TEXTURE_SETTINGS | osgUtil.Optimizer.REMOVE_LOADED_PROXY_NODES

    top=osg.Group()
    for p in patlist:
        #print "globbing-->",p
        flist=glob.glob1(basepath,p)
        for rf in flist:
            ff = os.path.join(basepath,rf)

            if os.path.exists(ff):
                #print "loading-->" , ff
                node = osgDB.readNodeFile(ff)
                opt.optimize(node,optimization_flags)

                if node :
                    if(handler):
                        (name,ext)=os.path.splitext(rf)
                        handler.node_handler(node,name)
                    if(useproxy):
                        pr=osg.ProxyNode()
                        pr.setLoadingExternalReferenceMode(osg.ProxyNode.DEFER_LOADING_TO_DATABASE_PAGER)
                        pr.setFileName(0,rf.replace("\\","/"))
                        top.addChild(pr)
                    else:
                        top.addChild(node)
                else:
                    print 'error loading', filename
            else:
                 print "missing-->" , ff
    top.thisown=False
    return top   




#--------------------------------------------------------------------------


class FileWriter(BaseOsgFileTranslator):
    ''' base class for controlled file writing '''

    def __init__(self):
        
        BaseOsgFileTranslator.__init__(self)
        
        self.with_textures = 0
        self.no_textures = 1
        self.incl_ext_ref = 0
        self.orig_ext_ref = 0
        self.iveout=1
        self.usetmp=1
        self.dummyfile='dummy'
        self.lastfile=""
        self.BuildOptionString()

        ##self.optionstring = "noTexturesInIVEFile useOriginalExternalReferences noWriteExternalReferenceFiles"
        
    def BuildOptionString(self):
        if(self.iveout):
            self.BuildIVEOptionString()
            self.out_fmt='.ive'
        else:
            self.out_fmt='.osg'
            self.suffix=""
   
    def BuildIVEOptionString(self):
        self.optionstring = "noWriteExternalReferenceFiles"
        self.suffix=""
        if(self.no_textures):
            self.optionstring = self.optionstring + " noTexturesInIVEFile"
            self.suffix=self.suffix + "_tn"
        else:
            if(self.with_textures):
                self.optionstring = self.optionstring + " includeImageFileInIVEFile"
                self.suffix=self.suffix + "_ti"

        if(self.incl_ext_ref):
            self.optionstring = self.optionstring + " inlineExternalReferencesInIVEFile"
            self.suffix=self.suffix + "_ei"

        else:
            if(self.orig_ext_ref):
                self.optionstring = self.optionstring + " useOriginalExternalReferences"
                self.suffix=self.suffix + "_eo"
       
    def SetOptionString(self,string):
        self.optionstring = string

    def WriteNode(self,node,basefile="",fullpath=False):
        ##print node
        if(node):
            if(basefile and fullpath):
                outfile=basefile+self.out_fmt
            else:
                if self.usetmp :
                    if not  os.path.exists(self.tmpdir) :
                        os.mkdir(self.tmpdir)
                    if not basefile :
                        basefile = self.dummyfile
                    outfile=os.path.join(self.tmpdir , basefile + self.suffix +self.out_fmt)
                else :
                    if not basefile :
                        print "error: empty basefile valid only in tmpdir"
                        return False
                    outfile=os.path.join(self.outdir , basefile + self.suffix +self.out_fmt)
                    if(os.path.exists(outfile)):
                        print "error: overrwriting only in tmpdir: file",outfile," exist"
                        return False
            
            current_dir = os.path.abspath(os.curdir)        
            #os.chdir(self.basepath)
            os.chdir(os.path.join(self.basepath,self.rel_imgdir))
            ret=osgDB.writeNodeFile_s(node, outfile, self.optionstring )  
            os.chdir(current_dir)     
            if(ret):
                self.lastfile=outfile
                return outfile
        else:
            print "error: invalid node in WriteNode"
            return ""

#--------------------------------------------------------------------------


class StringRepOsg(FileWriter):

    def __init__(self):
        
        FileWriter.__init__(self)
        self.iveout=0
        self.BuildOptionString()

    def Node2str(self,node,basefile=""):
        if(self.WriteNode(node,basefile)):
            f=open(self.lastfile, 'rb')
            s=f.read()
            f.close()
            return s
        else:
            return ""

    def StateSet2str(self,ss_in,basefile=""):
        g=osg.Group()
        g.setStateSet(ss_in)
##        ss=osg.StateSet(g.getStateSet())
##        sst=osg.StateSet()
##        for i in [0,1,2,3]:
##            tex = ss.getTextureAttribute(i,osg.StateAttribute.TEXTURE)
##            if tex:
##                #print "got texture channel", i, " -->",tex.className()
##                tx = osg.StateAttributeToTexture(tex)
##                if tx:
##                    sst.setTextureAttribute(i,tex)
##                    ss.removeTextureAttribute(i,osg.StateAttribute.TEXTURE)
##
##        #print "prova-->",ss.getTextureAttributeList().next()
##        gg.setStateSet(ss)
##        gg1.setStateSet(sst)
##        gg.addChild(gg1)
##        gg.thisown=False
##        gg1.thisown=False
##        ss.thisown=False
##        sst.thisown=False
        g.thisown=False
        return self.Node2str(g,basefile)


#--------------------------------------------------------------------------


class FileSizer(FileWriter):

    def __init__(self,verbose=False):
        self.verbose=verbose
        FileWriter.__init__(self)

    def EvaluateFileSize(self,path):
        (p,n) = os.path.split(path) 
        #os.chdir(p)
        (b,e)= os.path.splitext(n)
        if(self.verbose):
            print "EvaluateFileSize",p + "<<>>" + b + "<<>>" + e
        root = osgDB.readNodeFile( path)
        return self.EvaluateNodeSize(root,b)
    
    def EvaluateNodeSize(self,node,basefile=""):
        ##print node
        if(self.WriteNode(node,basefile)):
            return int(os.stat(self.lastfile)[6])
        else:
            print "error in WriteNode"
            return -1

##        if(node):
##            if not  os.path.exists(self.tmpdir) :
##                os.mkdir(self.tmpdir)
##            tmpfile=os.path.join(self.tmpdir , basefile + self.suffix +".ive")
##            if(osgDB.writeNodeFile_s(node, tmpfile, self.optionstring )):
##                self.lastfile=tmpfile
##                return int(os.stat(tmpfile)[6])
##        else:
##            print "error: invalid node in EvaluateNodeSize"
##            return -1

        

#--------------------------------------------------------------------------
class SizeHandler(BaseHandler,FileSizer): 
    ''' base class for sizing Osg nodes,'''
    def __init__(self):
        
        FileSizer.__init__(self)
    def node_handler(self,node,id=""):
        if(node):
            ##print node
            if(id):
                print "size of ",id,
                size = self.EvaluateNodeSize(node,id)
                print " = ",size

    def file_handler(self,file):
        if(id):
            print "size of ext file ",file,
            size=self.EvaluateFileSize(os.path.join(self.basepath,file))
            print " = ",size


#--------------------------------------------------------------------------
class InfoHandler(BaseHandler,FileSizer): 
    ''' base class for printing info on Osg nodes,'''
    def __init__(self):
        FileSizer.__init__(self)
    def node_handler(self,node,id=""):
        if(node):
            ##print node
            if(id):
                self.with_textures = 0
                self.no_textures = 1
                self.incl_ext_ref = 0
                self.orig_ext_ref = 1
                self.iveout=1

                self.BuildOptionString()
                size_tn = self.EvaluateNodeSize(node,id)
                
                self.no_textures = 0
                self.with_textures = 1
                self.BuildOptionString()
                size_ti = self.EvaluateNodeSize(node,id)
                
                multisize=(size_tn,size_ti)
            bs=node.getBound()
            c=bs.center()
            r=bs._radius
            bnd=(c.x(),c.y(),c.z(),r)
            v1=ListFileComponents(False)
            v1.verbose=False
            v1.basepath=self.basepath
            v1.visit_loaded_childs=True
            node.accept(v1)
            v1.ProcessImages()
            
            print id,multisize,bnd
            


    def file_handler(self,file):
        if(id):
            print "size of ext file ",file,
            size=self.EvaluateFileSize(os.path.join(self.basepath,file))
            print " = ",size



#--------------------------------------------------------------------------
class BaseRecursiveHandler(BaseHandler,BaseOsgFileTranslator): 
    ''' base class for sizing Osg nodes,'''
    def __init__(self,filepath):
        
        BaseOsgFileTranslator.__init__(self)
        
    def node_handler(self,node,id=""):
        if(node):
            ##print node
            if(id):
                print "size of ",id,
                size = self.EvaluateNodeSize(node,id)
                print " = ",size

    def file_handler(self,file):
        if(id):
            print "size of ext file ",file,
            size=self.EvaluateFileSize(os.path.join(self.basepath,file))
            print " = ",size




#--------------------------------------------------------------------------
class PrintStateset(StringRepOsg,Visitor): 
    ''' list all proxy '''

    def __init__(self,visitAllInstances=False):
        Visitor.__init__(self,visitAllInstances)
        StringRepOsg.__init__(self)

        self.stateset=dict()
        
    def visitStateSet(self,ss,parent):
        '''virtual func stateset '''
        print ss.className(), ss.getName(),"parent: ",parent.className()
        self.stateset[ss.this.__hex__()]=(ss,parent)
        
    def print_ss(self):
        for k in self.stateset.keys():
            (ss,par)=self.stateset[k]
            print "sono qui",par.getName()
            ss_split=split_texture_stateset(ss)
            if(ss_split):
                print self.StateSet2str(ss_split[0])
                print "########################################"
                print self.StateSet2str(ss_split[1])
            else:
                print self.StateSet2str(ss)


#--------------------------------------------------------------------------
def close_2_power(n):
    fit =1
    while n > 2 * fit : 
        fit = 2 * fit
    if n -fit > 2*fit -n :
        return 2*fit
    else :
        return fit

def get_bnd(node):
    if(node):
        bs=node.getBound()
        c=bs.center()
        r=bs._radius
        bnd=(c.x(),c.y(),c.z(),r)            
        return bnd
    
def build_binary_plod(node,filepath,dist,center=None,dmax=10000.0):
    lod=osg.PagedLOD()
    if(center):
        lod.setCenter(osg.Vec3(center[0],center[1],center[2]))
    else:
        lod.setCenterMode(osg.LOD.USE_BOUNDING_SPHERE_CENTER)
    if(node): 
        lod.setFileName(0,"")
        lod.setFileName(1,filepath.replace("\\","/"))
        lod.setRange(0,dist,dmax)
        lod.setRange(1,0.0,dist)
        lod.addChild(node)
    else:
        #this case there is just a range and no children
        lod.setFileName(0,filepath.replace("\\","/"))
        lod.setRange(0,0.0,dist)
       
    lod.thisown=False
    return lod

#--------------------------------------------------------------------------
def substnode(node,subst):
    for i in range(0,node.getNumParents()):
        parent=node.getParent(i)
        if(parent):
            for j in range(0,parent.getNumChildren()):
                if(parent.getChild(j).this==node.this):
                    if(parent.this != subst.this): 
                        #avoid replacing inside subst itself (if contains node as a direct child (pagelod case)
                        #TODO more general method for avoinding recursion
                        parent.setChild(j,subst)
                        print "substituted ",node.getName()," -->",parent.getName()
                        

#--------------------------------------------------------------------------
def split_texture_stateset(in_ss):
    ss=osg.StateSet(in_ss)
    sst=osg.StateSet()
    textures=0
    for i in [0,1,2,3]:
        tex = ss.getTextureAttribute(i,osg.StateAttribute.TEXTURE)
        if tex:
            #print "got texture channel", i, " -->",tex.className()
            tx = osg.StateAttributeToTexture(tex)
            if tx:
                textures = textures + 1
                sst.setTextureAttribute(i,tex)
                ss.removeTextureAttribute(i,osg.StateAttribute.TEXTURE)
    ss.thisown=False
    sst.thisown=False
    if(textures):
        return (ss,sst)
    else:
        return None


#--------------------------------------------------------------------------
def split_geometry(in_geode):
            print "geode-->",in_geode.getNumDrawables()
            for j in range(0,in_geode.getNumDrawables()):
                d  = g.getDrawable(j);
                if(d):
                    geode_new=osg.Geode()
                    geode_new.addDrawable(d)
                    pnode_new=osg.ProxyNode()
                    pnode_new.addChild(geode_new)
                    #write geode to ext file and set pnode filename to the new file
                    group_new=osg.Group()
                    dst = d.getStateSet()
                    if dst:                        
                        ss_split=split_texture_stateset(dst)
                        if(ss_split):
                            group_tex=osg.Group()
                            group_tex.setStateSet(ss_split[1])
                            group_tex.addChild(pnode_new)
                            group_new.setStateSet(ss_split[0])
                            group_new.addChild(group_tex)
                        else:
                            group_new.setStateSet(dst)
                            group_new.addChild(pnode_new)
                            
##                print self.StateSet2str(ss_split[0])
##                print "########################################"
##                print self.StateSet2str(ss_split[1])
##            else:
##                print self.StateSet2str(ss)
##                       for i in [0,1,2,3]:
##                            tex = dst.getTextureAttribute(i,osg.StateAttribute.TEXTURE)
##                            if tex:
##                                print "got texture channel", i, " -->",tex.className()
##                                tx = osg.StateAttributeToTexture(tex)
##                                if tx:
##                                    if self.textures.has_key(str(tx.this)) :
##                                        print "DUPLICATE TEX FOUND !!!!"
##                                    else:
##                                        im = tx.getImage(0)
##                                        if(im):
##                                            self.geodes[str(g.this)][1].append(str(tx.this))
##                                            tex_file_name=im.getFileName()
##                                            print "filename->",tex_file_name,"<-size->",im.s(),"<->",im.t()
##                                    
##                                            self.textures[str(tx.this)]=(tx,tex_file_name,dst,i)
####                                            image_attrib=(tx,im.s(),im.t())
##
##                                            if self.images.has_key(tex_file_name) :
##                                                print "DUPLICATE IMAGE FOUND !!!!"           
####                                                if self.images[tex_file_name] != image_attrib:
####                                                    print "previous image -->",self.images[tex_file_name]," now-->",image_attrib
##                                            else:
##                                                self.images[tex_file_name] = ((im.s(),im.t()),[],[])
##                                                #print "filename->",im.getFileName(),"<-size->",im.s(),"<->",im.t()
##                                            self.images[tex_file_name][1].append(str(tx.this))
##
##                                        else:
##                                            print "texture missing image"

#--------------------------------------------------------------------------
class PlodToProxy(Visitor): 
    ''' change all plods to proxynodes '''

    def __init__(self):
        Visitor.__init__(self,True)
        self.plods=dict()
        self.basepath=""
    def set_base_path(self,path):
        self.basepath=path
    
    def visitPagedLOD(self,plod): 
        if plod:
            if self.plods.has_key(plod.this.__hex__()) :
                print "PagedLOD instance  FOUND !!!!"
            else:                        
                self.plods[plod.this.__hex__()]=plod
                print plod
                print "unique plod childs: ",plod.getNumChildren(), " files: ",plod.getNumFileNames()
        return False
    
    def substPagedLOD(self):
        for k in self.plods.keys():
            p=self.plods[k]
            if p.getNumChildren() == 0 and p.getNumFileNames() == 1:
                proxy=osg.ProxyNode()
                proxy.setFileName(0,p.getFileName(0))
                substnode(p,proxy)
            

            
        

#--------------------------------------------------------------------------
class GeomSplitter(FileWriter,Visitor): 

    def __init__(self):
        self.iveout=1
        self.BuildOptionString()









#--------------------------------------------------------------------------
class ListFileComponents(BaseOsgFileTranslator,Visitor): 
    ''' list all proxy '''

    def __init__(self,visitAllInstances=False):
        Visitor.__init__(self,visitAllInstances)
        BaseOsgFileTranslator.__init__(self)
        self.visit_loaded_childs=False

        self.proxies=dict()
        self.plods=dict()
        self.images=dict()
        self.textures=dict()
        
        
    def visit_external(self,node,dictionary):
        ##print "visit_external:",node,dictionary
        if node:
            if dictionary.has_key(node.this.__hex__()) :
                if(self.verbose):
                    print node.className() + " instance  FOUND !!!!",
                    if(node.getName()):
                        print node.getName()
                    else:
                        print node.this.__hex__() 

            else:                        
                filenames=[]
                for i in range(0,node.getNumFileNames()):
                    f=node.getFileName(i)
                    if(f):
                        filenames.append(f)
                    else:
                        if(self.verbose):
                            print "SKIIPING VOID FILENAME"
                dictionary[node.this.__hex__()]=(node,filenames)
                if(self.verbose):
                    if(node.getName()):
                        print node.getName(),
                    else:
                        print node.this.__hex__(), 
                    print " files: ",filenames
        return node.getNumChildren() >0 and self.visit_loaded_childs

    def visitStateSet(self,ss, parent):
        ''' parent noto '''
        if ss:
            for i in [0,1,2,3]:
                tex = ss.getTextureAttribute(i,osg.StateAttribute.TEXTURE)
                if tex:
                    if(self.verbose):
                        print "got texture channel", i, " -->",tex.className()
                    tx = osg.StateAttributeToTexture(tex)
                    if tx:
                        if self.textures.has_key(tx.this.__hex__()) :
                            if(self.verbose):
                                print "INSTANCED TEX FOUND !!!!"
                        else:
                            im = tx.getImage(0)
                            if(im):
                                tex_file_name=im.getFileName()
                                if(self.verbose):
                                    print "filename->",tex_file_name,"<-size->",im.s(),"<->",im.t()
                        
                                self.textures[tx.this.__hex__()]=(tx,tex_file_name,ss,i)

                                if self.images.has_key(tex_file_name) :
                                    if(self.verbose):
                                        print "DUPLICATE FILE IMAGE FOUND !!!!"           
                                else:
                                    self.images[tex_file_name] = ((im.s(),im.t()),[],[])
                                    #print "filename->",im.getFileName(),"<-size->",im.s(),"<->",im.t()
                                self.images[tex_file_name][1].append(str(tx.this))
                                self.images[tex_file_name][2].append(im)

                            else:
                                print "texture missing image"
    
    def visitPagedLOD(self,plod): 
        ##print "visitPagedLOD"
        return self.visit_external(plod,self.plods)
    def visitProxyNode(self, pnode):
        ##print "visitProxyNode"
        return self.visit_external(pnode,self.proxies)

    def ProcessExternal(self,dictionary,handler=None):

        for p in  dictionary.keys(): 
            extname=self.name+dictionary[p][0].className()+str(dictionary.keys().index(p))
            for f in dictionary[p][1]:
                if(self.verbose):
                    print "processing file: ",f
                if(handler):
                    handler.file_handler(f)
            if(self.verbose):
                print "processing: ",extname
            if(handler):
                ret=handler.node_handler(dictionary[p][0],extname)
                if(ret):
                    print "SUBSTITUTING ",ret
                    
    def ProcessProxy(self,handler=None):
        self.ProcessExternal(self.proxies,handler)
    def ProcessPlod(self,handler=None):
        self.ProcessExternal(self.plods,handler)
    def ProcessImages(self,image_handler=None):
        self.imagesize_single=0
        self.imagesize_repeat=0
        for i in self.images.keys():
            fullpath=os.path.abspath(os.path.join(self.basepath,i))
            nref=len(self.images[i][2])
            (dx,dy)=self.images[i][0]
            if(os.path.exists(fullpath)):
                filesize = int(os.stat(fullpath)[6])
                self.imagesize_single += filesize 
                self.imagesize_repeat += filesize*nref
                im = Image.open(fullpath)
                if(self.images[i][0] != im.size):
                    print "ERROR DIFFERENT SIZE file-->",i
                num_bands=len(im.getbands())
                if(image_handler):
                    image_handler(i,self.images[i],im)
                else:
                    print "   ",i,nref,dx,dy,num_bands,filesize
            else:
                print "MISS",i,nref,dx,dy
        print "total img sz: ", self.imagesize_single , self.imagesize_repeat
            
    def GetRefFileList(self):
        flist=[]
        for dictionary in [self.plods,self.proxies]:
            for p in  dictionary.keys(): 
                for f in dictionary[p][1]:
                    extname=self.name+"_t_"+dictionary[p][0].className()+"_"+str(dictionary.keys().index(p))+"_f_"+str(dictionary[p][1].index(f))
                    flist.append((f,extname))
        return flist
    
    def ParseFile(self,file,handler=None):
 #       if(self.verbose):
        print "processing file: ",file
        node=self.LoadFile(file)
        if(node):
            node.accept(self)
            fl=self.GetRefFileList()
            print fl
            for rf in fl:
                nv=ListFileComponents(self.visitAllInstances)
                nv.verbose=self.verbose
                nv.visit_loaded_childs=self.visit_loaded_childs
                nv.ParseFile(rf[0],handler)

                
            
    def RecurseWalk(self,file,handler=None):
        if(self.verbose):
            print "processing file: ",file
        node=self.LoadFile(file)
        self.plods=dict()
        self.proxies=dict()
        node.accept(self)
        for dictionary in [self.plods,self.proxies]:
            for p in  dictionary.keys(): 
                extname=self.name+dictionary[p][0].className()+str(dictionary.keys().index(p))
                for f in dictionary[p][1]:
                    if(self.verbose):
                        print "recursing file: ",f
                    self.RecurseWalk(f,handler)
##                if(self.verbose):
##                    print "processing: ",extname
##                if(handler):
##                    ret=handler.node_handler(dictionary[p][0],extname)
##                    if(ret):
##                        print "SUBSTITUTING ",ret
        if(handler):
            handler.file_handler(file)

            



#--------------------------------------------------------------------------

def MultiLoad(basepath,patlist,useproxy=False,optimize=True,handler=None):
    opt=osgUtil.Optimizer()
    optimization_flags= osgUtil.Optimizer.SHARE_DUPLICATE_STATE | osgUtil.Optimizer.OPTIMIZE_TEXTURE_SETTINGS | osgUtil.Optimizer.REMOVE_LOADED_PROXY_NODES

    top=osg.Group()
    for p in patlist:
        #print "globbing-->",p
        flist=glob.glob1(basepath,p)
        for rf in flist:
            ff = os.path.join(basepath,rf)

            if os.path.exists(ff):
                #print "loading-->" , ff
                node = osgDB.readNodeFile(ff)
                opt.optimize(node,optimization_flags)

                if node :
                    if(handler):
                        (name,ext)=os.path.splitext(rf)
                        handler.node_handler(node,name)
                    if(useproxy):
                        pr=osg.ProxyNode()
                        pr.setLoadingExternalReferenceMode(osg.ProxyNode.DEFER_LOADING_TO_DATABASE_PAGER)
                        pr.setFileName(0,rf.replace("\\","/"))
                        top.addChild(pr)
                    else:
                        top.addChild(node)
                else:
                    print 'error loading', filename
            else:
                 print "missing-->" , ff
    top.thisown=False
    return top   


#--------------------------------------------------------------------------
class BaseCopyHandler(BaseHandler,FileSizer): 
    ''' base class for rewriting osg nodes,'''
    def __init__(self):
        FileWriter.__init__(self)
        self.image_suffix=""
        self.opt=osgUtil.Optimizer()
        self.optimization_flags = osgUtil.Optimizer.SHARE_DUPLICATE_STATE | osgUtil.Optimizer.OPTIMIZE_TEXTURE_SETTINGS | osgUtil.Optimizer.REMOVE_LOADED_PROXY_NODES
        self.LOD_par=dict()
        self.max_distance=100000.0
        self.LOD_par['LO']={
            "imgpar" : ((1.0/4.0,64 ),{'.*LightingMap.*' : (1.0/2.0,128  ) } ),
            "rngscl" : (16.0,{}),
            "iotype" : ({   'iveout' : True,
                            'no_textures' : False, 
                            'with_textures' : True,  
                            'incl_ext_ref' : False,
                            'orig_ext_ref' : True  
                        },{})
        }
        self.LOD_par['MI']={
            "imgpar" : ((1.0/2.0,128),{'.*LightingMap.*' : (1.0/2.0,256  ) } ),
            "rngscl" : (8.0,{}),
            "iotype" : ({   'iveout' : True,
                            'no_textures' : False, 
                            'with_textures' : True,  
                            'incl_ext_ref' : False,
                            'orig_ext_ref' : True  
                        },{})
            }
        self.LOD_par['HI']={
            "imgpar" : ((1.0    ,512),{'.*LightingMap.*' : (1.0    ,1024 ) } ),
            "rngscl" : (4.0,{}),
            "iotype" : ({   'iveout' : True,
                            'no_textures' : False, 
                            'with_textures' : True,  
                            'incl_ext_ref' : False,
                            'orig_ext_ref' : True  
                        },{})
 
        }
        







    def node_info(self,node):
        if(node):
            ##print node
            self.usetmp=True
            self.with_textures = 0
            self.no_textures = 1
            self.incl_ext_ref = 0
            self.orig_ext_ref = 1
            self.iveout=True

            self.BuildOptionString()
            size_tn = self.EvaluateNodeSize(node)
            
            self.no_textures = 0
            self.with_textures = 1
            self.BuildOptionString()
            size_ti = self.EvaluateNodeSize(node)
            
            multisize=(size_tn,size_ti)
##            bs=node.getBound()
##            c=bs.center()
##            r=bs._radius
##            bnd=(c.x(),c.y(),c.z(),r) 
            bnd=get_bnd(node)           
            return (multisize,bnd)
        else:
            return None


    def node_handler(self,node,id="",save=False):
        if(node):
            ret=self.node_info(node)
            print id,"-->",ret
            v1=ListFileComponents(False)
            v1.verbose=self.verbose
            v1.basepath=self.basepath
            v1.visit_loaded_childs=True
            node.accept(v1)
            
            v1.ProcessImages(self.image_info)
            

###############  building pagedlods
            self.image_suffix='HI'
            self.set_write_options()
            v1.ProcessImages(self.image_handler)
            outfile=self.WriteNode(node,id+"_"+self.image_suffix)
            (base,webfile)=os.path.split(outfile)
            print "LOD: ",self.image_suffix,"-->",outfile
            
            bnd=ret[1]
            dist=self.get_switch_distance(bnd)  #this is the switch distance for HI lod
            self.image_suffix='MI'
            v1.ProcessImages(self.image_handler) #now in node there is a version with MI res images
            plod=build_binary_plod(node,webfile,dist,bnd,self.max_distance)
            self.set_write_options()
##            self.iveout=False   #debugging: set out to osg
##            self.BuildOptionString()
            outfile=self.WriteNode(plod,id+"_"+self.image_suffix)
            (base,webfile)=os.path.split(outfile)
            print "LOD: ",self.image_suffix,"-->",outfile
            dist=self.get_switch_distance(bnd)  #this is the switch distance for MI lod
            
            if(ret[0][0] > 50000): #if the geometry is too big, just produce two levels, the LO is empty
                print "skipping LO lod, should be empty for ",outfile
                plod=build_binary_plod(None,webfile,dist,bnd,self.max_distance)
            else:
                self.image_suffix='LO'
                v1.ProcessImages(self.image_handler) #now in node there is a version with LO res images
                plod=build_binary_plod(node,webfile,dist,bnd,self.max_distance)
            if(save):
                self.set_write_options()
##                self.iveout=False   #debugging: set out to osg
##                self.BuildOptionString()
                outfile=self.WriteNode(plod,id+"_"+self.image_suffix)
                (base,webfile)=os.path.split(outfile)
                print "LOD: ",self.image_suffix,"-->",outfile
                return webfile
            else:
                return plod
            
##            for res in ['HI','MI','LO']:
####                self.image_resize_dim=res
##                self.image_suffix=res
##                v1.ProcessImages(self.image_handler)
##            
##                self.usetmp=False
##                self.iveout=False
##                self.BuildOptionString()
##                if(id):
##                    outfile=self.WriteNode(node,id+"_"+self.image_suffix)
##                    print "written ",outfile
##                self.iveout=True
##                self.no_textures = 0
##                self.with_textures = 1
##                self.BuildOptionString()
##                if(id):
##                    outfile=self.WriteNode(node,id+self.image_suffix)
##                    print "written ",outfile
            


    def file_loader(self,file):
        ff = os.path.join(self.basepath,file)
        if os.path.exists(ff):
            #print "loading-->" , ff
            node = osgDB.readNodeFile(ff)
            self.opt.optimize(node,self.optimization_flags)
            return node
        else:
            if(self.verbose):
                print "file NOT FOUND ",ff
            return None

    def image_resize(self,im,outname):
        newdim = self.img_target_size(im)
        if(newdim == im.size):
            print "WARNING RESIZE EQUAL image->",im.filename
        imr=im.resize(newdim)
        imr.save(outname)


    def image_info(self,rel_name,dict_entry,im):
        fullpath=os.path.abspath(os.path.join(self.basepath,rel_name))
        nref=len(dict_entry[2])
        (dx,dy)=dict_entry[0]
        filesize = int(os.stat(fullpath)[6])
        if(dict_entry[0] != im.size):
            print "ERROR DIFFERENT SIZE file-->",i
        num_bands=len(im.getbands())
        print "   ",rel_name,nref,dx,dy,num_bands,filesize

    def image_handler(self,rel_name,dict_entry,im):

        (p,n)=os.path.split(rel_name)
        (bn,ext)=os.path.splitext(n)
        outname=bn+self.image_suffix+ext
        webname=os.path.join(p,outname).replace("\\","/")
        if( not os.path.exists(os.path.join(self.outdir,p))): 
            os.makedirs(os.path.join(self.outdir,p))
        self.image_resize(im,os.path.join(self.outdir,p,outname))
        for i in dict_entry[2]:
            i.setFileName(webname)

    
    def img_target_size(self,im):
        #newdim=im.size
        (root,name)=os.path.split(im.filename)
        (scale,top)=(1.0,2048)
        #print self.LOD_par
        if(self.LOD_par.has_key(self.image_suffix)):
            #print self.image_suffix
            lev=self.LOD_par[self.image_suffix]
            if(lev.has_key("imgpar")):
                ((scale,top),exceptions)=lev["imgpar"]
                for i in exceptions.keys():
                    if(re.match(i,name)):
                        (scale,top)=exceptions[i]
        #print "Using ",scale, top
        newdim=dict()
        for k in [0,1]:
            newdim[k]=im.size[k] * scale
            if (newdim[k] > top) : newdim[k] = top
            newdim[k]=close_2_power(newdim[k])        
        n=(newdim[0],newdim[1])
        if(self.verbose): 
            print self.image_suffix, "Using ",scale, top,im.filename," from ",im.size," to ",n
        return n
        
    def set_write_options(self):
        self.usetmp=False
        if(self.LOD_par.has_key(self.image_suffix)):
            lev=self.LOD_par[self.image_suffix]
            if(lev.has_key("iotype")):
                (options,exceptions)=lev["iotype"]
                for opt in options.keys():
                    self.__setattr__(opt,options[opt])
                    if(self.verbose) : print "doing -->self."+opt+"=",options[opt]
            else:
                print 'Warning: no "iotype" in LOD_par[',self.image_suffix,']'
        else:
            print 'LOD_par has no "'+self.image_suffix+'" key'
        self.BuildOptionString()
        
    def get_switch_distance(self,bnd):
        #print bnd
        if(self.LOD_par.has_key(self.image_suffix)):
            lev=self.LOD_par[self.image_suffix]
            if(lev.has_key("rngscl")):
                (scale,exceptions)=lev["rngscl"]
                #print scale
                swdist=bnd[3]*scale
                if(self.verbose) : print "switch to ", self.image_suffix," scale ",scale,swdist
                return swdist
            else:
                print 'Warning: no "rngscl" in LOD_par[',self.image_suffix,']'
        else:
            print 'LOD_par has no "'+self.image_suffix+'" key'
        return self.max_distance

    def MultiGroup(self,patlist,useproxy=False):

        top=osg.Group()
        for p in patlist:
            #print "globbing-->",p
            flist=glob.glob1(self.basepath,p)
            for rf in flist:
                ff = os.path.join(self.basepath,rf)

                if os.path.exists(ff):
                    print "loading-->" , ff
                    node = self.file_loader(ff)

                    if node :
                        (name,ext)=os.path.splitext(rf)
                        if(useproxy):
                            pr=osg.ProxyNode()
                            pr.setLoadingExternalReferenceMode(osg.ProxyNode.DEFER_LOADING_TO_DATABASE_PAGER)
                            handled_file=self.node_handler(node,name,True)
                            if(handled_file):
                                pr.setFileName(0,handled_file.replace("\\","/"))
                                top.addChild(pr)
                        else:
                            print "node_handler name->",name,"<--"
                            handled_node=self.node_handler(node,name)
                            if(handled_node):
                                top.addChild(handled_node)
                    else:
                        print 'error loading', filename
                else:
                     print "missing-->" , ff
        top.thisown=False
        return top   
    

            
