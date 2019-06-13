#   Copyright (C) 2018 John Törnblom
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

AC_PREREQ([2.69])
AC_INIT(libvote, 0.2.1, john.tornblom@liu.se)
AC_COPYRIGHT([GNU Lesser General Public License version 3 or later])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIRS([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB

# Checks for header files.
AC_CHECK_HEADERS([stddef.h math.h])
AC_HEADER_ASSERT
AC_HEADER_STDC
AC_HEADER_STDBOOL

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([strstr])
AC_FUNC_MALLOC
AC_FUNC_MEMCMP

AM_INIT_AUTOMAKE([foreign])
AM_PROG_CC_STDC
AM_PROG_LIBTOOL

# Checks for optional python bindings deps
AM_PATH_PYTHON([2.7],, [:])
AX_PYTHON_MODULE([cffi])
AM_CONDITIONAL([HAVE_PYMOD_CFFI], [test "$HAVE_PYMOD_CFFI" = yes])

LT_INIT

AC_OUTPUT(Makefile ext/Makefile lib/Makefile inc/Makefile src/Makefile
          examples/Makefile bindings/python/Makefile)
