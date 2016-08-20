#ifndef CONFIG_H
#define CONFIG_H

#cmakedefine EMOUS_MODULE_PATH @EMOUS_MODULE_PATH@
#cmakedefine Mount_FOUND

#ifdef MOUNT_FOUND
#define MOUNT_FOUND
#endif

#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#endif // CONFIG_H