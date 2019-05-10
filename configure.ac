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
AC_INIT(libvote, 0.1.0, john.tornblom@liu.se)
AC_COPYRIGHT([GNU Lesser General Public License version 3 or later])
AC_CONFIG_SRCDIR([configure.ac])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_RANLIB

# Checks for libraries.
PKG_CHECK_MODULES([LIBJSON], [json-c])
AC_SUBST([LIBJSON_CFLAGS])
AC_SUBST([LIBJSON_LIBS])

# Checks for header files.
AC_CHECK_HEADERS([stddef.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([strstr])
AC_FUNC_MALLOC

AM_INIT_AUTOMAKE([foreign])
AM_PROG_CC_STDC
AM_PROG_LIBTOOL
AC_PATH_PROG(PYTHON, python)

LT_INIT

AC_OUTPUT(Makefile lib/Makefile inc/Makefile src/Makefile examples/Makefile
          bindings/python/Makefile)
