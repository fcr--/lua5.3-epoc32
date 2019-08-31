```
############################################
## WARNING:                               ##
##  My Psion5 broke, and that's why this  ##
##  project will not be supported by me.  ##
############################################
```

Original documentation:
```
This is Lua 5.3.4, released on 12 Jan 2017.

For installation instructions, license details, and 
further information about Lua, see doc/readme.html.
```

This is a port of Lua 5.3.4 for EPOC32, tested on PSION Series 5.

Since this PDA doesn't have a Floating Point Unit, you will probably be able to enjoy significant performance improvements over older versions.

It was based on the [Lua 5.1 port](http://www.freepoc.org/viewapp.php?id=32) by Reuben Thomas (@rrthomas), but several new changes were made:
* It now supports DLL modules (being way faster than their OPL counterparts).
* The building scripts for `Lua53.DLL` now ensure backwards compatibility by keeping track of entry points ordinals in a DEF file. Once an EXPORT_C symbol is added, its index in the DEF file (and thus in the DLL as well) will not be changed after recompilations; essential feature for DLL redistribution.

To build this piece of software you'll need the tricky to install http://epocemx.sourceforge.net/ SDK.
