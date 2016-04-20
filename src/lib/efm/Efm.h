#ifndef EFM_FM_H
#define EFM_FM_H

#include <Efl.h>
#include <Eina.h>
#include <Eo.h>

#ifdef EAPI
# undef EAPI
#endif

#define EAPI

#include "efm.eot.h"
#include "efm_file.eo.h"
#include "efm_filter.eo.h"
#include "efm_monitor.eo.h"
#include "efm.eo.h"

#undef EAPI

#endif