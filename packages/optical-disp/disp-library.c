
#include "lua.h"
#include "lauxlib.h"
#include "disp-library.h"
#include "lua-disp.h"
#include "disp-table.h"
#include "disp-ho.h"

struct data_table_silicon {
  int rows;
  int columns;
  int ref_count;
  float heap[341 * 2];
};

#include "si-paper-table.h"

int
disp_library_init (lua_State *L)
{
  struct disp_table *si;
  struct disp_ho *sio2, *vac;
  double ox_ho_values[] = {145.0, 15.78839, 0.0, 0.3333, 0.0};
  double vac_ho_values[] = {0, 30, 0, 0, 0};

  si = lua_newuserdata (L, disp_table_class->instance_size); 
  disp_table_init ((struct disp *) si);

  si->points_number = si_data_table.rows;
  si->lambda_min    = SI_PAPER_WVLEN_MIN;
  si->lambda_max    = SI_PAPER_WVLEN_MAX;
  si->lambda_stride = SI_PAPER_WVLEN_STRIDE;

  si->table_ref = (struct data_table *) & si_data_table;

  luaL_getmetatable (L, disp_mt_name);
  lua_setmetatable (L, -2);
  lua_setfield (L, -2, "si");

  sio2 = lua_newuserdata (L, ho_disp_class->instance_size);
  disp_ho_init (sio2, 1);

  disp_ho_set_ho_params (sio2, 0, ox_ho_values);

  luaL_getmetatable (L, disp_mt_name);
  lua_setmetatable (L, -2);
  lua_setfield (L, -2, "sio2");

  vac = lua_newuserdata (L, ho_disp_class->instance_size);
  disp_ho_init (vac, 2);

  disp_ho_set_ho_params (vac, 0, vac_ho_values);

  luaL_getmetatable (L, disp_mt_name);
  lua_setmetatable (L, -2);
  lua_setfield (L, -2, "void");

  return 0;
}
