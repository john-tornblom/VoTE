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

#ifndef VOTE_PIPELINE_H
#define VOTE_PIPELINE_H


typedef struct vote_pipeline vote_pipeline_t;

typedef void (vote_pipeline_del_cb_t)(void *ctx);


/**
 * Create a new pipeline element with *on_input* and *on_delete* event handlers.
 **/
vote_pipeline_t* vote_pipeline_new(void *ctx, vote_mapping_cb_t *on_input,
				   vote_pipeline_del_cb_t *on_delete);


/**
 * Delete a pipeline element, including all succeeding pipeline elements.
 **/
void vote_pipeline_del(vote_pipeline_t *p);


/**
 * Connect two pipeline elements.
 **/
void vote_pipeline_connect(vote_pipeline_t *src, vote_pipeline_t *sink);


/**
 * Stimulate the input of a pipeline element with a mapping.
 **/
vote_outcome_t vote_pipeline_input(const vote_pipeline_t *p, vote_mapping_t *m);


/**
 * Stimulate the succeeding pipeline element with a mapping.
 **/
vote_outcome_t vote_pipeline_output(const vote_pipeline_t *p, vote_mapping_t *m);


#endif //VOTE_PIPELINE_H
