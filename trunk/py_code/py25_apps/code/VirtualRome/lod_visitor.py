import osg
import osgDB
import re
import os
import sys
import glob
import visitorbase



class FindNamePattern(visitorbase.VisitorBase): 
    ''' trova tutti i nodi che fanno match con una regular expression
        il sottoalbero che fa match non viene visitato'''

    def __init__(self,match):
        visitorbase.VisitorBase.__init__(self)
        self.NodesHash = dict()
        self.names = dict()
        if(re.split('.*\((.*)\).*',match).__len__() > 1):
            #seems that the match is a re containing a submatch like (.*) so use it directly
            re_string=match
        else:
            #seems that the match is not a re containing a submatch like (.*) so use like a simple match, the name estracted is the first part
            re_string='(.*)'+match+'.*'
        print "match re-->",re_string,"<--"
        self.match=re.compile(re_string,re.IGNORECASE)


    def visitNode(self, node):
        '''virtual func to be redefined -- return False to stop visiting this branch'''
##        print "in visitGeneric",node.className(), node.getName(),"num parent",node.getNumParents()
        name=node.getName()
        if(name):
            print "node name--",name
            #split_list=re.split('(.*)'+self.match+'.*', name)
            split_list=self.match.split(name)
            if(split_list.__len__()>1):
                matched_name=split_list[1]
##                print "matched_name -->",matched_name
                if(matched_name):
                    self.NodesHash[node.this.__hex__()]=(matched_name,node)
                    if(self.names.has_key(matched_name)):
                        print "WARNING!!!  nodes >"+node.this.__hex__()+"<and>"+self.names[matched_name].this.__hex__()+"<match same name->"+matched_name
                        self.names[matched_name].append(node)
                    else:
                        self.names[matched_name]=[node]
##                    print "matched name -->" + matched_name + "node -->" + node.this.__hex__() + "<--"
                    return False

        return True

class SearchMatchInFiles():
    def __init__(self,path,optimizer=None):
        self.basepath=path
        self.optim=optimizer
    def find(self,name):
        nodes=[]
        flist=glob.glob(os.path.join(self.basepath,"*"+name+".*"))
        for f in flist:
            print "opening -->"+f+"<--"
            node=osgDB.readNodeFile(f)
            if(node):
                if(self.optim):
                    self.optim.optimize(node,osgUtil.Optimizer.FLATTEN_STATIC_TRANSFORMS)
                #cerco dentro il file un nodo che finisca con <name>
                v = FindNamePattern(".*("+name+")\s*$")
                node.accept(v)
                for k in v.NodesHash.keys():
                    print "found >"+k+"<-->"+v.NodesHash[k][0]+"<--in file",f
                    nodes.append(v.NodesHash[k][1])
        return(nodes)


#--------------------------------------------------------------------------
##def makeLOD(low_node,hi_node,boundscale=4.0):
##    bs=low_node.getBound()
##    switch_dist=bs._radius * boundscale
##    print "bound: ",bs.center().x(),bs.center().y(),bs.center().z(),bs._radius
##    ##osgDB.writeNodeFile(g,filename)
##    lod=osg.LOD()
##    lod.setCenter(bs.center())
##    lod.addChild(low_node)
##    lod.addChild(hi_node)
##    lod.setRange(0,switch_dist,100000000.0)
##    lod.setRange(1,0.0,switch_dist)
##    lod.thisown=False
##    return lod




class ListTexturesVisitor(osg.NodeVisitor): 
    """ListTexturesVisitor -- A NodeVisitor that list texture names."""
    base_class = osg.NodeVisitor
    def __init__(self):
        """aaaaaaaaaa"""
        #count textures
        osg.NodeVisitor.__init__(self,osg.NodeVisitor.TRAVERSE_ALL_CHILDREN)

        self.textures=dict()
        self.currnode=""
        self.currname=""
        self.images=dict()
        self.geodes=dict()
        self.basepath=""
        
           
    def apply(self, node):
        print node.className(), " ",node.getName()," instanced ",node.getNumParents()
        nm=node.getName()
        if "" != nm:
            self.currname=nm
        self.currnode=node
        st = node.getStateSet()
        if st:
            for i in [0,1,2,3]:
                tex = st.getTextureAttribute(i,osg.StateAttribute.TEXTURE)
                if tex:
                    #print "got texture channel", i, " -->",tex.className()
                    tx = osg.StateAttributeToTexture(tex)
                    if tx:
                        im = tx.getImage(0)
                        tex_file_name=im.getFileName()
                        print "filename->",tex_file_name,"<-size->",im.s(),"<->",im.t()

                        tex_attrib=(self.currnode,self.currname,tex_file_name)
                        if self.textures.has_key(tx) :
                            print "DUPLICATE TEX FOUND !!!!"
                            if self.textures[tx] != tex_attrib:
                                print "previous tex -->",self.textures[tx]," now-->",tex_attrib
                        else:
                            self.textures[tx]=tex_attrib
                        image_attrib=(tx,im.s(),im.t())
                        if self.images.has_key(tex_file_name) :
                            print "DUPLICATE IMAGE FOUND !!!!"           
                            if self.images[tex_file_name] != image_attrib:
                                print "previous image -->",self.images[tex_file_name]," now-->",image_attrib
                        else:
                            self.images[tex_file_name] = image_attrib
                            print "filename->",im.getFileName(),"<-size->",im.s(),"<->",im.t()
        g = osg.NodeToGeode(node)                
        if g:
            #print g,g.getNumParents()
            print str(g.this)
            if(self.geodes.has_key(str(g.this))):
                print "geode instance numpar-->",g.getNumParents()," skip parsing current name-->",self.currname
##                np=g.getParentalNodePaths()
##                nn=np.next()
##                while nn:
##                    print "-- ",nn.className()," ",nn.getName()
##                    nn=np.next()
                #for par in np:
                #    print "-->",par.getName()
            else:
                self.geodes[str(g.this)]=(g,[])
                print "geode-->",g.getNumDrawables()
                for j in range(0,g.getNumDrawables()):
                    d  = g.getDrawable(j);
                    dst = d.getStateSet()
                    if dst:
                        for i in [0,1,2,3]:
                            tex = dst.getTextureAttribute(i,osg.StateAttribute.TEXTURE)
                            if tex:
                                print "got texture channel", i, " -->",tex.className()
                                tx = osg.StateAttributeToTexture(tex)
                                if tx:
                                    if self.textures.has_key(str(tx.this)) :
                                        print "DUPLICATE TEX FOUND !!!!"
                                    else:
                                        im = tx.getImage(0)
                                        if(im):
                                            self.geodes[str(g.this)][1].append(str(tx.this))
                                            tex_file_name=im.getFileName()
                                            print "filename->",tex_file_name,"<-size->",im.s(),"<->",im.t()
                                    
                                            self.textures[str(tx.this)]=(tx,tex_file_name,dst,i)
##                                            image_attrib=(tx,im.s(),im.t())

                                            if self.images.has_key(tex_file_name) :
                                                print "DUPLICATE IMAGE FOUND !!!!"           
##                                                if self.images[tex_file_name] != image_attrib:
##                                                    print "previous image -->",self.images[tex_file_name]," now-->",image_attrib
                                            else:
                                                self.images[tex_file_name] = ((im.s(),im.t()),[],[])
                                                #print "filename->",im.getFileName(),"<-size->",im.s(),"<->",im.t()
                                            self.images[tex_file_name][1].append(str(tx.this))

                                        else:
                                            print "texture missing image"
##                                            tex_attrib=(self.currnode,self.currname,"")

##                                    if self.textures.has_key(tx.this) :
##                                        print "DUPLICATE TEX FOUND !!!!"
##                                        if self.textures[tx.this] != tex_attrib:
##                                            print "previous tex -->",self.textures[str(tx.this)]," now-->",tex_attrib
##                                    else:
##                                        self.textures[tx.this]=tex_attrib

            #osg.NodeVisitor.apply(node.getStateSet())
        osg.NodeVisitor.traverse(self, node)

    def set_base_path(self,path):
        self.basepath=path
    def printimages(self):
        for filename in self.images.keys():
            print "image file-->",filename,self.images[filename]

    def substimages(self):
        for tx in self.textures.keys():
            im = self.textures[tx][0].getImage(0)
            if(im):
                tex_file_name=self.textures[tx][1]
                im.setFileName(os.path.basename(tex_file_name))
                print "filename->",im.getFileName(),"<-size->",im.s(),"<->",im.t()
    def resample_images(self,rmin,rmax,out_format) :
        pass
    def countimages(self): return self.images.__len__() 
    def counttextures(self): return self.textures.__len__() 
    def printgeodes(self):
        print self.geodes.__len__()
        for k in self.geodes.keys():
            g=self.geodes[k][0]
            num=g.getNumParents()
            print g.getName()," ",num
            for i in range(0,num):
                print g.getName()," -->",g.getParent(i).getName()

#--------------------------------------------------------------------------
    def node2plod(self,node,hi_file,boundscale=4.0):
        bs=node.getBound()
        switch_dist=bs._radius * boundscale
        print "bound: ",bs.center().x(),bs.center().y(),bs.center().z(),bs._radius
        ##osgDB.writeNodeFile(g,filename)
        plod=osg.PagedLOD()
        plod.setCenter(bs.center())
        plod.addChild(node)
        plod.setRange(0,switch_dist,100000000.0)
        plod.setFileName(1,hi_file)
        plod.setRange(1,0.0,switch_dist)
        plod.thisown=False
        return plod

#--------------------------------------------------------------------------
    def substnode(self,node,subst):
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
    def rif_subst(self,match):
        for k in self.geodes.keys():
            (g,num)=self.geodes[k]
            gname=g.getName()
            if(gname):
                print "geode name--",gname
                split_list=re.split('(.*)'+match+'.*', gname)
                if(split_list.__len__()>1):
                    matched_name=split_list[1]
                    if(matched_name):
                        hi_file=os.path.join(self.basepath,matched_name+".osg")
                        if(os.path.exists(hi_file)):
                            print "replacing file-->",hi_file
                            pl=self.node2plod(g,os.path.basename(hi_file))
                            self.substnode(g,pl)
                        else:
                            print "file-->",hi_file," NOT FOUND!!!!!"
               
       
#--------------------------------------------------------------------------
    def node2box(self,node,hi_file,out_options="noTexturesInIVEFile useOriginalExternalReferences"):
        bs=node.getBound()
        bx=osg.Box(bs.center(),bs._radius)
        g=osg.Geode()
        dbx=osg.ShapeDrawable(bx)
        g.addDrawable(dbx)
        osgDB.writeNodeFile_s(node,hi_file,out_options)
        for pointer in [bs,bx,dbx,g]:
            pointer.thisown = False
        return self.node2plod(g,os.path.basename(hi_file))

#--------------------------------------------------------------------------
    def subst_small_images(self,node,hi_file,out_options="noTexturesInIVEFile useOriginalExternalReferences"):
        bs=node.getBound()
        bx=osg.Box(bs.center(),bs._radius)
        g=osg.Geode()
        dbx=osg.ShapeDrawable(bx)
        g.addDrawable(dbx)
        osgDB.writeNodeFile_s(node,hi_file,out_options)
        for pointer in [bs,bx,dbx,g]:
            pointer.thisown = False
        return self.node2plod(g,os.path.basename(hi_file))
        

#--------------------------------------------------------------------------
    def box_subst(self,out_format,out_options="noTexturesInIVEFile useOriginalExternalReferences"):
        count=0
        for k in self.geodes.keys():
            (g,num)=self.geodes[k]
            if(g):
                hi_file=os.path.join(self.basepath,out_format % count)
                pl=self.node2box(g,hi_file,out_options)
                self.substnode(g,pl)
                count=count + 1
                
        
        
    
def build_plod(filename):
    cen=osg.Vec3(10,10,0)
    rad=float(5)
    bx=osg.Box(cen,rad)
    g=osg.Geode()
    dbx=osg.ShapeDrawable(bx)
    g.addDrawable(dbx)
    plod=osg.PagedLOD()
    plod.setCenter(cen)
    plod.addChild(g)
    plod.setRange(0,20.0,1000000.0)
    plod.setFileName(1,"myfile.osg")
    plod.setRange(1,0.0,20.0)

    
#--------------------------------------------------------------------------
if __name__ == "__main__":


    import osgDB
    import sys
    import os
    from utility import *

    # locate the DataDir
    dir = os.getenv('DATADIR')
    if not dir:
        print 'env-var "DATADIR" not found, exiting'
        sys.exit()
    # there are good and bad data :-)
    dir = dir + 'bad\\'

    # open the test file
    filename = dir + 'f_pace.osg'
    node = osgDB.readNodeFile(filename)
    if not node : 
        print 'error loading', filename
        sys.exit()
    if(node):
        optim=osgUtil.Optimizer()
        optim.optimize(node,osgUtil.Optimizer.FLATTEN_STATIC_TRANSFORMS)

    # test a Visitor Subclass
    print '------- testing FindNamePattern --------'
    v = FindNamePattern('(.*)_RIF')
    node.accept(v)

    # print results
    s=SearchMatchInFiles(os.path.dirname(filename))
    print "risultati"
    
    for n in v.NodesHash.keys():
        print ">",n,"<-->",v.NodesHash[n][0],"<--"
    for name in v.names.keys():
        print "searching for -->"+name+"<--"
        listnodes =s.find(name)
        if(listnodes.__len__() == 1):
            for nlow in v.names[name]:
                #replace_with_LOD(nlow,make_group(listnodes[0]))
                replace_with_LOD(nlow,listnodes[0])
        else:
            if(listnodes.__len__() > 1):
                #multiple match: invoke user
                print "WARNING!!!! found multiple matches for >"+name+"<-"
            else:
                #no match: do nothing
                print "WARNING!!!! no matches found for >"+name+"<-"

    print '------- Done --------'
    



    test_wx=False
    if(test_wx):
        import wxosgviewer 
        
        app = wxosgviewer.App(0)  # importante: creare APP passando 0 
        viewer = app.getViewer()
    else:
    # create a viewer
        viewer = osgViewer.Viewer()

    # configure
        viewer.setThreadingModel(osgViewer.Viewer.SingleThreaded)
        viewer.addEventHandler(osgViewer.WindowSizeHandler())
        viewer.addEventHandler(osgViewer.StatsHandler())
    
    viewer.setSceneData(node)
##    hlt=HiLight(highlighter(wireboxed))
##    pickhandler1 = PickHandlerBase(hlt)
##    
##    viewer.addEventHandler(pickhandler1.__disown__());

    
    
    
    if(test_wx):
        app.MainLoop()
    else:
        # loop until done
        viewer.run()






##    #"OutputTextureFiles" per file osg
##    #"noTexturesInIVEFile includeImageFileInIVEFile compressImageData" in alternativa per file ive
##    #"inlineExternalReferencesInIVEFile noWriteExternalReferenceFiles" per file ive
##    
##    p = ListTexturesVisitor()
##
##    #n = osgDB.readNodeFile("D:/models/demo_virtrome/piante/frassino.ive")
##    #n = osgDB.readNodeFile("H:/vrome/models/modelli_tipo/oppidum/oppidum_opt.ive")
##    n = osgDB.readNodeFile("D:/prove/oppidum/oppidum_R2T_nuovo_opt.osg")
##    p.set_base_path("D:/prove/oppidum")
##    #fori#n = osgDB.readNodeFile("H:/vrome/models/_fori/f_nerva.osg")
##    #fori#p.set_base_path("H:/vrome/models/_fori")
##     
##
##    print n.className(), "<- fuori->",n.getName()
##    #n.accept(p.__disown__())
##    n.accept(p)
##
##    p.printimages()
##    sys.exit()
##    p.substimages()
##    print "found ",p.countimages()," images ",p.counttextures()," textures"
##    p.printgeodes()
##    #fori#p.rif_subst('_rif')
##    #fori#osgDB.writeNodeFile_s(n,"H:/vrome/models/_fori/f_nerva_plod.osg","noTexturesInIVEFile useOriginalExternalReferences")
##    p.box_subst("_gen_geode_%d.ive","includeImageFileInIVEFile useOriginalExternalReferences")
##    osgDB.writeNodeFile_s(n,"D:/prove/oppidum/oppidum_R2T_nuovo_plod.osg","noTexturesInIVEFile useOriginalExternalReferences")
##    #osgDB.writeNodeFile(n,"H:/vrome/models/modelli_tipo/oppidum/oppidum_nuovo1.osg",)
##    #osgDB.writeNodeFile_s(n,"H:/vrome/models/modelli_tipo/oppidum/oppidum_nuovo.ive","noTexturesInIVEFile useOriginalExternalReferences")