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
#include <vote.h>


typedef struct robustness_analysis {
  //user inputs
  vote_ensemble_t *ensemble;
  FILE            *output;
  size_t           timeout;
  real_t           margin;
  vote_dataset_t  *dataset;
  
  // intermediate data
  struct {
    time_t timer;
    size_t label;
  } current_sample;
  
  //analysis outputs
  size_t nb_passed;
  size_t nb_timeouts;
  time_t runtime;
} robustness_analysis_t;


/**
 * Check that a mapping maps to a specific label.
 **/
static vote_outcome_t
is_correct(void *ctx, vote_mapping_t *m) {
  vote_outcome_t o;
  robustness_analysis_t *a = (robustness_analysis_t*)ctx;

  if(time(NULL) > a->current_sample.timer) {
    a->nb_timeouts++;
    return VOTE_FAIL;
  }

  o = vote_mapping_check_argmax(m, a->current_sample.label);
  
  // serialize a counter-example
  if(o == VOTE_FAIL && a->output) {
    for(size_t i=0; i<m->nb_inputs; i++) {
      fprintf(a->output, "%1.17g,", m->inputs[i].lower +
	      (m->inputs[i].upper - m->inputs[i].lower) / 2);
    }
    fprintf(a->output, "%ld # predicted(%d)\n",
	    a->current_sample.label, vote_mapping_argmax(m));
  }
  
  return o;
}


/**
 * Run the robustness analysis.
 **/
static void
analyze_robustness(robustness_analysis_t *a) {
  vote_ensemble_t *e = a->ensemble;
  
  assert(a->dataset->nb_cols == e->nb_inputs + 1);

  a->runtime = time(NULL);
  for(size_t row=0; row<a->dataset->nb_rows; row++) {
    real_t *sample = vote_dataset_row(a->dataset, row);
    vote_bound_t bounds[e->nb_inputs];
    for(size_t i=0; i<e->nb_inputs; i++) {
      bounds[i].lower = sample[i];
      bounds[i].upper = sample[i];
    }

    a->current_sample.label = (size_t)roundf(sample[e->nb_inputs]);
    a->current_sample.timer = time(NULL) + a->timeout;

    // don't bother with samples that are classified incorrectly
    if(!vote_ensemble_absref(e, bounds, is_correct, a)) {
      continue;
    }

    //apply perturbation to input
    for(size_t i=0; i<e->nb_inputs; i++) {
      bounds[i].lower -= a->margin;
      bounds[i].upper += a->margin;
    }

    a->nb_passed += vote_ensemble_absref(e, bounds, is_correct, a);
  }

  a->runtime = time(NULL) - a->runtime;
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

  case 'o': //output
    if(!(a->output = fopen(arg, "w"))) {
      fprintf(stderr, "Unable to write to %s\n", arg);
      return ARGP_ERR_UNKNOWN;
    }
    break;

  case 'T': //timeout
    a->timeout = atof(arg);
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
     .doc="The addative margin to which the classifier should be robust against"},

    {.name="output", .key='o', .arg="PATH",
     .doc="Output counter examples in the CSV format to PATH"},

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
    .timeout = UINT_MAX
  };
    
  if(argp_parse(&argp, argc, argv, 0, 0, &a)) {
    exit(1);
  }

  printf("robustness:nb_inputs:  %ld\n", a.ensemble->nb_inputs);
  printf("robustness:nb_outputs: %ld\n", a.ensemble->nb_outputs);
  printf("robustness:nb_trees:   %ld\n", a.ensemble->nb_trees);
  printf("robustness:nb_nodes:   %ld\n", a.ensemble->nb_nodes);
  printf("robustness:margin:     %f\n",  a.margin);
  printf("robustness:nb_samples: %ld\n", a.dataset->nb_rows);
  
  analyze_robustness(&a);

  printf("robustness:passed:     %ld\n", a.nb_passed);
  printf("robustness:timeouts:   %ld\n", a.nb_timeouts);
  if(a.nb_timeouts) {
    printf("robustness:score:      [%f,%f]\n", (real_t)a.nb_passed / a.dataset->nb_rows,
	   (real_t)(a.nb_passed + a.nb_timeouts) / a.dataset->nb_rows);
  } else {
    printf("robustness:score:      %f\n", (real_t)a.nb_passed / a.dataset->nb_rows);
  }
  printf("robustness:runtime:    %lds\n", a.runtime);

  if(a.output) {
    fclose(a.output);
  }

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



