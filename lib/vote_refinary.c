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

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "vote.h"
#include "vote_refinary.h"
#include "vote_tree.h"
#include "vote_math.h"


static bool emit_leaves(vote_refinery_t* r, vote_mapping_t *m, size_t node_id);


vote_refinery_t*
vote_refinery_new(const vote_tree_t *t, vote_mapping_cb_t *cb, void* ctx) {
  vote_refinery_t* r = calloc(1, sizeof(vote_refinery_t));
  assert(r);

  r->tree     = t;
  r->emit_cb  = cb;
  r->emit_ctx = ctx;
  
  return r;
}


void
vote_refinery_del(vote_refinery_t* r) {
  free(r);
}


/**
 * Decend into children of a node, starting with the left child.
 **/
static bool
descend_left_first(vote_refinery_t* r, vote_mapping_t *m, size_t node_id) {
  int left_id = r->tree->left[node_id];
  int right_id = r->tree->right[node_id];
  real_t threshold = r->tree->threshold[node_id];
  int dim = r->tree->feature[node_id];

  // refine left split: [lower, threshold]
  if(m->inputs[dim].lower <= threshold) {
    vote_bound_t inputs[m->nb_inputs];
    vote_bound_t outputs[m->nb_outputs];
    vote_mapping_t msplit = {
      .inputs = inputs,
      .outputs = outputs,
      .nb_inputs = m->nb_inputs,
      .nb_outputs = m->nb_outputs
    };
    memcpy(msplit.inputs, m->inputs, m->nb_inputs * sizeof(vote_bound_t));
    memcpy(msplit.outputs, m->outputs, m->nb_outputs * sizeof(vote_bound_t));
  
    if(msplit.inputs[dim].upper > threshold) {
      msplit.inputs[dim].upper = threshold;
    }
    if(!emit_leaves(r, &msplit, left_id)) {
      return false;
    }
  }

  // refine right split: (threshold, upper]
  if(m->inputs[dim].upper > threshold) {
    if(m->inputs[dim].lower < threshold) {
      m->inputs[dim].lower = vote_nextafter(threshold, INFINITY);
    }
    return emit_leaves(r, m, right_id);
  }
  return true;
}


/**
 * Decend into children of a node, starting with the right child.
 **/
static bool
descend_right_first(vote_refinery_t* r, vote_mapping_t *m, size_t node_id) {
  int left_id = r->tree->left[node_id];
  int right_id = r->tree->right[node_id];
  real_t threshold = r->tree->threshold[node_id];
  int dim = r->tree->feature[node_id];

  // refine right split: (threshold, upper]
  if(m->inputs[dim].upper > threshold) {
    vote_bound_t inputs[m->nb_inputs];
    vote_bound_t outputs[m->nb_outputs];
    vote_mapping_t msplit = {
      .inputs = inputs,
      .outputs = outputs,
      .nb_inputs = m->nb_inputs,
      .nb_outputs = m->nb_outputs
    };
      
    memcpy(msplit.inputs, m->inputs, m->nb_inputs * sizeof(vote_bound_t));
    memcpy(msplit.outputs, m->outputs, m->nb_outputs * sizeof(vote_bound_t));
  
    if(msplit.inputs[dim].lower < threshold) {
      msplit.inputs[dim].lower = vote_nextafter(threshold, INFINITY);
    }
    if(!emit_leaves(r, &msplit, right_id)) {
      return false;
    }
  }

  // refine left split: [lower, threshold]
  if(m->inputs[dim].lower <= threshold) {
    if(m->inputs[dim].upper > threshold) {
      m->inputs[dim].upper = threshold;
    }

    return emit_leaves(r, m, left_id);
  }
  return true;
}


/**
 * Emit mappings associated with a node.
 **/
static bool
emit_leaves(vote_refinery_t* r, vote_mapping_t *m, size_t node_id) {
  int left_id = r->tree->left[node_id];
  int right_id = r->tree->right[node_id];

  // leaf node encountered, emit mapping
  if(left_id < 0 || right_id < 0) {
    assert(left_id < 0 && right_id < 0);
    
    for(size_t i=0; i<m->nb_outputs; i++) {
      m->outputs[i].upper += r->tree->value[node_id][i];
      m->outputs[i].lower += r->tree->value[node_id][i];
    }
    
    return r->emit_cb(r->emit_ctx, m);
  }

  // intermediate node encountered, decend into children (least volume first).
  //
  //        left       right
  //   |-----------|-----------|
  // lower     threshold     upper
  
  real_t threshold = r->tree->threshold[node_id];
  int dim = r->tree->feature[node_id];

  real_t right_width = m->inputs[dim].upper - threshold;
  real_t left_width = threshold - m->inputs[dim].lower;

  if(left_width < right_width) {
    return descend_left_first(r, m, node_id);
  } else {
    return descend_right_first(r, m, node_id);
  }  
}


bool
vote_refinery_emit(vote_refinery_t* r, vote_mapping_t *m) {
  const size_t root_id = 0;
  return emit_leaves(r, m, root_id);
}
