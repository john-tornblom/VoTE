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
 * Check if there are points in the ouput that fall
 * outside a given range.
 */
static bool
is_within_range(void *ctx, vote_mapping_t *m) {
  vote_bound_t *range = (vote_bound_t *)ctx;
  
  for(size_t i=0; i<m->nb_outputs; i++) {
    if(m->outputs[i].lower < range[i].lower) {
      return false;
    }

    if(m->outputs[i].upper > range[i].upper) {
      return false;
    }
  }
  
  return true;
}
 

/**
 * Check the plausibility of range property.
 **/
int main(int argc, char** argv) {
  vote_ensemble_t* e;
  vote_mapping_t* m;
  bool b;
  
  if(argc < 2) {
    printf("usage: %s <model file> <min y1> <max y1> <min y2> <max y2>...\n",
	   argv[0]);
    return 1;
  }

  if(!(e = vote_ensemble_load(argv[1]))) {
    printf("Unable to load model from %s\n", argv[1]);
    exit(1);
  }

  if(argc < (e->nb_outputs * 2) + 2) {
    printf("Expected %ld min/max arguments, got %d\n", e->nb_outputs * 2, argc - 2);
    exit(1);
  }

  printf("range:filename:   %s\n", argv[1]);
  printf("range:nb_inputs:  %ld\n", e->nb_inputs);
  printf("range:nb_outputs: %ld\n", e->nb_outputs);
  printf("range:nb_trees:   %ld\n", e->nb_trees);
  printf("range:nb_nodes:   %ld\n", e->nb_nodes);

  time_t t = time(NULL);
  vote_bound_t domain[e->nb_inputs];
  for(size_t i=0; i<e->nb_inputs; i++) {
    domain[i].lower = -INFINITY;
    domain[i].upper = INFINITY;
  }

  vote_bound_t range[e->nb_outputs];
  for(size_t i=0; i<e->nb_outputs; i++) {
    range[i].lower = atof(argv[2+(i*2)]);
    range[i].upper = atof(argv[3+(i*2)]);
  }

  m = vote_ensemble_approximate(e, domain);
  assert(m);
  
  b = (is_within_range(range, m) ||
       vote_ensemble_forall(e, domain, is_within_range, range));
       
  vote_mapping_del(m);
  vote_ensemble_del(e);

  printf("range:result:     %s\n", b ? "pass" : "fail");
  printf("range:runtime:    %lds\n", time(NULL) - t);
  
  return !b;
}

