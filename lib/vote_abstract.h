/* Copyright (C) 2019 John Törnblom

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

#ifndef VOTE_ABSTRACT_H
#define VOTE_ABSTRACT_H


#include "vote.h"
#include "vote_tree.h"
#include "vote_pipeline.h"


/**
 * Compute the join of a tree for a particular input region.
 **/
void vote_abstract_join_tree(const vote_tree_t *t,
			     const vote_bound_t *inputs, size_t nb_inputs,
			     vote_bound_t *outputs, size_t nb_outputs);


/**
 * Compute the join of a set of trees for a particular input region.
 **/
void
vote_abstract_join_trees(vote_tree_t *const*trees, size_t nb_trees,
			 const vote_bound_t *inputs, size_t nb_inputs,
			 vote_bound_t *outputs, size_t nb_outputs);


/**
 *
 **/
vote_pipeline_t* vote_abstract_pipeline(vote_tree_t *const*trees, size_t nb_trees,
					const vote_pipeline_t *postproc);
  

#endif //VOTE_ABSTRACT_H
