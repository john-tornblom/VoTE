/* Copyright (C) 2019 John Törnblom

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


#include <stdlib.h>
#include <assert.h>

#include "vote_postproc.h"
#include "vote_math.h"


typedef struct vote_postproc {
  const vote_ensemble_t *ensemble;
  vote_mapping_cb_t     *user_cb;
  void                  *user_ctx;
} vote_postproc_t;


/**
 * Post-processing used by random forests.
 **/
static void
vote_bound_division(vote_bound_t *values, size_t nb_values, size_t divisor) {
  for(size_t i=0; i<nb_values; i++) {
    values[i].lower /= divisor;
    values[i].upper /= divisor;
  }
}


/**
 * Post-processing algorithm used by some gradient boosting machines.
 **/
static void
vote_bound_softmax(vote_bound_t *values, size_t nb_values) {
  vote_bound_t sum = {0, 0};
  vote_bound_t off = {0, 0};
  real_t max = -VOTE_INFINITY;

  // This is used for numerical stabillity, see
  // http://www.deeplearningbook.org/contents/numerical.html
  for(size_t i=0; i<nb_values; i++) {
    max = vote_max(max, values[i].upper);
  }

  for(size_t i=0; i<nb_values; i++) {
    sum.lower += vote_exp(values[i].lower - max);
    sum.upper += vote_exp(values[i].upper - max);
  }

  // log(0) results in undefined behaviour
  assert(sum.lower != 0);
  assert(sum.upper != 0);

  off.lower = vote_log(sum.lower) + max;
  off.upper = vote_log(sum.upper) + max;

  //negate and swap bounds in the box that captures the offset
  real_t tmp = off.lower;
  off.lower = -off.upper;
  off.upper = -tmp;
    
  for(size_t i=0; i<nb_values; i++) {
    values[i].lower = vote_exp(off.lower + values[i].lower);
    values[i].upper = vote_exp(off.upper + values[i].upper);
  }
}


/**
 * Post-processing algorithm used by some gradient boosting machines.
 **/
static void
vote_bound_sigmoid(vote_bound_t *values, size_t nb_values) {
  for(size_t i=0; i<nb_values; i++) {
    values[i].lower = (vote_exp(values[i].lower) /
		       (1 + vote_exp(values[i].lower)));
    
    values[i].upper = (vote_exp(values[i].upper) /
		       (1 + vote_exp(values[i].upper)));
  }
}


void
vote_ensemble_postproc(const vote_ensemble_t *e, vote_bound_t *outputs) {
  switch(e->post_process) {
  case VOTE_POST_PROCESS_DIVISOR:
    vote_bound_division(outputs, e->nb_outputs, e->nb_trees);
    break;
    
  case VOTE_POST_PROCESS_SOFTMAX:
    vote_bound_softmax(outputs, e->nb_outputs);
    break;
    
  case VOTE_POST_PROCESS_SIGMOID:
    vote_bound_sigmoid(outputs, e->nb_outputs);
    break;
    
  default:
  case VOTE_POST_PROCESS_NONE:
    break;
  }
}


/**
 * Apply a post processing algorithm on a mapping.
 **/
static vote_outcome_t
vote_postproc_input(void *ctx, vote_mapping_t *m) {
  vote_postproc_t *pp = (vote_postproc_t*)ctx;

  vote_ensemble_postproc(pp->ensemble, m->outputs);
  
  return pp->user_cb(pp->user_ctx, m);
}


vote_pipeline_t*
vote_postproc_pipeline(const vote_ensemble_t *e, void *user_ctx,
		       vote_mapping_cb_t *user_cb) {
  vote_postproc_t *pp = calloc(1, sizeof(vote_postproc_t));
  vote_pipeline_t *p = vote_pipeline_new(pp, vote_postproc_input, free);
  
  assert(pp);

  pp->ensemble = e;
  pp->user_ctx = user_ctx;
  pp->user_cb  = user_cb;

  return p;
}
