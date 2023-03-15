My procedure for SMT soldering
==============================

It turns out that it is quite easy to
make solder masks on a laser cutter, and low-temperature lead-free
solder paste practically eliminates the risk of overheating
components. Here is what I do:

#. Design your layout and have your board manufactured. Do yourself
   and the environment a favor and pay the little extra for a lead-free finish.
#. Export the paste mask as an svg file with 0.005” (0.125 mm)
   shrinkage. This file can be loaded into Corel Draw for laser cutting.
#. Cut the paste mask out of overhead transparency. I use Apollo Laser
   Printer Transparency Film (#VCG7060E) placed on top of a sheet of
   aluminum. The film is 0.002” (0.050 mm) thick; the thickness of the
   aluminum is not important. Cut the *outlines* of the pads in
   vector mode. On my 75-W ULS laser cutter, I use 25% power, 100%
   speed for the pads, cutting three times. Then I cut the board
   outline with 65% power, 100% speed, once. This gives a nice flat
   surface on the back with slight ridges on the front. Check for
   hanging chads.
#. Tape the mask to the board. I use packing tape, but it is not
   critical. Use a stereomicroscope or a magnifier to ensure precise
   alignment. (A pair of +2.00 to +2.50 reading glasses makes for a
   remarkably effective head-mounted magnifier.)
#. Squeeze solder paste into the pads. The paste I have been using is
   Sn 42%/Bi 57%/Ag 1%, melting point 137 °C. It is described
   as “No-clean flux, 87% metal (20–38 microns).”  The package
   expires in 12 months, so don't buy too much at once. I squeeze it
   out of the syringe onto one side of the layout, then use a metal
   spatula or a plastic ruler to distribute it into the holes.
#. Remove the mask. This has to be done very carefully so as not to
   smudge the pads.
#. Preheat the board. I have a really cheap reflow workstation. The
   thermostat of the IR preheater is broken. No matter, use a
   thermometer to check that the temperature at the location of your
   board is around 100 °C. Preheating a PCB takes 2–3
   minutes. Don't worry about overheating: 100 °C is a profoundly
   safe temperature for all electronic components I've ever worked
   with. (Some plastic LED lenses may get a little soft, but they
   have never actually melted.)
#. Melt the solder. My reflow workstation comes with a mini heat
   gun. I set it to 175 °C and melt all the solder by moving the heat
   gun slowly over the board. You can see the paste turn
   shiny. Components may shift a little at this time; usually they
   settle very nicely to center over their pads. Be careful not to
   blow your parts away with too much air flow!
#. Cool the board. Remove the board from the heat using a pair of
   pliers (careful: it’s hot!) and watch the solder solidify under a
   dissection scope.
#. Clean up. It is very important to clean the metal spatula
   (and the mask if you plan to use it again).


A note on temperature
=====================

It might seem that 137 °C is a crazy low melting point, but it is
actually not unreasonable: For instance, the max. operating
temperature of Luxeon LEDs is 135 °C (at the diode junction). Thus,
the solder should never melt under normal operating conditions. In my
experience, soldering through-hole components onto the same board with
“regular” solder (tin-silver-copper, see :ref:`below <tscsolder>`)
doesn't transfer enough heat to melt these connections either.

Notes on solder types
=====================

Most solder formulations melt at a much higher temperature than the
one I use, which makes it much more difficult to avoid damage to
electronic components: Old fashioned leaded solder melts around
183 °C and other lead-free formulations tend to melt at even
higher temperatures. For reference, here are some common
formulations. All of the information in this section was taken from
https://en.wikipedia.org/wiki/Solder.

The formulation I use:
^^^^^^^^^^^^^^^^^^^^^^
  Sn42/Bi57/Ag1

    Melts at 137–139 °C. Patented by
    Motorola. Wiki says “Good thermal fatigue performance.”

Other lead-free formulations:
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. _tscsolder:

  The tin-silver-copper (Sn-Ag-Cu or “SAC”) family

    These typically have 3–4% silver, 0.5–0.7% copper, lots of tin,
    and have melting points around 217–220 °C. That requires pretty
    high soldering temperatures, which is obviously a challenge in a
    relatively poorly controlled environment. Sn-Ag-Cu-Mn formulations
    have a slightly lower melting point (211–215 °C) but still well
    above Sn-Pb (next section).
  
  In97/Ag3

    Melts at 143 °C. Reportedly used for cryogenic applications and
    photonic devices.

  In100 (or In99)

    Melts at 157 °C. Used in low-temperature
    physics and for soldering gold. Bonds to aluminum.

  Sn88/In8.0/Ag3.5/Bi0.5

    Melts at 197–208 °C, closer to the
    Sn/Pb types. Patented by Panasonic.

Leaded solder types:
^^^^^^^^^^^^^^^^^^^^

  60/40 Sn/Pb

    Melts around 188 °C.

  63/37 Sn/Pb

    Melts at exactly 183 °C, the lowest of all tin-lead allows.

Of course, you don't want to use these unless you have a really good reason.

