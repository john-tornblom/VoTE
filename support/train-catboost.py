#!/usr/bin/env python
# encoding: utf-8
# Copyright (C) 2019 John Törnblom
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
Train a gradient boost classifier using the catboost toolkit. Samples shall be
provided in the CSV file format using the comma delimiter (,) and with labels as
the last column. Trained models are serialized into a json format and persisted
to disk.
'''

import collections
import json
import logging
import optparse
import sys

import numpy as np

from catboost import CatBoostClassifier


logger = logging.getLogger('train-catboost')


def convert_catboost_json(filename):
    with open(filename, 'r') as f:
        cb = json.load(f)

    multiclass_params = json.loads(cb['model_info']['multiclass_params'])
    nb_inputs = len(cb['features_info']['float_features'])
    nb_outputs = multiclass_params['classes_count']

    tree_obj_list = list()
    for tree in cb['oblivious_trees']:

        tree_obj = dict()
        tree_obj['nb_inputs'] = nb_inputs
        tree_obj['nb_outputs'] = nb_outputs

        nb_splits = len(tree['splits'])
        depth = nb_splits + 1
        nb_nodes = (2 ** depth) - 1
        splits = list(reversed(tree['splits']))

        tree_obj['left'] = [-1] * nb_nodes
        tree_obj['right'] = [-1] * nb_nodes
        tree_obj['feature'] = [-1] * nb_nodes
        tree_obj['threshold'] = [None] * nb_nodes
        tree_obj['value'] = [[None] * nb_outputs] * nb_nodes

        tree_obj['left'][0:nb_nodes/2] = [ind for ind in range(2, nb_nodes, 2) if ind % 2 == 0]
        tree_obj['right'][0:nb_nodes/2] = [ind for ind in range(1, nb_nodes, 2) if ind % 2 == 1]

        for node_id in range(2 ** nb_splits - 1):
            d = int(np.log2(node_id + 1))
            tree_obj['feature'][node_id] = splits[d]['float_feature_index']
            tree_obj['threshold'][node_id] = splits[d]['border']

        queue = collections.deque(tree['leaf_values'])
        for node_id in range(2 ** nb_splits - 1, nb_nodes):
            values = [queue.pop() for _ in range(nb_outputs)]
            tree_obj['value'][node_id] = list(reversed(values))
                
        tree_obj_list.append(tree_obj)

    root_obj = dict(trees=tree_obj_list,
                    post_process='softmax')
    
    with open(filename, 'w') as f:
        json.dump(root_obj, f)


def main():
    parser = optparse.OptionParser(usage='%prog [options] <csv filename>')
    parser.set_description(__doc__.strip())
    
    parser.add_option('-B', dest='nb_trees', action='store',
                      help='Train a tree ensemble of INTEGER trees using gradient boosting',
                      metavar='INTEGER', default=None, type=int)
    
    parser.add_option('-d', dest='max_depth', action='store',
                      help='Limit depth of trees to INTEGER',
                      metavar='INTEGER', default=None, type=int)

    parser.add_option('-l', dest='learning_rate', action='store',
                      help='Learning rate for the gradient decent optimizer',
                      metavar='FLOAT', default=0.5, type=float)
    
    parser.add_option('-o', dest='output', action='store',
                      help='Save trained tree ensemble to PATH',
                      metavar='PATH', default=None)

    parser.add_option('-v', '--verbosity', dest='verbosity', action='count',
                      default=1, help='increase debug logging level')
    
    (opts, args) = parser.parse_args()
    if len(args) == 0 or not opts.output:
        parser.print_help()
        sys.exit(1)
        
    levels = {
              0: logging.ERROR,
              1: logging.WARNING,
              2: logging.INFO,
              3: logging.DEBUG,
    }
    logging.basicConfig(level=levels.get(opts.verbosity, logging.DEBUG))

    data = np.loadtxt(args[0], delimiter=',')
    features = data[:, :-1]
    labels = data[:,-1].astype('int')
    nb_classes = len(set(labels))

    logger.info('filename: %s', opts.output)
    logger.info('nb_inputs: %d', features.shape[1])
    logger.info('nb_outputs: %d', nb_classes)
    logger.info('train set: %d samples', len(features))
    
    m = CatBoostClassifier(max_depth=opts.max_depth,
                           num_trees=opts.nb_trees,
                           classes_count=nb_classes,
                           learning_rate=opts.learning_rate,
                           logging_level='Silent',
                           random_state=12345,
#                           task_type='GPU',
                           loss_function='MultiClass')

    m.fit(features, labels)

    m.save_model(opts.output, format='json')
    convert_catboost_json(opts.output)
    

if __name__ == '__main__':
    main()
        
