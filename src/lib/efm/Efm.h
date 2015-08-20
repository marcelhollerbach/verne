#ifndef EFM_FM_H
#define EFM_FM_H

#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Efl.h>
#include <Eina.h>
#include <Eo.h>

#include "efm.eot.h"
#include "efm_file.eo.h"
#include "efm_filter.eo.h"
#include "efm_monitor.eo.h"

#ifdef EAPI
# undef EAPI
#endif

#define EAPI

/*
 * Inits the fm_monitor
 */
EAPI int efm_init();
/*
 * shutdown the fm_monitor
 */
EAPI void efm_shutdown();
/*
 * Efm archive stuff
 */

EAPI Eina_Bool efm_archive_file_supported(const char *fileending);

EAPI Eina_Bool efm_archive_file_extract(const char *file, const char *goal);

EAPI const char *efm_archive_lasterror_get(void);

#undef EAPI

#endif