#ifndef EMOUS_H
#define EMOUS_H

#include <Eina.h>
#include <Eo.h>

#ifdef EFL_EO_API_SUPPORT
# ifndef EFL_BETA_API_SUPPORT
#   error "You have defined EO_API_SUPPORT but not BETA_API_SUPPORT this will break the world"
# else

#   include "emous_device.eo.h"
#   include "emous_device_type.eo.h"
#   include "emous_manager.eo.h"
# endif
#else
# include "emous_device.eo.legacy.h"
# include "emous_device_class.eo.legacy.h"
# include "emous_manager.eo.legacy.h"
#endif

EAPI int emous_init(void);
EAPI void emous_shutdown(void);

#endif
