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


#include <stdio.h>
#include <stddef.h>
#include <time.h>
#include <math.h>
#include <vote.h>


/**
 * Dumps throughput (number of mappings per second) to stdout.
 */
static bool
sample_throughput(void *ctx, vote_mapping_t *m) {
  VOTE_UNUSED(m);
  static size_t cnt = 0;
  time_t *prev = (time_t*)ctx;
  time_t curr = time(NULL);
  
  cnt++;
  
  if(*prev < curr - 1) {
    *prev = curr;
    fprintf(stdout, "\rthroughput:speed:      %2.2fM/s", cnt / 1000000.0f);
    fflush(stdout);
    cnt = 0;
  }
  return true;
}


/**
 * Parse command line arguments and launch program.
 **/
int main(int argc, char** argv) {
  if(argc < 2) {
    printf("usage: %s <model file>\n", argv[0]);
    return 1;
  }
  
  vote_ensemble_t* e = vote_ensemble_load(argv[1]);
  time_t t = time(NULL);
  vote_bound_t domain[e->nb_inputs];

  printf("throughput:filename:   %s\n", argv[1]);
  printf("throughput:nb_inputs:  %ld\n", e->nb_inputs);
  printf("throughput:nb_outputs: %ld\n", e->nb_outputs);
  printf("throughput:nb_trees:   %ld\n", e->nb_trees);
  printf("throughput:nb_nodes:   %ld\n", e->nb_nodes);
  printf("throughput:speed:      %2.2fM/s", 0.0);
  fflush(stdout);
  
  for(size_t i=0; i<e->nb_inputs; i++) {
    domain[i].lower = -INFINITY;
    domain[i].upper = INFINITY;
  }

  vote_ensemble_forall(e, domain, sample_throughput, &t);
  vote_ensemble_del(e);

  printf("\n");
  return 0;
}

