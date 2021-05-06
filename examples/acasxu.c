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


#include <argp.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <vote.h>


/**
 * Inputs
 **/
#define RHO   0
#define THETA 1
#define PSI   2
#define V_OWN 3
#define V_INT 4


/**
 * Outputs
 */
#define COC          0
#define WEAK_LEFT    1
#define WEAK_RIGHT   2
#define STRONG_LEFT  3
#define STRONG_RIGHT 4


/**
 * Constants
 **/
#define PI 3.141592


typedef struct acasxu_analysis {
  vote_ensemble_t *ensemble;
  uint8_t          property;
} acasxu_analysis_t;


static vote_outcome_t
check_property_1(void *ctx, vote_mapping_t *m) {
  VOTE_UNUSED(ctx);
  
  assert(m->inputs[RHO].lower >= 55947.691);
  assert(m->inputs[V_OWN].lower >= 1145);
  assert(m->inputs[V_INT].upper <= 60);
  
  if(m->outputs[COC].upper <= 1500) {
    return VOTE_PASS;
  }

  if(m->outputs[COC].lower > 1500) {
    return VOTE_FAIL;
  }

  return VOTE_UNSURE;
}


static vote_outcome_t
check_property_2(void *ctx, vote_mapping_t *m) {
  VOTE_UNUSED(ctx);
  
  assert(m->inputs[RHO].lower >= 55947.691);
  assert(m->inputs[V_OWN].lower >= 1145);
  assert(m->inputs[V_INT].upper <= 60);

  //check argmax != COC
  vote_outcome_t o = vote_mapping_check_argmax(m, COC);
  if(o == VOTE_PASS) return VOTE_FAIL;
  if(o == VOTE_FAIL) return VOTE_PASS;

  return VOTE_UNSURE;
}


static vote_outcome_t
check_property_3(void *ctx, vote_mapping_t *m) {
  VOTE_UNUSED(ctx);
  
  assert(m->inputs[RHO].lower >= 1500);
  assert(m->inputs[RHO].upper <= 1800);
  assert(m->inputs[THETA].lower >= -0.06);
  assert(m->inputs[THETA].upper <= 0.06);
  assert(m->inputs[PSI].lower >= 3.10);
  assert(m->inputs[V_OWN].lower >= 980);
  assert(m->inputs[V_INT].lower >= 960);

  //check argmin != COC
  vote_outcome_t o = vote_mapping_check_argmin(m, COC);
  if(o == VOTE_PASS) return VOTE_FAIL;
  if(o == VOTE_FAIL) return VOTE_PASS;

  return VOTE_UNSURE;
}


static vote_outcome_t
check_property_4(void *ctx, vote_mapping_t *m) {
  VOTE_UNUSED(ctx);
  
  assert(m->inputs[RHO].lower >= 1500);
  assert(m->inputs[RHO].upper <= 1800);
  assert(m->inputs[THETA].lower >= -0.06);
  assert(m->inputs[THETA].upper <= 0.06);
  assert(m->inputs[PSI].lower == 0);
  assert(m->inputs[PSI].upper == 0);
  assert(m->inputs[V_OWN].lower >= 1000);
  assert(m->inputs[V_INT].lower >= 700);
  assert(m->inputs[V_INT].upper <= 800);
  
  //check argmin != COC
  vote_outcome_t o = vote_mapping_check_argmin(m, COC);
  if(o == VOTE_PASS) return VOTE_FAIL;
  if(o == VOTE_FAIL) return VOTE_PASS;

  return VOTE_UNSURE;
}


static vote_outcome_t
check_property_5(void *ctx, vote_mapping_t *m) {
  VOTE_UNUSED(ctx);
  
  assert(m->inputs[RHO].lower >= 250);
  assert(m->inputs[RHO].upper <= 400);
  assert(m->inputs[THETA].lower >= 0.2);
  assert(m->inputs[THETA].upper <= 0.4);
  assert(m->inputs[PSI].lower >= -PI);
  assert(m->inputs[PSI].upper <= -PI + 0.005);
  assert(m->inputs[V_OWN].lower >= 100);
  assert(m->inputs[V_OWN].upper <= 400);
  assert(m->inputs[V_INT].lower >= 0);
  assert(m->inputs[V_INT].upper <= 400);
  
  return vote_mapping_check_argmin(m, STRONG_RIGHT);
}


static vote_outcome_t
check_property_6_a(void *ctx, vote_mapping_t *m) {
  VOTE_UNUSED(ctx);
  
  assert(m->inputs[RHO].lower >= 12000);
  assert(m->inputs[RHO].upper <= 62000);
  assert(m->inputs[THETA].lower >= 0.7);
  assert(m->inputs[THETA].upper <= PI);
  assert(m->inputs[PSI].lower >= -PI);
  assert(m->inputs[PSI].upper <= -PI + 0.005);
  assert(m->inputs[V_OWN].lower >= 100);
  assert(m->inputs[V_OWN].upper <= 1200);
  assert(m->inputs[V_INT].lower >= 0);
  assert(m->inputs[V_INT].upper <= 1200);
  
  return vote_mapping_check_argmin(m, COC);
}


static vote_outcome_t
check_property_6_b(void *ctx, vote_mapping_t *m) {
  VOTE_UNUSED(ctx);
  
  assert(m->inputs[RHO].lower >= 12000);
  assert(m->inputs[RHO].upper <= 62000);
  assert(m->inputs[THETA].lower >= -PI);
  assert(m->inputs[THETA].upper <= -0.7);
  assert(m->inputs[PSI].lower >= -PI);
  assert(m->inputs[PSI].upper <= -PI + 0.005);
  assert(m->inputs[V_OWN].lower >= 100);
  assert(m->inputs[V_OWN].upper <= 1200);
  assert(m->inputs[V_INT].lower >= 0);
  assert(m->inputs[V_INT].upper <= 1200);
  
  return vote_mapping_check_argmin(m, COC);
}


static vote_outcome_t
check_property_7_a(void *ctx, vote_mapping_t *m) {
  VOTE_UNUSED(ctx);
  
  assert(m->inputs[RHO].lower >= 0);
  assert(m->inputs[RHO].upper <= 60760);
  assert(m->inputs[THETA].lower >= -PI);
  assert(m->inputs[THETA].upper <= PI);
  assert(m->inputs[PSI].lower >= -PI);
  assert(m->inputs[PSI].upper <= PI);
  assert(m->inputs[V_OWN].lower >= 100);
  assert(m->inputs[V_OWN].upper <= 1200);
  assert(m->inputs[V_INT].lower >= 0);
  assert(m->inputs[V_INT].upper <= 1200);

  //check that argmin != strong right
  vote_outcome_t o = vote_mapping_check_argmin(m, STRONG_RIGHT);
  if(o == VOTE_FAIL) return VOTE_PASS;
  if(o == VOTE_PASS) return VOTE_FAIL;

  return VOTE_UNSURE;
}


static vote_outcome_t
check_property_7_b(void *ctx, vote_mapping_t *m) {
  VOTE_UNUSED(ctx);
  
  assert(m->inputs[RHO].lower >= 0);
  assert(m->inputs[RHO].upper <= 60760);
  assert(m->inputs[THETA].lower >= -PI);
  assert(m->inputs[THETA].upper <= PI);
  assert(m->inputs[PSI].lower >= -PI);
  assert(m->inputs[PSI].upper <= PI);
  assert(m->inputs[V_OWN].lower >= 100);
  assert(m->inputs[V_OWN].upper <= 1200);
  assert(m->inputs[V_INT].lower >= 0);
  assert(m->inputs[V_INT].upper <= 1200);

  //check that argmin != strong left
  vote_outcome_t o = vote_mapping_check_argmin(m, STRONG_LEFT);
  if(o == VOTE_FAIL) return VOTE_PASS;
  if(o == VOTE_PASS) return VOTE_FAIL;

  return VOTE_UNSURE;
}


static vote_outcome_t
check_property_8(void *ctx, vote_mapping_t *m) {
  VOTE_UNUSED(ctx);
  
  assert(m->inputs[RHO].lower >= 0);
  assert(m->inputs[RHO].upper <= 60760);
  assert(m->inputs[THETA].lower >= -PI);
  assert(m->inputs[THETA].upper <= -0.75 * PI);
  assert(m->inputs[PSI].lower >= -0.1);
  assert(m->inputs[PSI].upper <= 0.1);
  assert(m->inputs[V_OWN].lower >= 600);
  assert(m->inputs[V_OWN].upper <= 1200);
  assert(m->inputs[V_INT].lower >= 600);
  assert(m->inputs[V_INT].upper <= 1200);
  
  vote_outcome_t o1 = vote_mapping_check_argmin(m, WEAK_LEFT);
  vote_outcome_t o2 = vote_mapping_check_argmin(m, COC);

  if(o1 == VOTE_PASS || o2 == VOTE_PASS) {
    return VOTE_PASS;
  }

  if(o1 == VOTE_FAIL || o2 == VOTE_FAIL) {
    return VOTE_FAIL;
  }

  return VOTE_UNSURE;
}



static vote_outcome_t
check_property_9(void *ctx, vote_mapping_t *m) {
  VOTE_UNUSED(ctx);
  
  assert(m->inputs[RHO].lower >= 2000);
  assert(m->inputs[RHO].upper <= 7000);
  assert(m->inputs[THETA].lower >= -0.4);
  assert(m->inputs[THETA].upper <= -0.14);
  assert(m->inputs[PSI].lower >= -PI);
  assert(m->inputs[PSI].upper <= -PI + 0.01);
  assert(m->inputs[V_OWN].lower >= 100);
  assert(m->inputs[V_OWN].upper <= 150);
  assert(m->inputs[V_INT].lower >= 0);
  assert(m->inputs[V_INT].upper <= 150);
  
  return vote_mapping_check_argmin(m, STRONG_LEFT);
}



static vote_outcome_t
check_property_10(void *ctx, vote_mapping_t *m) {
  VOTE_UNUSED(ctx);
  
  assert(m->inputs[RHO].lower >= 36000);
  assert(m->inputs[RHO].upper <= 60760);
  assert(m->inputs[THETA].lower >= 0.7);
  assert(m->inputs[THETA].upper <= PI);
  assert(m->inputs[PSI].lower >= -PI);
  assert(m->inputs[PSI].upper <= -PI + 0.01);
  assert(m->inputs[V_OWN].lower >= 900);
  assert(m->inputs[V_OWN].upper <= 1200);
  assert(m->inputs[V_INT].lower >= 600);
  assert(m->inputs[V_INT].upper <= 1200);
  
  return vote_mapping_check_argmin(m, COC);
}


static bool
analyze_acasxu(acasxu_analysis_t *a) {
  vote_ensemble_t *e = a->ensemble;
  vote_bound_t domain[e->nb_inputs];
  
  for(size_t i=0; i<e->nb_inputs; i++) {
    domain[i].lower = -VOTE_INFINITY;
    domain[i].upper = VOTE_INFINITY;
  }
  
  switch(a->property) {
  case 1:
    domain[RHO].lower = 55947.691;
    domain[V_OWN].lower = 1145;
    domain[V_INT].upper = 60;

    return vote_ensemble_absref(e, domain, check_property_1, 0);
    
  case 2:
    domain[RHO].lower = 55947.691;
    domain[V_OWN].lower = 1145;
    domain[V_INT].upper = 60;

    return vote_ensemble_absref(e, domain, check_property_2, 0);

  case 3:
    domain[RHO].lower = 1500;
    domain[RHO].upper = 1800;
    domain[THETA].lower = -0.06;
    domain[THETA].upper = 0.06;
    domain[PSI].lower = 3.1;
    domain[V_OWN].lower = 1980;
    domain[V_INT].lower = 960;

    return vote_ensemble_absref(e, domain, check_property_3, 0);

  case 4:
    domain[RHO].lower = 1500;
    domain[RHO].upper = 1800;
    domain[THETA].lower = -0.06;
    domain[THETA].upper = 0.06;
    domain[PSI].lower = 0;
    domain[PSI].upper = 0;
    domain[V_OWN].lower = 1000;
    domain[V_INT].lower = 700;
    domain[V_INT].upper = 800;
    
    return vote_ensemble_absref(e, domain, check_property_4, 0);

  case 5:
    domain[RHO].lower = 250;
    domain[RHO].upper = 400;
    domain[THETA].lower = 0.2;
    domain[THETA].upper = 0.4;
    domain[PSI].lower = -PI;
    domain[PSI].upper = -PI + 0.005;
    domain[V_OWN].lower = 100;
    domain[V_OWN].upper = 400;
    domain[V_INT].lower = 0;
    domain[V_INT].upper = 400;
    
    return vote_ensemble_absref(e, domain, check_property_5, 0);

  case 6:
    domain[RHO].lower = 12000;
    domain[RHO].upper = 62000;
    domain[THETA].lower = 0.7;
    domain[THETA].upper = PI;
    domain[PSI].lower = -PI;
    domain[PSI].upper = -PI + 0.005;
    domain[V_OWN].lower = 100;
    domain[V_OWN].upper = 1200;
    domain[V_INT].lower = 0;
    domain[V_INT].upper = 1200;
    
    if(!vote_ensemble_absref(e, domain, check_property_6_a, 0)) {
      return false;
    }

    domain[THETA].lower = -PI;
    domain[THETA].upper = -0.7;

    return vote_ensemble_absref(e, domain, check_property_6_b, 0);

  case 7:
    domain[RHO].lower = 0;
    domain[RHO].upper = 60760;
    domain[THETA].lower = -PI;
    domain[THETA].upper = PI;
    domain[PSI].lower = -PI;
    domain[PSI].upper = PI;
    domain[V_OWN].lower = 100;
    domain[V_OWN].upper = 1200;
    domain[V_INT].lower = 0;
    domain[V_INT].upper = 1200;
    
    return (vote_ensemble_absref(e, domain, check_property_7_a, 0) &&
	    vote_ensemble_absref(e, domain, check_property_7_b, 0));

  case 8:
    domain[RHO].lower = 0;
    domain[RHO].upper = 60760;
    domain[THETA].lower = -PI;
    domain[THETA].upper = -0.75 * PI;
    domain[PSI].lower = -0.1;
    domain[PSI].upper = 0.1;
    domain[V_OWN].lower = 600;
    domain[V_OWN].upper = 1200;
    domain[V_INT].lower = 600;
    domain[V_INT].upper = 1200;
    
    return vote_ensemble_absref(e, domain, check_property_8, 0);

  case 9:
    domain[RHO].lower = 2000;
    domain[RHO].upper = 7000;
    domain[THETA].lower = -0.4;
    domain[THETA].upper = -0.14;
    domain[PSI].lower = -PI;
    domain[PSI].upper = -PI + 0.01;
    domain[V_OWN].lower = 100;
    domain[V_OWN].upper = 150;
    domain[V_INT].lower = 0;
    domain[V_INT].upper = 150;
    
    return vote_ensemble_absref(e, domain, check_property_9, 0);

  case 10:
    domain[RHO].lower = 36000;
    domain[RHO].upper = 60760;
    domain[THETA].lower = 0.7;
    domain[THETA].upper = PI;
    domain[PSI].lower = -PI;
    domain[PSI].upper = -PI + 0.01;
    domain[V_OWN].lower = 900;
    domain[V_OWN].upper = 1200;
    domain[V_INT].lower = 600;
    domain[V_INT].upper = 1200;
    
    return vote_ensemble_absref(e, domain, check_property_10, 0);

  default:
    fprintf(stderr, "Unknown property %d\n", a->property);
    return false;
  }


}


static error_t
parse_cb(int key, char *arg, struct argp_state *state) {
  acasxu_analysis_t *a = state->input;
  
  switch(key) {
  case 'p': //property
    a->property = atoi(arg);
    if(a->property < 1 || a->property > 10) {
      fprintf(stderr, "Unknown value of property (φ=%d)\n", a->property);
      return ARGP_ERR_UNKNOWN;
    }
    break;
    
  case ARGP_KEY_ARG: //PATH
    if(!(a->ensemble = vote_ensemble_load_file(arg))) {
      fprintf(stderr, "Unable to load model from %s\n", arg);
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


int main(int argc, char* argv[]) {
  struct argp_option opts[] = {
    {.name="property", .key='p', .arg="NUMBER",
     .doc="The property to verify (φ)"},
 
    {0}
  };
  
  struct argp argp = {
    .parser   = parse_cb,
    .doc      = "Verify a tree ensemble trained on the Reluplex ACAS Xu "
                "case-study",
    .args_doc = "PATH",
    .options  = opts
  };

  struct acasxu_analysis a = {
     .ensemble = NULL,
     .property = 0
  };
    
  if(argp_parse(&argp, argc, argv, 0, 0, &a)) {
    exit(1);
  }

  bool b = analyze_acasxu(&a);

  if(a.ensemble) {
    vote_ensemble_del(a.ensemble);
  }
  
  return b;
}

