
#define GSH_SF(SIGN, base)                  _GSH_SF_ ## SIGN(base, base)
#define GSH_SF_COMP(SIGN, base, spec)       _GSH_SF_ ## SIGN(base ## _ ## spec, base ## spec)
#define GSH_SF_ZERO_COMP(SIGN, base, spec)  _GSH_SF_ ## SIGN (base ## _zero_ ## spec, base ## spec ## zero)
#define GSH_SF_ZERO(SIGN, base)             _GSH_SF_ ## SIGN (base ## _zero, base)
#define GSH_SF_MODE(SIGN, gsl_name, name)   _GSH_SF_ ## SIGN ## _MODE(gsl_name, name)
#define GSH_SF_NAMES(SIGN, gsl_name, name)  _GSH_SF_ ## SIGN(gsl_name, name)
#define GSH_SF_CUSTOM(base)                 _GSH_SF_CUSTOM(base)

#define GSH_LUA_NAME(name) my_ ## name
#define GGSL_SF_NAME(nm) gsl_sf_ ## nm ## _e
