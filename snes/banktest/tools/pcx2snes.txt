+----------------+
| pcx2snes (win) |
+----------------+
 v1.1
 by eKid

to make the snes life a little easier

how it's used...

1. Run the program.
2. Click Import .PCX to load a pcx file.
3. Image should display in top-left corner.
4. (Optional) Swap around the colors in the palette by left clicking to select, and right clicking to swap.
5. Select Bit-Depth for export and set tile size (bottom left corner).
6. Click Export .inc and choose a filename to export to.
7. Your Done!

------------------------

Command Line mode

Usage:
  pcx2snes in.pcx [-oOut.inc] [-bBits] [-nName] [-d (16x16)] [-sColor]

  -parameters in brackets are all optional
  -there must be no space between the prefix and value

Parameter Description:

  -o		Specifies file to output to, if not supplied it will use the input file with a ".inc" extension.
  -b		Specifies bit depth, can be 2/4/8 for 4/16/256 colors, if not supplied it will use the bit depth of the PCX.
  -n		Specifies name to use as a prefix of the exported array, default is "Untitled".
  -s		Specifies an index to swap color 0 with, this is optional (mainly for transparency).
  -d		Tells pcx2snes to arrange for 16x16 mode.

------------------------

Questions/Comments/BUGS can be sent to mukunda51@hotmail.com, or you can find me on EFNet IRC - #snesdev as eKid!
