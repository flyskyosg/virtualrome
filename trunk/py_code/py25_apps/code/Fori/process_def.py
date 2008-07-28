'''
process_def



- Legge un file di definizione es: 'path'/f_pace.definition
    (vedere testData/f_pace.definition per la sintassi )

- si suppone che i file osg siano in path
- si suppone che le texture siano in path/images e siano in png
- l'output verra salvato in path/IVE

- apre il file di definizione (controllo sintassi)

- controlla che esistano tutti i file.osg interessati

- preprocessa tutti i file osg, sostituendo '.png' con '.dds'
  e li salva in path/tmp

- converte tutte le tex in DDS, Dxtc1 flippate
  e le salva in path/tmp/images

- individua tutti i modelli interessati (controlla che esistano)

- per ogni modello nella gerarchia viene creato un file IVE con le texture dentro
- i nodi foglia contengono solo geometria/tex
- i nodi interni contengono pagedLOD che richiamano i nodi successivi

- il nodo root ha una geometria associata invisibile che serve per fare il ResetCamera

- alla fine viene restituito il nome della radice della gerarchia da aprire

- si puo decidere se generare la gerarchia in OSG o in IVE,
  mediante il parametro save_as_ive

- il tutto e' implementato con una classe solo per poterlo re-istanziare
  ed evitare variabili globali

------------------------------------------------------------------------------
ultime feature aggiunte:
    ok - processing delle istanze (vedi connect2 nel file di definizione)
    ok - fine tuning delle distanze di switch dei lod

-------------------------------------------------------------------------
TODO:
    collegare piu file di definizioni in cascata
        bah - per adesso si puo' mergiare i file di definizione
        poi scovero' il modo di farlo
-------------------------------------------------------------------------
FUTURO:
    creazione assistita del def.file
------------------------------------------------------------------------------
'''
import os
import glob
import re
import osg
import osgDB
from visitorbase import VisitorBase

#-------------------------
class FindByName(VisitorBase):
    ''' Cerca per nome. Il nome puo contenere un asterisco. Restituisce la lista dei nodi trovati '''
    def __init__(self, name ):
        VisitorBase.__init__(self)
        self.results = []

        name = name.replace('*','.*')
        name = '^'+name+'$'
        self.rex = re.compile(name)

    def visitNode(self, node):
        if self.rex.match( node.getName() ):
            g = node.asGroup() # provo a convertirlo a gruppo -- ma i geodi non si castano
            if not g: g=node
            self.results.append( node )
            return False # skip the sub-tree
        return True
    
#------------------------------------------------------------------
class ProcessDef(object):
    def __init__(self, definition_file, save_as_ive=True ):
        
        self.def_file = definition_file

        self.osg_path = os.path.dirname(definition_file) + '\\'
        self.img_path = os.path.dirname(definition_file) + '\\images\\'
        self.ive_path = os.path.dirname(definition_file) + '\\ive\\'
        self.tmp_path = os.path.dirname(definition_file) + '\\tmp\\'
        self.dds_path = os.path.dirname(definition_file) + '\\tmp\\images\\'

        if not os.path.exists(self.osg_path):
            print 'osg path not exist',self.osg_path
            return
        if not os.path.exists(self.img_path):
            print 'images path not exist',self.img_path
            return
        if not os.path.exists(self.ive_path):
            os.mkdir(self.ive_path)
        if not os.path.exists(self.tmp_path):
            os.mkdir(self.tmp_path)
        if not os.path.exists(self.dds_path):
            os.mkdir(self.dds_path)

        if save_as_ive:
            self.ext = '.ive'
        else:
            self.ext = '.osg'
        
        self.hier     = {}  # gerarchia :  nodo->lista-di-nodi-richiamati
        self.root     = ''  # elemento radice della gerarchia
        
        self.osgFiles = {}  # nome-file-osg -> root-del-file
        self.osgNodes = {}  # nome-modello-osg -> root-del-modello-osg
        self.activate_size = {} # nome-modello -> pixel-size soglia di accensione

        self.garbage = osg.Group() # attacco qui tutti i nodi temporanei
                                  # per ritardare il messaggio di memory leak
                                  # che altrimenti mi incasina il logging

        self.fake_node = osg.Group() # nodo stupido usato in MakePLod

        self.ConvertTextures()
        self.ReadDefinition()  # legge def_file, crea hier, carica  le chiavi di osgFile e osgNodes
        self.OpenAllFiles()    # carica i valori di osgFiles
        self.OpenAllModels()   # carica i valori di osgNodes
        self.MakeHierarchy(self.root, 0 )
        
    #----------------------
    def GetIveRoot(self):
        return self.ive_path + self.root + self.ext

    #----------------------
    def ReadDefinition(self):
        ''' 
        legge una file ASCII che rappresenta una serie di dipendenze tra modelli in file diversi.
        le righe del file devono essere:    file1 modello1   file2 modello2
        
        che significa 
            modello1 in file1 e' figlio di modello2 in file2
            ( ovvero e' un livello di dettaglio piu alto )
        
        righe vuote sono ignorate.
        righe che iniziano con # sono ignorate.
        
        la procedura crea la gerarchia dei modelli 
        rappresentata con un dizionario
        [padre] -> [lista di figli]
        il nodo radice e' 'file@root'
        tutti gli altri nodi sono rappresentati come file@node
        
        la procedura inizializza osgFiles e osgNodes
        le chiavi di osgFiles sono i file-osg-coinvolti
        le chiavi di osgNodes sono i modelli-coinvolti, nella forma 'file@modello'
        '''
        
        f = open(self.def_file)
        if not f:
            print 'fallita apertura del file di definizione, filename=', self.def_file, " :-("
            return None

        print "lettura del file di definizione"
        
        linenum = -1
        for line in f:
            linenum +=1
            line = line.strip() # toglie gli spazi/tab iniziali e finali
            if not len(line):  continue
            if line[0] == '#': continue
            
            try:
                cmd, f1,n1,f2,n2, size = line.split()
            except:
                print 'Errore di sintassi: linea num.', linenum, line
                continue
            
            parent = f2+'@'+n2
            son    = f1+'@'+n1
            
            self.activate_size[son]=int(size)
            
            # riconosco e ricordo la root
            if n2 == 'root' and not self.hier.has_key(parent):
                self.root = parent
                self.hier[parent] = []
                self.activate_size[parent]=0

            # questo grantisce che l'albero e' tutto connesso
            if not self.hier.has_key(parent) :
                print "nodo chiamante non conosciuto: linea num.", linenum, line
                continue

            # l'asterisco puo comparire solo nei chiamati, non nei chiamanti
            if cmd == 'connect':
                if n2.find('*') != -1:
                    print "non si puo usare l'asterisco nei chiamanti: linea num.", linenum, line
                    continue
            elif cmd == 'connect2':
                if n1.find('*') != -1:
                    print "non si puo usare l'asterisco nel chiamato: linea num.", linenum, line
                    continue
            else:
                print 'comando non riconosciuto:',cmd,'i comandi validi sono connect e connect2'
            
            self.hier[parent].append(son)
            if not self.hier.has_key(son):
                self.hier[son] = [] 

            self.osgFiles[f1]=None     # le chiavi di questo dic. sono tutti i file menzionati
            self.osgFiles[f2]=None
            self.osgNodes[parent]=None # le chiavi di questo dic. sono tutti i modelli menzionati
            self.osgNodes[son]=None

    #----------------------
    def ConvertTextures(self):
        
        ''' entra in img_path,
            cerca tutti i file png,
            se non c'e' il corrispondente file dds,
            oppure se il file dds e' out-of-date,
            invoca nvdxt che converte il png in dds (dxt1c flippate)
            
            nota: siccome gli osg file sono processati e salvati in tmp_path
            allora i file dds sono salvato in dds_path == tmp_path\\images
        '''
        import __main__
        appfile = __main__.__file__
        appdir = os.path.dirname(appfile)
        nvdxt = appdir + '\\nvdxt.exe'
        
        print 'processing delle texture'
        png_list = glob.glob( self.img_path + '*.png')
        for png_file in png_list:
            name = os.path.basename(png_file)
            name = name.split('.')[0]
            dds_file = self.dds_path + name + '.dds'
            if os.path.exists( dds_file ):
                dds_mtime = os.path.getmtime(dds_file)
                png_mtime = os.path.getmtime(png_file)
                if dds_mtime >= png_mtime:
                    continue # dds is up-to-date
            
            cmd = nvdxt +' -file ' + png_file + ' -outfile ' + dds_file + ' -dxt1c -flip'
            print '------- converting ' + name + ' to DDS -------'
            # l'output del programma viene comunque mandato su stdout 
            os.system(cmd)        

    #----------------------
    def PreprocessOsgFile(self,name):
        '''
        sostituisce '.png' con '.dds'
        copia il file da osg_path a tmp_path 
        '''
        filename = self.osg_path + name + '.osg'
        f = open( filename, 'r' )
        buffer = f.read() # speriamo che si possa leggere tutto il file in un colpo solo
        f.close()

        buffer = buffer.replace( '.png"', '.dds"' )

        filename = self.tmp_path + name + '.osg'
        f = open( filename, 'w' )
        f.write( buffer )
        f.close()

    #----------------------
    def OpenAllFiles(self):
        ''' - controlla se tutti i file osg ci sono
            - li preprocessa
            - li apre
            - aggancia le root di ciascuno al dizionario osgFiles
        '''
        print 'controllo files'
        
        for i in self.osgFiles.keys():

            filename = self.osg_path + i + '.osg'

            if not os.path.exists(filename):
                print "il file non esiste !! ->", i, "<-"
                continue

            self.PreprocessOsgFile(i) # copy the file from osg_path to tmp_path

            node = osgDB.readNodeFile( self.tmp_path + i + '.osg' )
            if not node:
                print "errore durante l'apertura di ->", i, "<-"
                continue
            g = node.asGroup()
            if not g:
                print "fallito il cast a gruppo di: ->", i, "<-"
                continue
            self.osgFiles[i] = g

    #----------------------
    def OpenAllModels(self):
        ''' apre tutti i modelli e li aggancia in osgModels.
            alla root attacca un modello precotto box (TODO: migliorare)
        '''
        print 'controllo modelli'

        for model in self.osgNodes.keys():
            
            file  = model.split('@')[0]
            name  = model.split('@')[1]

            if name == 'root':
                fake_model = self.MakeFakeGeom( self.osgFiles[file] ) 
                self.osgNodes[model]= [fake_model]
                continue
            
            if not self.osgFiles.has_key(file):
                print "non posso caricare", model, " perche' il file non e' stato aperto"
            else:
                n = self.osgFiles[file]
                finder = FindByName(name)
                n.accept(finder)
                if not len( finder.results ):
                    print "non sono riuscito a trovare :", model, ":-("
                else:
                    self.osgNodes[model] = finder.results

    #----------------------
    def MakeHierarchy(self, name, lev ):
        ''' attraversa riscorsivamente la gerarchia e crea le pagine IVE corrispondenti '''

        if not self.hier.has_key(name):
            print 'MakeHierarchy error: -', name, '- nome non trovato nella gerarchia'
            return
        
        
        if self.hier[name] == []:  # e' una foglia
            print 'LEV='+str(lev)+' LEAF: ' + name
            nodes = self.osgNodes[name]
            n = self.MakeLeaf(name, nodes)
            self.SaveNode(n,name)
            return
        
        elif name.find('*') != -1: # nodo interno multiplo -- fare il plod sulle istanze
            print 'LEV='+str(lev)+' INST: ' + name
            nodes = self.osgNodes[name]
            child = self.hier[name][0]
            n  = self.MakeInstanceReplacer( name, nodes, child, lev  )
            self.SaveNode(n,name)
            
            self.MakeHierarchy( child, lev+1 )
        
        else: # nodo interno -- faccio il PLod semplice
            print 'LEV='+str(lev)+' PLOD: ' + name
            node = self.osgNodes[name][0] # livello di dettaglio basso
            children = self.hier[name]    # livello/i alto
            n  = self.MakePLod( name, node, children, lev  )
            self.SaveNode(n,name)
            
            # visito il sottoalbero
            for n in children:
                self.MakeHierarchy( n, lev+1 )

    #----------------------
    def MakeLeaf(self, name, nodes):
        
        root = osg.Group()
        for n in nodes:
            root.addChild(n)

        return root

    #----------------------
    def MakePLod( self, name, node, children, lev ):
        
        plod = osg.PagedLOD()
        plod.setRangeMode( osg.LOD.PIXEL_SIZE_ON_SCREEN )

        ##lod_range = [50,150,300,100000]
        ##plod.addChild( node, 0, lod_range[lev] )  
        
        sz = self.activate_size[children[0]]
        plod.addChild( node, 0, sz )  
        
        for c in children:
            sz = self.activate_size[c]
            c = c.replace('*','')
            ##plod.addChild( self.fake_node, lod_range[lev], 10000000, c + self.ext )
            plod.addChild( self.fake_node, sz, 10000000, c + self.ext )
        
        return plod

    #----------------------
    def MakeInstanceReplacer( self, name, nodes, child, lev ):
        ''' 
        nel MakePLod metto un plod SOPRA il nodo passato (che sarebbe il dettaglio basso)
        e poi faccio i collegamenti ai children (che sono i livelli alti)
        
        Qui ho un solo child (il livello alto)
        ma il livello basso e' puntato da tanti nodi (nodes)
        che puntano tutti allo stesso Geode.
        Il PLOD va inserito sopra il GEODE e sotto i nodes. 
        
        e poi devo stare attento che la pagina non venga sovrascritta.
        (Non viene sovrascritta -- perche non e' piu una foglia)
        '''

        ##lod_range = [50,150,300,100000]

        root = osg.Group()
        for n in nodes:
            root.addChild(n)
        
        # cerco il geode da sostituire
        geode = name.split('@')[1] + '-GEODE'
        finder = FindByName(geode)
        root.accept(finder)
        if not len( finder.results ):
            print "MakeInstanceReplacer: non sono riuscito a trovare il Geode:", geode, ":-("
        #elif len( finder.results ) > 1:
        #    print "MakeInstanceReplacer: ho tovato piu di un Geode :", geode, ":-( ????"
        else:
            # mah - gestiamo anche il caso di piu geodi .... puo succedere in max ...
            for geode in finder.results:
            
                # registro tutti i suoi padri
                parent_list = []
                for i in range(0, geode.getNumParents() ):
                    parent_list.append( geode.getParent(i) )
                
                self.garbage.addChild(geode) # non vorrei che muore se il ref-count gli va a zero
                
                # faccio il plod
                plod = osg.PagedLOD()
                plod.setRangeMode( osg.LOD.PIXEL_SIZE_ON_SCREEN )

                ##plod.addChild( geode,          0,               lod_range[lev]                     )
                ##plod.addChild( self.fake_node, lod_range[lev],  10000000,          child + self.ext  )
                sz = self.activate_size[child]
                plod.addChild( geode,          0,  sz                           )
                plod.addChild( self.fake_node, sz, 10000000,  child + self.ext  )
                
                # sostituisco il plod al geode
                for p in parent_list:
                    p.removeChild( geode )
                    p.addChild(plod)
            
        return root 

    #----------------------
    def SaveNode( self, node, name):
        
        name = name.replace('*','')
        filename = self.ive_path + name + self.ext
        osgDB.writeNodeFile( node, filename )
        
        # delays memory leak alerts
        self.garbage.addChild(node)
        node.thisown=False # boh - ho copiato Gigi

    #----------------------
    def MakeFakeGeom(self, node ):
        ''' 
        geometria fake per permettere il reset-camera.
        E' un cubo con le misure corrispondenti al BoundingSphere del nodo passato
        Con il colore quasi completamente trasparente ed in visualizzazione Points
        '''
        bs = node.getBound()
        c = bs.center()
        r = bs._radius
           
        box = osg.Box( c,r*2 )
        shape = osg.ShapeDrawable( box ) 
        geode = osg.Geode()
        geode.addDrawable( shape )
        ss = geode.getOrCreateStateSet()
        
        pm = osg.PolygonMode( osg.PolygonMode.FRONT_AND_BACK, osg.PolygonMode.POINT )
        ss.setAttribute (pm, osg.StateAttribute.ON )
        mat = osg.Material()
        mat.setDiffuse( osg.Material.FRONT_AND_BACK, osg.Vec4(1.0, 0.0, 0.0, 0.1 )  )
        ss.setAttribute( mat, osg.StateAttribute.ON )
        return geode

#--------------------------------------------------------------------------
if __name__ == "__main__":
    
    # DataDir
    import testdata
    dir = testdata.dir

    # test ProcessDef
    definition = dir + "f_pace\\f_pace.definition"
    p = ProcessDef(definition)
    print '--------------------------'
    print 'now open', p.GetIveRoot()
    
    

