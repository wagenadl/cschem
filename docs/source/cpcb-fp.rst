Working with filled planes
==========================

Filled planes are a convenient way to distribute ground or power
connections to different parts of a PCB. A ground plane can also serve
an important role in protecting your circuit from electromagnetic
interference.

Placing a filled plane
----------------------

Filled planes are placed in “Filled plane mode” simply by dragging
out a rectangle starting from an empty location on the PCB.

Editing a filled plane
----------------------

In “Filled plane mode,” you can move the corner points of a filled
plane around by dragging with the mouse. You can insert corners along
any edge simply by dragging the marker that automatically appears when
you hover near an edge while holding “Shift”. You can remove a corner
by pressing “Delete” when it is highlighted. If you hover the mouse
over an edge without holding “Shift”, an edge marker rather than a
corner marker appears. This is a convenient way to move the edge as a
whole. A potentially nonintuitive (but very helpful) behavior is that
perfectly horizontal edges can only be dragged in the vertical
direction and vice versa, whereas edges that are not parallel to a
principal axis can be dragged in any direction.

Deleting a filled plane
-----------------------

In “Filled plane mode,” you can delete a filled plane simply by
pressing “Delete” while hovering over its interior. In “Edit mode”
you can delete a filled plane by first selecting it and then pressing
“Delete.”

Moving a filled plane
---------------------

Move a filled plane in its entirety is done in “Edit mode” rather
than in “Filled plane mode,” simply by selecting it and dragging it
to a new location.


Making and breaking connections to a filled plane
-------------------------------------------------

In “Filled plane mode,” connections between pads and filled planes can
be created and removed by double clicking on the pad. Likewise,
connections can be made between plated holes and filled planes. Use
the “Layer” buttons at the bottom of the “Properties bar” to choose
whether a connection is made in the top or bottom copper layer. Note
that plated holes can only connect to one filled plane. This is to
prevent a mistake I have made too many times in other PCB layout
programs, where I meant to move a filled plane connection from the
bottom layer to the top layer but accidentally preserved the
connection in the bottom layer as well. (Because these connections lie
perfectly on top of one another, the bottom layer connection is easily
overlooked.) If you absolutely must connect two filled planes using a
single hole, you can create two separate holes first and drag them on
top of each other. (But don't tell anyone I said that.)
