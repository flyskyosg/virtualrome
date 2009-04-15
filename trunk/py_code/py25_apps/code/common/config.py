'''
Module config:

description:
the config is an application global object
used to store/retrieve persistent application settings.

to retrieve the config:
    import config
    conf = config.Get()

to write a value:
    conf.Write(key,value)       # for string value --- 
    conf.WriteInt(key,value)    # for integer value
    conf.WriteFloat(key,value)  # for float value
    conf.Flush()                # !! always call this after writing !!

to read a value:
    conf.Read(key, default="")        # Returns a string.  
    conf.ReadInt(key, default=0)      # Returns an int.  
    conf.ReadFloat(key, default=0.0)  # Returns a floating point number.  

-------------------------
'''
import os
import sys
import wx

# customize this at your will
appName = 'config'


# the config global object
# retrieve it by calling Get()
_config = None

# configFile -- dont touch this
import __main__
appfile = __main__.__file__
appdir = os.path.dirname(appfile)
configfile = appdir + '\\' + appName + '.ini'
configfile = configfile.replace('\\','/')

#----------------------------
def Get():
    global appName, _config, configfile
    if _config:
        return _config
    else:
        #if not wx.GetApp():
        #    print 'please create a wx.App before calling config.get()'
        #    return None --- non e' vero -- l'applicazione non serve in questo caso
        
        _config = wx.FileConfig(localFilename=configfile, style=wx.CONFIG_USE_LOCAL_FILE)
        return _config

#------------------------------------------------------------------------------
if __name__ == "__main__":

    appName = 'test' # dont overwrite virtualrome.ini
    c = Get()
    c.Write     ('key1','ciao')
    c.WriteInt  ('key2',1)
    c.WriteFloat('key3',1.23)
    c.Flush()
    
    v = c.Read('key1') 
    print v == 'ciao'

    v = c.ReadInt('key2') 
    print v == 1
    
    v = c.ReadFloat('key3') 
    print v == 1.23
    
    #example: store a list
    lst = [1,2,'a','b']
    c.Write('key4',str(lst))
    c.Flush()
    v = c.Read('key4') 
    v = eval( v )
    print v
    print v == lst
