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

#
# Gradient Boosting
#
for params in {"5 20","5 25","10 20","10 25","15 20","15 25"}; do
    params=($params)
    d=${params[0]}
    B=${params[1]}

    $SCRIPTDIR/train-catboost.py \
	$SCRIPTDIR/data/collision_detection.train.csv \
	-v \
	-B $B \
	-d $d \
	-o collision_detection.gb.B${B}.d${d}.json || exit 1

    $SCRIPTDIR/../src/vote_accuracy \
	collision_detection.gb.B${B}.d${d}.json \
	$SCRIPTDIR/data/collision_detection.test.csv \
	> collision_detection.gb.B${B}.d${d}.log || exit 1
    
    $SCRIPTDIR/../src/vote_range \
	collision_detection.gb.B${B}.d${d}.json \
	0 1 \
	0 1 \
	0 1 \
	0 1 \
	0 1 \
	0 1 \
	>> collision_detection.gb.B${B}.d${d}.log || exit 1
	
    $SCRIPTDIR/../src/vote_robustness \
	collision_detection.gb.B${B}.d${d}.json \
	$SCRIPTDIR/data/collision_detection.test.csv \
	0.05 \
	>> collision_detection.gb.B${B}.d${d}.log || exit 1
done



#
# Random forests
#
for params in {"10 20","10 25","15 20","15 25","20 20","20 25"}; do
    params=($params)
    d=${params[0]}
    B=${params[1]}

    $SCRIPTDIR/train-sklearn.py \
	$SCRIPTDIR/data/collision_detection.train.csv \
	-v \
	-B $B \
	-d $d \
	-o collision_detection.rf.B${B}.d${d}.json || exit 1

    $SCRIPTDIR/../src/vote_accuracy \
	collision_detection.rf.B${B}.d${d}.json \
	$SCRIPTDIR/data/collision_detection.test.csv \
	> collision_detection.rf.B${B}.d${d}.log || exit 1
    
    $SCRIPTDIR/../src/vote_range \
	collision_detection.rf.B${B}.d${d}.json \
	0 1 \
	0 1 \
	0 1 \
	0 1 \
	0 1 \
	0 1 \
	>> collision_detection.rf.B${B}.d${d}.log || exit 1
	
    $SCRIPTDIR/../src/vote_robustness \
	collision_detection.rf.B${B}.d${d}.json \
	$SCRIPTDIR/data/collision_detection.test.csv \
	0.05 \
	>> collision_detection.rf.B${B}.d${d}.log || exit 1
done


#
# Scalability
#
echo "" > collision_detection.rf.d10.cardinality.log
echo "" > collision_detection.gb.d10.cardinality.log
for B in $(seq 1 30); do
    $SCRIPTDIR/train-sklearn.py \
	$SCRIPTDIR/data/collision_detection.train.csv \
	-v \
	-B $B \
	-d 10 \
	-o collision_detection.rf.B${B}.d10.json || exit 1

    $SCRIPTDIR/train-catboost.py \
	$SCRIPTDIR/data/collision_detection.train.csv \
	-v \
	-B $B \
	-d 10 \
	-o collision_detection.gb.B${B}.d10.json || exit 1
    
    $SCRIPTDIR/../src/vote_cardinality \
	collision_detection.rf.B${B}.d10.json \
	>> collision_detection.rf.d10.cardinality.log &

    $SCRIPTDIR/../src/vote_cardinality \
	collision_detection.gb.B${B}.d10.json \
	>> collision_detection.gb.d10.cardinality.log &

    wait
done

