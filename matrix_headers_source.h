
extern void                FUNCTION (matrix, register)   (lua_State *L);

extern TYPE (gsl_matrix) * FUNCTION (matrix, push)       (lua_State *L,
							  int n1, int n2);

extern TYPE (gsl_matrix) * FUNCTION (matrix, push_raw)   (lua_State *L,
							  int n1, int n2);

extern void                FUNCTION (matrix, push_view)  (lua_State *L, 
							  TYPE (gsl_matrix) *m);

extern TYPE (gsl_matrix) * FUNCTION (matrix, check)      (lua_State *L, 
							  int index);

extern VIEW (gsl_matrix) * FUNCTION (matrix, check_view) (lua_State *L,
							  int idx);

extern void                FUNCTION (matrix, null_view)  (lua_State *L,
							  int index);


/* matrix helper functions */
extern void
FUNCTION (matrix, set_view_and_push) (lua_State *L, int index, double *data,
			  size_t n1, size_t n2, const double *src);

extern void
FUNCTION (matrix, jacob_copy_real_to_cmpl) (double *dest_cmpl, double *src_real,
					    size_t n, size_t p, size_t mult);

extern void
FUNCTION (matrix, jacob_copy_cmpl_to_real) (double *dest_real, double *src_cmpl,
					   size_t n, size_t p, size_t mult);

extern char const * const TYPE (name_matrix);
