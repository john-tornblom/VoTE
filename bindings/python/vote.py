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


try: import numpy as np
except: pass


def argmax(iterable):
    '''
    Returns the index of the largest value in an *iterable* of numbers.
    '''
    fvec = [float(el) for el in iterable]
    return _lib.vote_argmax(fvec, len(fvec))


def argmin(iterable):
    '''
    Returns the index of the smallest value in an *iterable* of numbers.
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
    Returns the index of the largest output value in a *mapping*.
    '''
    return _lib.vote_mapping_argmax(mapping)


def mapping_check_argmax(mapping, expected):
    '''
    Check if the argmax of a *mapping* is as *expected*.
    '''
    return _lib.vote_mapping_check_argmax(mapping, expected)


def mapping_argmin(mapping):
    '''
    Returns the index of the smallest output value in a *mapping*.
    '''
    return _lib.vote_mapping_argmin(mapping)


def mapping_check_argmin(mapping, expected):
    '''
    Check if the argmin of a *mapping* is as *expected*.
    '''
    return _lib.vote_mapping_check_argmin(mapping, expected)


def mapping_copy(mapping):
    '''
    Create a (deep) copy of a mapping.
    '''
    mapping = _lib.vote_mapping_copy(mapping)
    return _ffi.gc(mapping, _lib.vote_mapping_del)


@_ffi.def_extern()
def _vote_mapping_python_cb(ctx, mapping):
    '''
    Callback hook from VoTE Core forall/absref iterations.
    '''
    callback = _ffi.from_handle(ctx)
    return callback(mapping)


def _mk_bounds(dims, limits):
    '''
    Create an array of bounds with length *dims*, and initialize the bounds with
    *limits*, i.e. a list of pairs with the lower and upper limit
    in each dimension, e.g. [(0, 1), (0, 1)].
    '''
    limits = limits or list()
    bounds = _ffi.new('vote_bound_t[%d]' % dims)
    for ind in range(dims):
        bounds[ind].lower = -float('inf')
        bounds[ind].upper = float('inf')

    for ind, limit in enumerate(limits):
        bounds[ind].lower = limit[0]
        bounds[ind].upper = limit[1]

    return bounds


def _sklearn_dt_to_dict(tree):
    '''
    Convert a sklearn decision tree into a dictionary.
    '''
    
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


def _sklearn_rf_to_dict(inst):
    '''
    Convert a sklearn random forest into a dictionary.
    '''
    return dict(trees=[_sklearn_dt_to_dict(tree)
                       for tree in inst.estimators_],
                post_process='divisor')


class _NumPyJSONEncoder(json.JSONEncoder):
    def default(self, obj):
        ty = type(obj)
        
        if issubclass(ty, np.ndarray):
            return obj.tolist()
        elif issubclass(ty, np.float):
            return float(obj)
        elif issubclass(ty, np.integer):
            return int(obj)
        else:
            return json.JSONEncoder.default(self, obj)


class Ensemble:
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

    @classmethod
    def from_file(cls, filename):
        '''
        Load a VoTE ensemble from disk persisted in a JSON-based format.
        '''
        ptr = _lib.vote_ensemble_load_file(filename.encode('utf8'))
        return cls(ptr)
    
    @classmethod
    def from_string(cls, string):
        '''
        Load a VoTE ensemble from a a JSON-based formated *string*.
        '''
        ptr = _lib.vote_ensemble_load_string(string.encode('utf8'))
        return cls(ptr)

    @classmethod
    def from_sklearn(cls, instance):
        '''
        Convert an sklearn model *instance* into a VoTE ensemble.
        '''
        conv = {
            'RandomForestClassifier': _sklearn_rf_to_dict,
            'RandomForestRegressor': _sklearn_rf_to_dict,
        }
        name = type(instance).__name__
        if name not in conv:
            raise NotImplementedError
        
        d = conv[name](instance)
        return cls.from_string(json.dumps(d, cls=_NumPyJSONEncoder))

    @classmethod
    def from_xgboost(cls, booster):
        '''
        Convert an xgboost *booster* into a VoTE ensemble.
        '''
        if hasattr(booster, 'get_booster'):
            booster = booster.get_booster()

        buf = _ffi.from_buffer(booster.save_raw())
        ptr = _lib.vote_xgboost_load_blob(buf, len(buf))
        
        return cls(ptr)
    
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

    @property
    def nb_trees(self):
        '''
        The number of trees in this ensemble.
        '''
        return self.ptr.nb_trees

    @property
    def nb_nodes(self):
        '''
        The number of nodes in this ensemble.
        '''
        return self.ptr.nb_nodes

    @property
    def post_processing_algorithm(self):
        '''
        The post processing algorithm applied.
        '''
        tbl = ('none', 'divisor', 'softmax', 'sigmoid')
        return tbl[self.ptr.post_process]
    
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
        bounds = _mk_bounds(self.nb_inputs, domain)
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
        bounds = _mk_bounds(self.nb_inputs, domain)
        ctx = _ffi.new_handle(callback)
        cb = _lib._vote_mapping_python_cb
        return _lib.vote_ensemble_absref(self.ptr, bounds, cb, ctx)

    def approximate(self, domain=None):
        '''
        Approximate a pessimistic and sound mapping for a given input *domain*.
        '''
        bounds = _mk_bounds(self.nb_inputs, domain)
        ptr = _lib.vote_ensemble_approximate(self.ptr, bounds)
        return _ffi.gc(ptr, _lib.vote_mapping_del)

    def serialize(self):
        '''
        Serialize the ensemble into a JSON-formatted string.
        '''
        ptr = _lib.vote_ensemble_save_string(self.ptr)
        s = _ffi.string(ptr)
        _lib.free(ptr)
        
        return s.decode('utf-8')
    
