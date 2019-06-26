/* Copyright (C) 2019 John Törnblom

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


#include <stdio.h>
#include <vote.h>


/**
 * Convert an xgboost model to the VoTE json format.
 **/
int main(int argc, char** argv) {
  if(argc < 3) {
    printf("usage: %s <xgboost input file> <vote output file>\n", argv[0]);
    return 1;
  }

  vote_ensemble_t *e = vote_xgboost_load_file(argv[1]);
  bool b = vote_ensemble_save_file(e, argv[2]);
  
  vote_ensemble_del(e);

  return !b;
}

