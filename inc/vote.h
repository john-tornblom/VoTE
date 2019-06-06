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


#ifndef VOTE_H
#define VOTE_H

#include <stddef.h>
#include <stdbool.h>


#define VOTE_UNUSED(x) (void)(x)
#define USE_DOUBLE 0
typedef float real_t;


/**
 * The bound of a variable, i.e. its range.
 **/
typedef struct vote_bound {
  real_t lower;
  real_t upper;
} vote_bound_t;


/**
 * A mapping from an input region to an output range.
 **/
typedef struct vote_mapping {
  vote_bound_t* inputs;
  vote_bound_t* outputs;
  size_t        nb_inputs;
  size_t        nb_outputs;
} vote_mapping_t;


/**
 * Forward declaration of a tree, only used internally by VoTE.
 **/
struct vote_tree;
typedef struct vote_tree vote_tree_t;


/**
 * Post process an ensemble with an algorithm that differentiates e.g.
 * the random forest models from gradient boosting models during prediction.
 **/
typedef enum vote_post_process {
  VOTE_POST_PROCESS_NONE,
  VOTE_POST_PROCESS_DIVISOR,
  VOTE_POST_PROCESS_SOFTMAX
} vote_post_process_t;


/**
 * An ensemble is a collection of trees.
 **/
typedef struct vote_ensemble {
  vote_tree_t       **trees;
  size_t              nb_trees;
  size_t              nb_inputs;
  size_t              nb_outputs;
  size_t              nb_nodes;
  vote_post_process_t post_process;
} vote_ensemble_t;


/**
 * The outcome of a property checker can be inconclusive when approximations
 * are too conservative.
 **/
typedef enum vote_outcome {
  VOTE_UNSURE = -1,
  VOTE_FAIL   = 0,
  VOTE_PASS   = 1
} vote_outcome_t;


/**
 * Callback function prototype used to iterate input/output mappings.
 * Returning a conclusive outcome (pass/fail) stops the iterations.
 **/
typedef vote_outcome_t (vote_mapping_cb_t)(void *ctx, vote_mapping_t *mapping);


/**
 * Obtain the version number of VoTE.
 **/
const char* vote_version(void);


/**
 * Compute the argmax of a vector with reals.
 **/
size_t vote_argmax(const real_t* fvec, size_t length);


/**
 * Compute the argmin of a vector with reals.
 **/
size_t vote_argmin(const real_t* fvec, size_t length);


/**
 * Load a CSV file into memory. 
 *
 * A correctly formatted CSV file (using the comma delimiter) without
 * a header is assumed.
 **/
bool vote_load_csv(const char* filename, real_t **data,
		   size_t *nb_rows, size_t *nb_cols);


/**
 * Create a new mapping with the given input/output dimensions. Input bounds
 * are initialized to [-∞, ∞], and output bounds are initialized to [0, 0].
 **/
vote_mapping_t* vote_mapping_new(size_t input_dim, size_t output_dim);


/**
 * Create a deep copy of a mapping.
 **/
vote_mapping_t* vote_mapping_copy(const vote_mapping_t* m);


/**
 * Check if the argmax of a mapping is as expected.
 **/
vote_outcome_t vote_mapping_check_argmax(const vote_mapping_t* m, size_t expected);

/**
 * Check if the argmin of a mapping is as expected.
 **/
vote_outcome_t vote_mapping_check_argmin(const vote_mapping_t* m, size_t expected);


/**
 * Check if a mapping is precise (lower and upper output bounds).
 **/
bool vote_mapping_precise(const vote_mapping_t* m);


/**
 * Compute the argmax of a mapping.
 *
 * Note: Negative one (-1) indicate an inconclusive mapping.
 **/
int vote_mapping_argmax(const vote_mapping_t* m);


/**
 * Compute the argmin of a mapping.
 *
 * Note: Negative one (-1) indicate an inconclusive mapping.
 **/
int vote_mapping_argmin(const vote_mapping_t* m);


/**
 * Delete a mapping, including its bounds.
 **/
void vote_mapping_del(vote_mapping_t* m);


/**
 * Load an ensemble from disk persisted in a JSON-based format.
 **/
vote_ensemble_t *vote_ensemble_load_file(const char *filename);


/**
 * Load an ensemble from a a JSON-based formated string.
 **/
vote_ensemble_t *vote_ensemble_load_string(const char *filename);


/**
 * Delete an ensemble and all of its trees.
 **/
void vote_ensemble_del(vote_ensemble_t* f);


/**
 * Evaluate an ensemble on concrete values.
 **/
void vote_ensemble_eval(const vote_ensemble_t* f, const real_t *inputs,
			real_t *outputs);


/**
 * Iterate all feasible mappings of an ensemble for some input region.
 *
 * Returns true if all mappings were satisified, and false if any were unsatisfied.
 **/
bool vote_ensemble_forall(const vote_ensemble_t *f, const vote_bound_t* input_region,
			  vote_mapping_cb_t *cb, void* ctx);


/**
 * Iterate abstract mappings of an ensemble using a abstraction-refinement approach
 * for some input region.
 *
 * Returns true if all mappings were satisified, and false if any were unsatisfied.
 **/
bool vote_ensemble_absref(const vote_ensemble_t *f, const vote_bound_t* input_region,
			  vote_mapping_cb_t *cb, void* ctx);


/**
 * Approximate a pessimistic and sound mapping for a given input region.
 **/
vote_mapping_t *vote_ensemble_approximate(const vote_ensemble_t *f,
					  const vote_bound_t* input_region);


#endif //VOTE_H
