#include "elementary_ext_priv.h"

int ext_log_domain;

EAPI void
elm_ext_init()
{

   Elm_Theme *th = elm_theme_default_get();

   elm_theme_extension_add(th, THEME_PATH"/elm_ext.edc.edj");

   elm_theme_flush(th);

   ext_log_domain = eina_log_domain_register("elm_ext", NULL);
}
