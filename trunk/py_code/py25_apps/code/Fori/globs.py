'''
'''
import os

#--------------------------------------------------------------------------
SearchDir = ''

#--------------------------------------------------------------------------
def DataDir():
    # locate the DataDir
    dir = os.getenv('DATADIR')
    if not dir:
        print 'env-var "DATADIR" not found, exiting'
        sys.exit()
    return dir