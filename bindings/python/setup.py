#!/usr/bin/env python
# encoding: utf-8
# Copyright (C) 2019 John Törnblom
#
# This file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING. If not see
# <http://www.gnu.org/licenses/>.

import os
import sys
import unittest

try:
    from setuptools import setup
    from setuptools import Command
except ImportError:
    from distutils.core import setup
    from distutils.core  import Command

import build


class TestCommand(Command):
    description = "Execute unit tests"
    user_options = []

    def initialize_options(self): pass
    def finalize_options(self): pass

    def run(self):
        dirname = os.path.dirname(__file__) or '.'
        suite = unittest.TestLoader().discover(dirname)
        runner = unittest.TextTestRunner(verbosity=2, buffer=True)
        exit_code = not runner.run(suite).wasSuccessful()
        sys.exit(exit_code)

setup(
    name='vote',
    version='0.1',
    description='Toolsuite for analyzing input/output mappings of decision trees and tree ensembles',
    author='John Törnblom',
    author_email='john.tornblom@liu.se',
    url='https://github.com/john-tornblom/VoTE',
    license='LGPLv3+',
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'Intended Audience :: Science/Research',
        'Topic :: Software Development',
        'Topic :: Scientific/Engineering',
        'License :: OSI Approved :: GNU Lesser General Public License v3 or later (LGPLv3+)',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3.6'],
    py_modules=['vote'],
    ext_modules=[build.ffibuilder.distutils_extension()],
    cmdclass={'test': TestCommand}
)
