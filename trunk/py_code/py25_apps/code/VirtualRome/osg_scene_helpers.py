import os
import osg
import osgDB
import osgUtil

#--------------------------------------------------------------------------
class osgSceneCollection():
    def __init__(self,viewer):
        self.viewer=viewer
        self.root_collection=osgGroup()
        self.root = self.root_collection.grpnode
        self.viewer.setSceneData(self.root.__disown__())
        self.colls=[self.root_collection]
        self.curr_coll=self.root_collection
        
        
    def write(self,filename,optionstring="noTexturesInIVEFile useOriginalExternalReferences"):
        osgDB.writeNodeFile_s(self.root,filename,optionstring)
        
    def LoadStuff(self,url):
        return(self.curr_coll.addUrl(url))
##        if(self.switcher):
##            return (self.switcher.addUrl(url),True)
##        else:
##            n = osgDB.readNodeFile(url)
##            if(n):
##                self.root.addChild(n)
##                return (url,False)
##            else:
##                return ("",False)
        
    def addcollection(self,coll):
        self.root.addChild(coll.grpnode)
        self.colls.append(coll)
        self.curr_coll=coll
        
        
##    def toggle(self,name):
##        if(self.switcher):
##            self.switcher.set(name,-1)

class osgCollection():    
    def __init__(self):
        self.components=dict()
        self.optim=None
    def setname(self,name):
        self.grpnode.setName(name)
    def optimize(self,opt_flag=osgUtil.Optimizer.FLATTEN_STATIC_TRANSFORMS):
        self.optim=osgUtil.Optimizer()
        self.opt_flag=opt_flag
    def addUrl(self,url,name=""):
        print "addurl ",url
        if(".py"==os.path.splitext(url)[1]):
            print "processing python file -->",url
            d={}
            d['__file__']=os.path.abspath(url)
            try:
                execfile(d['__file__'],globals(),d)
            except:
                print "some errors while executing -->",url
            if(d.has_key('loaded_node')):
                n=d['loaded_node']
            else:
                print "file -->",url,"<-- has not set the variable >>loaded_node<<"
                n=None
        else:
            n = osgDB.readNodeFile(url)
        if(n):
            if(self.optim):
                self.optim.optimize(n,self.opt_flag)

            self.grpnode.addChild(n,True)
            ind=self.grpnode.getChildIndex(n)
            if(name==""): 
                name=os.path.basename(url)
                if(n.getName()):
                    if(n.getName() != "Scene Root"):
                        name=n.getName()
            key=name
            count = 0
            while(self.components.has_key(key)):
                key=name + "_" + str(count)
                count = count +1                        

            self.components[key]=(n,ind,url)
            print "added key-->",key
            return key
        else:
            return ""
class osgGroup(osgCollection):
    def __init__(self):
        osgCollection.__init__(self)
        self.grpnode=osg.Group()
class osgSwitcher(osgCollection):
    def __init__(self):
        osgCollection.__init__(self)
        self.grpnode=osg.Switch()
    def set(self,key,status=-1):
        if(self.components.has_key(key)):
            (node,ind,url) = self.components[key]
            if( status== -1 ):
                curr= not self.grpnode.getChildValue(node)
            else : curr=bool(status)
            self.grpnode.setChildValue(node,curr)
        else: print "key ->",key," not found"
    
    def setsingle(self,key):
        if(self.components.has_key(key)):
            self.grpnode.setSingleChildOn(self.components[key][1])
        else: 
            print "key ->",key," not found"
    
    