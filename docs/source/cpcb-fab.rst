Exporting for fabrication
=========================

The purpose of a PCB layout program is to allow you to make PCBs. CPCB
can therefore export your design to the industry-standard “Gerber”
file format [#f1]_.  To export a Gerber file, press “Control”+“E” or
choose “Export fabrication files” from the “File” menu.

Gerber files can be uploaded to a variety of fabricators on the
internet. A useful website to compare offers from many sources is
https://pcbshopper.com. Pro tip: Be sure to select lead-free surface
finish as well as lead-free solder. Don't poison youself.

Most fabricators have a way to verify that their interpretation of the
file you uploaded matches your intentions.

.. caution::

   Please use those tools, and note that by using CPCB, you accept the
   terms of the GNU General Public License. That means, among other
   things, that **the author cannot accept any liability for incorrect
   fabrication**, even if, for instance, CPCB exported patently
   incorrect Gerber files.

In the “Export” dialog, you have the option of exporting several additional types of files that can be used in the fabrication process

Bill of materials (BOM)
-----------------------

This is a CSV file mapping component identifiers in your design to
part numbers from manufacturers and vendors. If you are shopping for
parts yourself, the “Compact” option is useful. If you are outsourcing
your part placement, it may be better to export the long-form
list. PCB assemblers are very picky about the format of the BOMs they
accept, and they do not seem to agree on the preferred format. You
should compare CPCB’s output with their published examples and be
ready to reformat the file. LibreOffic Calc is very useful for that.

Pick-and-place (P&P) table
--------------------------

This is a CSV file listing the positions and orientations of all your
components, for automated PCB assembly. As for BOMs, assemblers are
very picky about the format of P&P files they accept.

A common problem is confusion about which orientation for a part
constitutes “up”. Always check whether the assembler correctly
interprets your design before submitting for actual fabrication. The
“PNP mode” (see :ref:`CPCB's user interface`) can be used to set the
nominal orientation of parts within CPCB.

You have a choice of exporting placement for all parts, or only for
surface-mounted parts.

List of unplaced items
----------------------

This is a simple CSV file listing any parts not exported into BOM or
PNP files. This is a useful final check on whether you have put all
the requisite information into the BOM table.

Solder paste mask
-----------------

If your layout involves any surface-mount components, you may wish to
produce a solder paste mask, for laser cutting. The number to the
right specifies the allowance for paste mask shrinkage. This number
can be used to make the holes slightly smaller than the solderable
area, to compensate for the kerf of a laser cutter. :ref:`My procedure
for SMT soldering` explains how I make masks out of transparency film.

Front panel stencil
-------------------

A small selection of connectors in CPCB's component library include
drawings of their front panel placement, comprising a line marking the
top of the PCB and outlines of the holes that need to be cut into the
front panel to accommodate the part. You can export an SVG file with
these markings as a starting point for designing a front panel for
your project. The produced SVG file looks at the panel from the
outside-in, and is thus a mirror image of the markings you see in the
“Panel” layer within CPCB.

.. _fnx1:

Footnote
--------

.. [#f1] A “Gerber file” is actually a whole set of files, packaged
   together in a “zip” archive.  Each layer of the PCB layout is
   represented as a distinct file in the archive. Additional files
   represent the locations of through holes. Confusingly, each of the
   individual files in the archive are also sometimes called “Gerber
   files”.
