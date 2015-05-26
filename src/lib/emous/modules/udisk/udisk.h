#ifndef EMOUS_UDISK_H
#define EMOUS_UDISK_H

#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#include <Eina.h>
#include <Emous.h>
#include <Eldbus.h>

#define CRIT(...)     EINA_LOG_DOM_CRIT(__log_domain, __VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR(__log_domain, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(__log_domain, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(__log_domain, __VA_ARGS__)
#define DBG(...)      EINA_LOG_DOM_DBG(__log_domain, __VA_ARGS__)

extern int __log_domain;

typedef struct {
  const char *displayname;
  const char *opath;
  Eina_Bool mounted;
  Eldbus_Proxy *proxy;
  Eldbus_Signal_Handler *changed;
  Eo *device;
  Eina_List *tmp_list;
} Device;

void _device_add(Device *d);
void _device_del(const char *opath);

Eina_Bool udisk_mount(Device *d);
Eina_Bool udisk_umount(Device *d);
Eina_Bool udisk_dbus_init();
void udisk_dbus_shutdown();



#endif