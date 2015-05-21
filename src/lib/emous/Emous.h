#ifndef EMOUS_H
#define EMOUS_H

#include <Eina.h>
#include <Eo.h>

#ifdef EFL_EO_API_SUPPORT
# include "emous_device.eo.h"
# include "emous_device_type.eo.h"
# include "emous_manager.eo.h"
#else
# include "emous_device.eo.legacy.h"
# include "emous_device_class.eo.legacy.h"
# include "emous_manager.eo.legacy.h"
#endif

EAPI int emous_init(void);
EAPI void emous_shutdown(void);

#endif
