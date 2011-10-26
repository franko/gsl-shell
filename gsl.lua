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

struct _gsl_rng;
typedef struct _gsl_rng gsl_rng;

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
]]

local gsl = (jit.os == 'Linux' and ffi.C or ffi.load('libgsl-0'))

return gsl
