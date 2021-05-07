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
Simple Unit tests for VoTE.
'''

import functools
import json
import os
import tempfile
import unittest

import numpy as np

import vote


def forall(testcase_cb):
    '''
    forall decorator for test cases
    '''    
    def mapping_cb(self, m):
        x = [m.inputs[dim] for dim in range(m.nb_inputs)]
        y = [m.outputs[dim] for dim in range(m.nb_outputs)]

        if vote.mapping_precise(m):
            testcase_cb(self, x, y)
            return vote.PASS
        else:
            return vote.UNSURE

    def test_proc(self):
        cb = functools.partial(mapping_cb, self)
        return self.ensemble.forall(cb)

    return test_proc


def approximate(testcase_cb):
    '''
    approximate decorator for test cases
    '''
    def test_proc(self):
        m = self.ensemble.approximate()
        x = [m.inputs[dim] for dim in range(m.nb_inputs)]
        y = [m.outputs[dim] for dim in range(m.nb_outputs)]
    
        testcase_cb(self, x, y)
        
    return test_proc


class SimpleVoTETestCase(unittest.TestCase):
    '''
    A base test case with a simple tree ensemble (d=2, B=2), and
    a corresponding hand-coded implementation (f) that acts as an oracle.
    '''
    
    serialized_ensemble = '''{
    "trees": [{
        "nb_inputs": 1,
        "nb_outputs": 1,
        "left": [1, 2, -1, -1, 5, -1, -1],
        "right": [4, 3, -1, -1, 6, -1, -1],
        "feature": [0, 0, -1, -1, 0, -1, -1],
        "threshold": [5, 1, -1, -1, 9, -1, -1],
        "value": [[-1], [-1], [0], [1], [-1], [2], [3]],
        "normalize": false
      }, {
        "nb_inputs": 1,
        "nb_outputs": 1,
        "left": [1, 2, -1, -1, 5, -1, -1],
        "right": [4, 3, -1, -1, 6, -1, -1],
        "feature": [0, 0, -1, -1, 0, -1, -1],
        "threshold": [2, 1, -1, -1, 6, -1, -1],
        "value": [[-1], [-1], [1], [0], [-1], [5], [2]],
        "normalize": false
      }
    ],
    "post_process": "divisor"}'''
    ensemble = None
    
    def setUp(self):
        self.ensemble = vote.Ensemble.from_string(self.serialized_ensemble)
        
    def t1(self, x):
        '''
        Hand-coded implementation of the first tree
        '''
        if   x <= 1: return 0
        elif x <= 5: return 1
        elif x <= 9: return 2
        else:        return 3

    def t2(self, x):
        '''
        Hand-coded implementation of the second tree
        '''
        if   x <= 1: return 1
        elif x <= 2: return 0
        elif x <= 6: return 5
        else:        return 2

    def f(self, x):
        '''
        Hand-coded implementation of the serialized tree ensemble
        '''
        return (self.t1(x) + self.t2(x)) / 2.0


class TestEnsembleBasics(SimpleVoTETestCase):
    
    def test_io_dims(self):
        self.assertEqual(self.ensemble.nb_inputs, 1)
        self.assertEqual(self.ensemble.nb_outputs, 1)

    def test_name_of_post_processing_algorithm(self):
        self.assertEqual(self.ensemble.post_processing_algorithm, 'divisor')
        
    def test_eval(self):
        for x in [1, 2, 5, 6, 9, float('inf')]:
            y = self.f(x)
            y_pred = self.ensemble.eval(x)
            self.assertAlmostEqual(y, y_pred[0])

    def test_serialize(self):
        o1 = json.loads(self.ensemble.serialize())
        o2 = json.loads(self.serialized_ensemble)
        self.assertEqual(o1, o2)
        
    
class TestMappingEdges(SimpleVoTETestCase):
    '''
    Check edges of mappings enumerated by VoTE against the oracle (f).
    '''
    
    @forall
    def test_precise_mapping_edges(self, x, y):
        x, y = x[0], y[0]
        
        self.assertEqual(y.lower, y.upper)
        y = y.lower
        
        self.assertAlmostEqual(y, self.f(x.lower))
        self.assertAlmostEqual(y, self.f(x.upper))

    @approximate
    def test_approximation_edges(self, x, y):
        y = y[0]

        xmin = 2 # [-inf, 2]
        xmax = 6 # (5, 6]
        self.assertLessEqual(y.lower, self.f(xmin))
        self.assertGreaterEqual(y.upper, self.f(xmax))

    
class TestForAll(SimpleVoTETestCase):
    '''
    Count number of conclusive mappings enumerated by VoTE
    '''
    count = 0

    def setUp(self):
        SimpleVoTETestCase.setUp(self)
        self.count = 0

    def increment_counter(self, m):
        self.assertTrue(vote.mapping_precise(m))
        self.count += 1
        return vote.PASS
        
    def increment_counter_to_3(self, m):
        self.assertTrue(vote.mapping_precise(m))
        self.count += 1
        if self.count < 3:
            return vote.PASS
        else:
            return vote.FAIL
        
    def test_exhaustive_forall(self):
        res = self.ensemble.forall(self.increment_counter)
        self.assertTrue(res)
        self.assertEqual(self.count, 6)

    def test_partial_forall(self):
        res = self.ensemble.forall(self.increment_counter_to_3)
        self.assertFalse(res)
        self.assertEqual(self.count, 3)


class TestAbsRef(SimpleVoTETestCase):
    outputs = None
    expected = set([
        (0.5, 0.5), 
        (3, 3),
        (0.5, 3),
        (3.5, 3.5), 
        (2, 2), 
        (2.5, 2.5),
        (0.5, 3),   # 1 < x <= 5: 1 + [0, 5]
        (2, 3.5),   # 5 < x <= 9: 2 + [2, 5]
        (0, 4)      # all trees abstracted
    ])
    
    def setUp(self):
        SimpleVoTETestCase.setUp(self)
        self.outputs = set()

    def add_outputs(self, m):
        self.outputs.add((m.outputs[0].lower,
                          m.outputs[0].upper))
        
        if vote.mapping_precise(m):
            return vote.PASS
        else:
            return vote.UNSURE
        
    def test_exhaustive_absref(self):
        res = self.ensemble.absref(self.add_outputs)
        self.assertTrue(res)
        self.assertEqual(self.expected, self.outputs)


class VoTEUtilityTestCase(SimpleVoTETestCase):
    
    serialized_ensemble = '''{
    "trees": [{
        "nb_inputs": 1,
        "nb_outputs": 3,
        "left": [1, 2, -1, -1, 5, -1, -1],
        "right": [4, 3, -1, -1, 6, -1, -1],
        "feature": [0, 0, -1, -1, 0, -1, -1],
        "threshold": [5, 1, -1, -1, 9, -1, -1],
        "value": [[-1,-1,-1], [-1,-1,-1], [0,1,2], [1,0,2],
                 [-1,-1,-1], [2,1,1], [3,0,0]]
      }
    ],
    "post_process": "divisor"}'''
    
    def test_argmax(self):
        self.assertEqual(0, vote.argmax([5,4,3,2,1]))
        self.assertEqual(1, vote.argmax([0,4,3,2,1]))
        self.assertEqual(2, vote.argmax([0,0,3,2,1]))
        self.assertEqual(3, vote.argmax([0,0,0,2,1]))
        self.assertEqual(4, vote.argmax([0,0,0,0,1]))

    def test_argmin(self):
        self.assertEqual(0, vote.argmin([0,4,3,2,1]))
        self.assertEqual(1, vote.argmin([5,0,3,2,1]))
        self.assertEqual(2, vote.argmin([5,4,0,2,1]))
        self.assertEqual(3, vote.argmin([5,4,3,0,1]))
        self.assertEqual(4, vote.argmin([5,4,3,2,0]))

    def test_mapping_argmax_unknown(self):
        m = self.ensemble.approximate()
        self.assertEqual(-1, vote.mapping_argmax(m))
    
    def test_mapping_argmin_unknown(self):
        m = self.ensemble.approximate()
        self.assertEqual(-1, vote.mapping_argmin(m))

    def test_mapping_argmin_first(self):
        m = self.ensemble.approximate()
        m.outputs[0].lower = 0
        m.outputs[0].upper = 0.9
        m.outputs[1].lower = 1
        m.outputs[1].upper = 1
        m.outputs[2].lower = 2
        m.outputs[2].upper = 2
        self.assertEqual(0, vote.mapping_argmin(m))

    def test_mapping_argmin_middle(self):
        m = self.ensemble.approximate()
        m.outputs[0].lower = 2
        m.outputs[0].upper = 2
        m.outputs[1].lower = 1
        m.outputs[1].upper = 1
        m.outputs[2].lower = 2
        m.outputs[2].upper = 2
        self.assertEqual(1, vote.mapping_argmin(m))

    def test_mapping_argmin_last(self):
        m = self.ensemble.approximate()
        m.outputs[0].lower = 3
        m.outputs[0].upper = 3
        m.outputs[1].lower = 2
        m.outputs[1].upper = 2
        m.outputs[2].lower = 1
        m.outputs[2].upper = 1
        self.assertEqual(2, vote.mapping_argmin(m))

    def test_mapping_argmax_first(self):
        m = self.ensemble.approximate()
        m.outputs[0].lower = 3
        m.outputs[0].upper = 3
        m.outputs[1].lower = 2
        m.outputs[1].upper = 2
        m.outputs[2].lower = 1
        m.outputs[2].upper = 1
        self.assertEqual(0, vote.mapping_argmax(m))
        
    def test_mapping_argmax_middle(self):
        m = self.ensemble.approximate()
        m.outputs[0].lower = 1
        m.outputs[0].upper = 1
        m.outputs[1].lower = 2
        m.outputs[1].upper = 2
        m.outputs[2].lower = 1
        m.outputs[2].upper = 1
        self.assertEqual(1, vote.mapping_argmax(m))

    def test_mapping_argmax_last(self):
        m = self.ensemble.approximate()
        m.outputs[0].lower = 0
        m.outputs[0].upper = 0
        m.outputs[1].lower = 1
        m.outputs[1].upper = 1
        m.outputs[2].lower = 2
        m.outputs[2].upper = 2
        self.assertEqual(2, vote.mapping_argmax(m))

    def test_mapping_check_argmax_unsure(self):
        m = self.ensemble.approximate()
        m.outputs[0].lower = 0
        m.outputs[0].upper = 1
        m.outputs[1].lower = 0
        m.outputs[1].upper = 1
        m.outputs[2].lower = 1
        m.outputs[2].upper = 1
        self.assertEqual(vote.UNSURE, vote.mapping_check_argmax(m, 0))
        self.assertEqual(vote.UNSURE, vote.mapping_check_argmax(m, 1))
        self.assertTrue(vote.mapping_check_argmax(m, 2))
        
    def test_mapping_check_argmax_same(self):
        m = self.ensemble.approximate()
        m.outputs[0].lower = 0
        m.outputs[0].upper = 0
        m.outputs[1].lower = 0
        m.outputs[1].upper = 0
        m.outputs[2].lower = 0
        m.outputs[2].upper = 0
        self.assertTrue(vote.mapping_check_argmax(m, 0))
        self.assertTrue(vote.mapping_check_argmax(m, 1))
        self.assertTrue(vote.mapping_check_argmax(m, 2))

    def test_mapping_check_argmax_last(self):
        m = self.ensemble.approximate()
        m.outputs[0].lower = 0
        m.outputs[0].upper = 0
        m.outputs[1].lower = 1
        m.outputs[1].upper = 1
        m.outputs[2].lower = 2
        m.outputs[2].upper = 2
        self.assertFalse(vote.mapping_check_argmax(m, 0))
        self.assertFalse(vote.mapping_check_argmax(m, 1))
        self.assertTrue(vote.mapping_check_argmax(m, 2))

    def test_mapping_check_argmax_middle(self):
        m = self.ensemble.approximate()
        m.outputs[0].lower = 0
        m.outputs[0].upper = 0
        m.outputs[1].lower = 1
        m.outputs[1].upper = 1
        m.outputs[2].lower = 0
        m.outputs[2].upper = 0
        self.assertFalse(vote.mapping_check_argmax(m, 0))
        self.assertTrue(vote.mapping_check_argmax(m, 1))
        self.assertFalse(vote.mapping_check_argmax(m, 2))

    def test_mapping_check_argmax_first(self):
        m = self.ensemble.approximate()
        m.outputs[0].lower = 1
        m.outputs[0].upper = 1
        m.outputs[1].lower = 0
        m.outputs[1].upper = 0
        m.outputs[2].lower = 0
        m.outputs[2].upper = 0
        self.assertTrue(vote.mapping_check_argmax(m, 0))
        self.assertFalse(vote.mapping_check_argmax(m, 1))
        self.assertFalse(vote.mapping_check_argmax(m, 2))

    def test_mapping_check_argmin_unsure(self):
        m = self.ensemble.approximate()
        m.outputs[0].lower = 0
        m.outputs[0].upper = 1
        m.outputs[1].lower = 0
        m.outputs[1].upper = 1
        m.outputs[2].lower = 0
        m.outputs[2].upper = 0
        self.assertEqual(vote.UNSURE, vote.mapping_check_argmin(m, 0))
        self.assertEqual(vote.UNSURE, vote.mapping_check_argmin(m, 1))
        self.assertTrue(vote.mapping_check_argmin(m, 2))
        
    def test_mapping_check_argmin_same(self):
        m = self.ensemble.approximate()
        m.outputs[0].lower = 0
        m.outputs[0].upper = 0
        m.outputs[1].lower = 0
        m.outputs[1].upper = 0
        m.outputs[2].lower = 0
        m.outputs[2].upper = 0
        self.assertTrue(vote.mapping_check_argmin(m, 0))
        self.assertTrue(vote.mapping_check_argmin(m, 1))
        self.assertTrue(vote.mapping_check_argmin(m, 2))

    def test_mapping_check_argmin_last(self):
        m = self.ensemble.approximate()
        m.outputs[0].lower = 2
        m.outputs[0].upper = 2
        m.outputs[1].lower = 1
        m.outputs[1].upper = 1
        m.outputs[2].lower = 0
        m.outputs[2].upper = 0
        self.assertFalse(vote.mapping_check_argmin(m, 0))
        self.assertFalse(vote.mapping_check_argmin(m, 1))
        self.assertTrue(vote.mapping_check_argmin(m, 2))

    def test_mapping_check_argmin_middle(self):
        m = self.ensemble.approximate()
        m.outputs[0].lower = 1
        m.outputs[0].upper = 1
        m.outputs[1].lower = 0
        m.outputs[1].upper = 0
        m.outputs[2].lower = 1
        m.outputs[2].upper = 1
        self.assertFalse(vote.mapping_check_argmin(m, 0))
        self.assertTrue(vote.mapping_check_argmin(m, 1))
        self.assertFalse(vote.mapping_check_argmin(m, 2))

    def test_mapping_check_argmin_first(self):
        m = self.ensemble.approximate()
        m.outputs[0].lower = 0
        m.outputs[0].upper = 0
        m.outputs[1].lower = 1
        m.outputs[1].upper = 1
        m.outputs[2].lower = 1
        m.outputs[2].upper = 1
        self.assertTrue(vote.mapping_check_argmin(m, 0))
        self.assertFalse(vote.mapping_check_argmin(m, 1))
        self.assertFalse(vote.mapping_check_argmin(m, 2))

    def test_mapping_precise(self):
        m = self.ensemble.approximate()
        m.outputs[0].lower = 0
        m.outputs[0].upper = 0
        m.outputs[1].lower = 1
        m.outputs[1].upper = 1
        m.outputs[2].lower = 2
        m.outputs[2].upper = 2

        for dim in range(3):
            self.assertTrue(vote.mapping_precise(m))
            m.outputs[dim].upper += 1
            self.assertFalse(vote.mapping_precise(m))
            m.outputs[dim].lower += 1


def check_mapping(oracle_fn, itype, otype, m, epsilon=0):
    center = [m.inputs[dim].lower + (m.inputs[dim].upper -
                                     m.inputs[dim].lower) / 2
              for dim in range(m.nb_inputs)]

    lo = np.array([itype(m.inputs[dim].lower)
          if itype(m.inputs[dim].lower) >= m.inputs[dim].lower
          else np.nextafter(m.inputs[dim].lower, np.inf, dtype=itype)
          for dim in range(m.nb_inputs)])

    hi = np.array([itype(m.inputs[dim].upper)
          if itype(m.inputs[dim].upper) <= m.inputs[dim].upper
          else np.nextafter(m.inputs[dim].upper, -np.inf, dtype=itype)
          for dim in range(m.nb_inputs)])
        
    X = np.array([lo + epsilon, center, hi - epsilon], dtype=itype)
    Y = oracle_fn(X)

    lo = [otype(m.outputs[dim].lower)
          if otype(m.outputs[dim].lower) >= m.outputs[dim].lower
          else np.nextafter(m.outputs[dim].lower, np.inf, dtype=otype)
          for dim in range(m.nb_outputs)]

    hi = [otype(m.outputs[dim].upper)
          if otype(m.outputs[dim].upper) <= m.outputs[dim].upper
          else np.nextafter(m.outputs[dim].upper, -np.inf, dtype=otype)
          for dim in range(m.nb_outputs)]
    
    for xvec, yvec in zip(X, Y):
        for dim in range(m.nb_outputs):
            if lo[dim] > yvec[dim] + epsilon:
                return vote.FAIL

            if hi[dim] < yvec[dim] - epsilon:
                return vote.FAIL
                
    if vote.mapping_precise(m):
        return vote.PASS
    else:
        return vote.UNSURE


class TestCatBoost(unittest.TestCase):
    '''
    CatBoost takes as input 64bit floats, and returns 64bit floats.
    CatBoost also usees an optimized variant of the exp() functions which produce 
    slightly different results than glibc exp(), hence an epsilon value of 1e-7.
    '''
    def test_binary_classification(self):
        from catboost import CatBoostClassifier
        from sklearn.datasets import make_classification
        
        X, Y = make_classification(n_classes=2, n_features=4, random_state=12345)
        m = CatBoostClassifier(iterations=1, loss_function='MultiClass',
                               thread_count=1, max_depth=4, random_state=12345)
        m.fit(X, Y)
        
        e = vote.Ensemble.from_catboost(m)
        pred = lambda X: m.predict_proba(X, thread_count=1)
        cb = functools.partial(check_mapping, pred, np.float64, np.float64, epsilon=1e-7)
        dom = [(np.finfo(np.float64).min, np.finfo(np.float64).max)] * e.nb_inputs

        self.assertTrue(e.forall(cb, dom))
        self.assertTrue(e.absref(cb, dom))

    def test_multiclass_classification(self):
        from catboost import CatBoostClassifier
        
        from sklearn.datasets import make_classification
        X, Y = make_classification(n_classes=3, n_clusters_per_class=2,
                                   n_informative=3, random_state=12345)
        m = CatBoostClassifier(iterations=3, loss_function='MultiClass',
                               thread_count=1, max_depth=4, random_state=12345)
        m.fit(X, Y)

        e = vote.Ensemble.from_catboost(m)
        pred = lambda X: m.predict_proba(X, thread_count=1)
        cb = functools.partial(check_mapping, pred, np.float64, np.float64, epsilon=1e-7)
        dom = [(np.finfo(np.float64).min, np.finfo(np.float64).max)] * e.nb_inputs

        self.assertTrue(e.forall(cb, dom))
        self.assertTrue(e.absref(cb, dom))

    def test_univariate_regression(self):
        from catboost import CatBoostRegressor
        from sklearn.datasets import make_regression

        X, Y = make_regression(n_targets=1, n_features=2, random_state=12345)
        m = CatBoostRegressor(iterations=3, max_depth=2, random_state=12345)
        m.fit(X, Y)
        
        e = vote.Ensemble.from_catboost(m)
        pred = lambda X: [[y] for y in m.predict(X, thread_count=1)]
        cb = functools.partial(check_mapping, pred, np.float64, np.float64, epsilon=1e-7)
        dom = [(np.finfo(np.float64).min, np.finfo(np.float64).max)] * e.nb_inputs
        
        self.assertTrue(e.forall(cb, dom))
        self.assertTrue(e.absref(cb, dom))

    def test_multivariate_regression(self):
        from catboost import CatBoostRegressor
        from sklearn.datasets import make_regression

        X, Y = make_regression(n_targets=3, random_state=12345)
        m = CatBoostRegressor(n_estimators=3, max_depth=4, random_state=12345,
                              loss_function='MultiRMSE')
        m.fit(X, Y)

        e = vote.Ensemble.from_catboost(m)
        pred = lambda X: m.predict(X, thread_count=1)
        cb = functools.partial(check_mapping, pred, np.float64, np.float64, epsilon=1e-7)
        dom = [(np.finfo(np.float64).min, np.finfo(np.float64).max)] * e.nb_inputs

        self.assertTrue(e.forall(cb, dom))
        self.assertTrue(e.absref(cb, dom))


class TestSciKitLearn(unittest.TestCase):
    '''
    sklearn casts inputs to float32 while returning float64
    '''
    nb_trees = 2

    def test_rf_binary_classification(self):
        from sklearn.ensemble import RandomForestClassifier
        from sklearn.datasets import make_classification

        X, Y = make_classification(n_classes=2, random_state=12345)
        m = RandomForestClassifier(n_estimators=self.nb_trees, random_state=12345)
        m.fit(X, Y)

        e = vote.Ensemble.from_sklearn(m)
        cb = functools.partial(check_mapping, m.predict_proba, np.float32, np.float64)
        dom = [(np.finfo(np.float32).min, np.finfo(np.float32).max)] * e.nb_inputs
        
        self.assertTrue(e.forall(cb, dom))
        self.assertTrue(e.absref(cb, dom))

    def test_rf_multiclass_classification(self):
        from sklearn.ensemble import RandomForestClassifier
        from sklearn.datasets import make_classification

        X, Y = make_classification(n_classes=3, n_clusters_per_class=2,
                                   n_informative=3, random_state=12345)
        m = RandomForestClassifier(n_estimators=self.nb_trees, random_state=12345)
        m.fit(X, Y)
        Y_pred = m.predict_proba(X)
        
        e = vote.Ensemble.from_sklearn(m)
        cb = functools.partial(check_mapping, m.predict_proba, np.float32, np.float64)
        dom = [(np.finfo(np.float32).min, np.finfo(np.float32).max)] * e.nb_inputs
        
        self.assertTrue(e.forall(cb, dom))
        self.assertTrue(e.absref(cb, dom))
        
    def test_rf_univariate_regression(self):
        from sklearn.ensemble import RandomForestRegressor
        from sklearn.datasets import make_regression

        X, Y = make_regression(n_targets=1, random_state=12345)
        m = RandomForestRegressor(n_estimators=self.nb_trees, random_state=12345)
        m.fit(X, Y)
        Y_pred = m.predict(X)

        e = vote.Ensemble.from_sklearn(m)
        pred = lambda X: [[x] for x in m.predict(X)]
        cb = functools.partial(check_mapping, pred, np.float32, np.float64)
        dom = [(np.finfo(np.float32).min, np.finfo(np.float32).max)] * e.nb_inputs
        
        self.assertTrue(e.forall(cb, dom))
        self.assertTrue(e.absref(cb, dom))

    def test_rf_multivariate_regression(self):
        from sklearn.ensemble import RandomForestRegressor
        from sklearn.datasets import make_regression

        X, Y = make_regression(n_targets=3, random_state=12345)
        m = RandomForestRegressor(n_estimators=self.nb_trees, random_state=12345)
        m.fit(X, Y)
        
        e = vote.Ensemble.from_sklearn(m)
        cb = functools.partial(check_mapping, m.predict, np.float32, np.float64)
        dom = [(np.finfo(np.float32).min, np.finfo(np.float32).max)] * e.nb_inputs
        
        self.assertTrue(e.forall(cb, dom))
        self.assertTrue(e.absref(cb, dom))


class TestXGBoost(unittest.TestCase):
    '''
    XGBoost use 32bit floats when summing up trees, hence suffer from
    precision loss compared to, e.g., sklearn who uses 64bit floats
    internally, hence an epsilon value of 1e-3.
    '''
    
    def test_binary_classification(self):
        from xgboost import XGBClassifier
        from sklearn.datasets import make_classification
        
        X, Y = make_classification(n_classes=2, n_features=4, random_state=12345)
        m = XGBClassifier(n_estimators=3, n_jobs=1, max_depth=4, random_state=12345)
        m.fit(X, Y)
        
        e = vote.Ensemble.from_xgboost(m)
        pred = lambda X: [[p[1]] for p in m.predict_proba(X)]
        cb = functools.partial(check_mapping, pred, np.float32, np.float32, epsilon=1e-3)
        dom = [(np.finfo(np.float32).min, np.finfo(np.float32).max)] * e.nb_inputs
        
        self.assertTrue(e.forall(cb, dom))
        self.assertTrue(e.absref(cb, dom))
            
    def test_univariate_regression(self):
        from xgboost import XGBRegressor
        from sklearn.datasets import make_regression

        X, Y = make_regression(n_targets=1, n_features=2, random_state=12345)
        m = XGBRegressor(n_estimators=3, base_score=0, n_jobs=1, max_depth=4, random_state=12345)
        m.fit(X, Y)
        
        e = vote.Ensemble.from_xgboost(m)
        pred = lambda X: [[y] for y in m.predict(X)]
        cb = functools.partial(check_mapping, pred, np.float32, np.float32, epsilon=1e-3)
        dom = [(np.finfo(np.float32).min, np.finfo(np.float32).max)] * e.nb_inputs
        
        self.assertTrue(e.forall(cb, dom))
        self.assertTrue(e.absref(cb, dom))

        
if __name__ == "__main__":
    unittest.main()
