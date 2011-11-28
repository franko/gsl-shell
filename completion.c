
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <lua.h>

#include "completion.h"
#include "gsl-shell.h"

static char *my_generator (const char *text, int state);

void initialize_readline()
{

  rl_completion_entry_function = my_generator;
  rl_basic_word_break_characters = " \t\n\"'~><=*+-/;,|[{(";
}

#define WORDS_BUFFER_SIZE 256
#define NODE_LIST_SIZE 8
char *my_generator (const char *text, int state)
{
  static int list_index, len;
  static const char *text_term;
  static char words_buffer[WORDS_BUFFER_SIZE];
  static char *node_list[NODE_LIST_SIZE];
  static int word_number;
  char *base_word, *word;
  lua_State *L = globalL;
  int k;

  if (!state)
    {
      const char *p, *curr = text;
      int node_counter = 0, words_index = 0;

      list_index = 0;
      word_number = 0;

      if (strlen (text) >= WORDS_BUFFER_SIZE)
	return NULL;

      for (p = strchr (curr, '.'); p; )
	{
	  char *new_word = words_buffer + words_index;
	  int wlen = p - curr;

	  if (node_counter >= NODE_LIST_SIZE)
	    return NULL;

	  memcpy (new_word, curr, wlen);
	  new_word[wlen] = 0;
	  words_index += wlen+1;

	  node_list[node_counter] = new_word;
	  node_counter ++;

	  curr = p+1;
	  p = strchr (curr, '.');
	  word_number++;
	}

      text_term = curr;
      len = strlen (text_term);

      {
	int wlen = curr - text;
	base_word = words_buffer + words_index;
	if (words_index + wlen + 1 >= WORDS_BUFFER_SIZE)
	  return NULL;
	memcpy (base_word, text, wlen);
	base_word[wlen] = 0;
      }
    }

  lua_pushvalue (L, LUA_GLOBALSINDEX);
  word = words_buffer;
  for (k = 0; k < word_number; k++)
    {
      if (!lua_istable(L, -1))
	goto pop_exit;

      lua_getfield(L, -1, word);
      lua_remove(L, -2);

      word = strchr (word, '\0') + 1;
    }
     
  if (!lua_istable(L, -1))
    goto pop_exit;

  lua_pushnil (L);
  for (k = 0; k < list_index; k++)
    {
      if (lua_next (L, -2) == 0)
	goto pop_exit;
      lua_pop (L, 1);
    }

  while (lua_next(L, -2) != 0)
    {
      const char *key = lua_tostring (L, -2);

      list_index ++;

      if (key)
	{
	  if (strncmp (key, text_term, len) == 0)
	    {
	      char *new_word;

	      if (lua_istable (L, -1))
		rl_completion_append_character = '.';
	      else
		rl_completion_suppress_append = 1;

	      if (asprintf (&new_word, "%s%s", base_word, key) < 0)
		{
		  lua_pop (L, 2);
		  goto pop_exit;
		}

	      lua_pop (L, 3);
	      return new_word;
	    }
	}

      lua_pop (L, 1);
    }

 pop_exit:
  lua_pop (L, 1);
  return NULL;
}
