
extern "C" {
  extern int gsl_shell_main (int argc, char **argv);
}

int agg_main (int argc, char **argv) {
  return gsl_shell_main (argc, argv);
};
