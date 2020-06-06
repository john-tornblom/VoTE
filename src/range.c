/* Copyright (C) 2018 John Törnblom

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


#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <vote.h>


/**
 * Print a mapping to stdout.
 */
static void
dump_mapping(vote_mapping_t *m) {
  for(size_t i=0; i<m->nb_inputs; i++) {
    if(i) {
      printf("\n                       ");
    }
    printf("x%ld in [%f, %f]", i, m->inputs[i].lower, m->inputs[i].upper);
  }
  
  for(size_t i=0; i<m->nb_outputs; i++) {
    printf("\n                       ");
    printf("y%ld in [%f, %f]", i, m->outputs[i].lower, m->outputs[i].upper);
  }
  printf("\n");
}


/**
 * Check if there are points in the ouput that fall
 * outside a given range.
 */
static vote_outcome_t
is_within_range(void *ctx, vote_mapping_t *m) {
  vote_bound_t *range = (vote_bound_t *)ctx;
  size_t nb_pass_dims = 0;
  bool precise = true;
  
  for(size_t i=0; i<m->nb_outputs; i++) {
    precise &= (m->outputs[i].lower == m->outputs[i].upper);
    nb_pass_dims += (m->outputs[i].lower >= range[i].lower &&
		     m->outputs[i].upper <= range[i].upper);
  }

  if(nb_pass_dims == m->nb_outputs) {
    return VOTE_PASS;
  } else if(precise) {
    printf("range:counter-example: ");
    dump_mapping(m);
    return VOTE_FAIL;
  } else {
    return VOTE_UNSURE;
  }
}
 

/**
 * Check the plausibility of range property.
 **/
int main(int argc, char** argv) {
  vote_ensemble_t* e;
  bool b;
  
  if(argc < 2) {
    printf("usage: %s <model file> <min y0> <max y0> <min y1> <max y1>...\n",
	   argv[0]);
    return 1;
  }

  if(!(e = vote_ensemble_load_file(argv[1]))) {
    printf("Unable to load model from %s\n", argv[1]);
    exit(1);
  }

  if(argc < (e->nb_outputs * 2) + 2) {
    printf("Expected %ld min/max arguments, got %d\n", e->nb_outputs * 2, argc - 2);
    exit(1);
  }

  printf("range:filename:        %s\n", argv[1]);
  printf("range:nb_inputs:       %ld\n", e->nb_inputs);
  printf("range:nb_outputs:      %ld\n", e->nb_outputs);
  printf("range:nb_trees:        %ld\n", e->nb_trees);
  printf("range:nb_nodes:        %ld\n", e->nb_nodes);

  time_t t = time(NULL);

  printf("range:requirement:     ");
  vote_bound_t domain[e->nb_inputs];
  for(size_t i=0; i<e->nb_inputs; i++) {
    domain[i].lower = -VOTE_INFINITY;
    domain[i].upper = VOTE_INFINITY;

    if(i) {
      printf("\n                       ");
    }
    printf("x%ld in [%f, %f]", i, domain[i].lower, domain[i].upper);
  }

  vote_bound_t range[e->nb_outputs];
  for(size_t i=0; i<e->nb_outputs; i++) {
    range[i].lower = atof(argv[2+(i*2)]);
    range[i].upper = atof(argv[3+(i*2)]);

    printf("\n                       ");
    printf("y%ld in [%f, %f]", i, range[i].lower, range[i].upper);
  }
  printf("\n");

  
  b = vote_ensemble_absref(e, domain, is_within_range, range);
  vote_ensemble_del(e);

  printf("range:result:          %s\n", b ? "pass" : "fail");
  printf("range:runtime:         %lds\n", time(NULL) - t);
  
  return !b;
}

