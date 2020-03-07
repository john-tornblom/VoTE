/* Copyright (C) 2020 John Törnblom

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
#include <argp.h>
#include <time.h>
#include <unistd.h>
#include <vote.h>

#include "workqueue.h"


/**
 *
 **/
typedef struct sample_analysis {
  vote_ensemble_t *ensemble;
  real_t           margin;
  real_t           timeout;
  real_t          *sample;
  size_t           label;

  struct timespec start_clock;
  struct timespec stop_clock;

  vote_outcome_t   outcome;
} sample_analysis_t;


/**
 *
 **/
typedef struct robustness_analysis {
  vote_ensemble_t *ensemble;
  real_t           sample_timeout;
  real_t           margin;
  size_t           threads;
  vote_dataset_t  *dataset;
} robustness_analysis_t;


/**
 * Calculate the time difference in seconds.
 **/
static real_t
timespec_diff(struct timespec *start, struct timespec *stop) {
  real_t sec = 0;
  real_t nsec = 0;

  if((stop->tv_nsec - start->tv_nsec) < 0) {
    sec = (real_t)stop->tv_sec - start->tv_sec - 1;
    nsec = (real_t)stop->tv_nsec - start->tv_nsec + 1000000000;
  } else {
    sec = (real_t)stop->tv_sec - start->tv_sec;
    nsec = (real_t)stop->tv_nsec - start->tv_nsec;
  }
  
  return sec + (nsec / 1e9);
}


/**
 * Check that a mapping maps to a specific label.
 **/
static vote_outcome_t
is_correct(void *ctx, vote_mapping_t *m) {
  struct timespec curr_clock;
  sample_analysis_t *a = (sample_analysis_t*)ctx;
  
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &curr_clock);
  
  if(timespec_diff(&a->start_clock, &curr_clock) > a->timeout) {
    a->outcome = VOTE_UNSURE;
    return VOTE_FAIL;
  }

  a->outcome = vote_mapping_check_argmax(m, a->label);

  return a->outcome;
}


/**
 * Analyze a given sample on a seperate thread.
 **/
static void
analyze_sample(void* ctx) {
  sample_analysis_t *a = (sample_analysis_t*)ctx;
  vote_bound_t bounds[a->ensemble->nb_inputs];

  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &a->start_clock);
  
  for(size_t i=0; i<a->ensemble->nb_inputs; i++) {
    bounds[i].lower = a->sample[i];
    bounds[i].upper = a->sample[i];
  }

  // don't bother with samples that are classified incorrectly
  if(vote_ensemble_absref(a->ensemble, bounds, is_correct, a)) {
    for(size_t i=0; i<a->ensemble->nb_inputs; i++) {
      bounds[i].lower -= a->margin;
      bounds[i].upper += a->margin;
    }
    vote_ensemble_absref(a->ensemble, bounds, is_correct, a);
  }

  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &a->stop_clock);
}


/**
 * Run the robustness analysis.
 **/
static void
analyze_robustness(robustness_analysis_t *a) {
  size_t nb_samples = a->dataset->nb_rows;
  sample_analysis_t analyses[nb_samples];
  workqueue_t *wq = workqueue_new();
  struct timespec start_clock;
  struct timespec stop_clock;
  
  for(size_t row=0; row<nb_samples; row++) {
    analyses[row].ensemble = a->ensemble;
    analyses[row].margin = a->margin;
    analyses[row].timeout = a->sample_timeout;
    
    analyses[row].sample = vote_dataset_row(a->dataset, row);
    analyses[row].label = (size_t)roundf(analyses[row].sample[a->ensemble->nb_inputs]);

    workqueue_schedule(wq, analyze_sample, &analyses[row]);
  }

  clock_gettime(CLOCK_REALTIME, &start_clock);
  workqueue_launch(wq, a->threads);
  clock_gettime(CLOCK_REALTIME, &stop_clock);

  real_t walltime = timespec_diff(&start_clock, &stop_clock);
  size_t passed = 0;
  size_t timeouts = 0;
  
  for(size_t row=0; row<nb_samples; row++) {
    passed += analyses[row].outcome == VOTE_PASS;
    timeouts += analyses[row].outcome == VOTE_UNSURE;
  }
  
  printf("robustness:dataset:    %s\n", a->dataset->filename);
  printf("robustness:margin:     %g\n", a->margin);
  printf("robustness:timeout:    %gs\n", a->sample_timeout);
  printf("robustness:nb_inputs:  %ld\n", a->ensemble->nb_inputs);
  printf("robustness:nb_outputs: %ld\n", a->ensemble->nb_outputs);
  printf("robustness:nb_trees:   %ld\n", a->ensemble->nb_trees);
  printf("robustness:nb_nodes:   %ld\n", a->ensemble->nb_nodes);
  printf("robustness:passed:     %ld\n", passed);
  printf("robustness:timeouts:   %ld\n", timeouts);

  if(timeouts) {
    printf("robustness:score:      [%g,%g]\n", (real_t)passed / nb_samples,
	   (real_t)(passed + timeouts) / nb_samples);
  } else {
    printf("robustness:score:      %g\n", (real_t)passed / nb_samples);
  }
  printf("robustness:runtime:    %gs\n", walltime);

  workqueue_del(wq);
}


/**
 * Parse command line arguments.
 **/
static error_t
parse_cb(int key, char *arg, struct argp_state *state) {
  robustness_analysis_t *a = state->input;
  
  switch(key) {
  case 'm': //model
    if(!(a->ensemble = vote_ensemble_load_file(arg))) {
      fprintf(stderr, "Unable to load model from %s\n", arg);
      return ARGP_ERR_UNKNOWN;
    }
    break;

  case 'M': //margin
    a->margin = atof(arg);
    break;

  case 'T': //timeout
    a->sample_timeout = atof(arg);
    break;

  case 't': //threads
    a->threads = atoi(arg);
    break;
    
  case ARGP_KEY_ARG: //CSV_FILE
    if(!(a->dataset = vote_csv_load(arg))) {
      fprintf(stderr, "Unable to load data from %s\n", arg);
      return ARGP_ERR_UNKNOWN;
    }
    break;

  case ARGP_KEY_END:
    if(state->arg_num < 1) {
      argp_usage(state);
    }
    break;
    
  default:
    return ARGP_ERR_UNKNOWN;
  }

  return 0;
}
  

/**
 * Parse command line arguments and launch the robustness analysis.
 **/
int
main(int argc, char** argv) {
  struct argp_option opts[] = {
    {.name="model", .key='m', .arg="PATH",
     .doc="Path to a serialized tree-based classifier"},

    {.name="margin", .key='M', .arg="NUMBER",
     .doc="The additive margin to which the classifier should be robust against"},

    {.name="threads", .key='t', .arg="NUMBER",
     .doc="Perform analyses concurrently on a given NUMBER of threads"},
    
    {.name="timeout", .key='T', .arg="NUMBER",
     .doc="Timeout the analysis of a sample after NUMBER seconds"},

    {0}
  };
  
  struct argp argp = {
    .parser   = parse_cb,
    .doc      = "Verify the robustness of a tree-based classifier against input "
                "perturbations to a set of samples stored in the CSV format.",
    .args_doc = "CSV_FILE",
    .options  = opts
  };

  struct robustness_analysis a = {
    .sample_timeout = UINT_MAX,
    .threads = sysconf(_SC_NPROCESSORS_ONLN)
  };
    
  if(argp_parse(&argp, argc, argv, 0, 0, &a)) {
    exit(1);
  }

  analyze_robustness(&a);

  if(a.ensemble) {
    vote_ensemble_del(a.ensemble);
  }

  if(a.dataset) {
    vote_dataset_del(a.dataset);
  }
}


/**
 * Accessed by argp_parse()
 **/
const char *argp_program_version = VERSION;
const char *argp_program_bug_address = PACKAGE_BUGREPORT;



