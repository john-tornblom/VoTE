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
import json

from _vote import ffi as _ffi
from _vote import lib as _lib


__version__ = _ffi.string(_lib.vote_version())

UNSURE = -1
FAIL = 0
PASS = 1


def argmax(iterable):
    '''
    Returns the index of the largest value in an *iterable* of numbers
    '''
    fvec = [float(el) for el in iterable]
    return _lib.vote_argmax(fvec, len(fvec))


def argmin(iterable):
    '''
    Returns the index of the smallest value in an *iterable* of numbers
    '''
    fvec = [float(el) for el in iterable]
    return _lib.vote_argmin(fvec, len(fvec))


def mapping_precise(mapping):
    '''
    Check if a *mapping* is precise, i.e. the output is a single point.
    '''
    return _lib.vote_mapping_precise(mapping)


def mapping_argmax(mapping):
    '''
    Returns the index of the largest value in a *mapping*
    '''
    return _lib.vote_mapping_argmax(mapping)


def mapping_check_argmax(mapping, expected):
    '''
    Check if the argmax of a *mapping* is as *expected*.
    '''
    return _lib.vote_mapping_check_argmax(mapping, expected)


def mapping_argmin(mapping):
    '''
    Returns the index of the smallest value in a *mapping*
    '''
    return _lib.vote_mapping_argmin(mapping)


def mapping_check_argmin(mapping, expected):
    '''
    Check if the argmin of a *mapping* is as *expected*.
    '''
    return _lib.vote_mapping_check_argmin(mapping, expected)


@_ffi.def_extern()
def _vote_mapping_python_cb(ctx, mapping):
    '''
    Callback hook from VoTE Core forall/absref iterations.
    '''
    callback = _ffi.from_handle(ctx)
    return callback(mapping)


def _sklearn_decision_tree_to_dict(tree):
    '''
    Convert a sklearn decision tree into a dictionary.
    '''
    import numpy as np
    
    def normalize(matrix):
        for row in matrix:
            row /= np.sum(row) or 1

        return matrix

    if tree._estimator_type == 'classifier':
        nb_outputs = tree.n_classes_
        value = normalize(np.squeeze(tree.tree_.value))
    else:
        nb_outputs = tree.n_outputs_
        value = np.squeeze(tree.tree_.value)
        if len(value.shape) == 1:
            value = value.reshape((len(value), 1))

    return dict(nb_inputs=tree.n_features_,
                nb_outputs=nb_outputs,
                left=tree.tree_.children_left,
                right=tree.tree_.children_right,
                feature=tree.tree_.feature,
                threshold=tree.tree_.threshold,
                value=value)


def _sklearn_random_forest_to_dict(inst):
    '''
    Convert a sklearn random forest into a dictionary.
    '''
    return dict(trees=[_sklearn_decision_tree_to_dict(tree)
                       for tree in inst.estimators_],
                post_process='divisor')


class _NumPyJSONEncoder(json.JSONEncoder):
    def default(self, obj):
        import numpy as np
        ty = type(obj)
        
        if issubclass(ty, np.ndarray):
            return obj.tolist()
        elif issubclass(ty, np.float):
            return float(obj)
        elif issubclass(ty, np.integer):
            return int(obj)
        else:
            return json.JSONEncoder.default(self, obj)


class Ensemble(object):
    '''
    An ensemble is a collection of trees that captures statistical properties 
    of a system-of-intrest.
    '''
    
    ptr = None

    def __init__(self, ptr):
        '''
        Initialize a Python object that wraps arround a pointer to a VoTE Core
        vote_ensemble_t data structure.
        '''
        self.ptr = ptr
        assert self.ptr
        
    def __del__(self):
        '''
        Free the memory pointed to by *self.ptr*.
        '''
        if self.ptr:
            _lib.vote_ensemble_del(self.ptr)

    @staticmethod
    def from_file(filename):
        '''
        Load a VoTE ensemble from disk persisted in a JSON-based format.
        '''
        ptr = _lib.vote_ensemble_load_file(filename.encode('utf8'))
        return Ensemble(ptr)
    
    @staticmethod
    def from_string(string):
        '''
        Load a VoTE ensemble from a a JSON-based formated *string*.
        '''
        ptr = _lib.vote_ensemble_load_string(string.encode('utf8'))
        return Ensemble(ptr)

    @staticmethod
    def from_sklearn(instance):
        '''
        Convert an sklearn model *instance* into a VoTE ensemble.
        '''
        conv = {
            'RandomForestClassifier': _sklearn_random_forest_to_dict,
            'RandomForestRegressor': _sklearn_random_forest_to_dict,
        }
        name = type(instance).__name__
        if name not in conv:
            raise NotImplementedError
        
        d = conv[name](instance)
        return Ensemble.from_string(json.dumps(d, cls=_NumPyJSONEncoder))
    
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
        Evaluate this ensemble on a concrete sample.
        '''
        inputs = _ffi.new('real_t[%d]' % self.nb_inputs, args)
        outputs = _ffi.new('real_t[%d]' % self.nb_outputs)

        _lib.vote_ensemble_eval(self.ptr, inputs, outputs)
        return list(outputs)

    def forall(self, callback, domain=None):
        '''
        Enumerate all precise mappings of this ensemble for some input *domain*
        until the *callback* function returns FAIL, or all mappings
        have been enumerated.

        Returns true if all mappings PASS the callback function, or
        false if any of the mappings FAIL the callback function.
        '''
        ctx = _ffi.NULL
        bounds = _ffi.new('vote_bound_t[%d]' % self.nb_inputs)
        cb = _ffi.callback('vote_mapping_cb_t')
        
        for ind in range(self.nb_inputs):
            bounds[ind].lower = -float('inf')
            bounds[ind].upper = float('inf')

        domain = domain or list()
        for ind, bound in enumerate(domain):
            bounds[ind].lower = bound[0]
            bounds[ind].upper = bound[1]
            
        ctx = _ffi.new_handle(callback)
        cb = _lib._vote_mapping_python_cb
        return _lib.vote_ensemble_forall(self.ptr, bounds, cb, ctx)

    def absref(self, callback, domain=None):
        '''
        Enumerate abstract mappings of this ensemble using an 
        abstraction-refinement approach for some input *domain*.

        Returns true if all conclusive mappings PASS the callback function, or
        false if any of the conclusive mappings FAIL the callback function.
        '''
        ctx = _ffi.NULL
        bounds = _ffi.new('vote_bound_t[%d]' % self.nb_inputs)
        cb = _ffi.callback('vote_mapping_cb_t')
        
        for ind in range(self.nb_inputs):
            bounds[ind].lower = -float('inf')
            bounds[ind].upper = float('inf')

        domain = domain or list()
        for ind, bound in enumerate(domain):
            bounds[ind].lower = bound[0]
            bounds[ind].upper = bound[1]
            
        ctx = _ffi.new_handle(callback)
        cb = _lib._vote_mapping_python_cb
        return _lib.vote_ensemble_absref(self.ptr, bounds, cb, ctx)

    def approximate(self, domain=None):
        '''
        Approximate a pessimistic and sound mapping for a given input *domain*.
        '''
        bounds = _ffi.new('vote_bound_t[%d]' % self.nb_inputs)
        
        for ind in range(self.nb_inputs):
            bounds[ind].lower = -float('inf')
            bounds[ind].upper = float('inf')

        domain = domain or list()
        for ind, bound in enumerate(domain):
            bounds[ind].lower = bound[0]
            bounds[ind].upper = bound[1]
            
        return _lib.vote_ensemble_approximate(self.ptr, bounds)

