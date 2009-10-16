import wx
import Image
import ImageFilter
import ImageEnhance

def pilToImage(pil):
    image = wx.EmptyImage(pil.size[0], pil.size[1])
    image.SetData(pil.convert('RGB').tostring())
    return image

def pilToBitmap(pil):
    return imageToBitmap(pilToImage(pil))


def imageToBitmap(image):
    return image.ConvertToBitmap()


imagename='J:\\models\\CaseMC\\3Danni30_giugno2009\\base\\images\\sedia19LightingMap.png'
a = wx.PySimpleApp()
pil=Image.open(imagename)
#wximg = wx.Image(imagename,wx.BITMAP_TYPE_PNG)
#wxbmp=wximg.ConvertToBitmap()
wxbmp=pilToBitmap(pil)
f = wx.Frame(None, -1, "Show JPEG demo")
f.SetSize( wxbmp.GetSize() )
wx.StaticBitmap(f,-1,wxbmp,(0,0))
f.Show(True)
def callback(evt,a=a,f=f):
# Closes the window upon any keypress
    key=evt.GetKeyCode()
    print evt.GetClassName(),"-->",evt.GetKeyCode()
    if(key == 113): # 'q'
        f.Close()
        a.ExitMainLoop()
    if(key == 316): # '->'
        imagename='J:\\models\\CaseMC\\3Danni30_giugno2009\\base\\images\\sedia20LightingMap.png'
        pil=Image.open(imagename)
        wxbmp=pilToBitmap(pil)
        wx.StaticBitmap(f,-1,wxbmp,(0,0))
        f.Show(True)
    if(key == 314): # '<-'
        imagename='J:\\models\\CaseMC\\3Danni30_giugno2009\\base\\images\\sedia18LightingMap.png'
        pil=Image.open(imagename)
        wxbmp=pilToBitmap(pil)
        wx.StaticBitmap(f,-1,wxbmp,(0,0))
        f.Show(True)

wx.EVT_CHAR(f,callback)
a.MainLoop()