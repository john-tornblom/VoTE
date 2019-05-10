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

import atexit
import functools
import json
import os
import tempfile
import unittest

import vote


def forall(testcase_cb):
    '''
    forall decorator for test cases
    '''    
    def mapping_cb(self, m):
        x = [m.inputs[dim] for dim in range(m.nb_inputs)]
        y = [m.outputs[dim] for dim in range(m.nb_outputs)]
    
        testcase_cb(self, x, y)
    
        return True

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
        "threshold": [5, 1, NaN, NaN, 9, NaN, NaN],
        "value": [[NaN], [NaN], [0], [1], [NaN], [2], [3]]
      }, {
        "nb_inputs": 1,
        "nb_outputs": 1,
        "left": [1, 2, -1, -1, 5, -1, -1],
        "right": [4, 3, -1, -1, 6, -1, -1],
        "feature": [0, 0, -1, -1, 0, -1, -1],
        "threshold": [2, 1, NaN, NaN, 6, NaN, NaN],
        "value": [[NaN], [NaN], [1], [0], [NaN], [5], [2]]
      }
    ],
    "post_process": "divisor"}'''
    ensemble = None
    
    def setUp(self):
        (_, filename) = tempfile.mkstemp()
        atexit.register(os.remove, filename)
        with open(filename, 'w') as fp:
            obj = json.loads(self.serialized_ensemble)
            json.dump(obj, fp)

        self.ensemble = vote.Ensemble(filename)
        
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

    def test_eval(self):
        for x in [1, 2, 5, 6, 9, float('inf')]:
            y = self.f(x)
            y_pred = self.ensemble.eval(x)
            self.assertAlmostEqual(y, y_pred[0])


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


class TestNumberOfMappings(SimpleVoTETestCase):
    '''
    Count number of mappings enumerated by VoTE
    '''
    count = 0

    def setUp(self):
        SimpleVoTETestCase.setUp(self)
        self.count = 0

    def increment_counter(self, m):
        self.count += 1
        
        return True
        
    def increment_counter_to_3(self, m):
        self.count += 1
        
        return self.count < 3

    def test_exhaustive_forall(self):
        res = self.ensemble.forall(self.increment_counter)
        
        self.assertTrue(res)
        self.assertEqual(self.count, 6)

    def test_partial_forall(self):
        res = self.ensemble.forall(self.increment_counter_to_3)
        
        self.assertFalse(res)
        self.assertEqual(self.count, 3)



class VoTEUtilityTestCase(SimpleVoTETestCase):
    
    serialized_ensemble = '''{
    "trees": [{
        "nb_inputs": 1,
        "nb_outputs": 3,
        "left": [1, 2, -1, -1, 5, -1, -1],
        "right": [4, 3, -1, -1, 6, -1, -1],
        "feature": [0, 0, -1, -1, 0, -1, -1],
        "threshold": [5, 1, NaN, NaN, 9, NaN, NaN],
        "value": [[NaN,NaN,NaN], [NaN,NaN,NaN], [0,1,2], [1,0,2],
                 [NaN,NaN,NaN], [2,1,1], [3,0,0]]
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

        
if __name__ == "__main__":
    unittest.main()
