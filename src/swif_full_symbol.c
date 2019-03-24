#ifdef WITH_LIBLC

/* A temporary adapter based on liblc */
#include "swif_full_symbol_lc.c"

#else /* WITH_LIBLC */

int swif_full_symbol_todo = 1; /* need symbols in the .o file on MacOS */

#endif /* WITH_LIBLC */
