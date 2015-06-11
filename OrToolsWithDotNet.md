# Introduction #

This file describes how to use the or-tools .NET binary archive.

OR-Tools is located at http://code.google.com/p/or-tools

The binary archives can be found [here](https://drive.google.com/folderview?id=0B2yUSpEp04BNdEU4QW5US1hvTzg&usp=sharing) ([mirror](https://app.box.com/s/gmgtnfqgl8ibrz5d1cm0)).

This module has been tested under:
  * ubuntu 10.04 and up (32 and 64 bit).
  * Mac OS X Lion with xcode 5.x (64 bit).
  * Microsoft Windows with Visual Studio 2012 and 2013 (32 and 64 bit).

Upon decompressing the archive, you will get the following structure:

```
or-tools/
  LICENSE-2.0.txt  <- Apache License
  README           <- This file
  data/            <- Data for the examples
  examples/        <- C# examples
  bin/             <- Directory containing assemblies and native libraries
```

# Instructions #

Running the examples will involve compiling them, then running them.

Let's compile examples/csflow.cs and run it.

on windows 32 bit:
```
   csc /target:exe /out:csflow.exe /platform:x86 /lib:bin /r:Google.OrTools.dll examples\csflow.cs
   csflow.exe
```

on windows 64 bit:
```
   csc /target:exe /out:csflow.exe /platform:x64 /lib:bin /r:Google.OrTools.dll examples\csflow.cs
   csflow.exe
```

On linux, and Mac OS X, you need on recent version of mono (3.2.x at least)
```
   mcs /target:exe /out:csflow.exe /platform:anycpu /lib:bin /r:Google.OrTools.dll examples/csflow.cs
   MONO_PATH=bin mono csflow.exe
```