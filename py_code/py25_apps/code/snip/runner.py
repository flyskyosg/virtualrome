import sys
import os

class runner(object):
    
    def __init__(self):
        self.mypythonbase=os.path.split(sys.executable)[0]
        self.mybase=os.path.split(self.mypythonbase)[0]
        self.myenv=dict()
        for i in os.environ.keys():
            self.myenv[i]=os.environ[i]
        self.myenv['PATH']=self.mypythonbase+os.path.pathsep+os.path.join(self.mybase,'osg_examples')
        self.myenv['OSG_FILE_PATH']=os.path.join(self.mybase,'models','OpenSceneGraph-Data')
    
    def run_example(self,example,arguments=[]):
        myarg=[os.path.join(self.mybase,'osg_examples',example)]
        for i in arguments:
            myarg.append(i)
        print self.myenv
        os.spawnve(os.P_NOWAIT,os.path.join(self.mybase,'osg_examples',example),myarg,self.myenv)

    def run_viewer(self,marguments=[]):
        myarg=[os.path.join(self.mypythonbase,'osgviewer')]
        for i in marguments:
            myarg.append(i)
        print self.myenv
        print "-->",myarg
        print os.environ
        os.spawnve(os.P_NOWAIT,os.path.join(self.mypythonbase,'osgviewer'), myarg,self.myenv)

        
#--------------------------------------------------------------------------
if __name__ == "__main__":

    r=runner()
    r.run_viewer(['cessna.osg'])

    r.run_example('osganimate',['cessna.osg'])

