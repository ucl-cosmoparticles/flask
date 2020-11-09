from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from subprocess import run, PIPE, DEVNULL
import os


class build_flask(build_ext):
    '''Flask extension builder.'''

    user_options = build_ext.user_options + [
        ('healpix-dir=', None, 'Path to HEALPix directory.'),
        ('healpix-include-dir=', None, 'Path to HEALPix include directory.'),
        ('healpix-lib-dir=', None, 'Path to HEALPix lib directory.'),
        ('healpix-data-dir=', None, 'Path to HEALPix data directory.'),
    ]

    def initialize_options(self):
        build_ext.initialize_options(self)
        self.healpix_dir = None
        self.healpix_include_dir = None
        self.healpix_lib_dir = None
        self.healpix_data_dir = None

    def finalize_options(self):
        build_ext.finalize_options(self)

        # initialise #define list
        self.define = self.define or []

        # try to get commit from working directory
        flask_commit = run(['git', 'rev-parse', '--short', 'HEAD'],
                           stdout=PIPE, stderr=DEVNULL, universal_newlines=True
                           ).stdout.strip() or 'missing'

        # add commit definition
        self.define += [
            ('FLASKCOMMIT', f'"{flask_commit}"'),
        ]

        # process healpix_dir if given
        if self.healpix_dir is not None:
            print(f'HEALPix path: {self.healpix_dir}')

            # must be a directory
            if not os.path.isdir(self.healpix_dir):
                raise ValueError(f'not a directory: {self.healpix_dir}')

            # infer include directory if not given explicitly
            if self.healpix_include_dir is None:
                self.healpix_include_dir = os.path.join(self.healpix_dir, 'include', 'healpix_cxx')

            # infer lib directory if not given explicitly
            if self.healpix_lib_dir is None:
                self.healpix_lib_dir = os.path.join(self.healpix_dir, 'lib')

            # infer data directory if not given explicitly
            if self.healpix_data_dir is None:
                self.healpix_data_dir = os.path.join(self.healpix_dir, 'data')

        # process healpix_include_dir if given
        if self.healpix_include_dir is not None:
            print(f'HEALPix include path: {self.healpix_include_dir}')

            if not os.path.isdir(self.healpix_include_dir):
                raise ValueError(f'not a directory: {self.healpix_include_dir}')

            self.include_dirs.append(self.healpix_include_dir)

        # process healpix_lib_dir if given
        if self.healpix_lib_dir is not None:
            print(f'HEALPix lib path: {self.healpix_lib_dir}')

            if not os.path.isdir(self.healpix_lib_dir):
                raise ValueError(f'not a directory: {self.healpix_lib_dir}')

            self.library_dirs.append(self.healpix_lib_dir)

        # healpix_data_dir must be given
        if self.healpix_data_dir is None:
            raise ValueError('missing healpix_data_dir option')

        print(f'HEALPix data path: {self.healpix_data_dir}')

        if not os.path.isdir(self.healpix_data_dir):
            raise ValueError(f'not a directory: {self.healpix_data_dir}')

        # add definitions related to HEALPix
        self.define += [
            ('HEALPIX_DATA', f'"{self.healpix_data_dir}"'),
            ('USEXCOMPLEX', 0),
            ('USEMAP2TGA', 0),
        ]


setup(
    name='pyFlask',
    package_dir={'': 'py'},
    py_modules=['pyFlask'],
    ext_modules=[Extension('_pyFlask',
        language='c++',
        sources=[
            'src/pyFlask_wrap.cxx',
            'src/pyFlask.cpp',
            'src/ParameterList.cpp',
            'src/Utilities.cpp',
            'src/gsl_aux.cpp',
            'src/s2kit10_naive.cpp',
            'src/interpol.cpp',
            'src/flask_aux.cpp',
            'src/Cosmology.cpp',
            'src/Integral.cpp',
            'src/GeneralOutput.cpp',
            'src/SelectionFunc.cpp',
            'src/RegularizeCov.cpp',
            'src/ClProcessing.cpp',
            'src/Maximize.cpp',
            'src/fitsfunctions.cpp',
            'src/lognormal.cpp',
            'src/FieldsDatabase.cpp',
            'src/Spline.cpp',
            'src/ini_config.cpp',
            'src/handle_ini_format.cpp',
        ],
        extra_compile_args=[
            '-fopenmp',
        ],
    )],
    cmdclass={'build_ext': build_flask},
)
