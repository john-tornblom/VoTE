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
# along with this program; see the file COPYING. If not, see
# <http://www.gnu.org/licenses/>.

bin_PROGRAMS = vote_accuracy \
               vote_cardinality \
               vote_mappings \
               vote_throughput \
               vote_iospace \
               vote_robustness \
               vote_range

vote_accuracy_SOURCES = accuracy.c
vote_accuracy_CFLAGS = -std=c99 -I../inc
vote_accuracy_LDADD = ../lib/libvote.la -lm

vote_cardinality_SOURCES = cardinality.c
vote_cardinality_CFLAGS = -std=c99 -I../inc
vote_cardinality_LDADD = ../lib/libvote.la -lm

vote_mappings_SOURCES = mappings.c
vote_mappings_CFLAGS = -std=c99 -I../inc
vote_mappings_LDADD = ../lib/libvote.la -lm

vote_throughput_SOURCES = throughput.c
vote_throughput_CFLAGS = -std=c99 -I../inc
vote_throughput_LDADD = ../lib/libvote.la -lm

vote_iospace_SOURCES = iospace.c
vote_iospace_CFLAGS = -std=c99 -I../inc
vote_iospace_LDADD = ../lib/libvote.la -lm

vote_robustness_SOURCES = robustness.c
vote_robustness_CFLAGS = -std=c99 -I../inc
vote_robustness_LDADD = ../lib/libvote.la -lm

vote_range_SOURCES = range.c
vote_range_CFLAGS = -std=c99 -I../inc
vote_range_LDADD = ../lib/libvote.la -lm
