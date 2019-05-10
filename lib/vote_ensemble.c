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
#include <string.h>
#include <json.h>

#include "vote.h"
#include "vote_math.h"
#include "vote_tree.h"
#include "vote_refinary.h"


vote_ensemble_t*
vote_ensemble_load(const char *filename) {
  json_object *obj;
  json_object* root = json_object_from_file(filename);
  assert(root);
  
  json_object_object_get_ex(root, "trees", &obj);
  array_list* array = json_object_get_array(obj);
  assert(array);
  
  vote_ensemble_t* e = calloc(1, sizeof(vote_ensemble_t));
  assert(e);
  
  e->nb_trees = array_list_length(array);
  e->trees = calloc(e->nb_trees, sizeof(vote_tree_t*));
  assert(e->trees);
  
  for(size_t i=0; i<e->nb_trees; i++) {
    json_object *el = array_list_get_idx(array, i);
    e->trees[i] = vote_tree_parse(el);

    if(i == 0) {
      e->nb_inputs = e->trees[i]->nb_inputs;
      e->nb_outputs = e->trees[i]->nb_outputs;
    } else {
      assert(e->nb_inputs == e->trees[i]->nb_inputs);
      assert(e->nb_outputs == e->trees[i]->nb_outputs);
    }
    e->nb_nodes += e->trees[i]->nb_nodes;
  }

  json_object_object_get_ex(root, "post_process", &obj);
  const char *post_process = json_object_get_string(obj);
  
  if(!strcmp(post_process, "none")) {
    e->post_process = VOTE_POST_PROCESS_NONE;
  } else if(!strcmp(post_process, "divisor")) {
    e->post_process = VOTE_POST_PROCESS_DIVISOR;
  } else if(!strcmp(post_process, "softmax")) {
    e->post_process = VOTE_POST_PROCESS_SOFTMAX;
  } else {
    assert(false && "unknown post-processing algorithm");
  }
  
  //This actually deletes the json_object. Possibly the worst
  //name choices in the history of computing.
  json_object_put(root);
  
  return e;
}



void
vote_ensemble_del(vote_ensemble_t* e) {
  for(size_t i=0; i<e->nb_trees; i++) {
    vote_tree_del(e->trees[i]);
  }

  free(e->trees);
  free(e);
}


/**
 * Keep track of data used in the final pipeline.
 **/
typedef struct vote_ensemble_callout {
  vote_mapping_cb_t *user_cb;
  void              *user_ctx;
  size_t             nb_trees;
} vote_ensemble_callout_t;


/**
 *
 **/
static void
vote_ensemble_divisor(vote_mapping_t *m, size_t nb_trees) {
  for(size_t i=0; i<m->nb_outputs; i++) {
    m->outputs[i].lower /= nb_trees;
    m->outputs[i].upper /= nb_trees;
  }
}

/**
 * Final pipeline which applies the softmax function to output scalars, then 
 * calls out to the user.
 **/
static void
vote_ensemble_softmax(vote_mapping_t *m) {
  vote_bound_t logsumexp = {0, 0};
  real_t max = 0;

#ifdef USE_DOUBLE
  for(size_t i=0; i<m->nb_outputs; i++) {
    max = vote_max(max, m->outputs[i].upper);
  }
#endif

  for(size_t i=0; i<m->nb_outputs; i++) {
    logsumexp.lower += vote_exp(m->outputs[i].lower - max);
    logsumexp.upper += vote_exp(m->outputs[i].upper - max);
  }

  logsumexp.lower = vote_log(logsumexp.lower);
  logsumexp.upper = vote_log(logsumexp.upper);
  
  logsumexp.lower += max;
  logsumexp.upper += max;
  
  for(size_t i=0; i<m->nb_outputs; i++) {
    m->outputs[i].lower = vote_exp(m->outputs[i].lower - logsumexp.upper);
    m->outputs[i].upper = vote_exp(m->outputs[i].upper - logsumexp.lower);
    assert(m->outputs[i].lower <= m->outputs[i].upper);
  }
}


/**
 * Final pipeline for a random forest which divide outputs with the number of
 * trees, then calls out to the user.
 **/
static bool
pipe_divisor_callout(void* ctx, vote_mapping_t *m) {
  vote_ensemble_callout_t *co = ctx;
  vote_ensemble_divisor(m, co->nb_trees);
  return co->user_cb(co->user_ctx, m);
}


/**
 * Final pipeline which applies the softmax function to output scalars, then 
 * calls out to the user.
 **/
static bool
pipe_softmax_callout(void* ctx, vote_mapping_t *m) {
  vote_ensemble_callout_t *co = ctx;
  vote_ensemble_softmax(m);
  return co->user_cb(co->user_ctx, m);
}


/**
 * Final pipeline which applies the softmax function to output scalars, then 
 * calls out to the user.
 **/
static bool
pipe_raw_callout(void* ctx, vote_mapping_t *m) {
  vote_ensemble_callout_t *co = ctx;
  return co->user_cb(co->user_ctx, m);
}

/**
 * Intermediate pipeline between trees in the ensemble.
 **/
static bool
pipe_forall_emit(void* ctx, vote_mapping_t *m) {
  vote_refinery_t* r = (vote_refinery_t*)ctx;
  return vote_refinery_emit(r, m);
}


bool
vote_ensemble_forall(const vote_ensemble_t *e, const vote_bound_t* input_region,
		   vote_mapping_cb_t *user_cb, void* user_ctx) {
  vote_ensemble_callout_t co;
  vote_refinery_t **r;
  vote_mapping_cb_t *cb;
  vote_mapping_t* m;
  bool res;
  void *ctx;
  
  co.user_cb = user_cb;
  co.user_ctx = user_ctx;
  co.nb_trees = e->nb_trees;

  switch(e->post_process) {
  default:
  case VOTE_POST_PROCESS_NONE:
    cb = user_cb;
    ctx = user_ctx;
    break;
  case VOTE_POST_PROCESS_DIVISOR:
    cb = pipe_divisor_callout;
    ctx = &co;
    break;
  case VOTE_POST_PROCESS_SOFTMAX:
    cb = pipe_softmax_callout;
    ctx = &co;
    break;
  }

  r = calloc(e->nb_trees, sizeof(vote_refinery_t*));
  assert(r);

  for(size_t i=0; i<e->nb_trees; i++) {
    ctx = r[i] = vote_refinery_new(e->trees[i], cb, ctx);
    cb = pipe_forall_emit;
  }

  m = vote_mapping_new(e->nb_inputs, e->nb_outputs);
  memcpy(m->inputs, input_region, e->nb_inputs * sizeof(vote_bound_t));
  res = cb(ctx, m);
  
  vote_mapping_del(m);
  for(size_t i=0; i<e->nb_trees; i++) {
    vote_refinery_del(r[i]);
  }
  free(r);
  
  return res;
}


/**
 * Fetch the output from an mapping when a concrete, fully-refined instance is
 * encountered.
 **/
static bool
pipe_eval_compute(void *ctx, vote_mapping_t* m) {
  real_t *outputs = ctx;

  for(size_t i=0; i<m->nb_outputs; i++) {
    assert(m->outputs[i].lower == m->outputs[i].upper);
    outputs[i] = m->outputs[i].lower;
  }

  return false;
}


void
vote_ensemble_eval(const vote_ensemble_t* e, const real_t *inputs, real_t *outputs) {
  vote_bound_t bounds[e->nb_inputs];

  for(size_t i=0; i<e->nb_inputs; i++) {
    bounds[i].lower = inputs[i];
    bounds[i].upper = inputs[i];
  }

  vote_ensemble_forall(e, bounds, pipe_eval_compute, outputs);
}


static bool
pipe_bound_join(void *ctx, vote_mapping_t* m) {
  vote_mapping_t** join = (vote_mapping_t**)ctx;
  if(!*join) {
    *join = vote_mapping_copy(m);
  } else {
    vote_mapping_join(m, *join);
  }
  
  return true;
}


vote_mapping_t *
vote_ensemble_approximate(const vote_ensemble_t *e, const vote_bound_t* input_region) {
  vote_mapping_t *res = vote_mapping_new(e->nb_inputs, e->nb_outputs);

  for(size_t i=0; i<e->nb_trees; i++) {
    vote_mapping_t* join = NULL;
    vote_refinery_t *r = vote_refinery_new(e->trees[i], pipe_bound_join, &join);
    vote_mapping_t *m = vote_mapping_new(e->nb_inputs, e->nb_outputs);
    memcpy(m->inputs, input_region, e->nb_inputs * sizeof(vote_bound_t));
    vote_refinery_emit(r, m);
    vote_refinery_del(r);
    vote_mapping_del(m);

    assert(join);
    for(size_t i=0; i<res->nb_outputs; i++) {
      res->outputs[i].lower += join->outputs[i].lower;
      res->outputs[i].upper += join->outputs[i].upper;
    }

    vote_mapping_del(join);
  }
  
  switch(e->post_process) {
  default:
  case VOTE_POST_PROCESS_DIVISOR:
    vote_ensemble_divisor(res, e->nb_trees);
    break;
  case VOTE_POST_PROCESS_SOFTMAX:
    vote_ensemble_softmax(res);
    break;
  }

  return res;
}
