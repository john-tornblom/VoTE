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

#include "parson.h"

#include "vote.h"
#include "vote_tree.h"


/**
 * Parse a JSON number array into an array or reals.
 **/
static void
vote_parse_floats(struct json_array_t *array, real_t** mem, size_t* length) {
  size_t _length = json_array_get_count(array);
  real_t* _mem = calloc(_length, sizeof(real_t));
  
  assert(_mem);
  
  for(size_t i=0; i<_length; i++) {
    _mem[i] = (real_t)json_array_get_number(array, i);
  }

  *length = _length;
  *mem = _mem;
}


/**
 * Encode an array of reals into a JSON number array.
 **/
static struct json_value_t*
vote_encode_reals(real_t* values, size_t length) {
  struct json_value_t* root = json_value_init_array();
  struct json_array_t* array = json_value_get_array(root);
  
  for(size_t i=0; i<length; i++) {
    json_array_append_number(array, values[i]);
  }

  return root;
}


/**
 * Parse a JSON number array into an array of integers.
 **/
static void
vote_parse_ints(struct json_array_t *array, int** mem, size_t* length) {
  size_t _length = json_array_get_count(array);
  int* _mem = calloc(_length, sizeof(int));
  
  assert(_mem);
  
  for(size_t i=0; i<_length; i++) {
    _mem[i] = (int)json_array_get_number(array, i);
  }

  *length = _length;
  *mem = _mem;
}


/**
 * Encode an array of integers into a JSON number array.
 **/
static struct json_value_t*
vote_encode_ints(int* values, size_t length) {
  struct json_value_t* root = json_value_init_array();
  struct json_array_t* array = json_value_get_array(root);
  
  for(size_t i=0; i<length; i++) {
    json_array_append_number(array, values[i]);
  }

  return root;
}


/**
 * Parse a JSON dictionary into a tree.
 **/
vote_tree_t *
vote_tree_parse(struct json_value_t *root) {
  struct json_object_t *obj;
  struct json_array_t *array;
  size_t length;
  
  vote_tree_t* tree = calloc(1, sizeof(vote_tree_t));
  assert(tree);

  assert(json_value_get_type(root) == JSONObject);
  obj = json_value_get_object(root);

  tree->nb_inputs = (size_t)json_object_get_number(obj, "nb_inputs");
  tree->nb_outputs = (size_t)json_object_get_number(obj, "nb_outputs");
  tree->normalize = json_object_get_boolean(obj, "normalize") > 0;
  
  array = json_object_get_array(obj, "left");
  vote_parse_ints(array, &tree->left, &length);
  tree->nb_nodes = length;
    
  array = json_object_get_array(obj, "right");
  vote_parse_ints(array, &tree->right, &length);
  assert(length == tree->nb_nodes);

  array = json_object_get_array(obj, "feature");
  vote_parse_ints(array, &tree->feature, &length);
  assert(length == tree->nb_nodes);

  array = json_object_get_array(obj, "threshold");
  vote_parse_floats(array, &tree->threshold, &length);
  assert(length == tree->nb_nodes);

  array = json_object_get_array(obj, "value");
  length = json_array_get_count(array);
  assert(length == tree->nb_nodes);
  
  tree->value = calloc(length, sizeof(real_t*));
  assert(tree->value);
  
  for(size_t i=0; i<tree->nb_nodes; i++) {
    struct json_array_t *vec = json_array_get_array(array, i);
    vote_parse_floats(vec, &tree->value[i], &length);
    assert(length == tree->nb_outputs);
  }

  return tree;
}


struct json_value_t*
vote_tree_encode(const vote_tree_t* t) {
  struct json_value_t* root = json_value_init_object();
  struct json_object_t* obj = json_value_get_object(root);
  struct json_value_t* value = json_value_init_array();
  struct json_array_t* array = json_value_get_array(value);
  
  json_object_set_number(obj, "nb_inputs", (double)t->nb_inputs);
  json_object_set_number(obj, "nb_outputs", (double)t->nb_outputs);
  json_object_set_boolean(obj, "normalize", t->normalize);

  json_object_set_value(obj, "left", vote_encode_ints(t->left, t->nb_nodes));
  json_object_set_value(obj, "right", vote_encode_ints(t->right, t->nb_nodes));
  json_object_set_value(obj, "feature", vote_encode_ints(t->feature, t->nb_nodes));
  json_object_set_value(obj, "threshold", vote_encode_reals(t->threshold, t->nb_nodes));
  json_object_set_value(obj, "value", value);
  
  for(size_t i=0; i<t->nb_nodes; i++) {
    json_array_append_value(array, vote_encode_reals(t->value[i], t->nb_outputs));
  }

  return root;
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

