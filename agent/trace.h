#include "config.h"

#ifdef HAVE_SYSTEMTAP
#include "probes.h"
#else
#define AGENT_SMALLOC(arg1) 
#define AGENT_SFREE()
#endif
