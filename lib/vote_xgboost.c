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

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vote.h"
#include "vote_tree.h"



typedef struct LearnerModelParam {
  float    base_score;
  unsigned num_feature;
  int      num_class;
  int      contain_extra_attrs;
  int      contain_eval_metrics;
  int      reserved[29];
} xgboost_learn_param_t;


typedef struct GBTreeModelParam {
  int     num_trees;
  int     num_roots;
  int     num_feature;
  int     pad_32bit;
  int64_t num_pbuffer_deprecated;
  int     num_output_group;
  int     size_leaf_vector;
  int     reserved[32];
} xgboost_model_param_t;


typedef struct TreeParam {
  int num_roots;
  int num_nodes;
  int num_deleted;
  int max_depth;
  int num_feature;
  int size_leaf_vector;
  int reserved[31];
} xgboost_tree_param_t;


typedef struct Node {
  int      parent;
  int      cleft;
  int      cright;
  unsigned sindex;
  float    value;
} xgboost_node_t;


typedef struct RTreeNodeStat {
  float loss_chg;
  float sum_hess;
  float base_weight;
  int   leaf_child_cnt;
} xgboost_node_stat_t;


static vote_ensemble_t*
vote_xgboost_load(FILE* f) {
  xgboost_learn_param_t learn_param;
  xgboost_model_param_t model_param;
  xgboost_tree_param_t tree_param;
  xgboost_node_stat_t node_stat;
  xgboost_node_t node;
  size_t booster_size, objective_size;
  char *objective, *booster;
  size_t read_size;
  char header[4];
  vote_tree_t* t;
  vote_ensemble_t *e;
  
  // header
  read_size = fread(header, sizeof(char), 4, f);
  assert(read_size == 4);

  if(memcmp(header, "binf", 4)) {
    rewind(f);
  }
  
  // LearnerModelParam
  read_size = fread(&learn_param, sizeof(learn_param), 1, f);
  assert(read_size == 1);

  // objective function
  read_size = fread(&objective_size, sizeof(objective_size), 1, f);
  assert(read_size == 1);

  objective = calloc(objective_size + 1, sizeof(char));
  assert(objective);
  
  read_size = fread(objective, sizeof(char), objective_size, f);
  assert(read_size == objective_size);

  // booster type
  read_size = fread(&booster_size, sizeof(booster_size), 1, f);
  assert(read_size == 1);
  
  booster = calloc(booster_size + 1, sizeof(char));
  assert(booster);
  
  read_size = fread(booster, sizeof(char), booster_size, f);
  assert(read_size == booster_size);

  // GBTreeModelParam
  read_size = fread(&model_param, sizeof(model_param), 1, f);
  assert(read_size == 1);

  e = calloc(1, sizeof(vote_ensemble_t));
  assert(e);

  if(strstr(objective, "reg:")) {
    e->nb_outputs = 1;
    e->post_process = VOTE_POST_PROCESS_NONE;
    
  } else if(strstr(objective, "binary:logistic")) {
    e->nb_outputs = 1;
    e->post_process = VOTE_POST_PROCESS_SIGMOID;
    
  } else if(strstr(objective, "multi:softprob")) {
    e->nb_outputs = learn_param.num_class;
    e->post_process = VOTE_POST_PROCESS_SOFTMAX;
    
  } else if(strstr(objective, "multi:softmax")) {
    e->nb_outputs = learn_param.num_class;
    e->post_process = VOTE_POST_PROCESS_SOFTMAX;
    
  } else {
    abort();
  }

  assert(learn_param.num_feature == (unsigned int)model_param.num_feature);
  
  e->nb_inputs = model_param.num_feature;
  e->nb_trees  = model_param.num_trees;
  e->trees     = calloc(e->nb_trees, sizeof(vote_tree_t*));
  assert(e->trees);

  // TreeParam
  for(int i=0; i<model_param.num_trees; i++) {
    read_size = fread(&tree_param, sizeof(tree_param), 1, f);
    assert(read_size == 1);

    t = e->trees[i] = calloc(1, sizeof(vote_tree_t));
    assert(t);

    t->nb_inputs  = tree_param.num_feature;
    t->nb_nodes   = tree_param.num_nodes;
    t->nb_outputs = e->nb_outputs;

    assert(t->nb_inputs == e->nb_inputs);

    t->left = calloc(t->nb_nodes, sizeof(int));
    assert(t->left);

    t->right = calloc(t->nb_nodes, sizeof(int));
    assert(t->right);

    t->feature = calloc(t->nb_nodes, sizeof(int));
    assert(t->feature);
    
    t->threshold = calloc(t->nb_nodes, sizeof(real_t));
    assert(t->threshold);

    t->value = calloc(t->nb_nodes, sizeof(real_t**));
    assert(t->value);
    
    // Node
    for(int j=0; j<tree_param.num_nodes; j++) {
      read_size = fread(&node, sizeof(node), 1, f);
      assert(read_size == 1);

      if(!i) {
	//node.value += learn_param.base_score;
      }

      t->left[j]      = node.cleft;
      t->right[j]     = node.cright;
      t->feature[j]   = -1;
      t->threshold[j] = 0;
      t->value[j]     = calloc(t->nb_outputs, sizeof(real_t));
      
      // leaf node
      if(node.cleft == -1) {
	switch(t->nb_outputs) {
	case 1:
	  t->value[j][0] = (real_t)node.value;
	  break;

	default:
	  t->value[j][i ? i % t->nb_outputs : 0] = (real_t)node.value;
	  break;
	}
      } else {
	t->threshold[j] = (real_t)node.value;
	t->feature[j]   = node.sindex & ((1U << 31) - 1U);
      }
    }
    
    // RTreeNodeStat
    for(int j=0; j<tree_param.num_nodes; j++) {
      read_size = fread(&node_stat, sizeof(node_stat), 1, f);
      assert(read_size == 1);
    }

    e->nb_nodes += t->nb_nodes;
  }

  return e;
}


vote_ensemble_t*
vote_xgboost_load_file(const char *filename) {
  vote_ensemble_t* e;
  FILE *f = fopen(filename, "rb");
  assert(f);

  e = vote_xgboost_load(f);
  fclose(f);
  
  return e;
}


vote_ensemble_t*
vote_xgboost_load_blob(void *data, size_t size) {
  vote_ensemble_t* e ;
  FILE *f = fmemopen(data, size, "rb");
  assert(f);

  e = vote_xgboost_load(f);
  fclose(f);
  
  return e;
}
