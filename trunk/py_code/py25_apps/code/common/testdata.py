'''
'''
import os

#---the data root directory----
dir = os.getenv('DATADIR')
if not dir:
    print 'env-var "DATADIR" not found, exiting'
    sys.exit()

