#!/usr/bin/env python
# encoding: utf-8
# Copyright (C) 2018 John Törnblom
#
# This file is part of VoTE (Verifier of Tree Ensembles).
#
# VoTE is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# VoTE is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
# for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with VoTE; see the files COPYING and COPYING.LESSER. If not,
# see <http://www.gnu.org/licenses/>.

import os
from cffi import FFI

ffibuilder = FFI()

path = os.path.dirname(__file__) or '.'
path = os.path.abspath(path)
binary = path + '/../../lib/.libs/libvote.a'
incdir = path + '/../../inc'

with open(incdir + '/vote.h') as f:
    vote_h = f.read()
    ffibuilder.set_source('_vote', vote_h,
                          extra_objects=[binary],
                          libraries=['m'])

vote_h = ''.join([line for line in vote_h.splitlines()
                  if not line.startswith('#')])

ffibuilder.cdef(vote_h + '''
extern "Python" vote_outcome_t _vote_mapping_python_cb(void *, vote_mapping_t*);
extern void free(void *ptr);
''')

if __name__ == "__main__":
    ffibuilder.compile(verbose=True)
    
