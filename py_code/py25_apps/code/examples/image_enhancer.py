import Image
import ImageFilter
import ImageEnhance

file2 = "J:\\models\\CaseMC\\3Danni30_giugno2009\\base\\images\\sedia20LightingMap.png"
im = Image.open(file2)
enhancer = ImageEnhance.Brightness(im)
bright_im = enhancer.enhance(4.0) #any value you want
bright_im.save("d:\sedia20LightingMap_b.png")
sharp_im = bright_im.filter(ImageFilter.SHARPEN)
sharp_im.save("d:\sedia20LightingMap.png")
