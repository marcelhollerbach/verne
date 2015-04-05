#include "elm_file_display_priv.h"

static int ctx = 0;

int log_domain;

void
log_init(void)
{
   if (ctx > 0)
     goto inc;

   log_domain = eina_log_domain_register("file_displayer", "cyan");

inc:
   ctx ++;
}

void
log_shutdown(void)
{
   ctx --;
   if (ctx > 0)
     return;
   eina_log_domain_unregister(log_domain);
}