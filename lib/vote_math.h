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


#ifndef VOTE_MATH_H
#define VOTE_MATH_H

#include <math.h>

#include "vote.h"


#if USE_DOUBLE
#define vote_nextafter(x, y) nextafter(x, y)
#define vote_max(x, y)       fmax(x, y)
#define vote_min(x, y)       fmin(x, y)
#define vote_exp(x)          exp(x)
#define vote_log(x)          log(x)
#else
#define vote_nextafter(x, y) nextafterf(x, y)
#define vote_max(x, y)       fmaxf(x, y)
#define vote_min(x, y)       fminf(x, y)
#define vote_exp(x)          expf(x)
#define vote_log(x)          logf(x)
#endif


#endif //VOTE_MATH_H
