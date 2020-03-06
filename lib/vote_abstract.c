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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "vote.h"
#include "vote_pipeline.h"
#include "vote_abstract.h"
#include "vote_math.h"


/**
 *
 **/
typedef struct vote_abstract {
  vote_tree_t     *const*trees;
  size_t                 nb_trees;
  const vote_pipeline_t *pipeline;
  const vote_pipeline_t *postproc;
} vote_abstract_t;


/**
 * Compute the join of a tree for a particular input region.
 **/
static void
vote_abstract_join_tree(const vote_tree_t *t, size_t node_id,
			const vote_bound_t *inputs, size_t nb_inputs,
			vote_bound_t *outputs, size_t nb_outputs) {
  int left_id = t->left[node_id];
  int right_id = t->right[node_id];
  real_t value[nb_outputs];
    
  if(left_id < 0 || right_id < 0) {

    memcpy(value, t->value[node_id], nb_outputs * sizeof(real_t));
    if(t->normalize) {
      vote_normalize(value, nb_outputs);
    }
  
    for(size_t i=0; i<nb_outputs; i++) {
      outputs[i].lower = vote_min(value[i], outputs[i].lower);
      outputs[i].upper = vote_max(value[i], outputs[i].upper);
    }
    return;
  }

  real_t threshold = t->threshold[node_id];
  int dim = t->feature[node_id];
    
  // left: [lower, threshold]
  if(inputs[dim].lower <= threshold) {
    vote_abstract_join_tree(t, left_id, inputs, nb_inputs, outputs, nb_outputs);
  }

  // right: (threshold, upper]
  if(inputs[dim].upper > threshold) {
    vote_abstract_join_tree(t, right_id, inputs, nb_inputs, outputs, nb_outputs);
  }
}


/**
 * Compute the join of a set of trees for a particular input region.
 **/
static void
vote_abstract_join_trees(vote_tree_t *const*trees, size_t nb_trees,
			 vote_mapping_t *m) {
  const size_t root_id = 0;
  vote_bound_t tree_outputs[m->nb_outputs];
  
  for(size_t i=0; i<nb_trees; i++) {
    for(size_t dim=0; dim<m->nb_outputs; dim++) {
      tree_outputs[dim].lower = VOTE_INFINITY;
      tree_outputs[dim].upper = -VOTE_INFINITY;
    }
    
    vote_abstract_join_tree(trees[i], root_id,
			    m->inputs, m->nb_inputs,
			    tree_outputs, m->nb_outputs);

    for(size_t dim=0; dim<m->nb_outputs; dim++) {
      m->outputs[dim].lower += tree_outputs[dim].lower;
      m->outputs[dim].upper += tree_outputs[dim].upper;
    }
  }
}


/**
 * Apply the abstraction algorithm on a mapping.
 **/
static vote_outcome_t
vote_abstract_input(void *ctx, vote_mapping_t *m) {
  vote_abstract_t *a = (vote_abstract_t*)ctx;
  vote_bound_t outputs[m->nb_outputs];
  vote_mapping_t join = {
    .inputs = m->inputs,
    .outputs = outputs,
    .nb_inputs = m->nb_inputs,
    .nb_outputs = m->nb_outputs
  };

  memcpy(outputs, m->outputs, m->nb_outputs * sizeof(vote_bound_t));
  vote_abstract_join_trees(a->trees, a->nb_trees, &join);
  vote_outcome_t o = vote_pipeline_input(a->postproc, &join);

  if(o == VOTE_UNSURE) {
    return vote_pipeline_output(a->pipeline, m);
  }

  return o;
}


vote_pipeline_t*
vote_abstract_pipeline(vote_tree_t *const*trees, size_t nb_trees,
		       const vote_pipeline_t *postproc) {
  vote_abstract_t *a = calloc(1, sizeof(vote_abstract_t));
  vote_pipeline_t *p = vote_pipeline_new(a, vote_abstract_input, free);
  
  assert(a);

  a->trees    = trees;
  a->nb_trees = nb_trees;
  a->pipeline = p;
  a->postproc = postproc;
  
  return p;
}

