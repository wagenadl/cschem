.. CSchem documentation master file, created by
   sphinx-quickstart on Sun Feb 26 13:42:21 2023.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

***************
CSchem and CPCB
***************

Introduction
============

CSchem and CPCB are a pair of programs for designing electronic
circuits and laying them out on a printed circuit board. These
programs are especially intended for users who may not be full-time
electrical engineers, but who occasionally design electronics in the
course of their work/study/hobbies. That is not to say that CSchem
could not be used to make complex designs, but rather that it will not
overcomplicate simple designs.

These leading principles guided CSchem development:

- The user interface should be clean and inviting;
- If you have used `Powerpoint` or `Inkscape`, drawing in CSchem
  should be intuitive.
- Circuits drawn with CSchem should look elegant enough to be included
  in a scientific paper without external editing, by default, and
  without special effort.

Here is a screenshot of CSchem while designing a simple opamp circuit:

  .. image:: eg-opamp.png
             :width: 428
             :align: center

and here is an example of the control circuit for a thermostatically
regulated behavior box, saved in “svg” format from CSchem:

  .. image:: eg-thermostat1.png
             :width: 700
             :align: center

CPCB was likely designed to be intuitive for casual users. Yet, out of
the box it can export “Gerber” files that can be directly sent to PCB
manufacturers. Here is a screenshot of CPCB showing a design of a
simple PMT amplifier:

  .. image:: eg-pmtamp.png
             :width: 500
             :align: center

CPCB does not have any “auto routing” facilities, but it does have a
“verify nets” function that checks whether the traces you drew on the
PCB faithfully correspond to the circuit you designed in
CSchem. Further, while you work on the PCB layout, CPCB can tell you
which components are still missing, and which connections still need
to be made. In the screenshot above, one connection is still missing,
and CPCB indicates (in blue) to where the pin under the mouse pointer
should be attached. Likewise, if you inadvertantly create a
short-circuit, CPCB indicates the spurious connections in pink.

Learn more
^^^^^^^^^^
                     
.. toctree::
   :maxdepth: 1
   :caption: The rest of this document is comprised of the following parts:

   install
   cschemtut
   cpcbtut
   cschem
   cpcb
   
   
