/* Copyright (C) 2019 John Törnblom

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING. If not, see
<http://www.gnu.org/licenses/>.  */


#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <math.h>
#include <vote.h>


/**
 * Count number of precise mappings.
 */
static vote_outcome_t
count_mapping(void *ctx, vote_mapping_t *m) {
  size_t *nb_mappings = (size_t*)ctx;

  assert(vote_mapping_precise(m));
  
  (*nb_mappings)++;
  
  return VOTE_PASS;
}


/**
 * Print the number of mappings of an ensemble to stdout.
 **/
int main(int argc, char** argv) {
  if(argc < 2) {
    printf("usage: %s <model file>\n", argv[0]);
    return 1;
  }

  size_t nb_mappings = 0;
  vote_ensemble_t* e = vote_ensemble_load(argv[1]);
  assert(e);

  printf("cardinality:filename:    %s\n", argv[1]);
  printf("cardinality:nb_inputs:   %ld\n", e->nb_inputs);
  printf("cardinality:nb_outputs:  %ld\n", e->nb_outputs);
  printf("cardinality:nb_trees:    %ld\n", e->nb_trees);
  printf("cardinality:nb_nodes:    %ld\n", e->nb_nodes);
  
  vote_bound_t domain[e->nb_inputs];
  for(size_t i=0; i<e->nb_inputs; i++) {
    domain[i].lower = -INFINITY;
    domain[i].upper = INFINITY;
  }
  
  vote_ensemble_forall(e, domain, count_mapping, &nb_mappings);
  vote_ensemble_del(e);

  printf("cardinality:nb_mappings: %ld\n", nb_mappings);

  return 0;
}

