Check the file 'doc/flask_manual.pdf' for all the information about FLASK.
Paper describing the code: Xavier et al. 2016, MNRAS Vol. 459, p. 3693 (arXiv:1602.08503)

## Prerequisites:

FLASK requires the following software to be pre-installed on your machine:
- Linux operating system
- OpenMP (for parallelisation)
- HEALPix (for mapping the sky)
- GNU Scientific Library (GSL)
- CFITSIO (to work with FITS format files)

## How to compile:

Edit the `Makefile` in the `src/` subdirectory according to your machine's requirements. If you are using a healpix version bellow 3.60, please pay attention to the `Makefile`.
In particular, the HEALDIR variable must be set to the location of Healpix installation on your machine.
The variable PYWRAPLIB1 must be set to the location of the C++ header files relevant to Python on your machine.


From Linux command prompt:
```shell
$ cd <Root directory of Flask>/src/
$ make
```
This will create...
(a)
A linux binary that can be run directly from the command prompt. 
It will be placed in the subdirectory that is defined through the variable BIN in the Makefile.
This is referred to as "Flask".
(b) 
A Python shared object, so that FLASK can be invoked from within Python as a Python module. 
It will be placed in the subdirectory that is defined through the variable PYF in the Makefile. 
This is referred to as "pyFlask".

If you don't wish to install the python caller or are having trouble with it, simply run `make cxx` to make just the C++ code.

### How to run the standard example:

Suppose:
```makefile
BIN=<Root directory of Flask>/bin
PYF=<Root directory of Flask>/py
```
### To run Flask
```shell
$ cd <Root directory of Flask>/
$ bin/flask example.config
```
If you want to define, for example, the input parameter RNDSEED=402 at the command prompt, then Flask should be run from the command prompt as:
```shell
$ bin/flask example.config RNDSEED: 402
```
### To call pyFlask
```python
$ cd <Root directory of Flask>/
$ python3
>>> from py import pyFlask
>>> pyFlask.flask(["flask","example.config"])
```
If you want to define, for example, the input parameter RNDSEED=402 from within Python environment, then pyFlask should be invoked from Python as:
```python
>>> pyFlask.flask(["flask","example.ini","RNDSEED:","402"])
```
