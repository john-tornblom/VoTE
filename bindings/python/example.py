#!/usr/bin/env python
# encoding: utf-8
# Copyright (C) 2018 John Törnblom
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
'''
Ensure that the plausibility of range property is satisfied, i.e. that
 output scalars are within a stated boundary.
'''

import sys
import vote


def plausibility_of_range(mapping, alpha=0, beta=1):
    minval = min([mapping.outputs[dim].lower
                  for dim in range(mapping.nb_outputs)])
                   
    maxval = max([mapping.outputs[dim].upper
                  for dim in range(mapping.nb_outputs)])

    if minval >= alpha and maxval <= beta:
        return vote.PASS
    elif vote.mapping_precise(mapping):
        return vote.FAIL
    else:
        return vote.UNSURE


e = vote.Ensemble.from_file(sys.argv[1]) # load model from disk
assert e.forall(plausibility_of_range)


