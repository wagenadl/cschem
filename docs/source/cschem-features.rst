Introduction: CSchem features and terminology
---------------------------------------------

CSchem circuit designs consist of a single, conceptually infinitely
large sheet containing “elements” and “connections.”

Elements are things like resistors and opamps, connections are simple
wires connecting between pins of elements. Each element has up to two
pieces of text associated with it: a circuit “reference” (e.g.,
“*R*:sub:`1`”, “*J*:sub:`2`”, or ”*A*:sub:`3.2`”) and a “part/value”
designation. The “part/value” designation is free-form. You can use
it for a resistor value (e.g., “10 kΩ” or “1 Ω 3 W”) or
for a part number (e.g., “OPA2350”) or for an arbitrary
label.

Connections are wires between (pins of) elements. Where wires meet, a
“junction” symbol is automatically inserted. Of course, wires can
also cross without electrical contact. Connections normally are
constrained to run horizontally or vertically with right-angle elbows.

The only graphical element that CSchem supports other than elements
and connections are arbitrary textual annotations that can be placed
anywhere on the sheet.

At present, CSchem does not have explicit support for buses with
multiple signal wires or for splitting a drawing across multiple
sheets.


