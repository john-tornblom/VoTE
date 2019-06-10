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
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <vote.h>


static vote_outcome_t
is_correct(void *ctx, vote_mapping_t *m) {
  real_t *sample = ctx;
  size_t expected = (size_t)roundf(sample[m->nb_inputs]);
  
  return vote_mapping_check_argmax(m, expected);
}


/**
 * Parse command line arguments and launch program.
 **/
int main(int argc, char** argv) {
  real_t *data;
  size_t nb_rows, nb_cols;
  real_t score = 0;
  vote_ensemble_t* e;
  real_t threshold;
  
  if(argc < 4) {
    printf("usage: %s <model file> <csv file> <threshold>\n", argv[0]);
    return 1;
  }

  if(!(e = vote_ensemble_load_file(argv[1]))) {
    printf("Unable to load model from %s\n", argv[1]);
    exit(1);
  }
  
  if(!vote_csv_load(argv[2], &data, &nb_rows, &nb_cols)) {
    printf("Unable to load data from %s\n", argv[2]);
    exit(1);
  }
  
  if(nb_cols != e->nb_inputs + 1) {
    printf("Unexpected number of columns in %s\n", argv[2]);
    exit(1);
  }

  threshold = atof(argv[3]);

  printf("robustness:filename:   %s\n", argv[1]);
  printf("robustness:nb_inputs:  %ld\n", e->nb_inputs);
  printf("robustness:nb_outputs: %ld\n", e->nb_outputs);
  printf("robustness:nb_trees:   %ld\n", e->nb_trees);
  printf("robustness:nb_nodes:   %ld\n", e->nb_nodes);
  printf("robustness:nb_samples: %ld\n", nb_rows);
  printf("robustness:threshold:  %f\n", threshold);

  time_t start_time = time(NULL);
  time_t max_sample_time = 0;
  
  for(size_t row=0; row<nb_rows; row++) {
    fprintf(stderr, "robustness:progress:   %ld/%ld", row+1, nb_rows);
    fflush(stderr);
    fprintf(stderr, "\r");
    
    real_t *sample = &data[row * nb_cols];
    vote_bound_t bounds[e->nb_inputs];

    for(size_t i=0; i<e->nb_inputs; i++) {
      bounds[i].lower = sample[i];
      bounds[i].upper = sample[i];
    }

    // don't bother with samples that are classified incorrectly
    vote_mapping_t *m = vote_ensemble_approximate(e, bounds);
    vote_outcome_t o = is_correct(sample, m);
    vote_mapping_del(m);

    assert(o != VOTE_UNSURE);
    if(o != VOTE_PASS) {
      continue;
    }
    
    for(size_t i=0; i<e->nb_inputs; i++) {
      bounds[i].lower -= threshold;
      bounds[i].upper += threshold;
    }

    time_t sample_time = time(NULL);
    score += vote_ensemble_absref(e, bounds, is_correct, sample);
    sample_time = time(NULL) - sample_time;

    if(sample_time > max_sample_time) {
      max_sample_time = sample_time;
    }
  }
  
  vote_ensemble_del(e);
  free(data);

  printf("robustness:score:      %f\n", score/nb_rows);
  printf("robustness:runtime:    %lds\n", time(NULL) - start_time);
  printf("robustness:maxtime:    %lds\n", max_sample_time);
  
  return 0;
}

