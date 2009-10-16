
import time

scene = f.canvas.root

top_ss=scene.getChild(0).getStateSet()
ba=top_ss.getUniform("bump_amount")

for i in range(0,100):
    ba.set(0.1*i % 1.0)
    v.frame()
    time.sleep(0.1)
    f.canvas.SwapBuffers()