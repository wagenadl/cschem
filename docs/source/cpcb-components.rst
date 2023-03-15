Working with components
=======================

CPCB ships with a small library of predefined components and it is
very easy to add your own to the library.

Placing library components onto a PCB
-------------------------------------

Press
“Control”+“Shift”+“O” to show the library (or select “Open
library” from the Tools menu). CPCB components are saved as SVG
files, so in some operating systems the Filer window will show
previews. Components may be placed on the PCB simply by dragging them
in from a Filer window. Please note that CPCB cannot handle arbitrary
SVG files, only the special SVG files that it creates itself, so do
not draw components in an external SVG editor and expect CPCB to
import them into your layout.

Creating new components
-----------------------

To create a new component from scratch, simply place holes, pads, and
outlines onto a PCB. It may be helpful to use “incremental”
coordinates to place features of the component relative to each
other. Be sure to assign pin numbers to all holes and pads. When done,
use “Control”+“G” or the menu to group the features together. A
default “reference” text will appear, which you could edit. For
instance, if you drew a new symbol for a diode, you might want to
replace the “X?” with “D?”. To save a copy of the component for
use in future layouts, press “Control”+“Shift”+“I” or select
“Save component…” from the “Tools” menu.

Editing components
------------------

To edit a component, the easiest thing to do is to double click on its
outline on the board. This will “enter the group” of the component,
hiding all PCB elements that are not part of the component. You can
now add, remove, or modify elements of the component at will. Double
click on the background to “leave” the group. At this point you may
want to  press “Control”+“Shift”+“I” (or select
“Save component…” from the “Tools” menu) to save the component,
either replacing the old version if you are simply fixing a mistake,
or as a copy if you are creating a new component based on an older
one.

If all you need to do is renumber some pins of a component, it is even
easier to just double click on those pins and type the new numbers in
the popup box.

.. _comppanel:

Using the components panel
--------------------------

The components pane serves  as a placeholder for components that are
part of a linked schematic but that have not yet been placed on the
PCB. On first use, it simply shows a list of those components,
identified by their “Reference” text, their “part/value” text (if
defined) and their number of pins. If you have used a similar
component before, CPCB may display a default outline package
instead. The following actions are available in the components pane:

- You can drag a component outline from the pane to the PCB.
- You can drag a component outline from a Filer window to the pane.
- You can drag a component outline from one tile in the pane to
  another to conveniently apply the same outline to multiple
  components.
- You can rotate or flip an outline by pressing “R”,
  “Shift”+“R”, or “F” after clicking it.
- You can copy a component outline from the PCB into the pane by
  selecting (only) that component on the PCB and clicking on the
  target tile with the middle mouse button or with “Control” and
  the left mouse button.

Instead of dragging component outlines directly onto the PCB from a
Filer window, it is often convenient to first drag them onto the
“components panel,” copy them to all like items, and then drag those
onto the PCB.
