--Eigensystem module

local ffi = require 'ffi'
local gsl = require 'gsl'

local sqrt, abs = math.sqrt, math.abs
local format = string.format

local check = require 'check'
local is_integer, is_real = check.is_integer, check.is_real
local matrix = require 'matrix'

------------------------------------------------------------

--Eigensystem struct und function definitions
ffi.cdef[[

typedef struct {
  size_t size;
  double * d;
  double * sd;
} gsl_eigen_symm_workspace;

gsl_eigen_symm_workspace * gsl_eigen_symm_alloc (const size_t n);
void gsl_eigen_symm_free (gsl_eigen_symm_workspace * w);
int gsl_eigen_symm (gsl_matrix * A, gsl_vector * eval, gsl_eigen_symm_workspace * w);

typedef struct {
  size_t size;
  double * d;
  double * sd;
  double * gc;
  double * gs;
} gsl_eigen_symmv_workspace;

gsl_eigen_symmv_workspace * gsl_eigen_symmv_alloc (const size_t n);
void gsl_eigen_symmv_free (gsl_eigen_symmv_workspace * w);
int gsl_eigen_symmv (gsl_matrix * A, gsl_vector * eval, gsl_matrix * evec, gsl_eigen_symmv_workspace * w);

typedef struct {
  size_t size;
  double * d;
  double * sd;
  double * tau;
} gsl_eigen_herm_workspace;

gsl_eigen_herm_workspace * gsl_eigen_herm_alloc (const size_t n);
void gsl_eigen_herm_free (gsl_eigen_herm_workspace * w);
int gsl_eigen_herm (gsl_matrix_complex * A, gsl_vector * eval,
                         gsl_eigen_herm_workspace * w);

typedef struct {
  size_t size;
  double * d;
  double * sd;
  double * tau;
  double * gc;
  double * gs;
} gsl_eigen_hermv_workspace;

gsl_eigen_hermv_workspace * gsl_eigen_hermv_alloc (const size_t n);
void gsl_eigen_hermv_free (gsl_eigen_hermv_workspace * w);
int gsl_eigen_hermv (gsl_matrix_complex * A, gsl_vector * eval,
                           gsl_matrix_complex * evec,
                           gsl_eigen_hermv_workspace * w);

typedef struct {
  size_t size;           /* matrix size */
  size_t max_iterations; /* max iterations since last eigenvalue found */
  size_t n_iter;         /* number of iterations since last eigenvalue found */
  size_t n_evals;        /* number of eigenvalues found so far */

  int compute_t;         /* compute Schur form T = Z^t A Z */

  gsl_matrix *H;         /* pointer to Hessenberg matrix */
  gsl_matrix *Z;         /* pointer to Schur vector matrix */
} gsl_eigen_francis_workspace;

gsl_eigen_francis_workspace * gsl_eigen_francis_alloc (void);
void gsl_eigen_francis_free (gsl_eigen_francis_workspace * w);
void gsl_eigen_francis_T (const int compute_t,
                          gsl_eigen_francis_workspace * w);
int gsl_eigen_francis (gsl_matrix * H, gsl_vector_complex * eval,
                       gsl_eigen_francis_workspace * w);
int gsl_eigen_francis_Z (gsl_matrix * H, gsl_vector_complex * eval,
                         gsl_matrix * Z,
                         gsl_eigen_francis_workspace * w);

typedef struct {
  size_t size;                 /* size of matrices */
  gsl_vector *diag;            /* diagonal matrix elements from balancing */
  gsl_vector *tau;             /* Householder coefficients */
  gsl_matrix *Z;               /* pointer to Z matrix */
  int do_balance;              /* perform balancing transformation? */
  size_t n_evals;              /* number of eigenvalues found */

  gsl_eigen_francis_workspace *francis_workspace_p;
} gsl_eigen_nonsymm_workspace;

gsl_eigen_nonsymm_workspace * gsl_eigen_nonsymm_alloc (const size_t n);
void gsl_eigen_nonsymm_free (gsl_eigen_nonsymm_workspace * w);
void gsl_eigen_nonsymm_params (const int compute_t, const int balance,
                               gsl_eigen_nonsymm_workspace *w);
int gsl_eigen_nonsymm (gsl_matrix * A, gsl_vector_complex * eval,
                       gsl_eigen_nonsymm_workspace * w);
int gsl_eigen_nonsymm_Z (gsl_matrix * A, gsl_vector_complex * eval,
                         gsl_matrix * Z, gsl_eigen_nonsymm_workspace * w);

typedef struct {
  size_t size;                 /* size of matrices */
  gsl_vector *work;            /* scratch workspace */
  gsl_vector *work2;           /* scratch workspace */
  gsl_vector *work3;           /* scratch workspace */

  gsl_matrix *Z;               /* pointer to Schur vectors */

  gsl_eigen_nonsymm_workspace *nonsymm_workspace_p;
} gsl_eigen_nonsymmv_workspace;

gsl_eigen_nonsymmv_workspace * gsl_eigen_nonsymmv_alloc (const size_t n);
void gsl_eigen_nonsymmv_free (gsl_eigen_nonsymmv_workspace * w);
void gsl_eigen_nonsymmv_params (const int balance,
                                gsl_eigen_nonsymmv_workspace *w);
int gsl_eigen_nonsymmv (gsl_matrix * A, gsl_vector_complex * eval,
                        gsl_matrix_complex * evec,
                        gsl_eigen_nonsymmv_workspace * w);
int gsl_eigen_nonsymmv_Z (gsl_matrix * A, gsl_vector_complex * eval,
                          gsl_matrix_complex * evec, gsl_matrix * Z,
                          gsl_eigen_nonsymmv_workspace * w);

typedef struct {
  size_t size;            /* size of matrices */
  gsl_eigen_symm_workspace *symm_workspace_p;
} gsl_eigen_gensymm_workspace;

gsl_eigen_gensymm_workspace * gsl_eigen_gensymm_alloc (const size_t n);
void gsl_eigen_gensymm_free (gsl_eigen_gensymm_workspace * w);
int gsl_eigen_gensymm (gsl_matrix * A, gsl_matrix * B,
                       gsl_vector * eval, gsl_eigen_gensymm_workspace * w);
int gsl_eigen_gensymm_standardize (gsl_matrix * A, const gsl_matrix * B);

typedef struct {
  size_t size;            /* size of matrices */
  gsl_eigen_symmv_workspace *symmv_workspace_p;
} gsl_eigen_gensymmv_workspace;

gsl_eigen_gensymmv_workspace * gsl_eigen_gensymmv_alloc (const size_t n);
void gsl_eigen_gensymmv_free (gsl_eigen_gensymmv_workspace * w);
int gsl_eigen_gensymmv (gsl_matrix * A, gsl_matrix * B,
                        gsl_vector * eval, gsl_matrix * evec,
                        gsl_eigen_gensymmv_workspace * w);

typedef struct {
  size_t size;            /* size of matrices */
  gsl_eigen_herm_workspace *herm_workspace_p;
} gsl_eigen_genherm_workspace;

gsl_eigen_genherm_workspace * gsl_eigen_genherm_alloc (const size_t n);
void gsl_eigen_genherm_free (gsl_eigen_genherm_workspace * w);
int gsl_eigen_genherm (gsl_matrix_complex * A, gsl_matrix_complex * B,
                       gsl_vector * eval, gsl_eigen_genherm_workspace * w);
int gsl_eigen_genherm_standardize (gsl_matrix_complex * A,
                                   const gsl_matrix_complex * B);

typedef struct {
  size_t size;            /* size of matrices */
  gsl_eigen_hermv_workspace *hermv_workspace_p;
} gsl_eigen_genhermv_workspace;

gsl_eigen_genhermv_workspace * gsl_eigen_genhermv_alloc (const size_t n);
void gsl_eigen_genhermv_free (gsl_eigen_genhermv_workspace * w);
int gsl_eigen_genhermv (gsl_matrix_complex * A, gsl_matrix_complex * B,
                        gsl_vector * eval, gsl_matrix_complex * evec,
                        gsl_eigen_genhermv_workspace * w);

typedef struct {
  size_t size;            /* size of matrices */
  gsl_vector *work;       /* scratch workspace */

  size_t n_evals;         /* number of eigenvalues found */
  size_t max_iterations;  /* maximum QZ iterations allowed */
  size_t n_iter;          /* number of iterations since last eigenvalue found */
  double eshift;          /* exceptional shift counter */

  int needtop;            /* need to compute top index? */

  double atol;            /* tolerance for splitting A matrix */
  double btol;            /* tolerance for splitting B matrix */

  double ascale;          /* scaling factor for shifts */
  double bscale;          /* scaling factor for shifts */

  gsl_matrix *H;          /* pointer to hessenberg matrix */
  gsl_matrix *R;          /* pointer to upper triangular matrix */

  int compute_s;          /* compute generalized Schur form S */
  int compute_t;          /* compute generalized Schur form T */

  gsl_matrix *Q;          /* pointer to left Schur vectors */
  gsl_matrix *Z;          /* pointer to right Schur vectors */
} gsl_eigen_gen_workspace;

gsl_eigen_gen_workspace * gsl_eigen_gen_alloc (const size_t n);
void gsl_eigen_gen_free (gsl_eigen_gen_workspace * w);
void gsl_eigen_gen_params (const int compute_s, const int compute_t,
                           const int balance, gsl_eigen_gen_workspace * w);
int gsl_eigen_gen (gsl_matrix * A, gsl_matrix * B,
                   gsl_vector_complex * alpha, gsl_vector * beta,
                   gsl_eigen_gen_workspace * w);
int gsl_eigen_gen_QZ (gsl_matrix * A, gsl_matrix * B,
                      gsl_vector_complex * alpha, gsl_vector * beta,
                      gsl_matrix * Q, gsl_matrix * Z,
                      gsl_eigen_gen_workspace * w);

typedef struct {
  size_t size;            /* size of matrices */

  gsl_vector *work1;      /* 1-norm of columns of A */
  gsl_vector *work2;      /* 1-norm of columns of B */
  gsl_vector *work3;      /* real part of eigenvector */
  gsl_vector *work4;      /* imag part of eigenvector */
  gsl_vector *work5;      /* real part of back-transformed eigenvector */
  gsl_vector *work6;      /* imag part of back-transformed eigenvector */

  gsl_matrix *Q;          /* pointer to left Schur vectors */
  gsl_matrix *Z;          /* pointer to right Schur vectors */

  gsl_eigen_gen_workspace *gen_workspace_p;
} gsl_eigen_genv_workspace;

gsl_eigen_genv_workspace * gsl_eigen_genv_alloc (const size_t n);
void gsl_eigen_genv_free (gsl_eigen_genv_workspace * w);
int gsl_eigen_genv (gsl_matrix * A, gsl_matrix * B,
                    gsl_vector_complex * alpha, gsl_vector * beta,
                    gsl_matrix_complex * evec,
                    gsl_eigen_genv_workspace * w);
int gsl_eigen_genv_QZ (gsl_matrix * A, gsl_matrix * B,
                       gsl_vector_complex * alpha, gsl_vector * beta,
                       gsl_matrix_complex * evec,
                       gsl_matrix * Q, gsl_matrix * Z,
                       gsl_eigen_genv_workspace * w);



typedef enum {
  GSL_EIGEN_SORT_VAL_ASC,
  GSL_EIGEN_SORT_VAL_DESC,
  GSL_EIGEN_SORT_ABS_ASC,
  GSL_EIGEN_SORT_ABS_DESC
}
gsl_eigen_sort_t;

/* Sort eigensystem results based on eigenvalues.
 * Sorts in order of increasing value or increasing
 * absolute value.
 *
 * exceptions: GSL_EBADLEN
 */

int gsl_eigen_symmv_sort(gsl_vector * eval, gsl_matrix * evec,
                         gsl_eigen_sort_t sort_type);

int gsl_eigen_hermv_sort(gsl_vector * eval, gsl_matrix_complex * evec,
                         gsl_eigen_sort_t sort_type);

int gsl_eigen_nonsymmv_sort(gsl_vector_complex * eval,
                            gsl_matrix_complex * evec,
                            gsl_eigen_sort_t sort_type);

int gsl_eigen_gensymmv_sort (gsl_vector * eval, gsl_matrix * evec,
                             gsl_eigen_sort_t sort_type);

int gsl_eigen_genhermv_sort (gsl_vector * eval, gsl_matrix_complex * evec,
                             gsl_eigen_sort_t sort_type);

int gsl_eigen_genv_sort (gsl_vector_complex * alpha, gsl_vector * beta,
                         gsl_matrix_complex * evec,
                         gsl_eigen_sort_t sort_type);

/* Prototypes for the schur module */

int gsl_schur_gen_eigvals(const gsl_matrix *A, const gsl_matrix *B,
                          double *wr1, double *wr2, double *wi,
                          double *scale1, double *scale2);

int gsl_schur_solve_equation(double ca, const gsl_matrix *A, double z,
                             double d1, double d2, const gsl_vector *b,
                             gsl_vector *x, double *s, double *xnorm,
                             double smin);

int gsl_schur_solve_equation_z(double ca, const gsl_matrix *A,
                               gsl_complex *z, double d1, double d2,
                               const gsl_vector_complex *b,
                               gsl_vector_complex *x, double *s,
                               double *xnorm, double smin);


/* The following functions are obsolete: */

/* Eigensolve by Jacobi Method
 *
 * The data in the matrix input is destroyed.
 *
 * exceptions:
 */
int
gsl_eigen_jacobi(gsl_matrix * matrix,
                      gsl_vector * eval,
                      gsl_matrix * evec,
                      unsigned int max_rot,
                      unsigned int * nrot);


/* Invert by Jacobi Method
 *
 * exceptions:
 */
int
gsl_eigen_invert_jacobi(const gsl_matrix * matrix,
                             gsl_matrix * ainv,
                             unsigned int max_rot);
]]

-------------------------------------------------------------------------------
eigen = {}

--Definitions of the sorting order of the eigenvalues
eigen.SORT_VAL_DESC = gsl.GSL_EIGEN_SORT_VAL_DESC
eigen.SORT_VAL_ASC = gsl.GSL_EIGEN_SORT_VAL_ASC
eigen.SORT_ABS_ASC = gsl.GSL_EIGEN_SORT_ABS_ASC
eigen.SORT_ABS_DESC = gsl.GSL_EIGEN_SORT_ABS_DESC

--Calculates the eigenvalues/eigenvectors of the symmetric matrix m
--the order can be used to determine the sorting of the eigenvalues according to their value
function eigen.symmv(m, order)
	local size = m.size1
	local A = matrix.copy(m)
    local eval = matrix.alloc(size, 1)
	local xeval = gsl.gsl_matrix_column(eval, 0)
    local evec = gsl.gsl_matrix_alloc (size, size)
    local w = gsl.gsl_eigen_symmv_alloc (size)
	order = order or gsl.GSL_EIGEN_SORT_VAL_DESC

    gsl_check(gsl.gsl_eigen_symmv (A, xeval, evec, w))
    gsl.gsl_eigen_symmv_free (w)

    gsl.gsl_eigen_symmv_sort (xeval, evec, order)

	return eval, evec
end

--Calculates the eigenvalues/eigenvectors of the real nonsymmetric matrix m
--the order can be used to determine the sorting of the eigenvalues according to their value
--TODO: Return the Schur form T which is the upper part of the matrix
--TODO: Check, the matrix m is being modified and may be copied before
function eigen.non_symm(m, order)

    local size = m.size1
	local A = matrix.copy(m)
    local eval = matrix.alloc(size, 1)
	local xeval = gsl.gsl_matrix_column(eval, 0)
    local evec = gsl.gsl_matrix_alloc (size, size)
    local w = gsl.gsl_eigen_nonsymmv_alloc (size)
	order = order or gsl.GSL_EIGEN_SORT_VAL_DESC

    gsl_check(gsl.gsl_eigen_nonsymmv (A, xeval, evec, w))
    gsl.gsl_eigen_nonsymmv_free (w)

    gsl.gsl_eigen_nonsymmv_sort (xeval, evec,  order)

	return eval,evec,A
end
