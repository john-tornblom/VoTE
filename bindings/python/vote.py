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
'''
VoTE (Verifier of Tree Ensembles) is a toolsuite for analyzing input/output
mappings of decision trees and tree ensembles.
'''

from _vote import ffi, lib


__version__ = ffi.string(lib.vote_version())


UNSURE = -1
FAIL = 0
PASS = 1


def argmax(iterable):
    '''
    Returns the index of the largest value in an *iterable* of numbers
    '''
    fvec = [float(el) for el in iterable]
    return lib.vote_argmax(fvec, len(fvec))


def argmin(iterable):
    '''
    Returns the index of the smallest value in an *iterable* of numbers
    '''
    fvec = [float(el) for el in iterable]
    return lib.vote_argmin(fvec, len(fvec))


def mapping_precise(mapping):
    '''
    Check if a *mapping* is precise, i.e. the output is a single point.
    '''
    return lib.vote_mapping_precise(mapping)


def mapping_argmax(mapping):
    '''
    Returns the index of the largest value in a *mapping*
    '''
    return lib.vote_mapping_argmax(mapping)


def mapping_check_argmax(mapping, expected):
    '''
    Check if the argmax of a *mapping* is as *expected*.
    '''
    return lib.vote_mapping_check_argmax(mapping, expected)


def mapping_argmin(mapping):
    '''
    Returns the index of the smallest value in a *mapping*
    '''
    return lib.vote_mapping_argmin(mapping)


def mapping_check_argmin(mapping, expected):
    '''
    Check if the argmin of a *mapping* is as *expected*.
    '''
    return lib.vote_mapping_check_argmin(mapping, expected)


@ffi.def_extern()
def vote_mapping_python_cb(ctx, mapping):
    '''
    Callback hook from libvote forall iterations.
    '''
    callback = ffi.from_handle(ctx)
    return callback(mapping)


class Ensemble(object):
    '''
    An ensemble is a collection of trees that captures statistical properties 
    of a system-of-intrest.
    '''
    
    ptr = None

    def __init__(self, filename):
        self.ptr = lib.vote_ensemble_load(filename)
        assert self.ptr
        
    def __del__(self):
        if self.ptr:
            lib.vote_ensemble_del(self.ptr)

    @property
    def nb_inputs(self):
        '''
        The number of input scalars accepted by this ensemble.
        '''
        return self.ptr.nb_inputs

    @property
    def nb_outputs(self):
        '''
        The number of output scalars emited by this ensemble.
        '''
        return self.ptr.nb_outputs

    def eval(self, *args):
        '''
        Evaluate this ensemble on concrete values.
        '''
        inputs = ffi.new('real_t[%d]' % self.nb_inputs, args)
        outputs = ffi.new('real_t[%d]' % self.nb_outputs)

        lib.vote_ensemble_eval(self.ptr, inputs, outputs)
        return list(outputs)

    def forall(self, callback, domain=None):
        '''
        Enumerate all conclusive mappings of this ensemble for some input *domain*
        until the *callback* function returns FAIL, or all mappings
        have been iterated.

        Returns true if all mappings PASS the callback function, or
        false if any of the mappings FAIL the callback function.
        '''
        ctx = ffi.NULL
        bounds = ffi.new('vote_bound_t[%d]' % self.nb_inputs)
        cb = ffi.callback('vote_mapping_cb_t')
        
        for ind in range(self.nb_inputs):
            bounds[ind].lower = -float('inf')
            bounds[ind].upper = float('inf')

        domain = domain or list()
        for ind, bound in enumerate(domain):
            bounds[ind].lower = bound[0]
            bounds[ind].upper = bound[1]
            
        ctx = ffi.new_handle(callback)
        cb = lib.vote_mapping_python_cb
        return lib.vote_ensemble_forall(self.ptr, bounds, cb, ctx)

    def absref(self, callback, domain=None):
        '''
        Enumerate abstract mappings of this ensemble using an 
        abstraction-refinement approach for some input *domain*.

        Returns true if all conclusive mappings PASS the callback function, or
        false if any of the conclusive mappings FAIL the callback function.
        '''
        ctx = ffi.NULL
        bounds = ffi.new('vote_bound_t[%d]' % self.nb_inputs)
        cb = ffi.callback('vote_mapping_cb_t')
        
        for ind in range(self.nb_inputs):
            bounds[ind].lower = -float('inf')
            bounds[ind].upper = float('inf')

        domain = domain or list()
        for ind, bound in enumerate(domain):
            bounds[ind].lower = bound[0]
            bounds[ind].upper = bound[1]
            
        ctx = ffi.new_handle(callback)
        cb = lib.vote_mapping_python_cb
        return lib.vote_ensemble_absref(self.ptr, bounds, cb, ctx)

    def approximate(self, domain=None):
        '''
        Approximate a pessimistic and sound mapping for a given input *domain*.
        '''
        bounds = ffi.new('vote_bound_t[%d]' % self.nb_inputs)
        
        for ind in range(self.nb_inputs):
            bounds[ind].lower = -float('inf')
            bounds[ind].upper = float('inf')

        domain = domain or list()
        for ind, bound in enumerate(domain):
            bounds[ind].lower = bound[0]
            bounds[ind].upper = bound[1]
            
        return lib.vote_ensemble_approximate(self.ptr, bounds)

