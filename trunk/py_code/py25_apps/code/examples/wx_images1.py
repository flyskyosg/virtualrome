import Image
import random
import time
import wx

def pilToImage(pil):
  image = wx.EmptyImage(pil.size[0], pil.size[1])
  image.SetData(pil.convert('RGB').tostring())
  return image

def imageToBitmap(image):
  return image.ConvertToBitmap()

def pilToBitmap(pil):
  return imageToBitmap(pilToImage(pil))

class Item(wx.Panel):
  """Displays an image that is computed on the fly"""

  def __init__(self, parent, itemid,file):
    wx.Panel.__init__(self, parent, style=wx.SIMPLE_BORDER |
wx.FULL_REPAINT_ON_RESIZE)
    #self.Bind(wx.EVT_PAINT, self.OnPaint)
    wx.EVT_PAINT(self, self.on_paint)
    self.Bind(wx.EVT_SIZE, self.OnSize)
    self.itemid = itemid
    self.bitmapsize = (-1,-1)
    #image = Image.new(file, self.GetClientSize())
    self.imgname=file
    image = Image.open(file)
    print "-->",self.GetClientSize()
    self.bitmap = pilToBitmap(image)

  def UpdateBitmap(self):
    if self.GetClientSize() != self.bitmapsize:
      self.bitmapsize = self.GetClientSize()
      print "update " + str(self.itemid) + " " + str(self.GetClientSize())
      time.sleep(0.1 + 0.1 * random.random())
      #image = Image.new('J:\\models\\CaseMC\\3Danni30_giugno2009\\base\\images\\sedia20LightingMap.png', self.GetClientSize())
      image = Image.open(self.imgname)
      self.bitmap = pilToBitmap(image)
    else:
      print "skip " + str(self.itemid)

  def OnSize(self, event):
    # Update bitmap
    self.UpdateBitmap()
    # Trigger paint event
    cmd = wx.CommandEvent(wx.EVT_PAINT.evtType[0])
    cmd.SetEventObject(self)
    cmd.SetId(self.GetId())
    self.GetEventHandler().ProcessEvent(cmd)

  def on_paint(self, event):
    dc = wx.PaintDC(self)
    dc.BeginDrawing()
    dc.DrawBitmap(self.bitmap, 0, 0)
    dc.EndDrawing()

class MyFrame(wx.Frame):
  """A display of three Item objects"""

  def __init__(self, *args, **kwargs):
    wx.Frame.__init__(self, *args, **kwargs)

    self.view = [
        Item(self,0,'J:\\models\\CaseMC\\3Danni30_giugno2009\\base\\images\\sedia20LightingMap.png'),
        Item(self,1,'J:\\models\\CaseMC\\3Danni30_giugno2009\\base\\images\\sedia21LightingMap.png'),
        Item(self,2,'J:\\models\\CaseMC\\3Danni30_giugno2009\\base\\images\\sedia20LightingMap.png')
    ];
    self.view[0].SetBackgroundColour(wx.Colour(100,0,0))
    self.view[1].SetBackgroundColour(wx.Colour(0,100,0))
    self.view[2].SetBackgroundColour(wx.Colour(0,0,100))

    sizerVer = wx.BoxSizer(wx.VERTICAL)
    sizerVer.Add(self.view[2], proportion = 1, flag = wx.EXPAND | wx.BOTTOM,
border = 5)
    sizerVer.Add(self.view[1], proportion = 1, flag = wx.EXPAND , border =
5)

    sizerHor = wx.BoxSizer(wx.HORIZONTAL)
    sizerHor.Add(self.view[0], proportion = 1, flag = wx.EXPAND | wx.ALL,
border = 5)
    sizerHor.Add(sizerVer,     proportion = 1, flag = wx.EXPAND | wx.TOP |
wx.BOTTOM | wx.RIGHT, border = 5)
    self.SetSizer(sizerHor)

myApp = wx.App(0)
frame = MyFrame(None, wx.ID_ANY, "Main Frame")
frame.Show()
myApp.MainLoop()

