#ifndef EFM_FM_H
#define EFM_FM_H

#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

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