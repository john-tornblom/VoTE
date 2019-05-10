/* Copyright (C) 2018 John Törnblom

   This file is part of VoTE (Verifier of Tree Ensembles).

VoTE is free software: you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any
later version.

VoTE is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
for more details.

You should have received a copy of the GNU Lesser General Public
License along with VoTE; see the files COPYING and COPYING.LESSER. If not,
see <http://www.gnu.org/licenses/>.  */


#ifndef VOTE_TREE_H
#define VOTE_TREE_H

#include <stddef.h>


/**
 * A Decision tree contains nodes with thresholds on input variables which 
 * determine the path traveled in the tree. Leaves carry values.
 **/
struct vote_tree {
  int* left;
  int* right;
  
  int*     feature;
  real_t*  threshold;
  real_t** value;

  size_t nb_inputs;
  size_t nb_outputs;
  size_t nb_nodes;
};


struct json_object;


/**
 * Parse a JSON dictonary into a tree.
 **/
vote_tree_t *vote_tree_parse(struct json_object *root);


/**
 * Delete a tree and all of its members.
 **/
void vote_tree_del(vote_tree_t* t);


#endif //VOTE_TREE_H
