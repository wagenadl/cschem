Introduction: CPCB features and terminology
===========================================

A document in CPCB consists of a single PCB that can be either
rectangular or round [#f1]_. The PCB can be populated by traces, pads,
holes, text, and more. These basic objects can be placed on any of
three layers: a bottom copper layer, a top copper layer, and a top
silkscreen layer. Most of the time, holes and pads on the copper
layers will be grouped together with some text and line segments on
the silkscreen layer to form a “component,” such as a connector, a
resistor, or an IC socket.

Object properties
-----------------

Objects have various properties, such as line width, hole diameter, or
pad size, and these can be edited by means of a toolbar on the right
of the main window. Objects can be placed on the PCB, connected with
traces, moved around, rotated, transferred to other layers, etc., all
with straightforward mouse interactions.

Predefined components
---------------------

CPCB comes with a small library of predefined components, but creating
your own custom components is very easy thanks to a flexible system of
coordinates and grids and quick access to functions to label pins.

Interaction with CSchem
------------------------

CPCB can be used stand-alone for small projects, but is a particularly
strong tool in combination with CSchem: If you “link” a CSchem
schematic to a PCB layout, CPCB will show a toolbar with all the
components that you have not yet placed, and will also highlight
connections that are required to complete the circuit as designed.

Interaction with the rest of the world
--------------------------------------

CPCB can export its designs in Gerber format, which is the
*lingua franca* that most online PCB fabrication services
understand.

.. _fny:

Footnote
---------

.. [#f1] More complex outline shapes are planned for the future.
