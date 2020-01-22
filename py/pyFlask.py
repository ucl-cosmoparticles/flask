# SWIG Version 4.0.1 (http://www.swig.org).
#
# Do not make changes to this file unless you know what you are doing!!!

from sys import version_info as _swig_python_version_info
if _swig_python_version_info < (2, 7, 0):
    raise RuntimeError("Python 2.7 or later required")

# Import the low-level C/C++ module
if __package__ or "." in __name__:
    from . import _pyFlask
else:
    import _pyFlask

try:
    import builtins as __builtin__
except ImportError:
    import __builtin__

def _swig_repr(self):
    try:
        strthis = "proxy of " + self.this.__repr__()
    except __builtin__.Exception:
        strthis = ""
    return "<%s.%s; %s >" % (self.__class__.__module__, self.__class__.__name__, strthis,)


def _swig_setattr_nondynamic_instance_variable(set):
    def set_instance_attr(self, name, value):
        if name == "thisown":
            self.this.own(value)
        elif name == "this":
            set(self, name, value)
        elif hasattr(self, name) and isinstance(getattr(type(self), name), property):
            set(self, name, value)
        else:
            raise AttributeError("You cannot add instance attributes to %s" % self)
    return set_instance_attr


def _swig_setattr_nondynamic_class_variable(set):
    def set_class_attr(cls, name, value):
        if hasattr(cls, name) and not isinstance(getattr(cls, name), property):
            set(cls, name, value)
        else:
            raise AttributeError("You cannot add class attributes to %s" % cls)
    return set_class_attr


def _swig_add_metaclass(metaclass):
    """Class decorator for adding a metaclass to a SWIG wrapped class - a slimmed down version of six.add_metaclass"""
    def wrapper(cls):
        return metaclass(cls.__name__, cls.__bases__, cls.__dict__.copy())
    return wrapper


class _SwigNonDynamicMeta(type):
    """Meta class to enforce nondynamic attributes (no new attributes) for a class"""
    __setattr__ = _swig_setattr_nondynamic_class_variable(type.__setattr__)



def cdata(ptr, nelements=1):
    return _pyFlask.cdata(ptr, nelements)

def memmove(data, indata):
    return _pyFlask.memmove(data, indata)

def cos(x):
    return _pyFlask.cos(x)

def sin(x):
    return _pyFlask.sin(x)

def tan(x):
    return _pyFlask.tan(x)

def acos(x):
    return _pyFlask.acos(x)

def asin(x):
    return _pyFlask.asin(x)

def atan(x):
    return _pyFlask.atan(x)

def atan2(y, x):
    return _pyFlask.atan2(y, x)

def cosh(x):
    return _pyFlask.cosh(x)

def sinh(x):
    return _pyFlask.sinh(x)

def tanh(x):
    return _pyFlask.tanh(x)

def exp(x):
    return _pyFlask.exp(x)

def log(x):
    return _pyFlask.log(x)

def log10(x):
    return _pyFlask.log10(x)

def pow(x, y):
    return _pyFlask.pow(x, y)

def sqrt(x):
    return _pyFlask.sqrt(x)

def fabs(x):
    return _pyFlask.fabs(x)

def ceil(x):
    return _pyFlask.ceil(x)

def floor(x):
    return _pyFlask.floor(x)

def fmod(x, y):
    return _pyFlask.fmod(x, y)
M_E = _pyFlask.M_E
M_LOG2E = _pyFlask.M_LOG2E
M_LOG10E = _pyFlask.M_LOG10E
M_LN2 = _pyFlask.M_LN2
M_LN10 = _pyFlask.M_LN10
M_PI = _pyFlask.M_PI
M_PI_2 = _pyFlask.M_PI_2
M_PI_4 = _pyFlask.M_PI_4
M_1_PI = _pyFlask.M_1_PI
M_2_PI = _pyFlask.M_2_PI
M_2_SQRTPI = _pyFlask.M_2_SQRTPI
M_SQRT2 = _pyFlask.M_SQRT2
M_SQRT1_2 = _pyFlask.M_SQRT1_2
class SwigPyIterator(object):
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc="The membership flag")

    def __init__(self, *args, **kwargs):
        raise AttributeError("No constructor defined - class is abstract")
    __repr__ = _swig_repr
    __swig_destroy__ = _pyFlask.delete_SwigPyIterator

    def value(self):
        return _pyFlask.SwigPyIterator_value(self)

    def incr(self, n=1):
        return _pyFlask.SwigPyIterator_incr(self, n)

    def decr(self, n=1):
        return _pyFlask.SwigPyIterator_decr(self, n)

    def distance(self, x):
        return _pyFlask.SwigPyIterator_distance(self, x)

    def equal(self, x):
        return _pyFlask.SwigPyIterator_equal(self, x)

    def copy(self):
        return _pyFlask.SwigPyIterator_copy(self)

    def next(self):
        return _pyFlask.SwigPyIterator_next(self)

    def __next__(self):
        return _pyFlask.SwigPyIterator___next__(self)

    def previous(self):
        return _pyFlask.SwigPyIterator_previous(self)

    def advance(self, n):
        return _pyFlask.SwigPyIterator_advance(self, n)

    def __eq__(self, x):
        return _pyFlask.SwigPyIterator___eq__(self, x)

    def __ne__(self, x):
        return _pyFlask.SwigPyIterator___ne__(self, x)

    def __iadd__(self, n):
        return _pyFlask.SwigPyIterator___iadd__(self, n)

    def __isub__(self, n):
        return _pyFlask.SwigPyIterator___isub__(self, n)

    def __add__(self, n):
        return _pyFlask.SwigPyIterator___add__(self, n)

    def __sub__(self, *args):
        return _pyFlask.SwigPyIterator___sub__(self, *args)
    def __iter__(self):
        return self

# Register SwigPyIterator in _pyFlask:
_pyFlask.SwigPyIterator_swigregister(SwigPyIterator)

class imaxdiv_t(object):
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc="The membership flag")
    __repr__ = _swig_repr
    quot = property(_pyFlask.imaxdiv_t_quot_get, _pyFlask.imaxdiv_t_quot_set)
    rem = property(_pyFlask.imaxdiv_t_rem_get, _pyFlask.imaxdiv_t_rem_set)

    def __init__(self):
        _pyFlask.imaxdiv_t_swiginit(self, _pyFlask.new_imaxdiv_t())
    __swig_destroy__ = _pyFlask.delete_imaxdiv_t

# Register imaxdiv_t in _pyFlask:
_pyFlask.imaxdiv_t_swigregister(imaxdiv_t)


def imaxabs(n):
    return _pyFlask.imaxabs(n)

def imaxdiv(numer, denom):
    return _pyFlask.imaxdiv(numer, denom)

def wcstoimax(nptr, endptr, base):
    return _pyFlask.wcstoimax(nptr, endptr, base)

def wcstoumax(nptr, endptr, base):
    return _pyFlask.wcstoumax(nptr, endptr, base)

def strtoimax(nptr, endptr, base):
    return _pyFlask.strtoimax(nptr, endptr, base)

def strtoumax(nptr, endptr, base):
    return _pyFlask.strtoumax(nptr, endptr, base)

def flask(argc):
    return _pyFlask.flask(argc)


