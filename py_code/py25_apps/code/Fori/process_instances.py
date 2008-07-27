import globs
from visitorbase import VisitorBase
import osg
import osgDB
import re

#-------------------------
# globals
ModelsDir  = ''    # directory where to search for osg files
CreatedLod = {}    # cached LOD geometry

#-------------------------
class FindInstances(VisitorBase):
    ''' Trova i nodi con nome che finisce in "_xx" (x=numero),
        Fa il casting a Gruppo,
        Li inserisce nella lista 'instances'
    '''
    def __init__(self):
        VisitorBase.__init__(self)
        self.instances = []
        self.rex = re.compile(".*_[0-9]+$")

    def visitNode(self, node):
        if self.rex.match( node.getName() ):
            g = node.asGroup()
            if not g:
                print ' *** FindInstancesVisitor: cast node to group failed *** ',  node.getName()
            else:
                self.instances.append( g )
            return False # skip the sub-tree
        return True

#-------------------------
class FindLevelOfDetail(VisitorBase):
    ''' cerca i nodi con nume che finisce in _lo,_mi,_hi '''
    def __init__(self):
        VisitorBase.__init__(self)
        self.lo = None
        self.mi = None
        self.hi = None

    def visitNode(self, node):
        name = node.getName().upper()
        if name[-3:] == '_LO'   or name[-4:] == '_LOW':
            self.lo = node
            return False 
        elif name[-3:] == '_MI' or name[-4:] == '_MID':
            self.mi = node
            return False 
        elif name[-3:] == '_HI' or name[-5:] == '_HIGH':
            self.hi = node
            return False 
        return True

#-------------------------
def ProcessInstances( root ):
    ''' Invoca ProcessInstance su ciascun nodo "istanza" '''
    
    fiv = FindInstances()
    root.accept(fiv)
    # per ogni istanza
    for n in fiv.instances:
        # trova il nome del LOD da usare
        rex = re.compile("(.*)(_[0-9]+)$")
        m =  rex.match(n.getName())
        lodname = m.groups()[0]
        lod = GetLod( lodname )
        if lod:
            # sostituisce il subtree con il LOD
            n.removeChildren (0, n.getNumChildren() )
            n.addChild( lod )
            lod.unref() # boh - non funzia
    return root
        
#-------------------------
def GetLod( name ):
    ''' nome -> LOD, le geometrie sono caricate una volta sola '''
    global ModelsDir, CreatedLod
    if not CreatedLod.has_key(name):

        lodFile = ModelsDir + name + '.osg'
        lodRoot = osgDB.readNodeFile(lodFile)
        if not lodRoot:
            print ' *** failed to open : ',  lodFile
            return None
        
        # adesso devo trovare low e hi, mi e' opzionale
        v = FindLevelOfDetail()
        lodRoot.accept(v)
        if not v.lo :
            print 'LOW level of detail not found in: ', lodFile
            return None
        if not v.hi :
            print 'HIGH level of detail not found in: ', lodFile
            return None
        
        CreatedLod[name] = (v.lo, v.mi, v.hi)
    
    lo,mi,hi = CreatedLod[name]
    lod = osg.LOD()
    lod.setRangeMode(osg.LOD.PIXEL_SIZE_ON_SCREEN)
    if mi:
        lod.addChild( lo, 10,  100   )
        lod.addChild( mi, 100, 500   )
        lod.addChild( hi, 500, 10000 )
    else:
        lod.addChild( lo, 10,  300   )
        lod.addChild( hi, 300, 10000 )
    return lod

#-------------------------
def testFindLevelOfDetail():
    ModelsDir = globs.DataDir() + 'sil\\'
    file = ModelsDir + 'f_pace_col_c.osg'
    node = osgDB.readNodeFile( file )
    v = FindLevelOfDetail()
    node.accept(v)
    print 'lo =', v.lo.getName()
    print 'mi =', v.mi.getName()
    print 'hi =', v.hi.getName()

#--------------------------------------------------------------------------
if __name__ == "__main__":

    ModelsDir = globs.DataDir() + 'sil\\'
    file = ModelsDir + 'istanze.osg'
    print 'loading' + file
    root = osgDB.readNodeFile(file)
    if not root:
        print ' *** loading failed *** ',
        exit
    print 'done loading'

    ProcessInstances( root )
