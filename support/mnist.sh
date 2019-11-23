#!/usr/bin/env bash
#   Copyright (C) 2018 John Törnblom
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

SCRIPTDIR="${BASH_SOURCE[0]}"
SCRIPTDIR="$(dirname "${SCRIPTDIR}")"

export PYTHONPATH=$SCRIPTDIR/../bindings/python

RANGE_ARG=""
for i in {1..784}; do
    RANGE_ARG="0 1 $RANGE_ARG"
done

#
# Gradient Boosting
#
for params in {"5 25","5 50","5 75","10 25","10 50","10 75"}; do
    params=($params)
    d=${params[0]}
    B=${params[1]}

    $SCRIPTDIR/train-catboost.py \
	$SCRIPTDIR/data/mnist.train.csv.gz \
	-v \
	-B $B \
	-d $d \
	-o mnist.gb.B${B}.d${d}.json || exit 1

    $SCRIPTDIR/../src/vote_accuracy \
	mnist.gb.B${B}.d${d}.json \
	$SCRIPTDIR/data/mnist.test.csv \
	> mnist.gb.B${B}.d${d}.log || exit 1

#    $SCRIPTDIR/../src/vote_range \
#	mnist.gb.B${B}.d${d}.json \
#	$RANGE_ARG \
#	>> mnist.gb.B${B}.d${d}.log || exit 1
	
    $SCRIPTDIR/../src/vote_robustness \
	mnist.gb.B${B}.d${d}.json \
	$SCRIPTDIR/data/mnist.test.csv \
	1 \
	>> mnist.gb.B${B}.d${d}.log || exit 1
done


#
# Random forests
#
for params in {"5 25","5 50","5 75","10 25","10 50","10 75"}; do
    params=($params)
    d=${params[0]}
    B=${params[1]}
    
    $SCRIPTDIR/train-sklearn.py \
	$SCRIPTDIR/data/mnist.train.csv.gz \
	-v \
	-B $B \
	-d $d \
	-o mnist.rf.B${B}.d${d}.json || exit 1

    $SCRIPTDIR/../src/vote_accuracy \
	mnist.rf.B${B}.d${d}.json \
	$SCRIPTDIR/data/mnist.test.csv \
	> mnist.rf.B${B}.d${d}.log || exit 1

    $SCRIPTDIR/../src/vote_range \
	mnist.rf.B${B}.d${d}.json \
	$RANGE_ARG \
	>> mnist.rf.B${B}.d${d}.log || exit 1
	
    $SCRIPTDIR/../src/vote_robustness \
	mnist.rf.B${B}.d${d}.json \
	$SCRIPTDIR/data/mnist.test.csv \
	1 \
	>> mnist.rf.B${B}.d${d}.log || exit 1
done


#
# Scalability
#
echo "" > mnist.rf.d10.cardinality.log
echo "" > mnist.gb.d10.cardinality.log
for B in $(seq 1 4); do
    $SCRIPTDIR/train-sklearn.py \
	$SCRIPTDIR/data/mnist.train.csv.gz \
	-v \
	-B $B \
	-d 10 \
	-o mnist.rf.B${B}.d10.json || exit 1

    $SCRIPTDIR/train-catboost.py \
	$SCRIPTDIR/data/mnist.train.csv.gz \
	-v \
	-B $B \
	-d 10 \
	-o mnist.gb.B${B}.d10.json || exit 1
    
    $SCRIPTDIR/../src/vote_cardinality \
	mnist.rf.B${B}.d10.json \
	>> mnist.rf.d10.cardinality.log &

    $SCRIPTDIR/../src/vote_cardinality \
	mnist.gb.B${B}.d10.json \
	>> mnist.gb.d10.cardinality.log &

    wait
done

