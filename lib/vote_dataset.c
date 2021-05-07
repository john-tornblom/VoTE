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


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#include "vote.h"


/**
 * List of tokens from the CSV format
 **/
typedef enum csv_token {
  TOK_INVALID,
  TOK_DELIMITER,
  TOK_STRING,
  TOK_NUMBER,
  TOK_COMMENT,
  TOK_LINEBREAK,
  TOK_EOF,
} csv_token_t;


/**
 * Keep track of the tokenizer state.
 */
typedef struct csv_tokenizer {
  csv_token_t  type;
  char*        value;
  unsigned int value_size;
  unsigned int value_pos;
  
  int   la;
  FILE* fp;
} csv_tokenizer_t;


/**
 * Keep track of the parser state.
 */
typedef struct csv_parser {
  real_t *memory;
  size_t  length;
  
  size_t width;
  size_t height;

  size_t curr_off;
  size_t curr_col;
} csv_parser_t;


typedef bool (*csv_token_cb)(void *ctx, csv_token_t token, const char* value);


/**
 * Check if a character is a delimiter.
 */
static bool
csv_tokenizer_is_delimiter(char ch) {
  return ch == ',';
}


/**
 * Check if a character is a linebreak.
 */
static bool
csv_tokenizer_is_linebreak(char ch) {
  return ch == '\n';
}


/**
 * Check if a character is a space.
 */
static bool
csv_tokenizer_is_space(char ch) {
  return (ch == ' ' || (ch >= '\t' && ch <= '\r' && ch != '\n'));
}


/**
 * Check if a character is a digit.
 */
static bool
csv_tokenizer_is_digit(char ch) {
  return (ch >= '0' && ch <= '9');
}


/**
 * Scan a character from input.
 */
static bool
csv_tokenizer_scan(csv_tokenizer_t *t) {
  if(t->la < 0) {
    return false;
  }
  
  if(t->value_pos >= t->value_size - 1) {
    t->value_size *= 2;
    t->value = realloc(t->value, sizeof(char) * t->value_size);
    assert(t->value);
  }

  t->value[t->value_pos++] = t->la;
  t->value[t->value_pos] = '\0';
  
  if(feof(t->fp)) {
    t->la = -1;
  } else {
    t->la = fgetc(t->fp);
  }

  return true;
}


/**
 * Skip a character from input.
 */
static void
csv_tokenizer_skip(csv_tokenizer_t *t) {
  if(feof(t->fp)) {
    t->la = -1;
  } else {
    t->la = fgetc(t->fp);
  }
}


/**
 * Skip blank characters from input.
 */
static void
csv_tokenizer_skip_blanks(csv_tokenizer_t *t) {
  while (csv_tokenizer_is_space(t->la)) {
    csv_tokenizer_skip(t);
  }
}


/**
 * Accept a string from input.
 */
static bool
csv_tokenizer_accept_string(csv_tokenizer_t *t) {
  if (t->la != '"') {
    return false;
  }
  csv_tokenizer_skip(t);
  
  while (t->la != '"') {
    if(!csv_tokenizer_scan(t)) {
      return false;
    }
    
    if(t->la == '"') {
      csv_tokenizer_skip(t);
      if (t->la == '"') {
	csv_tokenizer_scan(t);
      } else {
	break;
      }
    }
  }

  if(t->la == '"') {
    csv_tokenizer_skip(t);
  }
  
  t->type = TOK_STRING;
  return true;
}


/**
 * Accept a delimiter from input.
 */
static bool
csv_tokenizer_accept_delimiter(csv_tokenizer_t *t) {
  if(!csv_tokenizer_is_delimiter(t->la)) {
    return false;
  }
  
  csv_tokenizer_scan(t);
  t->type = TOK_DELIMITER;
  return true;
}


/**
 * Accept a delimiter from input.
 */
static bool
csv_tokenizer_accept_linebreak(csv_tokenizer_t *t) {
  if(!csv_tokenizer_is_linebreak(t->la)) {
    return false;
  }
  
  csv_tokenizer_scan(t);
  t->type = TOK_LINEBREAK;
  return true;
}


/**
 * Accept an integer or real from input.
 */
static bool
csv_tokenizer_accept_number(csv_tokenizer_t *t) {
  if (!csv_tokenizer_is_digit(t->la) && t->la != '-') {
    return false;
  }

  while (csv_tokenizer_is_digit(t->la) ||
	 t->la == '.' || t->la == '-' ||
	 t->la == 'e' || t->la == 'E') {
    csv_tokenizer_scan(t);
  }

  t->type = TOK_NUMBER;
  
  return true;
}


/**
 * Accept a comment.
 */
static bool
csv_tokenizer_accept_comment(csv_tokenizer_t *t) {
  if (t->la != '#') {
    return false;
  }

  while (t->la != '\n' && t->la != '\r' && t->la >= 0) {
    csv_tokenizer_scan(t);
  }
  
  t->type = TOK_COMMENT;
  return true;
}


/**
 * Accept end of file from input.
 */
static bool
csv_tokenizer_accept_eof(csv_tokenizer_t *t) {
  if(t->la >= 0) {
    return false;
  }
  
  t->type = TOK_EOF;
  return true;
}


/**
 * Progress input to the next token.
 */
static bool
csv_tokenizer_next(csv_tokenizer_t *t) {
  t->type = TOK_INVALID;
  t->value_pos = 0;
  t->value[0] = '\0';

  csv_tokenizer_skip_blanks(t);

  if(csv_tokenizer_accept_eof(t)) {
    return false;
  }
  
  if(csv_tokenizer_accept_string(t)) {
    return true;
  }
  
  if(csv_tokenizer_accept_delimiter(t)) {
    return true;
  }
  
  if(csv_tokenizer_accept_number(t)) {
    return true;
  }
  
  if(csv_tokenizer_accept_comment(t)) {
    return true;
  }

  if(csv_tokenizer_accept_linebreak(t)) {
    return true;
  }

  assert(false);
  
  return false;
}


/**
 * Tokenize text in a file and emit tokens to a callback function.
 */
static void
csv_tokenize_file(csv_token_cb cb, void* ctx, const char* filename) {
  csv_tokenizer_t t;
  bool running;
  
  t.fp         = fopen(filename, "r");
  t.value      = malloc(64);
  t.value_size = 64;
  t.la         = ' ';
  
  assert(t.fp);
  assert(t.value);
  
  do {
    running = csv_tokenizer_next(&t);
    running &= cb(ctx, t.type, t.value);
  } while (running);

  fclose(t.fp);
  free(t.value);
}


/**
 * Parse a csv token.
 **/
static bool
csv_parser_on_token(void *ctx, csv_token_t token, const char* value) {
  csv_parser_t *p = (csv_parser_t*)ctx;
  
  switch(token) {
  case TOK_COMMENT:
  case TOK_STRING:
    //drop comments and string values
    break;
    
  case TOK_NUMBER:
    assert(p->curr_off < p->length);
    p->memory[p->curr_off++] = (real_t)atof(value);
    break;

  case TOK_DELIMITER:
    p->curr_col++;
    break;

  case TOK_EOF:
  case TOK_LINEBREAK:

    //number rows with numbers --> height
    if(p->curr_col) {
      //determine width from first row with numbers
      if(!p->height) {
	p->width = p->curr_col + 1;
      }
      p->height++;
    }

    p->curr_col = 0;
    break;
    
  default:
  case TOK_INVALID:
    assert(false);
    break;
  }
  
  return true;
}


/**
 * Count the number of numbers in a csv file.
 **/
static bool
vote_csv_count_numbers(void *ctx, csv_token_t token, const char* value) {
  size_t *cnt = (size_t*)ctx;
  VOTE_UNUSED(value);
  
  if(token == TOK_NUMBER) {
    (*cnt)++;
  }
  
  return true;
}


vote_dataset_t*
vote_csv_load(const char* filename) {
  csv_parser_t p;
  vote_dataset_t* ds;
  size_t length = 0;
  
  csv_tokenize_file(vote_csv_count_numbers, &length, filename);
  if(!length) {
    return NULL;
  }
  
  p.memory    = calloc(length, sizeof(real_t));
  p.length    = length;
  p.curr_off  = 0;
  p.curr_col  = 0;
  p.width     = 0;
  p.height    = 0;

  assert(p.memory);
  
  csv_tokenize_file(csv_parser_on_token, &p, filename);

  assert(p.length == p.width * p.height);

  ds = calloc(1, sizeof(vote_dataset_t));
  assert(ds);

  ds->data     = p.memory;
  ds->nb_cols  = p.width;
  ds->nb_rows  = p.height;
  ds->filename = calloc(strlen(filename) + 1, sizeof(char));
  assert(ds->filename);
  strcpy(ds->filename, filename);
  
  return ds;
}


void
vote_dataset_del(vote_dataset_t* ds) {
  free(ds->filename);
  free(ds->data);
  free(ds);
}


real_t*
vote_dataset_row(vote_dataset_t* ds, size_t index) {
  return &ds->data[index * ds->nb_cols];
}


