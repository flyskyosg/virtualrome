import sys
import os
#os.environ['OSG_NOTIFY_LEVEL'] = 'DEBUG'
##os.environ['PATH'] = 'D:\\SE5\\msvc\\install\\bin'
##sys.path.append('D:/SE5/msvc/build/osgswig_svn/lib/python')
#
#os.environ['PATH'] = 'D:\\OsgPack\\msvc7\\install\\osg\\bin'
#sys.path.append('D:/OsgPack/msvc7/build/osgswig_svn/lib/python')
osg_python_swig_dir = os.path.join(os.path.dirname(os.path.dirname(sys.executable)),'osgpython_swig')
if os.path.exists(osg_python_swig_dir): 
    sys.path.append(osg_python_swig_dir)
osg_bin_dir = os.path.join(os.path.dirname(os.path.dirname(sys.executable)),'bin')
if os.path.exists(osg_bin_dir): 
    os.environ['PATH'] = osg_bin_dir + os.pathsep + os.environ['PATH']
