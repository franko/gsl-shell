local ffi = require 'ffi'

ffi.cdef[[
      struct _IO_FILE;
      typedef struct _IO_FILE FILE;

      extern struct _IO_FILE *stdin;
      extern struct _IO_FILE *stdout;
      extern struct _IO_FILE *stderr;

      void * malloc(size_t n);
      void free(void *p);

      enum {
	 GSL_SUCCESS  = 0,
	 GSL_FAILURE  = -1,
	 GSL_CONTINUE = -2,
	 GSL_EDOM     = 1,
	 GSL_ERANGE   = 2,
	 GSL_EFAULT   = 3,
	 GSL_EINVAL   = 4,
	 GSL_EFAILED  = 5,
	 GSL_EFACTOR  = 6,
	 GSL_ESANITY  = 7,
	 GSL_ENOMEM   = 8,
	 GSL_EBADFUNC = 9,
	 GSL_ERUNAWAY = 10,
	 GSL_EMAXITER = 11,
	 GSL_EZERODIV = 12,
	 GSL_EBADTOL  = 13,
	 GSL_ETOL     = 14,
	 GSL_EUNDRFLW = 15,
	 GSL_EOVRFLW  = 16,
	 GSL_ELOSS    = 17,
	 GSL_EROUND   = 18,
	 GSL_EBADLEN  = 19,
	 GSL_ENOTSQR  = 20,
	 GSL_ESING    = 21,
	 GSL_EDIVERGE = 22,
	 GSL_EUNSUP   = 23,
	 GSL_EUNIMPL  = 24,
	 GSL_ECACHE   = 25,
	 GSL_ETABLE   = 26,
	 GSL_ENOPROG  = 27,
	 GSL_ENOPROGJ = 28,
	 GSL_ETOLF    = 29,
	 GSL_ETOLX    = 30,
	 GSL_ETOLG    = 31,
	 GSL_EOF      = 32
   } ;

      const char * gsl_strerror (const int gsl_errno);

      typedef complex gsl_complex;

     typedef struct
     {
       size_t size;
       double * data;
       int ref_count;
     } gsl_block;

     typedef gsl_block gsl_block_complex;

     gsl_block *gsl_block_alloc (const size_t n);
     gsl_block *gsl_block_calloc (const size_t n);
     void gsl_block_free (gsl_block * b);

     gsl_block_complex *gsl_block_complex_alloc (const size_t n);
     gsl_block_complex *gsl_block_complex_calloc (const size_t n);
     void gsl_block_complex_free (gsl_block_complex * b);

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
	size_t size;
	size_t stride;
	double *data;
	gsl_block_complex *block;
	int owner;
     } gsl_vector_complex;

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
	size_t size1;
	size_t size2;
	size_t tda;
	double * data;
	gsl_block_complex * block;
	int owner;
     } gsl_matrix_complex;

     typedef gsl_vector gsl_vector_view;
     typedef gsl_vector gsl_vector_const_view;

     typedef gsl_matrix gsl_matrix_view;
     typedef gsl_matrix gsl_matrix_const_view;

     typedef gsl_vector_complex gsl_vector_complex_view;
     typedef gsl_vector_complex gsl_vector_complex_const_view;

     typedef gsl_matrix_complex gsl_matrix_complex_view;
     typedef gsl_matrix_complex gsl_matrix_complex_const_view;

     typedef struct
     {
	size_t size;
	size_t *data;
     } gsl_permutation;



double gsl_complex_arg (gsl_complex z);
double gsl_complex_abs (gsl_complex z);
double gsl_complex_abs2 (gsl_complex z);
double gsl_complex_logabs (gsl_complex z);



gsl_complex gsl_complex_add (gsl_complex a, gsl_complex b);
gsl_complex gsl_complex_sub (gsl_complex a, gsl_complex b);
gsl_complex gsl_complex_mul (gsl_complex a, gsl_complex b);
gsl_complex gsl_complex_div (gsl_complex a, gsl_complex b);

gsl_complex gsl_complex_add_real (gsl_complex a, double x);
gsl_complex gsl_complex_sub_real (gsl_complex a, double x);
gsl_complex gsl_complex_mul_real (gsl_complex a, double x);
gsl_complex gsl_complex_div_real (gsl_complex a, double x);

gsl_complex gsl_complex_add_imag (gsl_complex a, double y);
gsl_complex gsl_complex_sub_imag (gsl_complex a, double y);
gsl_complex gsl_complex_mul_imag (gsl_complex a, double y);
gsl_complex gsl_complex_div_imag (gsl_complex a, double y);

gsl_complex gsl_complex_conjugate (gsl_complex z);
gsl_complex gsl_complex_inverse (gsl_complex a);
gsl_complex gsl_complex_negative (gsl_complex a);



gsl_complex gsl_complex_sqrt (gsl_complex z);
gsl_complex gsl_complex_sqrt_real (double x);

gsl_complex gsl_complex_pow (gsl_complex a, gsl_complex b);
gsl_complex gsl_complex_pow_real (gsl_complex a, double b);

gsl_complex gsl_complex_exp (gsl_complex a);
gsl_complex gsl_complex_log (gsl_complex a);
gsl_complex gsl_complex_log10 (gsl_complex a);
gsl_complex gsl_complex_log_b (gsl_complex a, gsl_complex b);



gsl_complex gsl_complex_sin (gsl_complex a);
gsl_complex gsl_complex_cos (gsl_complex a);
gsl_complex gsl_complex_sec (gsl_complex a);
gsl_complex gsl_complex_csc (gsl_complex a);
gsl_complex gsl_complex_tan (gsl_complex a);
gsl_complex gsl_complex_cot (gsl_complex a);



gsl_complex gsl_complex_arcsin (gsl_complex a);
gsl_complex gsl_complex_arcsin_real (double a);
gsl_complex gsl_complex_arccos (gsl_complex a);
gsl_complex gsl_complex_arccos_real (double a);
gsl_complex gsl_complex_arcsec (gsl_complex a);
gsl_complex gsl_complex_arcsec_real (double a);
gsl_complex gsl_complex_arccsc (gsl_complex a);
gsl_complex gsl_complex_arccsc_real (double a);
gsl_complex gsl_complex_arctan (gsl_complex a);
gsl_complex gsl_complex_arccot (gsl_complex a);



gsl_complex gsl_complex_sinh (gsl_complex a);
gsl_complex gsl_complex_cosh (gsl_complex a);
gsl_complex gsl_complex_sech (gsl_complex a);
gsl_complex gsl_complex_csch (gsl_complex a);
gsl_complex gsl_complex_tanh (gsl_complex a);
gsl_complex gsl_complex_coth (gsl_complex a);



gsl_complex gsl_complex_arcsinh (gsl_complex a);
gsl_complex gsl_complex_arccosh (gsl_complex a);
gsl_complex gsl_complex_arccosh_real (double a);
gsl_complex gsl_complex_arcsech (gsl_complex a);
gsl_complex gsl_complex_arccsch (gsl_complex a);
gsl_complex gsl_complex_arctanh (gsl_complex a);
gsl_complex gsl_complex_arctanh_real (double a);
gsl_complex gsl_complex_arccoth (gsl_complex a);



gsl_vector *gsl_vector_alloc (const size_t n);
gsl_vector *gsl_vector_calloc (const size_t n);

gsl_vector *gsl_vector_alloc_from_block (gsl_block * b,
                                                     const size_t offset,
                                                     const size_t n,
                                                     const size_t stride);

gsl_vector *gsl_vector_alloc_from_vector (gsl_vector * v,
                                                      const size_t offset,
                                                      const size_t n,
                                                      const size_t stride);

void gsl_vector_free (gsl_vector * v);

gsl_vector_view
gsl_vector_view_array (double *v, size_t n);

gsl_vector_view
gsl_vector_view_array_with_stride (double *base,
                                         size_t stride,
                                         size_t n);

gsl_vector_const_view
gsl_vector_const_view_array (const double *v, size_t n);

gsl_vector_const_view
gsl_vector_const_view_array_with_stride (const double *base,
                                               size_t stride,
                                               size_t n);

gsl_vector_view
gsl_vector_subvector (gsl_vector *v,
                            size_t i,
                            size_t n);

gsl_vector_view
gsl_vector_subvector_with_stride (gsl_vector *v,
                                        size_t i,
                                        size_t stride,
                                        size_t n);

gsl_vector_const_view
gsl_vector_const_subvector (const gsl_vector *v,
                                  size_t i,
                                  size_t n);

gsl_vector_const_view
gsl_vector_const_subvector_with_stride (const gsl_vector *v,
                                              size_t i,
                                              size_t stride,
                                              size_t n);

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



gsl_matrix *
gsl_matrix_alloc (const size_t n1, const size_t n2);

gsl_matrix *
gsl_matrix_calloc (const size_t n1, const size_t n2);

gsl_matrix *
gsl_matrix_alloc_from_block (gsl_block * b,
                                   const size_t offset,
                                   const size_t n1,
                                   const size_t n2,
                                   const size_t d2);

gsl_matrix *
gsl_matrix_alloc_from_matrix (gsl_matrix * m,
                                    const size_t k1,
                                    const size_t k2,
                                    const size_t n1,
                                    const size_t n2);

gsl_vector *
gsl_vector_alloc_row_from_matrix (gsl_matrix * m,
                                        const size_t i);

gsl_vector *
gsl_vector_alloc_col_from_matrix (gsl_matrix * m,
                                        const size_t j);

void gsl_matrix_free (gsl_matrix * m);



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



gsl_matrix_complex *
gsl_matrix_complex_alloc (const size_t n1, const size_t n2);

gsl_matrix_complex *
gsl_matrix_complex_calloc (const size_t n1, const size_t n2);

gsl_matrix_complex *
gsl_matrix_complex_alloc_from_block (gsl_block_complex * b,
                                           const size_t offset,
                                           const size_t n1, const size_t n2, const size_t d2);

gsl_matrix_complex *
gsl_matrix_complex_alloc_from_matrix (gsl_matrix_complex * b,
                                            const size_t k1, const size_t k2,
                                            const size_t n1, const size_t n2);

gsl_vector_complex *
gsl_vector_complex_alloc_row_from_matrix (gsl_matrix_complex * m,
                                                const size_t i);

gsl_vector_complex *
gsl_vector_complex_alloc_col_from_matrix (gsl_matrix_complex * m,
                                                const size_t j);

void gsl_matrix_complex_free (gsl_matrix_complex * m);



gsl_matrix_complex_view
gsl_matrix_complex_submatrix (gsl_matrix_complex * m,
                            const size_t i, const size_t j,
                            const size_t n1, const size_t n2);

gsl_vector_complex_view
gsl_matrix_complex_row (gsl_matrix_complex * m, const size_t i);

gsl_vector_complex_view
gsl_matrix_complex_column (gsl_matrix_complex * m, const size_t j);

gsl_vector_complex_view
gsl_matrix_complex_diagonal (gsl_matrix_complex * m);

gsl_vector_complex_view
gsl_matrix_complex_subdiagonal (gsl_matrix_complex * m, const size_t k);

gsl_vector_complex_view
gsl_matrix_complex_superdiagonal (gsl_matrix_complex * m, const size_t k);

gsl_vector_complex_view
gsl_matrix_complex_subrow (gsl_matrix_complex * m,
                                 const size_t i, const size_t offset,
                                 const size_t n);

gsl_vector_complex_view
gsl_matrix_complex_subcolumn (gsl_matrix_complex * m,
                                    const size_t j, const size_t offset,
                                    const size_t n);

gsl_matrix_complex_view
gsl_matrix_complex_view_array (double * base,
                             const size_t n1,
                             const size_t n2);

gsl_matrix_complex_view
gsl_matrix_complex_view_array_with_tda (double * base,
                                      const size_t n1,
                                      const size_t n2,
                                      const size_t tda);

gsl_matrix_complex_view
gsl_matrix_complex_view_vector (gsl_vector_complex * v,
                              const size_t n1,
                              const size_t n2);

gsl_matrix_complex_view
gsl_matrix_complex_view_vector_with_tda (gsl_vector_complex * v,
                                       const size_t n1,
                                       const size_t n2,
                                       const size_t tda);


gsl_matrix_complex_const_view
gsl_matrix_complex_const_submatrix (const gsl_matrix_complex * m,
                                  const size_t i, const size_t j,
                                  const size_t n1, const size_t n2);

gsl_vector_complex_const_view
gsl_matrix_complex_const_row (const gsl_matrix_complex * m,
                            const size_t i);

gsl_vector_complex_const_view
gsl_matrix_complex_const_column (const gsl_matrix_complex * m,
                               const size_t j);

gsl_vector_complex_const_view
gsl_matrix_complex_const_diagonal (const gsl_matrix_complex * m);

gsl_vector_complex_const_view
gsl_matrix_complex_const_subdiagonal (const gsl_matrix_complex * m,
                                    const size_t k);

gsl_vector_complex_const_view
gsl_matrix_complex_const_superdiagonal (const gsl_matrix_complex * m,
                                      const size_t k);

gsl_vector_complex_const_view
gsl_matrix_complex_const_subrow (const gsl_matrix_complex * m,
                                       const size_t i, const size_t offset,
                                       const size_t n);

gsl_vector_complex_const_view
gsl_matrix_complex_const_subcolumn (const gsl_matrix_complex * m,
                                          const size_t j, const size_t offset,
                                          const size_t n);

gsl_matrix_complex_const_view
gsl_matrix_complex_const_view_array (const double * base,
                                   const size_t n1,
                                   const size_t n2);

gsl_matrix_complex_const_view
gsl_matrix_complex_const_view_array_with_tda (const double * base,
                                            const size_t n1,
                                            const size_t n2,
                                            const size_t tda);

gsl_matrix_complex_const_view
gsl_matrix_complex_const_view_vector (const gsl_vector_complex * v,
                                    const size_t n1,
                                    const size_t n2);

gsl_matrix_complex_const_view
gsl_matrix_complex_const_view_vector_with_tda (const gsl_vector_complex * v,
                                             const size_t n1,
                                             const size_t n2,
                                             const size_t tda);



void gsl_matrix_complex_set_zero (gsl_matrix_complex * m);
void gsl_matrix_complex_set_identity (gsl_matrix_complex * m);
void gsl_matrix_complex_set_all (gsl_matrix_complex * m, gsl_complex x);

int gsl_matrix_complex_fread (FILE * stream, gsl_matrix_complex * m) ;
int gsl_matrix_complex_fwrite (FILE * stream, const gsl_matrix_complex * m) ;
int gsl_matrix_complex_fscanf (FILE * stream, gsl_matrix_complex * m);
int gsl_matrix_complex_fprintf (FILE * stream, const gsl_matrix_complex * m, const char * format);

int gsl_matrix_complex_memcpy(gsl_matrix_complex * dest, const gsl_matrix_complex * src);
int gsl_matrix_complex_swap(gsl_matrix_complex * m1, gsl_matrix_complex * m2);

int gsl_matrix_complex_swap_rows(gsl_matrix_complex * m, const size_t i, const size_t j);
int gsl_matrix_complex_swap_columns(gsl_matrix_complex * m, const size_t i, const size_t j);
int gsl_matrix_complex_swap_rowcol(gsl_matrix_complex * m, const size_t i, const size_t j);

int gsl_matrix_complex_transpose (gsl_matrix_complex * m);
int gsl_matrix_complex_transpose_memcpy (gsl_matrix_complex * dest, const gsl_matrix_complex * src);

int gsl_matrix_complex_isnull (const gsl_matrix_complex * m);
int gsl_matrix_complex_ispos (const gsl_matrix_complex * m);
int gsl_matrix_complex_isneg (const gsl_matrix_complex * m);
int gsl_matrix_complex_isnonneg (const gsl_matrix_complex * m);

int gsl_matrix_complex_add (gsl_matrix_complex * a, const gsl_matrix_complex * b);
int gsl_matrix_complex_sub (gsl_matrix_complex * a, const gsl_matrix_complex * b);
int gsl_matrix_complex_mul_elements (gsl_matrix_complex * a, const gsl_matrix_complex * b);
int gsl_matrix_complex_div_elements (gsl_matrix_complex * a, const gsl_matrix_complex * b);
int gsl_matrix_complex_scale (gsl_matrix_complex * a, const gsl_complex x);
int gsl_matrix_complex_add_constant (gsl_matrix_complex * a, const gsl_complex x);
int gsl_matrix_complex_add_diagonal (gsl_matrix_complex * a, const gsl_complex x);

gsl_complex gsl_matrix_complex_get(const gsl_matrix_complex * m, const size_t i, const size_t j);
void gsl_matrix_complex_set(gsl_matrix_complex * m, const size_t i, const size_t j, const gsl_complex x);

gsl_complex * gsl_matrix_complex_ptr(gsl_matrix_complex * m, const size_t i, const size_t j);
const gsl_complex * gsl_matrix_complex_const_ptr(const gsl_matrix_complex * m, const size_t i, const size_t j);

gsl_permutation *gsl_permutation_alloc (const size_t n);
gsl_permutation *gsl_permutation_calloc (const size_t n);
void gsl_permutation_init (gsl_permutation * p);
void gsl_permutation_free (gsl_permutation * p);
int gsl_permutation_memcpy (gsl_permutation * dest, const gsl_permutation * src);

int gsl_permutation_fread (FILE * stream, gsl_permutation * p);
int gsl_permutation_fwrite (FILE * stream, const gsl_permutation * p);
int gsl_permutation_fscanf (FILE * stream, gsl_permutation * p);
int gsl_permutation_fprintf (FILE * stream, const gsl_permutation * p, const char *format);

size_t gsl_permutation_size (const gsl_permutation * p);
size_t * gsl_permutation_data (const gsl_permutation * p);

int gsl_permutation_swap (gsl_permutation * p, const size_t i, const size_t j);

int gsl_permutation_valid (const gsl_permutation * p);
void gsl_permutation_reverse (gsl_permutation * p);
int gsl_permutation_inverse (gsl_permutation * inv, const gsl_permutation * p);
int gsl_permutation_next (gsl_permutation * p);
int gsl_permutation_prev (gsl_permutation * p);
int gsl_permutation_mul (gsl_permutation * p, const gsl_permutation * pa, const gsl_permutation * pb);

int gsl_permutation_linear_to_canonical (gsl_permutation * q, const gsl_permutation * p);
int gsl_permutation_canonical_to_linear (gsl_permutation * p, const gsl_permutation * q);

size_t gsl_permutation_inversions (const gsl_permutation * p);
size_t gsl_permutation_linear_cycles (const gsl_permutation * p);
size_t gsl_permutation_canonical_cycles (const gsl_permutation * q);

size_t gsl_permutation_get (const gsl_permutation * p, const size_t i);

int gsl_permute_vector (const gsl_permutation * p, gsl_vector * v);
int gsl_permute_vector_inverse (const gsl_permutation * p, gsl_vector * v);



double gsl_linalg_householder_transform (gsl_vector * v);
gsl_complex gsl_linalg_complex_householder_transform (gsl_vector_complex * v);

int gsl_linalg_householder_hm (double tau,
                               const gsl_vector * v,
                               gsl_matrix * A);

int gsl_linalg_householder_mh (double tau,
                               const gsl_vector * v,
                               gsl_matrix * A);

int gsl_linalg_householder_hv (double tau,
                               const gsl_vector * v,
                               gsl_vector * w);

int gsl_linalg_householder_hm1 (double tau,
                                gsl_matrix * A);

int gsl_linalg_complex_householder_hm (gsl_complex tau,
                                       const gsl_vector_complex * v,
                                       gsl_matrix_complex * A);

int gsl_linalg_complex_householder_mh (gsl_complex tau,
                                       const gsl_vector_complex * v,
                                       gsl_matrix_complex * A);

int gsl_linalg_complex_householder_hv (gsl_complex tau,
                                       const gsl_vector_complex * v,
                                       gsl_vector_complex * w);



int gsl_linalg_hessenberg_decomp(gsl_matrix *A, gsl_vector *tau);
int gsl_linalg_hessenberg_unpack(gsl_matrix * H, gsl_vector * tau,
                                 gsl_matrix * U);
int gsl_linalg_hessenberg_unpack_accum(gsl_matrix * H, gsl_vector * tau,
                                       gsl_matrix * U);
int gsl_linalg_hessenberg_set_zero(gsl_matrix * H);
int gsl_linalg_hessenberg_submatrix(gsl_matrix *M, gsl_matrix *A,
                                    size_t top, gsl_vector *tau);


int gsl_linalg_hessenberg(gsl_matrix *A, gsl_vector *tau);




int gsl_linalg_hesstri_decomp(gsl_matrix * A, gsl_matrix * B,
                              gsl_matrix * U, gsl_matrix * V,
                              gsl_vector * work);

int
gsl_linalg_SV_decomp (gsl_matrix * A,
                      gsl_matrix * V,
                      gsl_vector * S,
                      gsl_vector * work);

int
gsl_linalg_SV_decomp_mod (gsl_matrix * A,
                          gsl_matrix * X,
                          gsl_matrix * V,
                          gsl_vector * S,
                          gsl_vector * work);

int gsl_linalg_SV_decomp_jacobi (gsl_matrix * A,
                                 gsl_matrix * Q,
                                 gsl_vector * S);

int
gsl_linalg_SV_solve (const gsl_matrix * U,
                     const gsl_matrix * Q,
                     const gsl_vector * S,
                     const gsl_vector * b,
                     gsl_vector * x);

int gsl_linalg_LU_decomp (gsl_matrix * A, gsl_permutation * p, int *signum);

int gsl_linalg_LU_solve (const gsl_matrix * LU,
                         const gsl_permutation * p,
                         const gsl_vector * b,
                         gsl_vector * x);

int gsl_linalg_LU_svx (const gsl_matrix * LU,
                       const gsl_permutation * p,
                       gsl_vector * x);

int gsl_linalg_LU_refine (const gsl_matrix * A,
                          const gsl_matrix * LU,
                          const gsl_permutation * p,
                          const gsl_vector * b,
                          gsl_vector * x,
                          gsl_vector * residual);

int gsl_linalg_LU_invert (const gsl_matrix * LU,
                          const gsl_permutation * p,
                          gsl_matrix * inverse);

double gsl_linalg_LU_det (gsl_matrix * LU, int signum);
double gsl_linalg_LU_lndet (gsl_matrix * LU);
int gsl_linalg_LU_sgndet (gsl_matrix * lu, int signum);



int gsl_linalg_complex_LU_decomp (gsl_matrix_complex * A,
                                  gsl_permutation * p,
                                  int *signum);

int gsl_linalg_complex_LU_solve (const gsl_matrix_complex * LU,
                                 const gsl_permutation * p,
                                 const gsl_vector_complex * b,
                                 gsl_vector_complex * x);

int gsl_linalg_complex_LU_svx (const gsl_matrix_complex * LU,
                               const gsl_permutation * p,
                               gsl_vector_complex * x);

int gsl_linalg_complex_LU_refine (const gsl_matrix_complex * A,
                                  const gsl_matrix_complex * LU,
                                  const gsl_permutation * p,
                                  const gsl_vector_complex * b,
                                  gsl_vector_complex * x,
                                  gsl_vector_complex * residual);

int gsl_linalg_complex_LU_invert (const gsl_matrix_complex * LU,
                                  const gsl_permutation * p,
                                  gsl_matrix_complex * inverse);

gsl_complex gsl_linalg_complex_LU_det (gsl_matrix_complex * LU,
                                       int signum);

double gsl_linalg_complex_LU_lndet (gsl_matrix_complex * LU);

gsl_complex gsl_linalg_complex_LU_sgndet (gsl_matrix_complex * LU,
                                          int signum);



int gsl_linalg_QR_decomp (gsl_matrix * A,
                          gsl_vector * tau);

int gsl_linalg_QR_solve (const gsl_matrix * QR,
                         const gsl_vector * tau,
                         const gsl_vector * b,
                         gsl_vector * x);

int gsl_linalg_QR_svx (const gsl_matrix * QR,
                       const gsl_vector * tau,
                       gsl_vector * x);

int gsl_linalg_QR_lssolve (const gsl_matrix * QR,
                           const gsl_vector * tau,
                           const gsl_vector * b,
                           gsl_vector * x,
                           gsl_vector * residual);


int gsl_linalg_QR_QRsolve (gsl_matrix * Q,
                           gsl_matrix * R,
                           const gsl_vector * b,
                           gsl_vector * x);

int gsl_linalg_QR_Rsolve (const gsl_matrix * QR,
                          const gsl_vector * b,
                          gsl_vector * x);

int gsl_linalg_QR_Rsvx (const gsl_matrix * QR,
                        gsl_vector * x);

int gsl_linalg_QR_update (gsl_matrix * Q,
                          gsl_matrix * R,
                          gsl_vector * w,
                          const gsl_vector * v);

int gsl_linalg_QR_QTvec (const gsl_matrix * QR,
                         const gsl_vector * tau,
                         gsl_vector * v);

int gsl_linalg_QR_Qvec (const gsl_matrix * QR,
                        const gsl_vector * tau,
                        gsl_vector * v);

int gsl_linalg_QR_QTmat (const gsl_matrix * QR,
                         const gsl_vector * tau,
                         gsl_matrix * A);

int gsl_linalg_QR_unpack (const gsl_matrix * QR,
                          const gsl_vector * tau,
                          gsl_matrix * Q,
                          gsl_matrix * R);

int gsl_linalg_R_solve (const gsl_matrix * R,
                        const gsl_vector * b,
                        gsl_vector * x);

int gsl_linalg_R_svx (const gsl_matrix * R,
                      gsl_vector * x);




int gsl_linalg_QRPT_decomp (gsl_matrix * A,
                            gsl_vector * tau,
                            gsl_permutation * p,
                            int *signum,
                            gsl_vector * norm);

int gsl_linalg_QRPT_decomp2 (const gsl_matrix * A,
                             gsl_matrix * q, gsl_matrix * r,
                             gsl_vector * tau,
                             gsl_permutation * p,
                             int *signum,
                             gsl_vector * norm);

int gsl_linalg_QRPT_solve (const gsl_matrix * QR,
                           const gsl_vector * tau,
                           const gsl_permutation * p,
                           const gsl_vector * b,
                           gsl_vector * x);


int gsl_linalg_QRPT_svx (const gsl_matrix * QR,
                         const gsl_vector * tau,
                         const gsl_permutation * p,
                         gsl_vector * x);

int gsl_linalg_QRPT_QRsolve (const gsl_matrix * Q,
                             const gsl_matrix * R,
                             const gsl_permutation * p,
                             const gsl_vector * b,
                             gsl_vector * x);

int gsl_linalg_QRPT_Rsolve (const gsl_matrix * QR,
                             const gsl_permutation * p,
                             const gsl_vector * b,
                             gsl_vector * x);

int gsl_linalg_QRPT_Rsvx (const gsl_matrix * QR,
                           const gsl_permutation * p,
                           gsl_vector * x);

int gsl_linalg_QRPT_update (gsl_matrix * Q,
                            gsl_matrix * R,
                            const gsl_permutation * p,
                            gsl_vector * u,
                            const gsl_vector * v);



int gsl_linalg_LQ_decomp (gsl_matrix * A, gsl_vector * tau);

int gsl_linalg_LQ_solve_T (const gsl_matrix * LQ, const gsl_vector * tau,
			 const gsl_vector * b, gsl_vector * x);

int gsl_linalg_LQ_svx_T (const gsl_matrix * LQ, const gsl_vector * tau,
                         gsl_vector * x);

int gsl_linalg_LQ_lssolve_T (const gsl_matrix * LQ, const gsl_vector * tau,
			   const gsl_vector * b, gsl_vector * x,
			   gsl_vector * residual);

int gsl_linalg_LQ_Lsolve_T (const gsl_matrix * LQ, const gsl_vector * b,
			  gsl_vector * x);

int gsl_linalg_LQ_Lsvx_T (const gsl_matrix * LQ, gsl_vector * x);

int gsl_linalg_L_solve_T (const gsl_matrix * L, const gsl_vector * b,
			gsl_vector * x);

int gsl_linalg_LQ_vecQ (const gsl_matrix * LQ, const gsl_vector * tau,
			gsl_vector * v);

int gsl_linalg_LQ_vecQT (const gsl_matrix * LQ, const gsl_vector * tau,
			 gsl_vector * v);

int gsl_linalg_LQ_unpack (const gsl_matrix * LQ, const gsl_vector * tau,
			  gsl_matrix * Q, gsl_matrix * L);

int gsl_linalg_LQ_update (gsl_matrix * Q, gsl_matrix * R,
			  const gsl_vector * v, gsl_vector * w);
int gsl_linalg_LQ_LQsolve (gsl_matrix * Q, gsl_matrix * L,
			   const gsl_vector * b, gsl_vector * x);



int gsl_linalg_PTLQ_decomp (gsl_matrix * A, gsl_vector * tau,
			    gsl_permutation * p, int *signum,
			    gsl_vector * norm);

int gsl_linalg_PTLQ_decomp2 (const gsl_matrix * A, gsl_matrix * q,
			     gsl_matrix * r, gsl_vector * tau,
			     gsl_permutation * p, int *signum,
			     gsl_vector * norm);

int gsl_linalg_PTLQ_solve_T (const gsl_matrix * QR,
			   const gsl_vector * tau,
			   const gsl_permutation * p,
			   const gsl_vector * b,
			   gsl_vector * x);

int gsl_linalg_PTLQ_svx_T (const gsl_matrix * LQ,
                           const gsl_vector * tau,
                           const gsl_permutation * p,
                           gsl_vector * x);

int gsl_linalg_PTLQ_LQsolve_T (const gsl_matrix * Q, const gsl_matrix * L,
			     const gsl_permutation * p,
			     const gsl_vector * b,
			     gsl_vector * x);

int gsl_linalg_PTLQ_Lsolve_T (const gsl_matrix * LQ,
			    const gsl_permutation * p,
			    const gsl_vector * b,
			    gsl_vector * x);

int gsl_linalg_PTLQ_Lsvx_T (const gsl_matrix * LQ,
			  const gsl_permutation * p,
			  gsl_vector * x);

int gsl_linalg_PTLQ_update (gsl_matrix * Q, gsl_matrix * L,
			    const gsl_permutation * p,
			    const gsl_vector * v, gsl_vector * w);



int gsl_linalg_cholesky_decomp (gsl_matrix * A);

int gsl_linalg_cholesky_solve (const gsl_matrix * cholesky,
                               const gsl_vector * b,
                               gsl_vector * x);

int gsl_linalg_cholesky_svx (const gsl_matrix * cholesky,
                             gsl_vector * x);

int gsl_linalg_cholesky_invert(gsl_matrix * cholesky);

int gsl_linalg_cholesky_decomp_unit(gsl_matrix * A, gsl_vector * D);



int gsl_linalg_complex_cholesky_decomp (gsl_matrix_complex * A);

int gsl_linalg_complex_cholesky_solve (const gsl_matrix_complex * cholesky,
                                       const gsl_vector_complex * b,
                                       gsl_vector_complex * x);

int gsl_linalg_complex_cholesky_svx (const gsl_matrix_complex * cholesky,
                                     gsl_vector_complex * x);



int gsl_linalg_symmtd_decomp (gsl_matrix * A,
                              gsl_vector * tau);

int gsl_linalg_symmtd_unpack (const gsl_matrix * A,
                              const gsl_vector * tau,
                              gsl_matrix * Q,
                              gsl_vector * diag,
                              gsl_vector * subdiag);

int gsl_linalg_symmtd_unpack_T (const gsl_matrix * A,
                                gsl_vector * diag,
                                gsl_vector * subdiag);



int gsl_linalg_hermtd_decomp (gsl_matrix_complex * A,
                              gsl_vector_complex * tau);

int gsl_linalg_hermtd_unpack (const gsl_matrix_complex * A,
                              const gsl_vector_complex * tau,
                              gsl_matrix_complex * U,
                              gsl_vector * diag,
                              gsl_vector * sudiag);

int gsl_linalg_hermtd_unpack_T (const gsl_matrix_complex * A,
                                gsl_vector * diag,
                                gsl_vector * subdiag);

int gsl_linalg_HH_solve (gsl_matrix * A, const gsl_vector * b, gsl_vector * x);
int gsl_linalg_HH_svx (gsl_matrix * A, gsl_vector * x);

int gsl_linalg_solve_symm_tridiag (const gsl_vector * diag,
                                   const gsl_vector * offdiag,
                                   const gsl_vector * b,
                                   gsl_vector * x);

int gsl_linalg_solve_tridiag (const gsl_vector * diag,
                                   const gsl_vector * abovediag,
                                   const gsl_vector * belowdiag,
                                   const gsl_vector * b,
                                   gsl_vector * x);


int gsl_linalg_solve_symm_cyc_tridiag (const gsl_vector * diag,
                                       const gsl_vector * offdiag,
                                       const gsl_vector * b,
                                       gsl_vector * x);

int gsl_linalg_solve_cyc_tridiag (const gsl_vector * diag,
                                  const gsl_vector * abovediag,
                                  const gsl_vector * belowdiag,
                                  const gsl_vector * b,
                                  gsl_vector * x);




int gsl_linalg_bidiag_decomp (gsl_matrix * A,
                              gsl_vector * tau_U,
                              gsl_vector * tau_V);

int gsl_linalg_bidiag_unpack (const gsl_matrix * A,
                              const gsl_vector * tau_U,
                              gsl_matrix * U,
                              const gsl_vector * tau_V,
                              gsl_matrix * V,
                              gsl_vector * diag,
                              gsl_vector * superdiag);

int gsl_linalg_bidiag_unpack2 (gsl_matrix * A,
                               gsl_vector * tau_U,
                               gsl_vector * tau_V,
                               gsl_matrix * V);

int gsl_linalg_bidiag_unpack_B (const gsl_matrix * A,
                                gsl_vector * diag,
                                gsl_vector * superdiag);



int gsl_linalg_balance_matrix (gsl_matrix * A, gsl_vector * D);
int gsl_linalg_balance_accum (gsl_matrix * A, gsl_vector * D);
int gsl_linalg_balance_columns (gsl_matrix * A, gsl_vector * D);

enum CBLAS_ORDER {CblasRowMajor=101, CblasColMajor=102};
enum CBLAS_TRANSPOSE {CblasNoTrans=111, CblasTrans=112, CblasConjTrans=113};
enum CBLAS_UPLO {CblasUpper=121, CblasLower=122};
enum CBLAS_DIAG {CblasNonUnit=131, CblasUnit=132};
enum CBLAS_SIDE {CblasLeft=141, CblasRight=142};

typedef  size_t  CBLAS_INDEX_t;
typedef  enum CBLAS_ORDER       CBLAS_ORDER_t;
typedef  enum CBLAS_TRANSPOSE   CBLAS_TRANSPOSE_t;
typedef  enum CBLAS_UPLO        CBLAS_UPLO_t;
typedef  enum CBLAS_DIAG        CBLAS_DIAG_t;
typedef  enum CBLAS_SIDE        CBLAS_SIDE_t;

int gsl_blas_ddot (const gsl_vector * X,
                   const gsl_vector * Y,
                   double * result
                   );

int  gsl_blas_zdotu (const gsl_vector_complex * X,
                     const gsl_vector_complex * Y,
                     gsl_complex * dotu);

int  gsl_blas_zdotc (const gsl_vector_complex * X,
                     const gsl_vector_complex * Y,
                     gsl_complex * dotc);


double gsl_blas_dnrm2  (const gsl_vector * X);
double gsl_blas_dasum  (const gsl_vector * X);
double gsl_blas_dznrm2 (const gsl_vector_complex * X);
double gsl_blas_dzasum (const gsl_vector_complex * X);


CBLAS_INDEX_t gsl_blas_idamax (const gsl_vector * X);
CBLAS_INDEX_t gsl_blas_izamax (const gsl_vector_complex * X);

int  gsl_blas_dswap (gsl_vector * X,
                     gsl_vector * Y);

int  gsl_blas_dcopy (const gsl_vector * X,
                     gsl_vector * Y);

int  gsl_blas_daxpy (double alpha,
                     const gsl_vector * X,
                     gsl_vector * Y);

int  gsl_blas_zswap (gsl_vector_complex * X,
                     gsl_vector_complex * Y);

int  gsl_blas_zcopy (const gsl_vector_complex * X,
                     gsl_vector_complex * Y);

int  gsl_blas_zaxpy (const gsl_complex alpha,
                     const gsl_vector_complex * X,
                     gsl_vector_complex * Y);


int  gsl_blas_drotg (double a[], double b[], double c[], double s[]);

int  gsl_blas_drotmg (double d1[], double d2[], double b1[],
                      double b2, double P[]);

int  gsl_blas_drot (gsl_vector * X,
                    gsl_vector * Y,
                    const double c, const double s);

int  gsl_blas_drotm (gsl_vector * X,
                     gsl_vector * Y,
                     const double P[]);

void gsl_blas_dscal  (double alpha, gsl_vector * X);
void gsl_blas_zscal  (const gsl_complex alpha, gsl_vector_complex * X);
void gsl_blas_zdscal (double alpha, gsl_vector_complex * X);

int  gsl_blas_dgemv (CBLAS_TRANSPOSE_t TransA,
                     double alpha,
                     const gsl_matrix * A,
                     const gsl_vector * X,
                     double beta,
                     gsl_vector * Y);

int  gsl_blas_dtrmv (CBLAS_UPLO_t Uplo,
                     CBLAS_TRANSPOSE_t TransA, CBLAS_DIAG_t Diag,
                     const gsl_matrix * A,
                     gsl_vector * X);

int  gsl_blas_dtrsv (CBLAS_UPLO_t Uplo,
                     CBLAS_TRANSPOSE_t TransA, CBLAS_DIAG_t Diag,
                     const gsl_matrix * A,
                     gsl_vector * X);

int  gsl_blas_zgemv (CBLAS_TRANSPOSE_t TransA,
                     const gsl_complex alpha,
                     const gsl_matrix_complex * A,
                     const gsl_vector_complex * X,
                     const gsl_complex beta,
                     gsl_vector_complex * Y);

int  gsl_blas_ztrmv (CBLAS_UPLO_t Uplo,
                     CBLAS_TRANSPOSE_t TransA, CBLAS_DIAG_t Diag,
                     const gsl_matrix_complex * A,
                     gsl_vector_complex * X);

int  gsl_blas_ztrsv (CBLAS_UPLO_t Uplo,
                     CBLAS_TRANSPOSE_t TransA, CBLAS_DIAG_t Diag,
                     const gsl_matrix_complex * A,
                     gsl_vector_complex *X);

int  gsl_blas_dsymv (CBLAS_UPLO_t Uplo,
                     double alpha,
                     const gsl_matrix * A,
                     const gsl_vector * X,
                     double beta,
                     gsl_vector * Y);
int  gsl_blas_dger (double alpha,
                    const gsl_vector * X,
                    const gsl_vector * Y,
                    gsl_matrix * A);

int  gsl_blas_dsyr (CBLAS_UPLO_t Uplo,
                    double alpha,
                    const gsl_vector * X,
                    gsl_matrix * A);

int  gsl_blas_dsyr2 (CBLAS_UPLO_t Uplo,
                     double alpha,
                     const gsl_vector * X,
                     const gsl_vector * Y,
                     gsl_matrix * A);


int  gsl_blas_zhemv (CBLAS_UPLO_t Uplo,
                     const gsl_complex alpha,
                     const gsl_matrix_complex * A,
                     const gsl_vector_complex * X,
                     const gsl_complex beta,
                     gsl_vector_complex * Y);

int  gsl_blas_zgeru (const gsl_complex alpha,
                     const gsl_vector_complex * X,
                     const gsl_vector_complex * Y,
                     gsl_matrix_complex * A);

int  gsl_blas_zgerc (const gsl_complex alpha,
                     const gsl_vector_complex * X,
                     const gsl_vector_complex * Y,
                     gsl_matrix_complex * A);

int  gsl_blas_zher (CBLAS_UPLO_t Uplo,
                    double alpha,
                    const gsl_vector_complex * X,
                    gsl_matrix_complex * A);

int  gsl_blas_zher2 (CBLAS_UPLO_t Uplo,
                     const gsl_complex alpha,
                     const gsl_vector_complex * X,
                     const gsl_vector_complex * Y,
                     gsl_matrix_complex * A);

int  gsl_blas_dgemm (CBLAS_TRANSPOSE_t TransA,
                     CBLAS_TRANSPOSE_t TransB,
                     double alpha,
                     const gsl_matrix * A,
                     const gsl_matrix * B,
                     double beta,
                     gsl_matrix * C);

int  gsl_blas_dsymm (CBLAS_SIDE_t Side,
                     CBLAS_UPLO_t Uplo,
                     double alpha,
                     const gsl_matrix * A,
                     const gsl_matrix * B,
                     double beta,
                     gsl_matrix * C);

int  gsl_blas_dsyrk (CBLAS_UPLO_t Uplo,
                     CBLAS_TRANSPOSE_t Trans,
                     double alpha,
                     const gsl_matrix * A,
                     double beta,
                     gsl_matrix * C);

int  gsl_blas_dsyr2k (CBLAS_UPLO_t Uplo,
                      CBLAS_TRANSPOSE_t Trans,
                      double alpha,
                      const  gsl_matrix * A,
                      const  gsl_matrix * B,
                      double beta,
                      gsl_matrix * C);

int  gsl_blas_dtrmm (CBLAS_SIDE_t Side,
                     CBLAS_UPLO_t Uplo, CBLAS_TRANSPOSE_t TransA,
                     CBLAS_DIAG_t Diag,
                     double alpha,
                     const gsl_matrix * A,
                     gsl_matrix * B);

int  gsl_blas_dtrsm (CBLAS_SIDE_t Side,
                     CBLAS_UPLO_t Uplo, CBLAS_TRANSPOSE_t TransA,
                     CBLAS_DIAG_t Diag,
                     double alpha,
                     const gsl_matrix * A,
                     gsl_matrix * B);


int  gsl_blas_zgemm (CBLAS_TRANSPOSE_t TransA,
                     CBLAS_TRANSPOSE_t TransB,
                     const gsl_complex alpha,
                     const gsl_matrix_complex * A,
                     const gsl_matrix_complex * B,
                     const gsl_complex beta,
                     gsl_matrix_complex * C);

int  gsl_blas_zsymm (CBLAS_SIDE_t Side,
                     CBLAS_UPLO_t Uplo,
                     const gsl_complex alpha,
                     const gsl_matrix_complex * A,
                     const gsl_matrix_complex * B,
                     const gsl_complex beta,
                     gsl_matrix_complex * C);

int  gsl_blas_zsyrk (CBLAS_UPLO_t Uplo,
                     CBLAS_TRANSPOSE_t Trans,
                     const gsl_complex alpha,
                     const gsl_matrix_complex * A,
                     const gsl_complex beta,
                     gsl_matrix_complex * C);

int  gsl_blas_zsyr2k (CBLAS_UPLO_t Uplo,
                      CBLAS_TRANSPOSE_t Trans,
                      const gsl_complex alpha,
                      const gsl_matrix_complex * A,
                      const gsl_matrix_complex * B,
                      const gsl_complex beta,
                      gsl_matrix_complex *C);

int  gsl_blas_ztrmm (CBLAS_SIDE_t Side,
                     CBLAS_UPLO_t Uplo, CBLAS_TRANSPOSE_t TransA,
                     CBLAS_DIAG_t Diag,
                     const gsl_complex alpha,
                     const gsl_matrix_complex * A,
                     gsl_matrix_complex * B);

int  gsl_blas_ztrsm (CBLAS_SIDE_t Side,
                     CBLAS_UPLO_t Uplo, CBLAS_TRANSPOSE_t TransA,
                     CBLAS_DIAG_t Diag,
                     const gsl_complex alpha,
                     const gsl_matrix_complex * A,
                     gsl_matrix_complex * B);

int  gsl_blas_zhemm (CBLAS_SIDE_t Side,
                     CBLAS_UPLO_t Uplo,
                     const gsl_complex alpha,
                     const gsl_matrix_complex * A,
                     const gsl_matrix_complex * B,
                     const gsl_complex beta,
                     gsl_matrix_complex * C);

int  gsl_blas_zherk (CBLAS_UPLO_t Uplo,
                     CBLAS_TRANSPOSE_t Trans,
                     double alpha,
                     const gsl_matrix_complex * A,
                     double beta,
                     gsl_matrix_complex * C);

int  gsl_blas_zher2k (CBLAS_UPLO_t Uplo,
                      CBLAS_TRANSPOSE_t Trans,
                      const gsl_complex alpha,
                      const gsl_matrix_complex * A,
                      const gsl_matrix_complex * B,
                      double beta,
                      gsl_matrix_complex * C);

]]

ffi.cdef [[
typedef struct
{
    size_t k;
    size_t km1;
    size_t l;
    size_t nbreak;
    size_t n;

    gsl_vector *knots;
    gsl_vector *deltal;
    gsl_vector *deltar;
    gsl_vector *B;
} gsl_bspline_workspace;

typedef struct
{
    size_t k;
    gsl_matrix *A;
    gsl_matrix *dB;
} gsl_bspline_deriv_workspace;

gsl_bspline_workspace * gsl_bspline_alloc(const size_t k, const size_t nbreak);

void gsl_bspline_free(gsl_bspline_workspace *w);

size_t gsl_bspline_ncoeffs(gsl_bspline_workspace * w);
size_t gsl_bspline_order(gsl_bspline_workspace * w);
size_t gsl_bspline_nbreak(gsl_bspline_workspace * w);
double gsl_bspline_breakpoint(size_t i, gsl_bspline_workspace * w);
double gsl_bspline_greville_abscissa(size_t i, gsl_bspline_workspace *w);

int gsl_bspline_knots(const gsl_vector *breakpts, gsl_bspline_workspace *w);

int gsl_bspline_knots_uniform(const double a, const double b,
                              gsl_bspline_workspace *w);

int gsl_bspline_eval(const double x, gsl_vector *B, gsl_bspline_workspace *w);

int
gsl_bspline_eval_nonzero(const double x,
                         gsl_vector *Bk,
                         size_t *istart,
                         size_t *iend,
                         gsl_bspline_workspace *w);

gsl_bspline_deriv_workspace *
gsl_bspline_deriv_alloc(const size_t k);

void
gsl_bspline_deriv_free(gsl_bspline_deriv_workspace *w);

int
gsl_bspline_deriv_eval(const double x,
                       const size_t nderiv,
                       gsl_matrix *dB,
                       gsl_bspline_workspace *w,
                       gsl_bspline_deriv_workspace *dw);

int
gsl_bspline_deriv_eval_nonzero(const double x,
                               const size_t nderiv,
                               gsl_matrix *dB,
                               size_t *istart,
                               size_t *iend,
                               gsl_bspline_workspace *w,
                               gsl_bspline_deriv_workspace *dw);
]]

ffi.cdef[[
      typedef struct
      {
	 size_t n; /* number of observations */
	 size_t p; /* number of parameters */
	 gsl_matrix * A;
	 gsl_matrix * Q;
	 gsl_matrix * QSI;
	 gsl_vector * S;
	 gsl_vector * t;
	 gsl_vector * xt;
	 gsl_vector * D;
      } gsl_multifit_linear_workspace;

      gsl_multifit_linear_workspace * gsl_multifit_linear_alloc (size_t n, size_t p);

      void gsl_multifit_linear_free (gsl_multifit_linear_workspace * work);

      int
      gsl_multifit_linear (const gsl_matrix * X,
			   const gsl_vector * y,
			   gsl_vector * c,
			   gsl_matrix * cov,
			   double * chisq,
			   gsl_multifit_linear_workspace * work);

      int
      gsl_multifit_wlinear (const gsl_matrix * X,
			    const gsl_vector * w,
			    const gsl_vector * y,
			    gsl_vector * c,
			    gsl_matrix * cov,
			    double * chisq,
			    gsl_multifit_linear_workspace * work);

typedef struct
  {
    const char *name;
    unsigned long int max;
    unsigned long int min;
    size_t size;
    void (*set) (void *state, unsigned long int seed);
    unsigned long int (*get) (void *state);
    double (*get_double) (void *state);
  }
gsl_rng_type;

typedef struct
  {
    const gsl_rng_type * type;
    void *state;
  }
gsl_rng;

const gsl_rng_type ** gsl_rng_types_setup(void);

const gsl_rng_type *gsl_rng_default;
unsigned long int gsl_rng_default_seed;

gsl_rng *gsl_rng_alloc (const gsl_rng_type * T);
int gsl_rng_memcpy (gsl_rng * dest, const gsl_rng * src);
gsl_rng *gsl_rng_clone (const gsl_rng * r);

void gsl_rng_free (gsl_rng * r);

void gsl_rng_set (const gsl_rng * r, unsigned long int seed);
unsigned long int gsl_rng_max (const gsl_rng * r);
unsigned long int gsl_rng_min (const gsl_rng * r);
const char *gsl_rng_name (const gsl_rng * r);

int gsl_rng_fread (FILE * stream, gsl_rng * r);
int gsl_rng_fwrite (FILE * stream, const gsl_rng * r);

size_t gsl_rng_size (const gsl_rng * r);
void * gsl_rng_state (const gsl_rng * r);

void gsl_rng_print_state (const gsl_rng * r);

const gsl_rng_type * gsl_rng_env_setup (void);

unsigned long int gsl_rng_get (const gsl_rng * r);
double gsl_rng_uniform (const gsl_rng * r);
double gsl_rng_uniform_pos (const gsl_rng * r);
unsigned long int gsl_rng_uniform_int (const gsl_rng * r, unsigned long int n);

unsigned int gsl_ran_bernoulli (const gsl_rng * r, double p);
double gsl_ran_bernoulli_pdf (const unsigned int k, double p);

double gsl_ran_beta (const gsl_rng * r, const double a, const double b);
double gsl_ran_beta_pdf (const double x, const double a, const double b);

unsigned int gsl_ran_binomial (const gsl_rng * r, double p, unsigned int n);
unsigned int gsl_ran_binomial_knuth (const gsl_rng * r, double p, unsigned int n);
unsigned int gsl_ran_binomial_tpe (const gsl_rng * r, double p, unsigned int n);
double gsl_ran_binomial_pdf (const unsigned int k, const double p, const unsigned int n);

double gsl_ran_exponential (const gsl_rng * r, const double mu);
double gsl_ran_exponential_pdf (const double x, const double mu);

double gsl_ran_exppow (const gsl_rng * r, const double a, const double b);
double gsl_ran_exppow_pdf (const double x, const double a, const double b);

double gsl_ran_cauchy (const gsl_rng * r, const double a);
double gsl_ran_cauchy_pdf (const double x, const double a);

double gsl_ran_chisq (const gsl_rng * r, const double nu);
double gsl_ran_chisq_pdf (const double x, const double nu);

double gsl_ran_erlang (const gsl_rng * r, const double a, const double n);
double gsl_ran_erlang_pdf (const double x, const double a, const double n);

double gsl_ran_fdist (const gsl_rng * r, const double nu1, const double nu2);
double gsl_ran_fdist_pdf (const double x, const double nu1, const double nu2);

double gsl_ran_flat (const gsl_rng * r, const double a, const double b);
double gsl_ran_flat_pdf (double x, const double a, const double b);

double gsl_ran_gamma (const gsl_rng * r, const double a, const double b);
double gsl_ran_gamma_int (const gsl_rng * r, const unsigned int a);
double gsl_ran_gamma_pdf (const double x, const double a, const double b);
double gsl_ran_gamma_mt (const gsl_rng * r, const double a, const double b);
double gsl_ran_gamma_knuth (const gsl_rng * r, const double a, const double b);

double gsl_ran_gaussian (const gsl_rng * r, const double sigma);
double gsl_ran_gaussian_ratio_method (const gsl_rng * r, const double sigma);
double gsl_ran_gaussian_ziggurat (const gsl_rng * r, const double sigma);
double gsl_ran_gaussian_pdf (const double x, const double sigma);

double gsl_ran_ugaussian (const gsl_rng * r);
double gsl_ran_ugaussian_ratio_method (const gsl_rng * r);
double gsl_ran_ugaussian_pdf (const double x);

double gsl_ran_gaussian_tail (const gsl_rng * r, const double a, const double sigma);
double gsl_ran_gaussian_tail_pdf (const double x, const double a, const double sigma);

double gsl_ran_ugaussian_tail (const gsl_rng * r, const double a);
double gsl_ran_ugaussian_tail_pdf (const double x, const double a);

void gsl_ran_bivariate_gaussian (const gsl_rng * r, double sigma_x, double sigma_y, double rho, double *x, double *y);
double gsl_ran_bivariate_gaussian_pdf (const double x, const double y, const double sigma_x, const double sigma_y, const double rho);

double gsl_ran_landau (const gsl_rng * r);
double gsl_ran_landau_pdf (const double x);

unsigned int gsl_ran_geometric (const gsl_rng * r, const double p);
double gsl_ran_geometric_pdf (const unsigned int k, const double p);

unsigned int gsl_ran_hypergeometric (const gsl_rng * r, unsigned int n1, unsigned int n2, unsigned int t);
double gsl_ran_hypergeometric_pdf (const unsigned int k, const unsigned int n1, const unsigned int n2, unsigned int t);

double gsl_ran_gumbel1 (const gsl_rng * r, const double a, const double b);
double gsl_ran_gumbel1_pdf (const double x, const double a, const double b);

double gsl_ran_gumbel2 (const gsl_rng * r, const double a, const double b);
double gsl_ran_gumbel2_pdf (const double x, const double a, const double b);

double gsl_ran_logistic (const gsl_rng * r, const double a);
double gsl_ran_logistic_pdf (const double x, const double a);

double gsl_ran_lognormal (const gsl_rng * r, const double zeta, const double sigma);
double gsl_ran_lognormal_pdf (const double x, const double zeta, const double sigma);

unsigned int gsl_ran_logarithmic (const gsl_rng * r, const double p);
double gsl_ran_logarithmic_pdf (const unsigned int k, const double p);

unsigned int gsl_ran_pascal (const gsl_rng * r, double p, unsigned int n);
double gsl_ran_pascal_pdf (const unsigned int k, const double p, unsigned int n);

double gsl_ran_pareto (const gsl_rng * r, double a, const double b);
double gsl_ran_pareto_pdf (const double x, const double a, const double b);

unsigned int gsl_ran_poisson (const gsl_rng * r, double mu);
double gsl_ran_poisson_pdf (const unsigned int k, const double mu);

double gsl_ran_rayleigh (const gsl_rng * r, const double sigma);
double gsl_ran_rayleigh_pdf (const double x, const double sigma);

double gsl_ran_rayleigh_tail (const gsl_rng * r, const double a, const double sigma);
double gsl_ran_rayleigh_tail_pdf (const double x, const double a, const double sigma);

double gsl_ran_tdist (const gsl_rng * r, const double nu);
double gsl_ran_tdist_pdf (const double x, const double nu);

double gsl_ran_laplace (const gsl_rng * r, const double a);
double gsl_ran_laplace_pdf (const double x, const double a);

double gsl_ran_levy (const gsl_rng * r, const double c, const double alpha);
double gsl_ran_levy_skew (const gsl_rng * r, const double c, const double alpha, const double beta);

double gsl_ran_weibull (const gsl_rng * r, const double a, const double b);
double gsl_ran_weibull_pdf (const double x, const double a, const double b);

double gsl_cdf_ugaussian_P (const double x);
double gsl_cdf_ugaussian_Q (const double x);

double gsl_cdf_ugaussian_Pinv (const double P);
double gsl_cdf_ugaussian_Qinv (const double Q);

double gsl_cdf_gaussian_P (const double x, const double sigma);
double gsl_cdf_gaussian_Q (const double x, const double sigma);

double gsl_cdf_gaussian_Pinv (const double P, const double sigma);
double gsl_cdf_gaussian_Qinv (const double Q, const double sigma);

double gsl_cdf_gamma_P (const double x, const double a, const double b);
double gsl_cdf_gamma_Q (const double x, const double a, const double b);

double gsl_cdf_gamma_Pinv (const double P, const double a, const double b);
double gsl_cdf_gamma_Qinv (const double Q, const double a, const double b);

double gsl_cdf_cauchy_P (const double x, const double a);
double gsl_cdf_cauchy_Q (const double x, const double a);

double gsl_cdf_cauchy_Pinv (const double P, const double a);
double gsl_cdf_cauchy_Qinv (const double Q, const double a);

double gsl_cdf_laplace_P (const double x, const double a);
double gsl_cdf_laplace_Q (const double x, const double a);

double gsl_cdf_laplace_Pinv (const double P, const double a);
double gsl_cdf_laplace_Qinv (const double Q, const double a);

double gsl_cdf_rayleigh_P (const double x, const double sigma);
double gsl_cdf_rayleigh_Q (const double x, const double sigma);

double gsl_cdf_rayleigh_Pinv (const double P, const double sigma);
double gsl_cdf_rayleigh_Qinv (const double Q, const double sigma);

double gsl_cdf_chisq_P (const double x, const double nu);
double gsl_cdf_chisq_Q (const double x, const double nu);

double gsl_cdf_chisq_Pinv (const double P, const double nu);
double gsl_cdf_chisq_Qinv (const double Q, const double nu);

double gsl_cdf_exponential_P (const double x, const double mu);
double gsl_cdf_exponential_Q (const double x, const double mu);

double gsl_cdf_exponential_Pinv (const double P, const double mu);
double gsl_cdf_exponential_Qinv (const double Q, const double mu);

double gsl_cdf_exppow_P (const double x, const double a, const double b);
double gsl_cdf_exppow_Q (const double x, const double a, const double b);

double gsl_cdf_tdist_P (const double x, const double nu);
double gsl_cdf_tdist_Q (const double x, const double nu);

double gsl_cdf_tdist_Pinv (const double P, const double nu);
double gsl_cdf_tdist_Qinv (const double Q, const double nu);

double gsl_cdf_fdist_P (const double x, const double nu1, const double nu2);
double gsl_cdf_fdist_Q (const double x, const double nu1, const double nu2);

double gsl_cdf_fdist_Pinv (const double P, const double nu1, const double nu2);
double gsl_cdf_fdist_Qinv (const double Q, const double nu1, const double nu2);

double gsl_cdf_beta_P (const double x, const double a, const double b);
double gsl_cdf_beta_Q (const double x, const double a, const double b);

double gsl_cdf_beta_Pinv (const double P, const double a, const double b);
double gsl_cdf_beta_Qinv (const double Q, const double a, const double b);

double gsl_cdf_flat_P (const double x, const double a, const double b);
double gsl_cdf_flat_Q (const double x, const double a, const double b);

double gsl_cdf_flat_Pinv (const double P, const double a, const double b);
double gsl_cdf_flat_Qinv (const double Q, const double a, const double b);

double gsl_cdf_lognormal_P (const double x, const double zeta, const double sigma);
double gsl_cdf_lognormal_Q (const double x, const double zeta, const double sigma);

double gsl_cdf_lognormal_Pinv (const double P, const double zeta, const double sigma);
double gsl_cdf_lognormal_Qinv (const double Q, const double zeta, const double sigma);

double gsl_cdf_gumbel1_P (const double x, const double a, const double b);
double gsl_cdf_gumbel1_Q (const double x, const double a, const double b);

double gsl_cdf_gumbel1_Pinv (const double P, const double a, const double b);
double gsl_cdf_gumbel1_Qinv (const double Q, const double a, const double b);

double gsl_cdf_gumbel2_P (const double x, const double a, const double b);
double gsl_cdf_gumbel2_Q (const double x, const double a, const double b);

double gsl_cdf_gumbel2_Pinv (const double P, const double a, const double b);
double gsl_cdf_gumbel2_Qinv (const double Q, const double a, const double b);

double gsl_cdf_weibull_P (const double x, const double a, const double b);
double gsl_cdf_weibull_Q (const double x, const double a, const double b);

double gsl_cdf_weibull_Pinv (const double P, const double a, const double b);
double gsl_cdf_weibull_Qinv (const double Q, const double a, const double b);

double gsl_cdf_pareto_P (const double x, const double a, const double b);
double gsl_cdf_pareto_Q (const double x, const double a, const double b);

double gsl_cdf_pareto_Pinv (const double P, const double a, const double b);
double gsl_cdf_pareto_Qinv (const double Q, const double a, const double b);

double gsl_cdf_logistic_P (const double x, const double a);
double gsl_cdf_logistic_Q (const double x, const double a);

double gsl_cdf_logistic_Pinv (const double P, const double a);
double gsl_cdf_logistic_Qinv (const double Q, const double a);

double gsl_cdf_binomial_P (const unsigned int k, const double p, const unsigned int n);
double gsl_cdf_binomial_Q (const unsigned int k, const double p, const unsigned int n);

double gsl_cdf_poisson_P (const unsigned int k, const double mu);
double gsl_cdf_poisson_Q (const unsigned int k, const double mu);

double gsl_cdf_geometric_P (const unsigned int k, const double p);
double gsl_cdf_geometric_Q (const unsigned int k, const double p);

double gsl_cdf_negative_binomial_P (const unsigned int k, const double p, const double n);
double gsl_cdf_negative_binomial_Q (const unsigned int k, const double p, const double n);

double gsl_cdf_pascal_P (const unsigned int k, const double p, const unsigned int n);
double gsl_cdf_pascal_Q (const unsigned int k, const double p, const unsigned int n);

double gsl_cdf_hypergeometric_P (const unsigned int k, const unsigned int n1,
                                 const unsigned int n2, const unsigned int t);
double gsl_cdf_hypergeometric_Q (const unsigned int k, const unsigned int n1,
                                 const unsigned int n2, const unsigned int t);

struct gsl_sf_result_struct {
  double val;
  double err;
};
typedef struct gsl_sf_result_struct gsl_sf_result;

struct gsl_sf_result_e10_struct {
  double val;
  double err;
  int    e10;
};
typedef struct gsl_sf_result_e10_struct gsl_sf_result_e10;

typedef int gsl_mode_t;

int gsl_sf_airy_Ai_e(const double x, const gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_airy_Ai(const double x, gsl_mode_t mode);
int gsl_sf_airy_Bi_e(const double x, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_airy_Bi(const double x, gsl_mode_t mode);
int gsl_sf_airy_Ai_scaled_e(const double x, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_airy_Ai_scaled(const double x, gsl_mode_t mode);
int gsl_sf_airy_Bi_scaled_e(const double x, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_airy_Bi_scaled(const double x, gsl_mode_t mode);
int gsl_sf_airy_Ai_deriv_e(const double x, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_airy_Ai_deriv(const double x, gsl_mode_t mode);
int gsl_sf_airy_Bi_deriv_e(const double x, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_airy_Bi_deriv(const double x, gsl_mode_t mode);
int gsl_sf_airy_Ai_deriv_scaled_e(const double x, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_airy_Ai_deriv_scaled(const double x, gsl_mode_t mode);
int gsl_sf_airy_Bi_deriv_scaled_e(const double x, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_airy_Bi_deriv_scaled(const double x, gsl_mode_t mode);
int gsl_sf_airy_zero_Ai_e(unsigned int s, gsl_sf_result * result);
double gsl_sf_airy_zero_Ai(unsigned int s);
int gsl_sf_airy_zero_Bi_e(unsigned int s, gsl_sf_result * result);
double gsl_sf_airy_zero_Bi(unsigned int s);
int gsl_sf_airy_zero_Ai_deriv_e(unsigned int s, gsl_sf_result * result);
double gsl_sf_airy_zero_Ai_deriv(unsigned int s);
int gsl_sf_airy_zero_Bi_deriv_e(unsigned int s, gsl_sf_result * result);
double gsl_sf_airy_zero_Bi_deriv(unsigned int s);

int gsl_sf_bessel_J0_e(const double x,  gsl_sf_result * result);
double gsl_sf_bessel_J0(const double x);
int gsl_sf_bessel_J1_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_J1(const double x);
int gsl_sf_bessel_Jn_e(int n, double x, gsl_sf_result * result);
double gsl_sf_bessel_Jn(const int n, const double x);
int gsl_sf_bessel_Jn_array(int nmin, int nmax, double x, double * result_array);
int gsl_sf_bessel_Y0_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_Y0(const double x);
int gsl_sf_bessel_Y1_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_Y1(const double x);
int gsl_sf_bessel_Yn_e(int n,const double x, gsl_sf_result * result);
double gsl_sf_bessel_Yn(const int n,const double x);
int gsl_sf_bessel_Yn_array(const int nmin, const int nmax, const double x, double * result_array);
int gsl_sf_bessel_I0_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_I0(const double x);
int gsl_sf_bessel_I1_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_I1(const double x);
int gsl_sf_bessel_In_e(const int n, const double x, gsl_sf_result * result);
double gsl_sf_bessel_In(const int n, const double x);
int gsl_sf_bessel_In_array(const int nmin, const int nmax, const double x, double * result_array);
int gsl_sf_bessel_I0_scaled_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_I0_scaled(const double x);
int gsl_sf_bessel_I1_scaled_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_I1_scaled(const double x);
int gsl_sf_bessel_In_scaled_e(int n, const double x, gsl_sf_result * result);
double gsl_sf_bessel_In_scaled(const int n, const double x);
int gsl_sf_bessel_In_scaled_array(const int nmin, const int nmax, const double x, double * result_array);
int gsl_sf_bessel_K0_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_K0(const double x);
int gsl_sf_bessel_K1_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_K1(const double x);
int gsl_sf_bessel_Kn_e(const int n, const double x, gsl_sf_result * result);
double gsl_sf_bessel_Kn(const int n, const double x);
int gsl_sf_bessel_Kn_array(const int nmin, const int nmax, const double x, double * result_array);
int gsl_sf_bessel_K0_scaled_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_K0_scaled(const double x);
int gsl_sf_bessel_K1_scaled_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_K1_scaled(const double x);
int gsl_sf_bessel_Kn_scaled_e(int n, const double x, gsl_sf_result * result);
double gsl_sf_bessel_Kn_scaled(const int n, const double x);
int gsl_sf_bessel_Kn_scaled_array(const int nmin, const int nmax, const double x, double * result_array);
int gsl_sf_bessel_j0_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_j0(const double x);
int gsl_sf_bessel_j1_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_j1(const double x);
int gsl_sf_bessel_j2_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_j2(const double x);
int gsl_sf_bessel_jl_e(const int l, const double x, gsl_sf_result * result);
double gsl_sf_bessel_jl(const int l, const double x);
int gsl_sf_bessel_jl_array(const int lmax, const double x, double * result_array);
int gsl_sf_bessel_jl_steed_array(const int lmax, const double x, double * jl_x_array);
int gsl_sf_bessel_y0_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_y0(const double x);
int gsl_sf_bessel_y1_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_y1(const double x);
int gsl_sf_bessel_y2_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_y2(const double x);
int gsl_sf_bessel_yl_e(int l, const double x, gsl_sf_result * result);
double gsl_sf_bessel_yl(const int l, const double x);
int gsl_sf_bessel_yl_array(const int lmax, const double x, double * result_array);
int gsl_sf_bessel_i0_scaled_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_i0_scaled(const double x);
int gsl_sf_bessel_i1_scaled_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_i1_scaled(const double x);
int gsl_sf_bessel_i2_scaled_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_i2_scaled(const double x);
int gsl_sf_bessel_il_scaled_e(const int l, double x, gsl_sf_result * result);
double gsl_sf_bessel_il_scaled(const int l, const double x);
int gsl_sf_bessel_il_scaled_array(const int lmax, const double x, double * result_array);
int gsl_sf_bessel_k0_scaled_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_k0_scaled(const double x);
int gsl_sf_bessel_k1_scaled_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_k1_scaled(const double x);
int gsl_sf_bessel_k2_scaled_e(const double x, gsl_sf_result * result);
double gsl_sf_bessel_k2_scaled(const double x);
int gsl_sf_bessel_kl_scaled_e(int l, const double x, gsl_sf_result * result);
double gsl_sf_bessel_kl_scaled(const int l, const double x);
int gsl_sf_bessel_kl_scaled_array(const int lmax, const double x, double * result_array);
int gsl_sf_bessel_Jnu_e(const double nu, const double x, gsl_sf_result * result);
double gsl_sf_bessel_Jnu(const double nu, const double x);
int gsl_sf_bessel_Ynu_e(double nu, double x, gsl_sf_result * result);
double gsl_sf_bessel_Ynu(const double nu, const double x);
int gsl_sf_bessel_sequence_Jnu_e(double nu, gsl_mode_t mode, size_t size, double * v);
int gsl_sf_bessel_Inu_scaled_e(double nu, double x, gsl_sf_result * result);
double gsl_sf_bessel_Inu_scaled(double nu, double x);
int gsl_sf_bessel_Inu_e(double nu, double x, gsl_sf_result * result);
double gsl_sf_bessel_Inu(double nu, double x);
int gsl_sf_bessel_Knu_scaled_e(const double nu, const double x, gsl_sf_result * result);
double gsl_sf_bessel_Knu_scaled(const double nu, const double x);
int gsl_sf_bessel_Knu_scaled_e10_e(const double nu, const double x, gsl_sf_result_e10 * result);
int gsl_sf_bessel_Knu_e(const double nu, const double x, gsl_sf_result * result);
double gsl_sf_bessel_Knu(const double nu, const double x);
int gsl_sf_bessel_lnKnu_e(const double nu, const double x, gsl_sf_result * result);
double gsl_sf_bessel_lnKnu(const double nu, const double x);
int gsl_sf_bessel_zero_J0_e(unsigned int s, gsl_sf_result * result);
double gsl_sf_bessel_zero_J0(unsigned int s);
int gsl_sf_bessel_zero_J1_e(unsigned int s, gsl_sf_result * result);
double gsl_sf_bessel_zero_J1(unsigned int s);
int gsl_sf_bessel_zero_Jnu_e(double nu, unsigned int s, gsl_sf_result * result);
double gsl_sf_bessel_zero_Jnu(double nu, unsigned int s);

int gsl_sf_clausen_e(double x, gsl_sf_result * result);
double gsl_sf_clausen(const double x);

int gsl_sf_hydrogenicR_1_e(const double Z, const double r, gsl_sf_result * result);
double gsl_sf_hydrogenicR_1(const double Z, const double r);
int gsl_sf_hydrogenicR_e(const int n, const int l, const double Z, const double r, gsl_sf_result * result);
double gsl_sf_hydrogenicR(const int n, const int l, const double Z, const double r);
int gsl_sf_coulomb_wave_FG_e(const double eta, const double x,
                    const double lam_F,
                    const int  k_lam_G,
                    gsl_sf_result * F, gsl_sf_result * Fp,
                    gsl_sf_result * G, gsl_sf_result * Gp,
                    double * exp_F, double * exp_G);
int gsl_sf_coulomb_wave_F_array(
  double lam_min, int kmax,
  double eta, double x,
  double * fc_array,
  double * F_exponent
  );
int gsl_sf_coulomb_wave_FG_array(double lam_min, int kmax,
	                        double eta, double x,
	                        double * fc_array, double * gc_array,
	                        double * F_exponent,
	                        double * G_exponent
	                        );
int gsl_sf_coulomb_wave_FGp_array(double lam_min, int kmax,
	                        double eta, double x,
	                        double * fc_array, double * fcp_array,
	                        double * gc_array, double * gcp_array,
	                        double * F_exponent,
	                        double * G_exponent
	                        );
int gsl_sf_coulomb_wave_sphF_array(double lam_min, int kmax,
	                                double eta, double x,
	                                double * fc_array,
	                                double * F_exponent
	                                );
int gsl_sf_coulomb_CL_e(double L, double eta, gsl_sf_result * result);
int gsl_sf_coulomb_CL_array(double Lmin, int kmax, double eta, double * cl);

int gsl_sf_coupling_3j_e(int two_ja, int two_jb, int two_jc,
	                    int two_ma, int two_mb, int two_mc,
	                    gsl_sf_result * result
	                    );
double gsl_sf_coupling_3j(int two_ja, int two_jb, int two_jc,
	                  int two_ma, int two_mb, int two_mc
	                  );
int gsl_sf_coupling_6j_e(int two_ja, int two_jb, int two_jc,
	                 int two_jd, int two_je, int two_jf,
	                 gsl_sf_result * result
	                 );
double gsl_sf_coupling_6j(int two_ja, int two_jb, int two_jc,
	                  int two_jd, int two_je, int two_jf
	                  );
int gsl_sf_coupling_RacahW_e(int two_ja, int two_jb, int two_jc,
	                     int two_jd, int two_je, int two_jf,
	                     gsl_sf_result * result
	                     );
double gsl_sf_coupling_RacahW(int two_ja, int two_jb, int two_jc,
	                      int two_jd, int two_je, int two_jf
	                      );
int gsl_sf_coupling_9j_e(int two_ja, int two_jb, int two_jc,
	                 int two_jd, int two_je, int two_jf,
	                 int two_jg, int two_jh, int two_ji,
	                 gsl_sf_result * result
	                 );
double gsl_sf_coupling_9j(int two_ja, int two_jb, int two_jc,
	                  int two_jd, int two_je, int two_jf,
	                  int two_jg, int two_jh, int two_ji
	                  );
int gsl_sf_coupling_6j_INCORRECT_e(int two_ja, int two_jb, int two_jc,
	                           int two_jd, int two_je, int two_jf,
	                           gsl_sf_result * result
	                           );
double gsl_sf_coupling_6j_INCORRECT(int two_ja, int two_jb, int two_jc,
	                            int two_jd, int two_je, int two_jf
	                            );

int     gsl_sf_dawson_e(double x, gsl_sf_result * result);
double     gsl_sf_dawson(double x);

int     gsl_sf_debye_1_e(const double x, gsl_sf_result * result);
double     gsl_sf_debye_1(const double x);
int     gsl_sf_debye_2_e(const double x, gsl_sf_result * result);
double     gsl_sf_debye_2(const double x);
int     gsl_sf_debye_3_e(const double x, gsl_sf_result * result);
double     gsl_sf_debye_3(const double x);
int     gsl_sf_debye_4_e(const double x, gsl_sf_result * result);
double     gsl_sf_debye_4(const double x);
int     gsl_sf_debye_5_e(const double x, gsl_sf_result * result);
double     gsl_sf_debye_5(const double x);
int     gsl_sf_debye_6_e(const double x, gsl_sf_result * result);
double     gsl_sf_debye_6(const double x);

int     gsl_sf_dilog_e(const double x, gsl_sf_result * result);
double  gsl_sf_dilog(const double x);
int gsl_sf_complex_dilog_xy_e(
  const double x,
  const double y,
  gsl_sf_result * result_re,
  gsl_sf_result * result_im
  );

int	gsl_sf_complex_dilog_e(
  const double r,
  const double theta,
  gsl_sf_result * result_re,
  gsl_sf_result * result_im
  );

int	gsl_sf_complex_spence_xy_e(
  const double x,
  const double y,
  gsl_sf_result * real_sp,
  gsl_sf_result * imag_sp
  );

int gsl_sf_ellint_Kcomp_e(double k, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_ellint_Kcomp(double k, gsl_mode_t mode);
int gsl_sf_ellint_Ecomp_e(double k, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_ellint_Ecomp(double k, gsl_mode_t mode);
int gsl_sf_ellint_Pcomp_e(double k, double n, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_ellint_Pcomp(double k, double n, gsl_mode_t mode);
int gsl_sf_ellint_Dcomp_e(double k, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_ellint_Dcomp(double k, gsl_mode_t mode);
int gsl_sf_ellint_F_e(double phi, double k, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_ellint_F(double phi, double k, gsl_mode_t mode);
int gsl_sf_ellint_E_e(double phi, double k, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_ellint_E(double phi, double k, gsl_mode_t mode);
int gsl_sf_ellint_P_e(double phi, double k, double n, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_ellint_P(double phi, double k, double n, gsl_mode_t mode);
int gsl_sf_ellint_D_e(double phi, double k, double n, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_ellint_D(double phi, double k, double n, gsl_mode_t mode);
int gsl_sf_ellint_RC_e(double x, double y, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_ellint_RC(double x, double y, gsl_mode_t mode);
int gsl_sf_ellint_RD_e(double x, double y, double z, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_ellint_RD(double x, double y, double z, gsl_mode_t mode);
int gsl_sf_ellint_RF_e(double x, double y, double z, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_ellint_RF(double x, double y, double z, gsl_mode_t mode);
int gsl_sf_ellint_RJ_e(double x, double y, double z, double p, gsl_mode_t mode, gsl_sf_result * result);
double gsl_sf_ellint_RJ(double x, double y, double z, double p, gsl_mode_t mode);

int gsl_sf_elljac_e(double u, double m, double * sn, double * cn, double * dn);

int gsl_sf_erfc_e(double x, gsl_sf_result * result);
double gsl_sf_erfc(double x);
int gsl_sf_log_erfc_e(double x, gsl_sf_result * result);
double gsl_sf_log_erfc(double x);
int gsl_sf_erf_e(double x, gsl_sf_result * result);
double gsl_sf_erf(double x);
int gsl_sf_erf_Z_e(double x, gsl_sf_result * result);
int gsl_sf_erf_Q_e(double x, gsl_sf_result * result);
double gsl_sf_erf_Z(double x);
double gsl_sf_erf_Q(double x);
int gsl_sf_hazard_e(double x, gsl_sf_result * result);
double gsl_sf_hazard(double x);

int gsl_sf_exp_e(const double x, gsl_sf_result * result);
double gsl_sf_exp(const double x);
int gsl_sf_exp_e10_e(const double x, gsl_sf_result_e10 * result);
int gsl_sf_exp_mult_e(const double x, const double y, gsl_sf_result * result);
double gsl_sf_exp_mult(const double x, const double y);
int gsl_sf_exp_mult_e10_e(const double x, const double y, gsl_sf_result_e10 * result);
int gsl_sf_expm1_e(const double x, gsl_sf_result * result);
double gsl_sf_expm1(const double x);
int gsl_sf_exprel_e(const double x, gsl_sf_result * result);
double gsl_sf_exprel(const double x);
int gsl_sf_exprel_2_e(double x, gsl_sf_result * result);
double gsl_sf_exprel_2(const double x);
int gsl_sf_exprel_n_e(const int n, const double x, gsl_sf_result * result);
double gsl_sf_exprel_n(const int n, const double x);
int gsl_sf_exprel_n_CF_e(const double n, const double x, gsl_sf_result * result);
int gsl_sf_exp_err_e(const double x, const double dx, gsl_sf_result * result);
int gsl_sf_exp_err_e10_e(const double x, const double dx, gsl_sf_result_e10 * result);
int gsl_sf_exp_mult_err_e(const double x, const double dx, const double y, const double dy, gsl_sf_result * result);
int gsl_sf_exp_mult_err_e10_e(const double x, const double dx, const double y, const double dy, gsl_sf_result_e10 * result);

int     gsl_sf_expint_E1_e(const double x, gsl_sf_result * result);
double  gsl_sf_expint_E1(const double x);
int     gsl_sf_expint_E2_e(const double x, gsl_sf_result * result);
double  gsl_sf_expint_E2(const double x);
int     gsl_sf_expint_En_e(const int n, const double x, gsl_sf_result * result);
double  gsl_sf_expint_En(const int n, const double x);
int     gsl_sf_expint_E1_scaled_e(const double x, gsl_sf_result * result);
double  gsl_sf_expint_E1_scaled(const double x);
int     gsl_sf_expint_E2_scaled_e(const double x, gsl_sf_result * result);
double  gsl_sf_expint_E2_scaled(const double x);
int     gsl_sf_expint_En_scaled_e(const int n, const double x, gsl_sf_result * result);
double  gsl_sf_expint_En_scaled(const int n, const double x);
int     gsl_sf_expint_Ei_e(const double x, gsl_sf_result * result);
double  gsl_sf_expint_Ei(const double x);
int     gsl_sf_expint_Ei_scaled_e(const double x, gsl_sf_result * result);
double  gsl_sf_expint_Ei_scaled(const double x);
int     gsl_sf_Shi_e(const double x, gsl_sf_result * result);
double  gsl_sf_Shi(const double x);
int     gsl_sf_Chi_e(const double x, gsl_sf_result * result);
double  gsl_sf_Chi(const double x);
int     gsl_sf_expint_3_e(const double x, gsl_sf_result * result);
double  gsl_sf_expint_3(double x);
int     gsl_sf_Si_e(const double x, gsl_sf_result * result);
double  gsl_sf_Si(const double x);
int     gsl_sf_Ci_e(const double x, gsl_sf_result * result);
double  gsl_sf_Ci(const double x);
int     gsl_sf_atanint_e(const double x, gsl_sf_result * result);
double  gsl_sf_atanint(const double x);

int     gsl_sf_fermi_dirac_m1_e(const double x, gsl_sf_result * result);
double     gsl_sf_fermi_dirac_m1(const double x);
int     gsl_sf_fermi_dirac_0_e(const double x, gsl_sf_result * result);
double     gsl_sf_fermi_dirac_0(const double x);
int     gsl_sf_fermi_dirac_1_e(const double x, gsl_sf_result * result);
double     gsl_sf_fermi_dirac_1(const double x);
int     gsl_sf_fermi_dirac_2_e(const double x, gsl_sf_result * result);
double     gsl_sf_fermi_dirac_2(const double x);
int     gsl_sf_fermi_dirac_int_e(const int j, const double x, gsl_sf_result * result);
double     gsl_sf_fermi_dirac_int(const int j, const double x);
int     gsl_sf_fermi_dirac_mhalf_e(const double x, gsl_sf_result * result);
double     gsl_sf_fermi_dirac_mhalf(const double x);
int     gsl_sf_fermi_dirac_half_e(const double x, gsl_sf_result * result);
double     gsl_sf_fermi_dirac_half(const double x);
int     gsl_sf_fermi_dirac_3half_e(const double x, gsl_sf_result * result);
double     gsl_sf_fermi_dirac_3half(const double x);
int     gsl_sf_fermi_dirac_inc_0_e(const double x, const double b, gsl_sf_result * result);
double     gsl_sf_fermi_dirac_inc_0(const double x, const double b);

int gsl_sf_lngamma_e(double x, gsl_sf_result * result);
double gsl_sf_lngamma(const double x);
int gsl_sf_lngamma_sgn_e(double x, gsl_sf_result * result_lg, double *sgn);
int gsl_sf_gamma_e(const double x, gsl_sf_result * result);
double gsl_sf_gamma(const double x);
int gsl_sf_gammastar_e(const double x, gsl_sf_result * result);
double gsl_sf_gammastar(const double x);
int gsl_sf_gammainv_e(const double x, gsl_sf_result * result);
double gsl_sf_gammainv(const double x);
int gsl_sf_lngamma_complex_e(double zr, double zi, gsl_sf_result * lnr, gsl_sf_result * arg);
int gsl_sf_taylorcoeff_e(const int n, const double x, gsl_sf_result * result);
double gsl_sf_taylorcoeff(const int n, const double x);
int gsl_sf_fact_e(const unsigned int n, gsl_sf_result * result);
double gsl_sf_fact(const unsigned int n);
int gsl_sf_doublefact_e(const unsigned int n, gsl_sf_result * result);
double gsl_sf_doublefact(const unsigned int n);
int gsl_sf_lnfact_e(const unsigned int n, gsl_sf_result * result);
double gsl_sf_lnfact(const unsigned int n);
int gsl_sf_lndoublefact_e(const unsigned int n, gsl_sf_result * result);
double gsl_sf_lndoublefact(const unsigned int n);
int gsl_sf_lnchoose_e(unsigned int n, unsigned int m, gsl_sf_result * result);
double gsl_sf_lnchoose(unsigned int n, unsigned int m);
int gsl_sf_choose_e(unsigned int n, unsigned int m, gsl_sf_result * result);
double gsl_sf_choose(unsigned int n, unsigned int m);
int gsl_sf_lnpoch_e(const double a, const double x, gsl_sf_result * result);
double gsl_sf_lnpoch(const double a, const double x);
int gsl_sf_lnpoch_sgn_e(const double a, const double x, gsl_sf_result * result, double * sgn);
int gsl_sf_poch_e(const double a, const double x, gsl_sf_result * result);
double gsl_sf_poch(const double a, const double x);
int gsl_sf_pochrel_e(const double a, const double x, gsl_sf_result * result);
double gsl_sf_pochrel(const double a, const double x);
int gsl_sf_gamma_inc_Q_e(const double a, const double x, gsl_sf_result * result);
double gsl_sf_gamma_inc_Q(const double a, const double x);
int gsl_sf_gamma_inc_P_e(const double a, const double x, gsl_sf_result * result);
double gsl_sf_gamma_inc_P(const double a, const double x);
int gsl_sf_gamma_inc_e(const double a, const double x, gsl_sf_result * result);
double gsl_sf_gamma_inc(const double a, const double x);
int gsl_sf_lnbeta_e(const double a, const double b, gsl_sf_result * result);
double gsl_sf_lnbeta(const double a, const double b);
int gsl_sf_lnbeta_sgn_e(const double x, const double y, gsl_sf_result * result, double * sgn);
int gsl_sf_beta_e(const double a, const double b, gsl_sf_result * result);
double gsl_sf_beta(const double a, const double b);
int gsl_sf_beta_inc_e(const double a, const double b, const double x, gsl_sf_result * result);
double gsl_sf_beta_inc(const double a, const double b, const double x);

int gsl_sf_gegenpoly_1_e(double lambda, double x, gsl_sf_result * result);
int gsl_sf_gegenpoly_2_e(double lambda, double x, gsl_sf_result * result);
int gsl_sf_gegenpoly_3_e(double lambda, double x, gsl_sf_result * result);
double gsl_sf_gegenpoly_1(double lambda, double x);
double gsl_sf_gegenpoly_2(double lambda, double x);
double gsl_sf_gegenpoly_3(double lambda, double x);
int gsl_sf_gegenpoly_n_e(int n, double lambda, double x, gsl_sf_result * result);
double gsl_sf_gegenpoly_n(int n, double lambda, double x);
int gsl_sf_gegenpoly_array(int nmax, double lambda, double x, double * result_array);

int gsl_sf_hyperg_0F1_e(double c, double x, gsl_sf_result * result);
double gsl_sf_hyperg_0F1(const double c, const double x);
int gsl_sf_hyperg_1F1_int_e(const int m, const int n, const double x, gsl_sf_result * result);
double gsl_sf_hyperg_1F1_int(const int m, const int n, double x);
int gsl_sf_hyperg_1F1_e(const double a, const double b, const double x, gsl_sf_result * result);
double gsl_sf_hyperg_1F1(double a, double b, double x);
int gsl_sf_hyperg_U_int_e(const int m, const int n, const double x, gsl_sf_result * result);
double gsl_sf_hyperg_U_int(const int m, const int n, const double x);
int gsl_sf_hyperg_U_int_e10_e(const int m, const int n, const double x, gsl_sf_result_e10 * result);
int gsl_sf_hyperg_U_e(const double a, const double b, const double x, gsl_sf_result * result);
double gsl_sf_hyperg_U(const double a, const double b, const double x);
int gsl_sf_hyperg_U_e10_e(const double a, const double b, const double x, gsl_sf_result_e10 * result);
int gsl_sf_hyperg_2F1_e(double a, double b, const double c, const double x, gsl_sf_result * result);
double gsl_sf_hyperg_2F1(double a, double b, double c, double x);
int gsl_sf_hyperg_2F1_conj_e(const double aR, const double aI, const double c, const double x, gsl_sf_result * result);
double gsl_sf_hyperg_2F1_conj(double aR, double aI, double c, double x);
int gsl_sf_hyperg_2F1_renorm_e(const double a, const double b, const double c, const double x, gsl_sf_result * result);
double gsl_sf_hyperg_2F1_renorm(double a, double b, double c, double x);
int gsl_sf_hyperg_2F1_conj_renorm_e(const double aR, const double aI, const double c, const double x, gsl_sf_result * result);
double gsl_sf_hyperg_2F1_conj_renorm(double aR, double aI, double c, double x);
int     gsl_sf_hyperg_2F0_e(const double a, const double b, const double x, gsl_sf_result * result);
double     gsl_sf_hyperg_2F0(const double a, const double b, const double x);

int gsl_sf_laguerre_1_e(const double a, const double x, gsl_sf_result * result);
int gsl_sf_laguerre_2_e(const double a, const double x, gsl_sf_result * result);
int gsl_sf_laguerre_3_e(const double a, const double x, gsl_sf_result * result);
double gsl_sf_laguerre_1(double a, double x);
double gsl_sf_laguerre_2(double a, double x);
double gsl_sf_laguerre_3(double a, double x);
int     gsl_sf_laguerre_n_e(const int n, const double a, const double x, gsl_sf_result * result);
double     gsl_sf_laguerre_n(int n, double a, double x);

int     gsl_sf_lambert_W0_e(double x, gsl_sf_result * result);
double  gsl_sf_lambert_W0(double x);
int     gsl_sf_lambert_Wm1_e(double x, gsl_sf_result * result);
double  gsl_sf_lambert_Wm1(double x);

int     gsl_sf_legendre_Pl_e(const int l, const double x, gsl_sf_result * result);
double  gsl_sf_legendre_Pl(const int l, const double x);
int gsl_sf_legendre_Pl_array(
  const int lmax, const double x,
  double * result_array
  );
int gsl_sf_legendre_Pl_deriv_array(
  const int lmax, const double x,
  double * result_array,
  double * result_deriv_array
  );
int gsl_sf_legendre_P1_e(double x, gsl_sf_result * result);
int gsl_sf_legendre_P2_e(double x, gsl_sf_result * result);
int gsl_sf_legendre_P3_e(double x, gsl_sf_result * result);
double gsl_sf_legendre_P1(const double x);
double gsl_sf_legendre_P2(const double x);
double gsl_sf_legendre_P3(const double x);
int gsl_sf_legendre_Q0_e(const double x, gsl_sf_result * result);
double gsl_sf_legendre_Q0(const double x);
int gsl_sf_legendre_Q1_e(const double x, gsl_sf_result * result);
double gsl_sf_legendre_Q1(const double x);
int gsl_sf_legendre_Ql_e(const int l, const double x, gsl_sf_result * result);
double gsl_sf_legendre_Ql(const int l, const double x);
int     gsl_sf_legendre_Plm_e(const int l, const int m, const double x, gsl_sf_result * result);
double  gsl_sf_legendre_Plm(const int l, const int m, const double x);
int gsl_sf_legendre_Plm_array(
  const int lmax, const int m, const double x,
  double * result_array
  );
int gsl_sf_legendre_Plm_deriv_array(
  const int lmax, const int m, const double x,
  double * result_array,
  double * result_deriv_array
  );
int     gsl_sf_legendre_sphPlm_e(const int l, int m, const double x, gsl_sf_result * result);
double  gsl_sf_legendre_sphPlm(const int l, const int m, const double x);
int gsl_sf_legendre_sphPlm_array(
  const int lmax, int m, const double x,
  double * result_array
  );
int gsl_sf_legendre_sphPlm_deriv_array(
  const int lmax, const int m, const double x,
  double * result_array,
  double * result_deriv_array
  );
int gsl_sf_legendre_array_size(const int lmax, const int m);
int gsl_sf_conicalP_half_e(const double lambda, const double x, gsl_sf_result * result);
double gsl_sf_conicalP_half(const double lambda, const double x);
int gsl_sf_conicalP_mhalf_e(const double lambda, const double x, gsl_sf_result * result);
double gsl_sf_conicalP_mhalf(const double lambda, const double x);
int gsl_sf_conicalP_0_e(const double lambda, const double x, gsl_sf_result * result);
double gsl_sf_conicalP_0(const double lambda, const double x);
int gsl_sf_conicalP_1_e(const double lambda, const double x, gsl_sf_result * result);
double gsl_sf_conicalP_1(const double lambda, const double x);
int gsl_sf_conicalP_sph_reg_e(const int l, const double lambda, const double x, gsl_sf_result * result);
double gsl_sf_conicalP_sph_reg(const int l, const double lambda, const double x);
int gsl_sf_conicalP_cyl_reg_e(const int m, const double lambda, const double x, gsl_sf_result * result);
double gsl_sf_conicalP_cyl_reg(const int m, const double lambda, const double x);
int gsl_sf_legendre_H3d_0_e(const double lambda, const double eta, gsl_sf_result * result);
double gsl_sf_legendre_H3d_0(const double lambda, const double eta);
int gsl_sf_legendre_H3d_1_e(const double lambda, const double eta, gsl_sf_result * result);
double gsl_sf_legendre_H3d_1(const double lambda, const double eta);
int gsl_sf_legendre_H3d_e(const int l, const double lambda, const double eta, gsl_sf_result * result);
double gsl_sf_legendre_H3d(const int l, const double lambda, const double eta);
int gsl_sf_legendre_H3d_array(const int lmax, const double lambda, const double eta, double * result_array);

int gsl_sf_log_e(const double x, gsl_sf_result * result);
double gsl_sf_log(const double x);
int gsl_sf_log_abs_e(const double x, gsl_sf_result * result);
double gsl_sf_log_abs(const double x);
int gsl_sf_complex_log_e(const double zr, const double zi, gsl_sf_result * lnr, gsl_sf_result * theta);
int gsl_sf_log_1plusx_e(const double x, gsl_sf_result * result);
double gsl_sf_log_1plusx(const double x);
int gsl_sf_log_1plusx_mx_e(const double x, gsl_sf_result * result);
double gsl_sf_log_1plusx_mx(const double x);

int     gsl_sf_pow_int_e(double x, int n, gsl_sf_result * result);
double  gsl_sf_pow_int(const double x, const int n);

int     gsl_sf_psi_int_e(const int n, gsl_sf_result * result);
double  gsl_sf_psi_int(const int n);
int     gsl_sf_psi_e(const double x, gsl_sf_result * result);
double  gsl_sf_psi(const double x);
int     gsl_sf_psi_1piy_e(const double y, gsl_sf_result * result);
double  gsl_sf_psi_1piy(const double y);
int gsl_sf_complex_psi_e(
  const double x,
  const double y,
  gsl_sf_result * result_re,
  gsl_sf_result * result_im
  );
int     gsl_sf_psi_1_int_e(const int n, gsl_sf_result * result);
double  gsl_sf_psi_1_int(const int n);
int     gsl_sf_psi_1_e(const double x, gsl_sf_result * result);
double  gsl_sf_psi_1(const double x);
int     gsl_sf_psi_n_e(const int n, const double x, gsl_sf_result * result);
double  gsl_sf_psi_n(const int n, const double x);

int     gsl_sf_synchrotron_1_e(const double x, gsl_sf_result * result);
double     gsl_sf_synchrotron_1(const double x);
int     gsl_sf_synchrotron_2_e(const double x, gsl_sf_result * result);
double     gsl_sf_synchrotron_2(const double x);

int     gsl_sf_transport_2_e(const double x, gsl_sf_result * result);
double     gsl_sf_transport_2(const double x);
int     gsl_sf_transport_3_e(const double x, gsl_sf_result * result);
double     gsl_sf_transport_3(const double x);
int     gsl_sf_transport_4_e(const double x, gsl_sf_result * result);
double     gsl_sf_transport_4(const double x);
int     gsl_sf_transport_5_e(const double x, gsl_sf_result * result);
double     gsl_sf_transport_5(const double x);

int gsl_sf_zeta_int_e(const int n, gsl_sf_result * result);
double gsl_sf_zeta_int(const int n);
int gsl_sf_zeta_e(const double s, gsl_sf_result * result);
double gsl_sf_zeta(const double s);
int gsl_sf_zetam1_e(const double s, gsl_sf_result * result);
double gsl_sf_zetam1(const double s);
int gsl_sf_zetam1_int_e(const int s, gsl_sf_result * result);
double gsl_sf_zetam1_int(const int s);
int gsl_sf_hzeta_e(const double s, const double q, gsl_sf_result * result);
double gsl_sf_hzeta(const double s, const double q);
int gsl_sf_eta_int_e(int n, gsl_sf_result * result);
double gsl_sf_eta_int(const int n);
int gsl_sf_eta_e(const double s, gsl_sf_result * result);
double gsl_sf_eta(const double s);

/*
 * ===========================================================================
 * Prototypes for level 1 BLAS functions (complex are recast as routines)
 * ===========================================================================
 */
double cblas_ddot(const int N, const double *X, const int incX,
                  const double *Y, const int incY);

/*
 * Functions having prefixes Z and C only
 */
void   cblas_zdotu_sub(const int N, const void *X, const int incX,
                       const void *Y, const int incY, void *dotu);
void   cblas_zdotc_sub(const int N, const void *X, const int incX,
                       const void *Y, const int incY, void *dotc);


/*
 * Functions having prefixes S D SC DZ
 */
double cblas_dnrm2(const int N, const double *X, const int incX);
double cblas_dasum(const int N, const double *X, const int incX);

double cblas_dznrm2(const int N, const void *X, const int incX);
double cblas_dzasum(const int N, const void *X, const int incX);


/*
 * Functions having standard 4 prefixes (S D C Z)
 */
size_t cblas_isamax(const int N, const float  *X, const int incX);
size_t cblas_idamax(const int N, const double *X, const int incX);
size_t cblas_icamax(const int N, const void   *X, const int incX);
size_t cblas_izamax(const int N, const void   *X, const int incX);

/*
 * ===========================================================================
 * Prototypes for level 1 BLAS routines
 * ===========================================================================
 */

/*
 * Routines with standard 4 prefixes (s, d, c, z)
 */

void cblas_dswap(const int N, double *X, const int incX,
                 double *Y, const int incY);
void cblas_dcopy(const int N, const double *X, const int incX,
                 double *Y, const int incY);
void cblas_daxpy(const int N, const double alpha, const double *X,
                 const int incX, double *Y, const int incY);

void cblas_zswap(const int N, void *X, const int incX,
                 void *Y, const int incY);
void cblas_zcopy(const int N, const void *X, const int incX,
                 void *Y, const int incY);
void cblas_zaxpy(const int N, const void *alpha, const void *X,
                 const int incX, void *Y, const int incY);


/*
 * Routines with S and D prefix only
 */

void cblas_drotg(double *a, double *b, double *c, double *s);
void cblas_drotmg(double *d1, double *d2, double *b1, const double b2, double *P);
void cblas_drot(const int N, double *X, const int incX,
                double *Y, const int incY, const double c, const double  s);
void cblas_drotm(const int N, double *X, const int incX,
                double *Y, const int incY, const double *P);


/*
 * Routines with S D C Z CS and ZD prefixes
 */
void cblas_dscal(const int N, const double alpha, double *X, const int incX);
void cblas_zscal(const int N, const void *alpha, void *X, const int incX);
void cblas_zdscal(const int N, const double alpha, void *X, const int incX);
]]


if jit.os == 'Linux' then
	return ffi.C
elseif jit.os == 'OSX' then
	return ffi.load('libgsl.0.dylib')
else
	return ffi.load('libgsl-0')
end
