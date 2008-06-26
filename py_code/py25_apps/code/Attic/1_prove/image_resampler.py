import Image
import os
import sys

for i in ('nvtt','ImageMagick-6.3.9-Q16'):
    for j in  (os.path.dirname(os.path.dirname(sys.executable)),os.path.join(os.path.dirname(os.path.dirname(sys.executable)),'image_proc')):
        if os.path.exists(os.path.join(j,i)): 
            os.environ['PATH'] = os.path.join(j,i) + os.pathsep + os.environ['PATH']
            break


def close_2_power(n):
    fit =1
    while n > 2 * fit : 
        fit = 2 * fit
#prefer subsample versus oversample ...  see 0.5 factor
    if 0.5 * (n -fit) > 2*fit -n :
        return 2*fit
    else :
        return fit

class ImageResampler():
    def __init__(self,im):
        #count textures
        self.in_image=im
        pass
    def fit_image(self,rmin,rmax):
        return(fit_image(self.in_image,rmin,rmax))
        
    def resample_image(self,newsize,out_file,cache=True) :
         resample_image(self.in_image,newsize,out_file) 

    def resample_set(self,rmin,rmax,out_format) :
        resampled_list=[]
        for i in self.fit_image(rmin,rmax) :
            out_file=out_format % i
            self.resample_image(i,out_file)
            resampled_list.append((out_file,i))

        
def fit_image(in_image,rmin,rmax) :
    lmin=close_2_power(rmin)
    lmax=close_2_power(rmax)
    p2_size=(min(lmax,close_2_power(in_image.size[0])),min(lmax,close_2_power(in_image.size[1])))
    size_list=[p2_size]
#    while ((p2_size[0] > lmax) | (p2_size[1] > lmax)) & (p2_size[0] > 2 * lmin) & (p2_size[1] > 2 * lmin):
    while  (p2_size[0] > 2 * lmin) & (p2_size[1] > 2 * lmin):
        p2_size=(p2_size[0] / 2,p2_size[1] / 2)
        size_list.append(p2_size)
#        print "-->",p2_size
        
    p2_size=(max(p2_size[0],lmin),max(p2_size[1],lmin))
    p2_size=(min(p2_size[0],lmax),min(p2_size[1],lmax))
        #return in_image.resize(newsize, Image.ANTIALIAS ) 
    return size_list
#  
def resample_image(in_image,newsize,out_file,cache=True,quality=70) :
    (root,ext)=os.path.splitext(out_file)
    if(""==ext):
        if(in_image.size[0] * in_image.size[1] > 512*512):
            ext=".jpg"
        else:
            ext=".dds"
        out_file= out_file + ext
    if(os.path.exists(out_file) and cache):
        print "skipping generation of file ",out_file
    else:
        if(in_image):
            if(".dds" == ext):
#                newsize=fit_image(in_image,min(8,rmin),rmax)
                out_image = in_image.resize(newsize, Image.ANTIALIAS )
                tmpfile=root+"_tmp.png"
                out_image.save(tmpfile)
                os.system("nvcompress -bc1 -fast -nomips " +  tmpfile +" " + out_file)
                os.remove(tmpfile)
            if((ext==".jpg") or (ext==".jpeg") or (ext=="png")):
#                newsize=fit_image(in_image,min(8,rmin),rmax)
                out_image = in_image.resize(newsize, Image.ANTIALIAS )
                if((ext==".jpg") or (ext==".jpeg")):
                    out_image.save(out_file,"quality="+str(quality))
            
        
if __name__ == "__main__":
#    imr=ImageResampler(Image.open("H:/vrome/models/modelli_tipo/oppidum/images/basolato3_256.tga"))
    imr=ImageResampler(Image.open("D:/prove/oppidum/tetto_ricostr.tga"))
    #imr = Image.open("H:/vrome/models/modelli_tipo/oppidum/images/basolato3_256.tga")
    imr.resample_set(16,1024,"D:/prove/oppidum/tetto_ricostr_resize_%d_%d.dds")
#    print fit_image(Image.open("H:/vrome/models/modelli_tipo/oppidum/images/basolato3_256.tga"),4,64)
    #resample_image("H:/vrome/models/modelli_tipo/oppidum/images/basolato3_256.tga",4,64,"H:/vrome/models/modelli_tipo/oppidum/images/basolato3_256_tiny.dds") 