import osg
import osgDB
import re
import os
import sys
import glob
from visitorbase import VisitorBase

def node_id(node):
    return(">"+node.this.__hex__()+"<->"+node.getName()+"<")
    
class FindNamePattern(VisitorBase): 
    ''' trova tutti i nodi che fanno match con una regular expression
        il sottoalbero che fa match non viene visitato'''

    def __init__(self,match):
        VisitorBase.__init__(self)
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
            #print "node name--",name
            #split_list=re.split('(.*)'+self.match+'.*', name)
            split_list=self.match.split(name)
            if(split_list.__len__()>1):
                matched_name=split_list[1]
                #print "matched_name -->",matched_name
                if(matched_name):
                    self.NodesHash[node.this.__hex__()]=(matched_name,node)
                    if(self.names.has_key(matched_name)):
                        print "WARNING!!!  nodes "+node_id(node)+ " and "+node_id(self.names[matched_name][0])+" match same name->"+matched_name
                        self.names[matched_name].append(node)
                    else:
                        self.names[matched_name]=[node]
                    print "matched name -->" + matched_name + "<-node -->" + node.this.__hex__() + "<--"
                    return False

        return True

class SearchMatchInFiles():
    def __init__(self,path,optimizer=None):
        self.basepath=path
        self.optim=optimizer
        self.open_files=dict()
    def find(self,name,flist=[]):
        nodes=[]
        if(flist.__len__()==0):
            #cerco un file che finisca con <name>.qualcosa(osg,ive)
            flist=glob.glob(os.path.join(self.basepath,"*"+name+".*"))
            
#        flist=glob.glob(os.path.join(self.basepath,"f_pace_mi.osg"))
#       flist=glob.glob(os.path.join(self.basepath,"f_pace_col*.osg"))
        for f in flist:
            print "opening -->"+f+"<--"
            if(self.open_files.has_key(f)):
                node=self.open_files[f]
            else:
                node=osgDB.readNodeFile(f)
            if(node):
                self.open_files[f]=node
                if(self.optim):
                    self.optim.optimize(node,osgUtil.Optimizer.FLATTEN_STATIC_TRANSFORMS)
                #cerco dentro il file un nodo che finisca con <name>
#                v = FindNamePattern(".*("+name+").*$")
                v = FindNamePattern("^("+name+"(?:_low?|_mid?|_hig?h?)?(?:_rif)?)$")
                node.accept(v)
                for k in v.NodesHash.keys():
                    print "found >"+k+"<-->"+v.NodesHash[k][0]+"<--in file",f
                    nodes.append(v.NodesHash[k][1])
        return(nodes)


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
    dir = dir + 'bad2\\'

    # open the test file
    filename = dir + 'f_pace_low.osg'
    node = osgDB.readNodeFile(filename)
    if not node : 
        print 'error loading', filename
        sys.exit()
    if(node):
        optim=osgUtil.Optimizer()
        optim.optimize(node,osgUtil.Optimizer.FLATTEN_STATIC_TRANSFORMS)

    # test a Visitor Subclass
    print '------- testing FindNamePattern --------'
    #v = FindNamePattern('(.*)_mi_RIF')
    v = FindNamePattern('(.*)(?:_low?|_mid?|_hig?h?)_RIF')
    node.accept(v)

    # print results
    s=SearchMatchInFiles(os.path.dirname(filename),optim)
    print "risultati"
    
    for n in v.NodesHash.keys():
        print ">",n,"<-->",v.NodesHash[n][0],"<--"
    for name in v.names.keys():
        print "searching for -->"+name+"<--"
        listnodes =s.find(name,glob.glob(os.path.join(os.path.dirname(filename),"f_pace_mi.osg")))
        if(listnodes.__len__() == 1):
            for nlow in v.names[name]:
                #replace_with_LOD(nlow,make_group(listnodes[0]))
                replace_with_LOD(nlow,listnodes[0])
        else:
            if(listnodes.__len__() > 1):
                #multiple match: invoke user
                print "WARNING!!!! found multiple matches for >"+name+"<-"
                for nn in listnodes:
                    print "MULTMATCH-->",nn.getName()
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






