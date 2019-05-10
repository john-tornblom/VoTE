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


#include <assert.h>
#include <stdlib.h>
#include <json.h>

#include "vote.h"
#include "vote_tree.h"


/**
 * Parse a JSON float array.
 **/
static void
vote_parse_floats(json_object *obj, real_t** mem, size_t* length) {
  array_list* array = json_object_get_array(obj);
  size_t _length = array_list_length(array);
  real_t* _mem = calloc(_length, sizeof(real_t));
  
  assert(_mem);
  
  for(size_t i=0; i<_length; i++) {
    json_object *el = array_list_get_idx(array, i);
    _mem[i] = (real_t)json_object_get_double(el);
  }

  *length = _length;
  *mem = _mem;
}


/**
 * Parse a JSON integer array.
 **/
static void
vote_parse_ints(json_object *obj, int** mem, size_t* length) {
  array_list* array = json_object_get_array(obj);
  size_t _length = array_list_length(array);
  int* _mem = calloc(_length, sizeof(int));
  
  assert(_mem);
  
  for(size_t i=0; i<_length; i++) {
    json_object *el = array_list_get_idx(array, i);
    _mem[i] = json_object_get_int(el);
  }

  *length = _length;
  *mem = _mem;
}


/**
 * Parse a JSON dictionary into a tree.
 **/
vote_tree_t *
vote_tree_parse(json_object *root) {
  json_object *obj;
  array_list* array;
  size_t length;
  
  vote_tree_t* tree = calloc(1, sizeof(vote_tree_t));
  assert(tree);
  
  json_object_object_get_ex(root, "nb_inputs", &obj);
  tree->nb_inputs = json_object_get_int(obj);

  json_object_object_get_ex(root, "nb_outputs", &obj);
  tree->nb_outputs = json_object_get_int(obj);

  json_object_object_get_ex(root, "left", &obj);
  vote_parse_ints(obj, &tree->left, &length);
  tree->nb_nodes = length;
    
  json_object_object_get_ex(root, "right", &obj);
  vote_parse_ints(obj, &tree->right, &length);
  assert(length == tree->nb_nodes);
 
  json_object_object_get_ex(root, "feature", &obj);
  vote_parse_ints(obj, &tree->feature, &length);
  assert(length == tree->nb_nodes);
    
  json_object_object_get_ex(root, "threshold", &obj);
  vote_parse_floats(obj, &tree->threshold, &length);
  assert(length == tree->nb_nodes);
    
  json_object_object_get_ex(root, "value", &obj);
  array = json_object_get_array(obj);
  length = array_list_length(array);
  assert(length == tree->nb_nodes);
  
  tree->value = calloc(length, sizeof(real_t*));
  assert(tree->value);
  
  for(size_t i=0; i<tree->nb_nodes; i++) {
    obj = array_list_get_idx(array, i);
    vote_parse_floats(obj, &tree->value[i], &length);
    assert(length == tree->nb_outputs);
  }

  return tree;
}


void
vote_tree_del(vote_tree_t* t) {
  for(size_t i=0; i<t->nb_nodes; i++) {
    free(t->value[i]);
  }
  
  free(t->left);
  free(t->right);
  free(t->feature);
  free(t->threshold);
  free(t->value);
  free(t);
}

