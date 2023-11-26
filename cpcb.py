#!/usr/bin/python3

'''Simple library to create CPCB files programmatically'''

def dim(mm):
    return int(10000*mm+.5)

S = SILK = 1
T = TOP = 2
B = BOTTOM = 3

class CPCB:
    def __init__(self, viaid=.3, viaod=.6):
        self.width = 0
        self.height = 0
        self.traces = []
        self.holes = []
        self.pads = []
        self.npholes = []
        self.slots = []
        self.vias = []
        self.lastx = 0
        self.lasty = 0
        self.lasttw = 0.3
        self.lastl = T
        self.viaid = viaid
        self.viaod = viaod

    def hole(self, x, y, id, od):
        self.holes.append((x,y,id,od))
        self.lastx = x
        self.lasty = y

    def via(self):
        self.vias.append((self.lastx,self.lasty,self.viaid,self.viaod))
        self.lastl=T if self.lastl==B else B

    def pad(self, x, y, w, h, layer):
        self.pads.append((x,y,w,h,layer))
        self.lastx = x
        self.lasty = y

    def nphole(self, x, y, d):
        self.npholes.append(x, y, d)
        self.lastx = x
        self.lasty = y

    def slot(self, x, y, w, h, od):
        self.slots.append((x,y,w,h,od))
        self.lastx = x
        self.lasty = y

    def trace(self, x, y, x2, y2,w,layer):
        self.traces.append((x,y,x2,y2,w,layer))
        self.lastx = x2
        self.lasty = y2
        self.lasttw = w
        self.lastl = layer

    def cont(self, x2, y2):
        self.traces.append((self.lastx,self.lasty,x2,y2,self.lasttw,self.lastl))
        self.lastx = x2
        self.lasty = y2

    def grow(self):
        for x,y,id,od in self.holes:
            w = x+od
            h = y+od
            self.width = max(self.width, w)
            self.height = max(self.height, h)
        for x,y,w,h,layer in self.pads:
            w = x+w
            h = y+h
            self.width = max(self.width, w)
            self.height = max(self.height, h)
        for x,y,x2,y2,w,layer in self.traces:
            h = max(y,y2)+w
            w = max(x,x2)+w
            self.width = max(self.width, w)
            self.height = max(self.height, h)
        for x,y,d in self.npholes:
            w = x+d
            h = y+d
            self.width = max(self.width, w)
            self.height = max(self.height, h)
        for x,y,w,h,od in self.slots:
            dd = od - min(w,h)
            w = x+w+dd
            h = y+h+dd
            self.width = max(self.width, w)
            self.height = max(self.height, h)

    def writeheader(self, fd):
        fd.write(f'''<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<cpcb xmlns="http://www.danielwagenaar.net/cpcb-ns.html">
  <board>
    <outline shape="rect" w="{dim(self.width)}" h="{dim(self.height)}"/>
    <options grid="10000" metric="1" planesvis="1" layer1vis="1" layer2vis="1" layer3vis="1" layer4vis="1"/>
  </board>
  <group>
''')

    def writefooter(self, fd):
        fd.write('''  </group>
</cpcb>
''')
        
    def writeholes(self, fd):
        for x,y,id,od in self.holes:
            fd.write(f'''    <hole p="{dim(x)} {dim(y)}" id="{dim(id)}" od="{dim(od)}" ref=""/>\n''')
        for x,y,id,od in self.vias:
            fd.write(f'''    <hole p="{dim(x)} {dim(y)}" id="{dim(id)}" od="{dim(od)}" via="1" ref=""/>\n''')

    def writepads(self, fd):
        for x,y,w,h,layer in self.pads:
            fd.write(f'''    <pad p="{dim(x)} {dim(y)}" w="{dim(w)}" h="{dim(h)}" l="{layer}" ref=""/>\n''')

    def writetraces(self, fd):
        for x,y,x2,y2,w,layer in self.traces:
            fd.write(f'''    <trace p1="{dim(x)} {dim(y)}" p2="{dim(x2)} {dim(y2)}" w="{dim(w)}" l="{layer}"/>\n''')
            

    def writenpholes(self, fd):
        for x,y,d in self.npholes:
            fd.write(f'''    <hole p="{dim(x)} {dim(y)}" d="{dim(d)}"/>\n''')

    def writeslots(self, fd):
        for x,y,w,h,od in self.slots:
            if w>h:
                fd.write(f'''    <hole p="{dim(x)} {dim(y)}" id="{dim(h)}" od="{dim(od)}" sl="{dim(w-h)}" rot="0"/>\n''')
            else:
                fd.write(f'''    <hole p="{dim(x)} {dim(y)}" id="{dim(w)}" od="{dim(od)}" sl="{dim(h-w)}" rot="90"/>\n''')

        
    def write(self, ofn):
        self.grow()
        with open(ofn, 'w') as fd:
            self.writeheader(fd)
            self.writetraces(fd)
            self.writeholes(fd)
            self.writeslots(fd)
            self.writenpholes(fd)
            self.writepads(fd)
            self.writefooter(fd)

#%%

lw = .25
pcb = CPCB()
x0 = 5
for s in [-1,1]:
    x = x0 + s*8.64/2
    pcb.slot(x, 7.13, .6, 1.6, 1.1)
    pcb.slot(x, 7.13 - 5.36, .6, 1.6, 1.1)
    y0 = 7.13-.5-.35 # ?
    y1 = 7.13+.4 # ??
    pcb.pad(x0 + s*6.2/2, y0, 1, .7, T) # far ground
    pcb.trace(x0 + s*6.2/2, y0, x0 + s*8.64/2, 7.13, .7, T)
    pcb.trace(x0 + s*6*.5, y1, x0 + s*8.64/2, 7.13, lw, T)

    # near and far pads
    for k in range(6):
        pcb.pad(x0 + s*(k*.5+.25), y0, .3, .7, T) # far
        pcb.pad(x0 + s*(k*.5+.5), y1, .3, .7, T) # near

    ye = y1 + 2
    # near traces
    for k in [0, 1, 3, 4]:
        dx = -1 if k>2 else 0
        pcb.trace(x0 + s*(k*.5+.5), y1,
                  x0 + s*(k*.5+.5), y1 + .4,
                  lw, T)

    # 5V traces
    pcb.trace(x0 + s*(2*.5+.5), y1,
              x0 + s*(2*.5+.5), y1-.4, .3, T)
    pcb.cont(x0 + s*(2*.5+.25), y0+.4)
    pcb.cont(x0 + s*(2*.5+.25), y0)


for s in [-1,1]:
    for k in range(4):
        stp = 1 if s<0 else 0
        x = x0 + s*(k+stp)*.7 + .15
        dy = 0 if k>=2 else 0
        y = y0 - 2 - dy - k + s/4
        p = k+1 if k>=2 else k
        xp = x0 + s*(p+.5)*.5
        yp = y0 - [1.2, 1.05, .7, .5][k]
        if s>0 and k<2:
            yp -= .4
        pcb.trace(xp, y0,   xp, yp,  lw, T)
        if k>=2:
            xl = xp + s*(.2 + (3-k)*.3)
            yl = yp - .2 -  (3-k)*.3
            pcb.cont(xl, yl)
            xt = x0 + s*(3.9+(k-2)*.2)
        else:
            xt = x
            yl = yp - abs(x-xp)*1.5
        pcb.cont(xt, yl)
        if k>=2:
            xs = xt + s*(.2 + (k-2)*.3)
            ys = yl - abs(xs-xt)
            pcb.cont(xs,ys)
            yq = 4. - (k-2)*.17
            pcb.cont(xs,yq)
            pcb.cont(xs-s*1.2,yq-1.2)
        pcb.cont(x,y)
        pcb.via()
        if k==0 and s==1:
            x = x0 - .3
            pcb.cont(x, y+.5)
        else:
            pcb.cont(x-.2*s,y-.2)
            x = 2*x0-x-.35*s-.05
            pcb.cont(x+.6*s,y-.2)
            pcb.cont(x,y+.2)
            pcb.cont(x,y+.8)
            x = x + .2*s
            pcb.cont(x,y+1)
            if s<0:
                if k>0:
                    y += 1
                pcb.cont(x,y+1)
                x = x - .2
                pcb.cont(x,y+1.2)
            if k>1:
                y += 1
                pcb.cont(x, y+1.2)
            x = x + k*.05*s
            if s<0:
                x -= .05
            pcb.cont(x, y+1.5)
        x = x0 - s*(k+.5)*.65
        pcb.cont(x,y0)
        y = 8.3+((k+s/2)%2)*.4
        pcb.cont(x,y)
        pcb.via()

for s in [-1,1]:
    for k in range(4):
        xp = x0 - s*(k+1)*.5
        if k>=2:
            xp -= s*.5
        x = x0 - s*(k+.5)*.65
        y = 8.3+((k+s/2)%2)*.4
        if s<0 and (k==0 or k==2):
            pcb.trace(xp, y1+.4, x, y1+.8, lw, T)
            pcb.cont(x, y)
        else:
            pcb.trace(xp, y1+.4, x, y, lw, T)
        

for s in [-1, 1]:
    # make 5V lines
    xp = x0 + s*(2*.5+.25)
    pcb.trace(xp, y0, xp, y0-.9, .3, T)
    dx = .9
    y = y0-1.-dx
    pcb.cont(xp+s*dx,y)
    x = xp+1.9*s
    pcb.lasttw = .5
    x1 = x+.2*s
    pcb.cont(x1, y)
    pcb.via()
    x -= .05*s
    pcb.trace(x+.2*s, y, x, y+.5, .6, B)
    pcb.cont(x, y0+1.7)
    pcb.cont(x+.2*s, y0+2)
    pcb.cont(x+.2*s, 8.7)
    x1 += .1*s
    pcb.trace(x1, y, x1, 2, .25, B)
    pcb.cont(x1,.6)
    pcb.cont(x1-s*.3,.3)
    pcb.cont(x0, .3)

              
for s in [-1, 1]:
    x = x0 + s*8.64/2
    pcb.trace(x, 7.13, x, 7.13 - 5.36, .6, B)
    pcb.cont(x, 8.7)

            
pcb.write("/home/wagenaar/Desktop/test.cpcb")
