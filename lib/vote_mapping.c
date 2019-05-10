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
#include <string.h>
#include <assert.h>

#include "vote.h"
#include "vote_math.h"

vote_mapping_t*
vote_mapping_new(size_t input_dim, size_t output_dim) {
  assert(input_dim);
  assert(output_dim);
  
  vote_mapping_t* m = calloc(1, sizeof(vote_mapping_t));
  assert(m);
  
  m->inputs = calloc(input_dim, sizeof(vote_bound_t));
  assert(m->inputs);
  
  m->outputs = calloc(output_dim, sizeof(vote_bound_t));
  assert(m->outputs);

  m->nb_inputs = input_dim;
  m->nb_outputs = output_dim;
    
  for(size_t i=0; i<m->nb_inputs; i++) {
    m->inputs[i].lower = -INFINITY;
    m->inputs[i].upper = INFINITY;
  }

  return m;
}


vote_mapping_t*
vote_mapping_copy(const vote_mapping_t* m) {
  vote_mapping_t* c = malloc(sizeof(vote_mapping_t));
  assert(c);
  
  c->nb_inputs = m->nb_inputs;
  c->nb_outputs = m->nb_outputs;

  c->inputs = malloc(c->nb_inputs * sizeof(vote_bound_t));
  assert(c->inputs);
  
  c->outputs = malloc(c->nb_outputs * sizeof(vote_bound_t));
  assert(c->outputs);
  
  memcpy(c->inputs, m->inputs, m->nb_inputs * sizeof(vote_bound_t));
  memcpy(c->outputs, m->outputs, m->nb_outputs * sizeof(vote_bound_t));

  return c;
}


void
vote_mapping_del(vote_mapping_t* m) {
  free(m->inputs);
  free(m->outputs);
  free(m);
}


int
vote_mapping_argmax(const vote_mapping_t* m) {
  size_t k = 0;

  for(size_t i=0; i<m->nb_outputs; i++) {
    if(m->outputs[i].upper > m->outputs[k].upper) {
      k = i;
    }
  }

  if(m->outputs[k].lower == m->outputs[k].upper) {
    return k;
  }

  for(size_t i=0; i<m->nb_outputs; i++) {
    if(m->outputs[i].upper > m->outputs[k].lower && i != k) {
      return -1;
    }
  }

  return k;
}


int
vote_mapping_argmin(const vote_mapping_t* m) {
  size_t k = 0;

  for(size_t i=0; i<m->nb_outputs; i++) {
    if(m->outputs[i].lower < m->outputs[k].lower) {
      k = i;
    }
  }

  if(m->outputs[k].lower == m->outputs[k].upper) {
    return k;
  }

  for(size_t i=0; i<m->nb_outputs; i++) {
    if(m->outputs[i].lower < m->outputs[k].upper && i != k) {
      return -1;
    }
  }

  return k;
}


void
vote_mapping_join(const vote_mapping_t *m, vote_mapping_t *join) {
  for(size_t i=0; i<m->nb_inputs; i++) {
    join->inputs[i].lower = vote_min(join->inputs[i].lower, m->inputs[i].lower);
    join->inputs[i].upper = vote_max(join->inputs[i].upper, m->inputs[i].upper);
  }

  for(size_t i=0; i<m->nb_outputs; i++) {
    join->outputs[i].lower = vote_min(join->outputs[i].lower, m->outputs[i].lower);
    join->outputs[i].upper = vote_max(join->outputs[i].upper, m->outputs[i].upper);
  }
}

