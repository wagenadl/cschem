#!/usr/bin/python3

import sys
import re
import xml.etree.ElementTree as ET

# ifn = sys.argv[1]
ifn = '/home/wagenaar/Desktop/voltage-to-dual-current.schem'

components = {}
junctions = {}
ports = {}
connections = {}

def stripbraces(s):
    return re.sub("{.*}", "", s)

def nameval(x):
    if 'name' in x:
        if 'value' in x:
            return '%s (%s)' % (x['name'], x['value'])
        else:
            return x['name']
    elif 'value' in x:
        return '(%s)' % x['value']
    elif 'type' in x:
        return x['type']
    else:
        return '{%s}' % x['id']

def parsecircuit(circ):
    for child in circ:
        t = stripbraces(child.tag)
        id = child.attrib['id']
        if t=="component":
            components[id] = nameval(child.attrib)
        elif t=='junction':
            junctions[id] = 1
        elif t=="port":
            ports[id] = nameval(child.attrib)
        elif t=="connection":
            connections[id] = (child.attrib['from'], child.attrib['to'])
        else:
            raise Exception('Unexpected tag: ' + t)

print('Loading', ifn)
tree = ET.parse(ifn)
root = tree.getroot()
for child in root:
    if stripbraces(child.tag)=="circuit":
        parsecircuit(child)

# Now, extract nets
# A net is a set of connected nodes
nets = {}
repmap = {}
for con in connections.values():
    f = con[0]
    t = con[1]
    if t in repmap:
        nets[repmap[t]].add(f)
        repmap[f] = repmap[t]
    elif f in repmap:
        nets[repmap[f]].add(t)
        repmap[t] = repmap[f]
    else:
        repmap[f] = f
        repmap[t] = f
        nets[f] = {f, t}

names = {}
for k,v in components.items():
    names[k] = v
for k,v in ports.items():
    names[k] = v

namednets = {}
for n in nets.values():
    rep = None
    for k in n:
        if k.endswith(':'):
            if k[:-1] in ports:
                rep = k
        elif rep is None:
            rep = k
    if rep==None:
        raise Exception('No representative')
    elt,pin = rep.split(':', 2)
    print('Net:', names[elt], pin)
    nn = set()
    for k in n:
        elt,pin = k.split(':', 2)
        if elt in names:
            name = names[elt]
            namebits = name.split(' ')
            pinbits = pin.split('/')
            if pin=='':
                nn.add(namebits[0])
            else:
                nn.add(namebits[0] + ':' + pinbits[-1])
    joins = set()
    uni = nn
    for k in nn:
        if k in namednets:
            uni = uni.union(namednets[k])
    for k in nn:
        namednets[k] = uni

seen = set()
for k,v in namednets.items():
    if k not in seen:
        seen = seen.union(v)
        print(v)
