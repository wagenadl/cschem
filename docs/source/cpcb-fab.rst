Exporting for fabrication
=========================

The purpose of a PCB layout program is to allow you to make PCBs. CPCB
can therefore export your design to the industry-standard “Gerber”
file format [#f1]_.
To export a Gerber file, press “Control”+“E” or choose “Export
Gerber” from the “File” menu.

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
   fabrication**, even if CPCB exported patently incorrect Gerber
   files.

Exporting solder paste masks
----------------------------

If your layout involves any surface-mount components, you may wish to
produce a solder paste mask. To export a paste mask,
press “Control”+“Shift”+“E” or choose “Export paste mask” from
the File menu. After specifying a file name, you will be asked to
specify the desired “shrinkage for cutouts.” This number can be used
to make the holes slightly smaller than the solderable area, to
compensate for the kerf of a laser cutter.

.. _fnx:

Footnote
--------

.. [#f1] It may be worth emphasizing that a “Gerber file” is actually
   a whole set of files, packaged together in a “zip”
   archive.  Each layer of the PCB layout is represented as a distinct
   file in the archive. Additional files represent the locations of
   through holes.
