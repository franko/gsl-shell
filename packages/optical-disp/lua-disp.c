
#include <lua.h>
#include <lauxlib.h>
#include "disp-utils.h"
#include "disp-ho.h"
#include "disp-table.h"
#include "disp-library.h"

#define check_disp(L, idx) (struct disp *) luaL_checkudata (L, (idx), disp_mt_name)

extern int  luaopen_disp             (lua_State *L);

static int  get_n_value              (lua_State *L);
static int  get_n_deriv              (lua_State *L);
static int  ho_builder               (lua_State *L);
static int  lua_load_nk              (lua_State *L);
static int  disp_lua_samples         (lua_State *L);
static int  disp_lua_sample_iter     (lua_State *L);
static int  disp_lua_fp_number       (lua_State *L);
static int  disp_lua_apply           (lua_State *L);
static int  disp_lua_dealloc         (lua_State *L);
static int  disp_lua_copy            (lua_State *L);
static int  get_sampling_length      (lua_State *L);
static int  get_sample               (lua_State *L);
static int  get_sample_raw           (lua_State *L, struct disp *d, 
				      struct sampling_intfc *intfc,
				      lua_Integer index);


static struct disp_ho * ho_builder_rec (lua_State *L, int nb_osc);

static void ho_retrieve_param (lua_State *L, const char *name,
			       double *ptrval, int is_optional);


static const struct luaL_Reg disp_methods[] = {
  {"__gc",          disp_lua_dealloc},
  {"__len",         get_sampling_length},
  {"get_n",         get_n_value},
  {"get_n_deriv",   get_n_deriv},
  {"fit_params_nb", disp_lua_fp_number},
  {"apply",         disp_lua_apply},
  {"copy",          disp_lua_copy},
  {"sample",        get_sample},
  {"samples",       disp_lua_samples},
  {NULL, NULL}
};

static const struct luaL_Reg disp_functions[] = {
  {"ho",      ho_builder},
  {"load_nk", lua_load_nk},
  {NULL, NULL}
};

const char * const ERR_MSG_NOT_SAMPLE = "not a tabular dispersion";

const char *disp_mt_name = "Disp.t";

int
get_n_value (lua_State *L)
{
  struct disp *d = check_disp (L, 1);
  lua_Complex n;
  lua_Number lam;

  luaL_argcheck (L, lua_isnumber (L, 2), 2, 
		 "the wavelength should be a number");
  lam = lua_tonumber (L, 2);

  n = DISP(d)->n_value (d, lam);

  lua_pushcomplex (L, n);

  return 1;
}


int
get_n_deriv (lua_State *L)
{
  struct disp *d = check_disp (L, 1);
  const char * fit_param_name = luaL_checklstring (L, 2, NULL);
  lua_Number lam;
  lua_Complex n;
  int fp_index;

  luaL_argcheck (L, lua_isnumber (L, 3), 3, 
		 "the wavelength should be a number");
  lam = lua_tonumber (L, 3);

  fp_index = DISP(d)->decode_param_string (fit_param_name);
  if (fp_index < 0 || fp_index >= DISP(d)->fp_number (d))
    {
      return luaL_error (L, "invalid fit parameter: %s", fit_param_name);
    }

  n = DISP(d)->n_deriv (d, fp_index, lam);

  lua_pushcomplex (L, n);

  return 1;
}

int
disp_lua_fp_number (lua_State *L)
{
  struct disp *d = check_disp (L, 1);
  int nb = DISP(d)->fp_number (d);
  lua_pushinteger (L, nb);
  return 1;
}

int
disp_lua_sample_iter (lua_State *L)
{
  struct disp *d = check_disp (L, lua_upvalueindex(1));
  lua_Integer k = lua_tointeger (L, lua_upvalueindex(2));
  lua_Integer n;
  
  if (DISP(d)->sampling_intfc == NULL)
    return luaL_error (L, ERR_MSG_NOT_SAMPLE);

  n = DISP(d)->sampling_intfc->length (d);

  if (k <= n)
    {
      get_sample_raw (L, d, DISP(d)->sampling_intfc, k);
      lua_pushinteger (L, k+1);
      lua_replace (L, lua_upvalueindex(2));
      return 2;
    }

  return 0;
}

int
disp_lua_samples (lua_State *L)
{
  struct disp *d = check_disp (L, 1);

  if (DISP(d)->sampling_intfc == NULL)
    return luaL_error (L, ERR_MSG_NOT_SAMPLE);

  lua_pushinteger (L, 1);
  lua_pushcclosure (L, disp_lua_sample_iter, 2);
  return 1;
}

int
disp_lua_apply (lua_State *L)
{
  struct disp *d = check_disp (L, 1);
  const char *fit_param_name;
  lua_Number val;
  int fp_index;

  fit_param_name = luaL_checklstring (L, 2, NULL);
  val = luaL_checknumber (L, 3);

  fp_index = DISP(d)->decode_param_string (fit_param_name);
  if (fp_index < 0 || fp_index >= DISP(d)->fp_number (d))
    {
      return luaL_error (L, "invalid fit parameter: %s", fit_param_name);
    }

  DISP(d)->apply_param (d, fp_index, val);

  return 0;
}

int
disp_lua_dealloc (lua_State *L)
{
  struct disp *d = check_disp (L, 1);
  DISP(d)->dealloc (d);
  return 0;
}

int
disp_lua_copy (lua_State *L)
{
  struct disp *src = check_disp (L, 1);
  struct disp *copy;

  copy = lua_newuserdata (L, DISP(src)->instance_size);
  disp_set_class (copy, src->dclass);
  DISP(copy)->copy (copy, src);

  luaL_getmetatable (L, disp_mt_name);
  lua_setmetatable (L, -2);

  return 1;
}

void
ho_retrieve_param (lua_State *L, const char *name, double *ptrval,
		   int is_optional)
{
  lua_pushstring (L, name);
  lua_gettable (L, 2);

  if (lua_isnil (L, 3) || !lua_isnumber (L, 3))
    {
      if (is_optional)
	{
	  lua_pop (L, 1);
	  return;
	}

      luaL_error (L, "parameter '%s' not given or invalid", name);
    }

  *ptrval = lua_tonumber (L, 3);
  lua_pop (L, 1);
}

struct disp_ho *
ho_builder_rec (lua_State *L, int nb_osc)
{
  static const char *pname[] = {"nosc", "en", "eg", "nu", "phi", NULL};
  static const char is_opt[] = {0, 0, 1, 1, 1};
  double ho_value[] = {0.0, 0.0, 0.0, 1/3.0, 0.0};
  struct disp_ho *ho;
  int k;

  lua_rawgeti (L, 1, nb_osc + 1);

  if (lua_isnil (L, 2))
    {
      lua_pop (L, 1);

      ho = (struct disp_ho *) lua_newuserdata (L, ho_disp_class->instance_size);
      disp_ho_init (ho, nb_osc);

      return ho;
    }

  luaL_checktype (L, 2, LUA_TTABLE);

  for (k = 0; pname[k]; k++)
    {
      ho_retrieve_param (L, pname[k], &ho_value[k], is_opt[k]);
    }

  lua_pop (L, 1);
  ho = ho_builder_rec (L, nb_osc + 1);

  disp_ho_set_ho_params (ho, nb_osc, ho_value);

  return ho;
}

int
ho_builder (lua_State *L)
{
  luaL_checktype (L, 1, LUA_TTABLE);

  ho_builder_rec (L, 0);

  luaL_getmetatable (L, disp_mt_name);
  lua_setmetatable (L, -2);

  return 1;
}

int
lua_load_nk (lua_State *L)
{
  const char *filename = luaL_checklstring (L, 1, NULL);
  struct disp *dt;
  size_t isize = disp_table_class->instance_size;
  const char *error_msg;

  dt = (struct disp *) lua_newuserdata (L, isize); 
  disp_table_init (dt);

  if (disp_table_load_nk_file (dt, filename, &error_msg) != 0)
    {
      return luaL_error (L, "Error while reading nk file %s: %s", 
			 filename, error_msg);
    }

  luaL_getmetatable (L, disp_mt_name);
  lua_setmetatable (L, -2);

  return 1;
}

int
get_sampling_length (lua_State *L)
{
  struct disp *d = check_disp (L, 1);

  if (DISP(d)->sampling_intfc == NULL)
    return luaL_error (L, ERR_MSG_NOT_SAMPLE);

  lua_pushinteger (L, DISP(d)->sampling_intfc->length (d));
  return 1;
}

int
get_sample_raw (lua_State *L, struct disp *d, struct sampling_intfc *intfc,
		lua_Integer index)
{
  struct sample const * sample;
  lua_Integer len;

  len    = intfc->length (d);
  sample = intfc->get_sample (d, index);

  if (index < 1 || index > len)
    return 0;

  lua_pushnumber (L, sample->lambda);
  lua_pushcomplex (L, sample->n);

  return 2;
}

int
get_sample (lua_State *L)
{
  struct disp *d = check_disp (L, 1);
  lua_Integer index;

  if (DISP(d)->sampling_intfc == NULL)
    return luaL_error (L, ERR_MSG_NOT_SAMPLE);

  index = luaL_checkinteger (L, 2);

  return get_sample_raw (L, d, DISP(d)->sampling_intfc, index);
}

int
luaopen_disp (lua_State *L)
{
  luaL_newmetatable (L, disp_mt_name);
  lua_pushvalue (L, -1);
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, disp_methods);

  luaL_register (L, "disp", disp_functions);

  disp_library_init (L);

  return 0;
}
