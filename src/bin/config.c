#include "main.h"

#define CONFIG_NAME "verne-config"
#define CONFIG_VERSION "0.3"
#define CONFIG_KEY CONFIG_NAME"-"CONFIG_VERSION
static Eet_Data_Descriptor *edd;

Config *config;

Config*
_config_standart_new()
{
   Config *c;

   c = calloc(1, sizeof(Config));
   c->mime_type_open = eina_hash_string_small_new(NULL);

   return c;
}

void
_config_free(Config *c)
{
    eina_hash_free(c->mime_type_open);
    free(c);
}

void
_config_read()
{
   Eet_File *cf;
   char path[PATH_MAX];

   if (config)
     {
        _config_free(config);
        config = NULL;
     }

   snprintf(path, sizeof(path), "%s/%s", efreet_config_home_get(), "verne.eet");

   cf = eet_open(path, EET_FILE_MODE_READ);

   if (cf)
     {
        config = eet_data_read(cf, edd, CONFIG_KEY);
        eet_close(cf);
     }

   if (!config)
     {
        config = _config_standart_new();
        config_flush();
     }
}

void
config_init(void)
{
   Eet_Data_Descriptor_Class eddc;

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Config);

   edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_HASH_STRING(edd, Config, "mime_type_open", mime_type_open);

   _config_read();
}

void
config_flush(void)
{
   Eet_File *cf;
   char path[PATH_MAX];

   if (!config)
     return;

   snprintf(path, sizeof(path), "%s/%s", efreet_config_home_get(), "verne.eet");

   cf = eet_open(path, EET_FILE_MODE_WRITE);

   if (!cf)
     {
        printf("Failed to open config file");
     }


   if (!eet_data_write(cf, edd, CONFIG_KEY, config, 0))
     {
        printf("Failed to write config file");
     }

   eet_close(cf);
}

void
config_shutdown(void)
{
    config_flush();

    if (config)
      _config_free(config);
}