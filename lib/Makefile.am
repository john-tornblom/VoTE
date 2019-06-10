#   Copyright (C) 2019 John Törnblom
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

lib_LTLIBRARIES = libvote.la

libvote_la_CFLAGS = -I../inc -I../ext -std=c99 -D_DEFAULT_SOURCE -Wextra \
                    -Werror -pedantic -Wdouble-promotion -Wfloat-conversion \
                    -fPIC
libvote_la_SOURCES = vote_mapping.c \
                     vote_tree.c \
                     vote_ensemble.c \
                     vote_pipeline.c \
                     vote_refinary.c \
                     vote_abstract.c \
                     vote_postproc.c \
                     vote_csv.c \
                     vote_utils.c

libvote_la_LIBADD = ../ext/libparson.la -lm
libvote_la_LDFLAGS = -version-info 1:0:0

