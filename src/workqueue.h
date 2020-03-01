/* Copyright (C) 2020 John Törnblom

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


#ifndef WORKQUEUE_H
#define WORKQUEUE_H


/**
 *
 **/
typedef struct workqueue workqueue_t;


/**
 *
 **/
typedef void (workqueue_cb_t)(void *ctx);


/**
 *
 **/
workqueue_t *workqueue_new();


/**
 *
 **/
void workqueue_schedule(workqueue_t *wq, workqueue_cb_t *cb, void *ctx);


/**
 *
 **/
void workqueue_launch(workqueue_t* wq, unsigned short nb_threads);


/**
 *
 **/
void workqueue_del(workqueue_t *wq);


#endif //WORKQUEUE_H
