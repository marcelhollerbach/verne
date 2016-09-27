#ifndef EMOUS_UDISK_H
#define EMOUS_UDISK_H

#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#include <Eo.h>
#include <Eina.h>
#include <Eldbus.h>

#include "Emous.h"

#define EAPI

#include "emous_device_udisks.eo.h"
#include "emous_type_udisks.eo.h"

#define UDISKS2_BUS "org.freedesktop.UDisks2"
#define UDISKS2_PATH "/org/freedesktop/UDisks2"
#define UDISKS2_INTERFACE "org.freedesktop.UDisks2"
#define UDISKS2_INTERFACE_FILESYSTEM "org.freedesktop.UDisks2.Filesystem"

#define CRIT(...)     EINA_LOG_DOM_CRIT(__log_domain, __VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR(__log_domain, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(__log_domain, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(__log_domain, __VA_ARGS__)
#define DBG(...)      EINA_LOG_DOM_DBG(__log_domain, __VA_ARGS__)

extern int __log_domain;
extern Eldbus_Connection *con;
extern Eina_Hash *devices;
extern Eina_Hash *drives;
extern Eina_List *devices_list;
extern Emous_Type *emous_type;

Emous_Device* _emous_device_new(Eldbus_Message_Iter *dict, const char **opath);


#endif