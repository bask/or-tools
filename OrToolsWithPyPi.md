# Getting OR-Tools from Pypi #

## Getting Python ##

OR-Tools is compatible with python **2.6** and **2.7**. Compatibility with python 3.3 is underway, but protobuf is not yet compatible.

On linux, python should be installed by default.

On Mac OS X, you need to install XCode Command Line Tools (for Mountain Lion and up).

On Windows, you need to install python from http://www.python.org/download/releases/2.7/ . Please choose the 32 bit or 64 bit version.

## Upgrading setuptools ##

Sometimes, you get weird errors when trying to install ortools. I recomment upgrading setuptools using the instructions found here:

https://pypi.python.org/pypi/setuptools#installation-instructions

## Installing or-tools from the examples archive ##

ortools is available on pypi.
The best way to install it is to download the Google.OrTools.python.examples archive from [here](https://drive.google.com/folderview?id=0B2yUSpEp04BNdEU4QW5US1hvTzg&usp=sharing) ([mirror](https://app.box.com/s/gmgtnfqgl8ibrz5d1cm0)).

Unpack it, go in the ortools\_examples directory and run:

Linux and Mac OS X:
if you have root privileges:
```
sudo python2.7 setup.py install
```

or

```
python2.7 setup.py install --user
```

Windows:
```
c:\python27\python.exe setup.py install
```

This should download the _ortools_ package, as well as its dependencies (protobuf, google apputils).

## Alternate installation methods ##

You can install ortools differently, although this is not recommended.

### Using easy\_install ###

```
easy_install ortools
```

Note that pip is not supported. You must use easy\_install.
Note that (1) easy\_install is not installed by default, and (2) you will not get the ortools examples for testing or browsing.

### Using .msi files on windows ###

Using .msi file will not pull dependencies.
You will need to install protobuf and google-apputils from pypi.

# Running one example #

Once Google.OrTools.python.examples is unpacked, you should see the following directory structure:
```
ortools_examples\
  data\           <- Data directory for the examples.
  examples\       <- All python examples.
  LICENSE-2.0.txt <- Apache 2.0 License file.
  README.txt      <- README file.
  setup.py        <- setup file.
```

you should be able to run any examples in the examples/ directory

Linux and Mac OS X:
```
python2.7 examples/golomb8.py
```

Windows:
```
c:\python27\python.exe examples\golomb8.py
```

# Troubleshooting #

## ImportError: No module named _pywraplp ##_

Most likely, you are using windows XP. Unfortunately, ortools needs visual studio 2012 or up, which is incompatible with Windows XP.

Another common error is to use pip, this tool pulls the wrong archive with most likely a wrong platform. So use easy\_install, or better the installation instructions using the Google.OrTools.python.examples archive.