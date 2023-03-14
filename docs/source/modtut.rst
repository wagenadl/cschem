Tutorial: Modifying a circuit
=============================

Let's say we want to modify the circuit we drew in the previous
tutorial to turn it into an AC amplifier by inserting some AC
filters. This is straightforward enough. You can just hover over a
connection and press Delete to cut it.

  .. image:: mod-cut.png
             :width: 750
             :align: center

You may notice I moved J1 and J2 a little to make additional space.

Now we can just drag in the extra components and rotate them as needed:

  .. image:: mod-new.png
             :width: 750
             :align: center

The exact component values can wait, so let’s hide the “P/V”
placeholders by hovering and pressing Backspace, and draw the new
connections.

  .. image:: mod-recon.png
             :width: 750
             :align: center

Now imagine that we actually want the input to float relative to the
output. That means the shell of J1 should be connected to our system
ground by way of a capacitor rather than directly. Let's cut the wire.

  .. image:: mod-red.png
             :width: 750
             :align: center

That left a dangling line, which is turned red. The same may happen if
we delete a component. (Try deleting R3, for instance.) It is fine to
leave the red wire for now, after all, we are about to connect our new
capacitor to it. Alternatively, all dangling wires can be deleted by
pressing Ctrl+B or choosing “Remove dangling connections” from the
“Tools” menu. (Of course, you can also delete individual dangling
connections by hovering over them and pressing “Delete”.)

Wires turn back to healthy black when a new connection is made to them:

  .. image:: mod-c3.png
             :width: 750
             :align: center

Conclusion
----------

Don't build that circuit. It contains several flaws. For instance, the
input to A1 should probably be referenced to 2.5V rather than
ground. And there should be a resistor between the pins of J2. Perhaps
J2 should be floated. You get the picture. Try making those
adjustments as a further exercise! What can you do to make the circuit
easier to read? Do you think the input stage looks better drawn like
this:

  .. image:: mod-alt.png
             :width: 400
             :align: center

Does that show more clearly that the input to A1 is going to hit the
negative rail all the time, necessitating referencing the input to 2.5
V? Is this:

  .. image:: mod-25.png
             :width: 400
             :align: center

a good solution?

I hope that editing in CSchem is so intuitive as to invite quick
editing and copying of intermediate versions of this nature into your
lab notebook. (Don't have one? Why not? They are great for home use as
well. Try my `NotedELN <https://github.com/wagenadl/notedeln>`_. You can
copy-and-paste images from CSchem directly into NotedELN.)
