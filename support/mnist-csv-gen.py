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
Download the MNIST dataset and save it as CSV file
'''

import csv
import logging
import optparse
import sys

from sklearn.datasets import fetch_mldata
from sklearn.model_selection import train_test_split


def save_csv(filename, X, Y):
    with open(filename, 'w') as f:
        w = csv.writer(f, delimiter=',')
        for x, y in zip(X, Y):
            row = list(x)
            row.append(y)
            w.writerow(row)


def main():
    parser = optparse.OptionParser(usage='%prog [options]')
    parser.set_description(__doc__.strip())
    
    parser.add_option('--train', dest='train', action='store',
                      help='Save training csv file to PATH',
                      metavar='PATH', default=None)

    parser.add_option('--test', dest='test', action='store',
                      help='Save testing csv file to PATH',
                      metavar='PATH', default=None)

    parser.add_option('--verify', dest='verify', action='store',
                      help='Copy one sample of each class from the test set and'
                      'save in csv file to PATH',
                      metavar='PATH', default=None)
    
    parser.add_option('-v', '--verbosity', dest='verbosity', action='count',
                      default=1, help='increase debug logging level')
    
    (opts, args) = parser.parse_args()
    levels = {
              0: logging.ERROR,
              1: logging.WARNING,
              2: logging.INFO,
              3: logging.DEBUG,
    }
    logging.basicConfig(level=levels.get(opts.verbosity, logging.DEBUG))

    mnist = fetch_mldata('MNIST original')
    X_train, X_test, Y_train, Y_test = train_test_split(mnist.data,
                                                        mnist.target,
                                                        test_size=0.15,
                                                        random_state=12345)
                
    if opts.train:
        save_csv(opts.train, X_train, Y_train)

    if opts.test:
        save_csv(opts.test, X_test, Y_test)

    
if __name__ == '__main__':
    main()
