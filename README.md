# comclash
Complex Clash - reverse engineering tool for Watcom 9.5C LE executables

Source tree:
============

  * doc/        --> Documentation
  * builder/    --> the builder source code
  * patch/      --> THIS IS WHERE YOU SHOULD EDIT!!!! these files will be used to rebuild de sources
  * src/        --> Here go the RE-generated sources. Don't edit these files, because ALL CHANGES HERE WILL BE LOST ON NEXT REGENERATION

### Step by step:

  1. Clone this repository to a local directory (e.g. /home/user/prj). Now you you'll have the directory /home/user/prj/camclash.  Chdir into it.

  2. Edit file builder/config.lua with your favorite editor.

  3. Change the line originalexe=/home/user/dos/games/cc/CCLASH.EXE to the name of your executable.

  4. Change the line buildpath=/home/user/dos/games/cc to the name of the diretory where to put the generated EXEs.

  5. Save the file and quit the editor.

  6. Type ''make builder'' to compile the builder.
   
  7. If all went OK, it's time to compile and run your generatede EXE. Type ''make setup-original'' to prepare compilation for an original EXE.

  8. Type ''make run'' to compile and test the generated EXE in DOSBOX.

  9. That's it.

### Building the re-generated EXE (default make target):

    $ make exe

### Setup for generating an unmodified EXE:

    $ make setup-original

### Setup for generating an MLOOK patched EXE:

    $ make setup-mlook

### Testing (running) the re-generated EXE under DOSBOX:

    $ make run

### Re-generating the source code for the EXE:

    $ make gen

### Cleaning up the source code for the EXE:

    $ make gclean

### Building the builder:

    $ make builder

### Cleaning up the source code for the builder:

    $ make bclean

### Comparing the unmodded generated EXE with the original CCLASH.EXE:

    $ make compare

