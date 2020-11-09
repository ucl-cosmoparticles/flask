Check the file 'doc/flask_manual.pdf' for all the information about FLASK.
Paper describing the code: Xavier et al. 2016, MNRAS Vol. 459, p. 3693 (arXiv:1602.08503)

## Prerequisites:

FLASK requires the following software to be pre-installed on your machine:
- Linux operating system
- OpenMP (for parallelisation)
- HEALPix (for mapping the sky): Instructions for compiling Flask and pyFlask for healpix 3.60+ can be found bellow.
- GNU Scientific Library (GSL)
- CFITSIO (to work with FITS format files)

## How to compile:
### For Healpix 3.60+
Because pyFlask uses shared libraries at the moment, this requires Healpix and Libsharp to be re-compiled using the flag `-fpic` to create these files.
"Simply" re-run healpix's `./configure` to re-compile both libraries adding the `-fpic` flag to both. In case pyFlask is not necessary, you can avoid re-compiling Healpix and libsharp by making only the C++ part of Flask (see bellow).

### Flask Compilation
Edit the `Makefile` in the `src/` subdirectory according to your machine's requirements. If you are using a healpix version bellow 3.60, please pay attention to the `Makefile`.
In particular, the HEALDIR variable must be set to the location of Healpix installation on your machine.

From Linux command prompt:
```shell
$ cd <Root directory of Flask>/src/
$ make
```
This will create a linux binary that can be run directly from the command prompt.
It will be placed in the subdirectory that is defined through the variable BIN in the Makefile.
This is referred to as "Flask".

### pyFlask Compilation
Edit the [`setup.cfg`](setup.cfg) configuration file according to your machine's
requirements. In particular, the `healpix_dir` variable must be set to the
location of a Healpix installation on your machine. Once everything is set up,
you can install the pyFlask module as usual with pip:

```shell
$ cd <Root directory of Flask>/
$ pip3 install .
```

Alternatively, you can build the pyFlask module in the working directory:

```shell
$ cd <Root directory of Flask>/
$ python3 setup.py build_ext --inplace
```

This will build the Python module, which is placed in the `py` subdirectory.


### How to run the standard example:

### To run Flask
Suppose:
```makefile
BIN=<Root directory of Flask>/bin
```
Then:
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
>>> import pyFlask
>>> pyFlask.flask(["flask","example.config"])
```
If you want to define, for example, the input parameter RNDSEED=402 from within Python environment, then pyFlask should be invoked from Python as:
```python
>>> pyFlask.flask(["flask","example.ini","RNDSEED:","402"])
```
If pyFlask was build in place, then you need to invoke Python in the `py`
subdirectory in order for `import pyFlask` to find the module.
