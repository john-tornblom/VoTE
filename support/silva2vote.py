#!/usr/bin/env python2
# encoding: utf-8
# Copyright (C) 2020 John Törnblom
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
Convert a tree ensemble from the silva format to the VoTE format
'''

import json


class SilvaParser:
    f = None
    la = (None, None)
    node_id = None
    post_proc = None
    
    def __init__(self, f):
        self.f = f
        self.nextline()
        self.node_id = -1
        self.post_proc = 'divisor'
        
    def nextline(self):
        ret = self.la
        self.la = self.f.readline().strip().split(' ', 1)
        return ret
    
    def lookahead(self, value):
        return value == self.la[0]
    
    def parse(self):
        ty, params = self.nextline()
        params = params.split(' ')
        
        name = 'parse_%s' % ty.replace('-', '_')
        fn = getattr(self, name)
        return fn(*params)

    def parse_classifier_forest(self, nb_trees):
        nb_trees = int(nb_trees)
        d = dict(trees=[self.parse() for _ in range(nb_trees)])
        d['post_process'] = self.post_proc
        return d

    def parse_classifier_decision_tree(self, nb_inputs, nb_classes):
        nb_inputs = int(nb_inputs)
        nb_classes = int(nb_classes)
        lefts = list()
        rights = list()
        features= list()
        thresholds = list()
        values = list()
        
        _ = self.nextline() # drop labels

        self.parse_node(lefts, rights, features, thresholds, values, nb_classes)
        for lst in (lefts, rights, features, thresholds, values):
            lst.insert(0, lst.pop())

        return dict(nb_inputs=nb_inputs,
                    nb_outputs=nb_classes,
                    normalize=self.post_proc == 'divisor',
                    left=lefts,
                    right=rights,
                    feature=features,
                    threshold=thresholds,
                    value=values)

    def parse_node(self, lefts, rights, features, thresholds, values, nb_classes):

        if self.lookahead('LEAF') or self.lookahead('LEAF_LOGARITHMIC'):
            value = self.parse()
            lefts.append(-1)
            rights.append(-1)
            features.append(-1)
            thresholds.append(-1)
            values.append(value)
            return len(values)
        
        elif self.lookahead('SPLIT'):
            feature, threshold = self.parse()
            left_id = self.parse_node(lefts, rights, features,
                                      thresholds, values, nb_classes)
            right_id = self.parse_node(lefts, rights, features,
                                       thresholds, values, nb_classes)

            lefts.append(left_id)
            rights.append(right_id)
            features.append(feature)
            thresholds.append(threshold)
            values.append([-1] * nb_classes)
                
            return len(values)

    def parse_SPLIT(self, dim, threshold):
        return int(dim), float(threshold)

    def parse_LEAF(self, *args):
        value = [float(v) for v in args]
        return value

    def parse_LEAF_LOGARITHMIC(self, *args):
        self.post_proc = 'softmax'
        value = [float(v) for v in args]
        return value


if __name__ == '__main__':
    import sys
    d = SilvaParser(sys.stdin).parse()
    print json.dumps(d)
