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

#ifndef VOTE_POSTPROC_H
#define VOTE_POSTPROC_H


#include "vote.h"
#include "vote_pipeline.h"

/**
 * Apply the post-processing function to the outputs of a tree ensemble.
 **/
void vote_ensemble_postproc(const vote_ensemble_t *e, vote_bound_t *outputs);


/**
 * Create a post-processing component for a pipeline.
 **/
vote_pipeline_t* vote_postproc_pipeline(const vote_ensemble_t *e, void *user_ctx,
					vote_mapping_cb_t *user_cb);
  

#endif //VOTE_POSTPROC_H
