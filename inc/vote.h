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
 * Callback function prototype used to iterate input/output mappings.
 * returning false cancels iterations.
 **/
typedef bool (vote_mapping_cb_t)(void *ctx, vote_mapping_t *mapping);


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
 * Join two mappings.
 **/
void vote_mapping_join(const vote_mapping_t *m, vote_mapping_t *j);


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
vote_ensemble_t *vote_ensemble_load(const char *filename);


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
 * Iterate all feasible mappings of an ensemble for some input region
 * until the callback function returns false, or all mappings
 * have been iterated.
 *
 * Returns true if all mappings were accepted by the callback function, or false
 * if the iteration was canceled.
 **/
bool vote_ensemble_forall(const vote_ensemble_t *f, const vote_bound_t* input_region,
			  vote_mapping_cb_t *cb, void* ctx);


/**
 * Approximate a pessimistic and sound mapping for a given input region.
 **/
vote_mapping_t *vote_ensemble_approximate(const vote_ensemble_t *f,
					  const vote_bound_t* input_region);


#endif //VOTE_H
