import StringIO
import Image
import ImageFilter
import ImageEnhance


pil=Image.open('J:\\models\\CaseMC\\3Danni30_giugno2009\\base\\images\\pav_pranzo5LightingMap.png')
f = StringIO.StringIO()
pil.save(f,"PNG")
d=f.getvalue()

# Tested with wxPython 2.3.4.2 and PIL 1.1.3.
from wxPython import wx
import Image             # Only if you need and use the PIL library.

def bitmapToPil(bitmap):
    return imageToPil(bitmapToImage(bitmap))

def bitmapToImage(bitmap):
    return wx.wxImageFromBitmap(bitmap)


def pilToBitmap(pil):
    return imageToBitmap(pilToImage(pil))

def pilToImage(pil):
    image = wx.wxEmptyImage(pil.size[0], pil.size[1])
    image.SetData(pil.convert('RGB').tostring())
    return image

#Or, if you want to copy alpha channels too (available from wxPython 2.5)
def piltoimage(pil,alpha=True):
   if alpha:
       image = apply( wx.EmptyImage, pil.size )
       image.SetData( pil.convert( "RGB").tostring() )
       image.SetAlphaData(pil.convert("RGBA").tostring()[3::4])
   else:
       image = wx.EmptyImage(pil.size[0], pil.size[1])
       new_image = pil.convert('RGB')
       data = new_image.tostring()
       image.SetData(data)
   return image


def imageToPil(image):
    pil = Image.new('RGB', (image.GetWidth(), image.GetHeight()))
    pil.fromstring(image.GetData())
    return pil

def imageToBitmap(image):
    return image.ConvertToBitmap()

