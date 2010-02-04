#ifndef DISPERS_H
#define DISPERS_H

#include "cmpl.h"

struct disp;

struct sample {
  double lambda;
  cmpl n;
};

struct sampling_intfc {
  struct sample * (*get_sample)(struct disp *, int index);
  int (*length)(struct disp *);
};

struct disp_class {
  int instance_size;

  const char *short_id;
  const char *full_id;
                   
  /* methods to copy and free dispersions */
  void (*free)(struct disp *d);
  void (*dealloc)(struct disp *d);
  void (*copy)(struct disp *dest, struct disp *src);

  cmpl (*n_value)            (const struct disp *d, double lam);
  int  (*fp_number)          (const struct disp *d);
  cmpl (*n_deriv)            (struct disp *d, int fit_param, double lam);
  int  (*apply_param)        (struct disp *d, int fit_param,
                              double val);

  /* class methods */
  int  (*decode_param_string)(const char *param);
  int  (*encode_param)       (char * param_buffer, int param_buf_size, int fp);

  /* sampling interface */
  struct sampling_intfc *sampling_intfc;
};

struct disp {
  struct disp_class *dclass;
};

extern struct disp * disp_alloc               (struct disp_class *class);
extern void          disp_set_class           (struct disp *d, 
					       struct disp_class *class);
extern void          disp_shallow_copy        (struct disp *dest,
					       struct disp *src);
extern int           disp_fp_number           (const struct disp *d);
extern int           disp_apply_param         (struct disp *d, int fit_param,
					       double val);
extern int           disp_decode_param_string (const char *param);

#define DISP(d) (((struct disp *) (d))->dclass)

#endif
