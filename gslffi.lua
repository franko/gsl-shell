ffi = require 'ffi'

ffi.cdef[[
     struct _IO_FILE;
     typedef struct _IO_FILE FILE;

     typedef struct
     {
       size_t size;
       double * data;
     } gsl_block;

     typedef struct
     {
       size_t size;
       size_t stride;
       double * data;
       gsl_block * block;
       int owner;
     } gsl_vector;

     typedef struct
     {
       size_t size1;
       size_t size2;
       size_t tda;
       double * data;
       gsl_block * block;
       int owner;
     } gsl_matrix;

     typedef struct
     {
	gsl_vector vector;
     } gsl_vector_view;

     typedef gsl_vector_view gsl_vector_const_view;

     typedef struct
     {
	gsl_matrix matrix;
     } gsl_matrix_view;

     typedef gsl_matrix_view gsl_matrix_const_view;

void gsl_vector_set_zero (gsl_vector * v);
void gsl_vector_set_all (gsl_vector * v, double x);
int gsl_vector_set_basis (gsl_vector * v, size_t i);

int gsl_vector_fread (FILE * stream, gsl_vector * v);
int gsl_vector_fwrite (FILE * stream, const gsl_vector * v);
int gsl_vector_fscanf (FILE * stream, gsl_vector * v);
int gsl_vector_fprintf (FILE * stream, const gsl_vector * v,
                              const char *format);

int gsl_vector_memcpy (gsl_vector * dest, const gsl_vector * src);

int gsl_vector_reverse (gsl_vector * v);

int gsl_vector_swap (gsl_vector * v, gsl_vector * w);
int gsl_vector_swap_elements (gsl_vector * v, const size_t i, const size_t j);

double gsl_vector_max (const gsl_vector * v);
double gsl_vector_min (const gsl_vector * v);
void gsl_vector_minmax (const gsl_vector * v, double * min_out, double * max_out);

size_t gsl_vector_max_index (const gsl_vector * v);
size_t gsl_vector_min_index (const gsl_vector * v);
void gsl_vector_minmax_index (const gsl_vector * v, size_t * imin, size_t * imax);

int gsl_vector_add (gsl_vector * a, const gsl_vector * b);
int gsl_vector_sub (gsl_vector * a, const gsl_vector * b);
int gsl_vector_mul (gsl_vector * a, const gsl_vector * b);
int gsl_vector_div (gsl_vector * a, const gsl_vector * b);
int gsl_vector_scale (gsl_vector * a, const double x);
int gsl_vector_add_constant (gsl_vector * a, const double x);

int gsl_vector_isnull (const gsl_vector * v);
int gsl_vector_ispos (const gsl_vector * v);
int gsl_vector_isneg (const gsl_vector * v);
int gsl_vector_isnonneg (const gsl_vector * v);

double gsl_vector_get (const gsl_vector * v, const size_t i);
void gsl_vector_set (gsl_vector * v, const size_t i, double x);
double * gsl_vector_ptr (gsl_vector * v, const size_t i);
const double * gsl_vector_const_ptr (const gsl_vector * v, const size_t i);

gsl_matrix_view 
gsl_matrix_submatrix (gsl_matrix * m, 
                            const size_t i, const size_t j, 
                            const size_t n1, const size_t n2);

gsl_vector_view 
gsl_matrix_row (gsl_matrix * m, const size_t i);

gsl_vector_view 
gsl_matrix_column (gsl_matrix * m, const size_t j);

gsl_vector_view 
gsl_matrix_diagonal (gsl_matrix * m);

gsl_vector_view 
gsl_matrix_subdiagonal (gsl_matrix * m, const size_t k);

gsl_vector_view 
gsl_matrix_superdiagonal (gsl_matrix * m, const size_t k);

gsl_vector_view
gsl_matrix_subrow (gsl_matrix * m, const size_t i,
                         const size_t offset, const size_t n);

gsl_vector_view
gsl_matrix_subcolumn (gsl_matrix * m, const size_t j,
                            const size_t offset, const size_t n);

gsl_matrix_view
gsl_matrix_view_array (double * base,
                             const size_t n1, 
                             const size_t n2);

gsl_matrix_view
gsl_matrix_view_array_with_tda (double * base, 
                                      const size_t n1, 
                                      const size_t n2,
                                      const size_t tda);


gsl_matrix_view
gsl_matrix_view_vector (gsl_vector * v,
                              const size_t n1, 
                              const size_t n2);

gsl_matrix_view
gsl_matrix_view_vector_with_tda (gsl_vector * v,
                                       const size_t n1, 
                                       const size_t n2,
                                       const size_t tda);


gsl_matrix_const_view 
gsl_matrix_const_submatrix (const gsl_matrix * m, 
                                  const size_t i, const size_t j, 
                                  const size_t n1, const size_t n2);

gsl_vector_const_view 
gsl_matrix_const_row (const gsl_matrix * m, 
                            const size_t i);

gsl_vector_const_view 
gsl_matrix_const_column (const gsl_matrix * m, 
                               const size_t j);

gsl_vector_const_view
gsl_matrix_const_diagonal (const gsl_matrix * m);

gsl_vector_const_view 
gsl_matrix_const_subdiagonal (const gsl_matrix * m, 
                                    const size_t k);

gsl_vector_const_view 
gsl_matrix_const_superdiagonal (const gsl_matrix * m, 
                                      const size_t k);

gsl_vector_const_view
gsl_matrix_const_subrow (const gsl_matrix * m, const size_t i,
                               const size_t offset, const size_t n);

gsl_vector_const_view
gsl_matrix_const_subcolumn (const gsl_matrix * m, const size_t j,
                                  const size_t offset, const size_t n);

gsl_matrix_const_view
gsl_matrix_const_view_array (const double * base,
                                   const size_t n1, 
                                   const size_t n2);

gsl_matrix_const_view
gsl_matrix_const_view_array_with_tda (const double * base, 
                                            const size_t n1, 
                                            const size_t n2,
                                            const size_t tda);

gsl_matrix_const_view
gsl_matrix_const_view_vector (const gsl_vector * v,
                                    const size_t n1, 
                                    const size_t n2);

gsl_matrix_const_view
gsl_matrix_const_view_vector_with_tda (const gsl_vector * v,
                                             const size_t n1, 
                                             const size_t n2,
                                             const size_t tda);

/* Operations */

void gsl_matrix_set_zero (gsl_matrix * m);
void gsl_matrix_set_identity (gsl_matrix * m);
void gsl_matrix_set_all (gsl_matrix * m, double x);

int gsl_matrix_fread (FILE * stream, gsl_matrix * m) ;
int gsl_matrix_fwrite (FILE * stream, const gsl_matrix * m) ;
int gsl_matrix_fscanf (FILE * stream, gsl_matrix * m);
int gsl_matrix_fprintf (FILE * stream, const gsl_matrix * m, const char * format);
 
int gsl_matrix_memcpy(gsl_matrix * dest, const gsl_matrix * src);
int gsl_matrix_swap(gsl_matrix * m1, gsl_matrix * m2);

int gsl_matrix_swap_rows(gsl_matrix * m, const size_t i, const size_t j);
int gsl_matrix_swap_columns(gsl_matrix * m, const size_t i, const size_t j);
int gsl_matrix_swap_rowcol(gsl_matrix * m, const size_t i, const size_t j);
int gsl_matrix_transpose (gsl_matrix * m);
int gsl_matrix_transpose_memcpy (gsl_matrix * dest, const gsl_matrix * src);

double gsl_matrix_max (const gsl_matrix * m);
double gsl_matrix_min (const gsl_matrix * m);
void gsl_matrix_minmax (const gsl_matrix * m, double * min_out, double * max_out);

void gsl_matrix_max_index (const gsl_matrix * m, size_t * imax, size_t *jmax);
void gsl_matrix_min_index (const gsl_matrix * m, size_t * imin, size_t *jmin);
void gsl_matrix_minmax_index (const gsl_matrix * m, size_t * imin, size_t * jmin, size_t * imax, size_t * jmax);

int gsl_matrix_isnull (const gsl_matrix * m);
int gsl_matrix_ispos (const gsl_matrix * m);
int gsl_matrix_isneg (const gsl_matrix * m);
int gsl_matrix_isnonneg (const gsl_matrix * m);

int gsl_matrix_add (gsl_matrix * a, const gsl_matrix * b);
int gsl_matrix_sub (gsl_matrix * a, const gsl_matrix * b);
int gsl_matrix_mul_elements (gsl_matrix * a, const gsl_matrix * b);
int gsl_matrix_div_elements (gsl_matrix * a, const gsl_matrix * b);
int gsl_matrix_scale (gsl_matrix * a, const double x);
int gsl_matrix_add_constant (gsl_matrix * a, const double x);
int gsl_matrix_add_diagonal (gsl_matrix * a, const double x);


double   gsl_matrix_get(const gsl_matrix * m, const size_t i, const size_t j);
void    gsl_matrix_set(gsl_matrix * m, const size_t i, const size_t j, const double x);
double * gsl_matrix_ptr(gsl_matrix * m, const size_t i, const size_t j);
const double * gsl_matrix_const_ptr(const gsl_matrix * m, const size_t i, const size_t j);
]]
