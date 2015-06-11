This file describes how to install the or-tools C++ binary archive.

The archives can be found [here](https://drive.google.com/folderview?id=0B2yUSpEp04BNdEU4QW5US1hvTzg&usp=sharing) ([mirror](https://app.box.com/s/gmgtnfqgl8ibrz5d1cm0)).

OR-Tools is located at http://code.google.com/p/or-tools

These modules have been tested under:
  * ubuntu 12.04 and up (32 and 64 bit).
  * Mac OS X Lion and up with xcode 5.x (64 bit, Mac OS X 10.8 and 10.9).
  * Microsoft Visual studio 2012 and 2013 (32 and 64 bit).

Upon decompressing the archive, you will get the following structure:

```
or-tools/
  LICENSE-2.0.txt    <- Apache License
  README             <- This file
  examples/          <- C++ examples
  lib/               <- directory containing the ortools library
  include/           <- all include files
  Makefile/          <- Main makefile
```

To compile an example, you must add `<or-tools>/include` to the list of include directories, and link with the `ortools` library in `<or-tools>/lib` (`ortools.lib` on windows, `libortools.so` on unix, `libortools.dylib` on mac os X).

Some examples come with compilation directives in the Makefile.