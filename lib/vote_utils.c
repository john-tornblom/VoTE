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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "vote.h"


size_t
vote_argmax(const real_t* fvec, size_t length) {
  size_t m = 0;

  for(size_t i=1; i<length; i++) {
    if(fvec[i] > fvec[m]) {
      m = i;
    }
  }

  return m;
}


size_t
vote_argmin(const real_t* fvec, size_t length) {
  size_t m = 0;

  for(size_t i=1; i<length; i++) {
    if(fvec[i] < fvec[m]) {
      m = i;
    }
  }

  return m;
}


void
vote_normalize(real_t* vec, size_t length) {
  real_t sum = 0;

  for(size_t i=0; i<length; i++) {
    sum += vec[i];
  }

  assert(sum != 0);
  
  for(size_t i=0; i<length; i++) {
    vec[i] /= sum;
  }
}


const char*
vote_version(void) {
  return VERSION;
}
