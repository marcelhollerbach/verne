#ifndef EMOUS_H
#define EMOUS_H

#ifdef EAPI
# undef EAPI
#endif

#define EAPI

#include <Eina.h>
#include <Eo.h>

#include "emous_device.eo.h"
#include "emous.eo.h"
#include "emous_type.eo.h"
#include "emous_manager.eo.h"

#undef EAPI

#endif
