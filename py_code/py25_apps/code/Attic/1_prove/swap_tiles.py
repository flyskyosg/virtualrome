import os
import sys
import glob
import shutil

def move_files(old_dir,new_dir,moved_dir):
    try:
        os.mkdir(moved_dir)
    except (IOError, os.error):
        print moved_dir," Exists"
    print "sono qui"
    for i in os.listdir(new_dir):
        oldfile = os.path.join(old_dir,os.path.basename(i))
        if(os.path.exists(oldfile)):
            print "moving ",oldfile," ",moved_dir
            shutil.move(oldfile,moved_dir)
        
        
if __name__ == '__main__':
    basedir = "Q:/vrome/web/terreni/attuale/ive_16_256_dds_l12_subtile"
    move_files(os.path.join(basedir,"attuale_subtile_L3_X1_Y5"),os.path.join(basedir,"attuale_subtile_L5_X7_Y21"),os.path.join(basedir,"moved"))