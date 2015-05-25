#include "Elementary_Ext.h"

int log_domain;

void
elm_ext_init()
{

   Elm_Theme *th = elm_theme_default_get();

   elm_theme_extension_add(th, THEME_PATH"/elm_ext.edc.edj");

   elm_theme_flush(th);

   log_domain = eina_log_domain_register("elm_ext", NULL);
}
