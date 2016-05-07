#include "../elementary_ext_priv.h"

#define GTK_FILESEP "file://"
#define GTK_FILESEP_LEN sizeof(FILESEP) - 1

#define CONFIG_NAME "config"
#define CONFIG_VERSION "0.2"
#define CONFIG_KEY CONFIG_NAME"-"CONFIG_VERSION

typedef struct $
{
  const char *path;
  Eio_Monitor *filewatcher;
  Eet_Data_Descriptor *eed_config;
} Config_Context;

static int counter = 0;
static Config_Context *ctx;
Config *config = NULL;

static void
_standart_setup(Config *c)
{
  c->display_gtk = 1;
  c->viewname = eina_stringshare_add("Grid");
  c->icon_size = 110;

  c->sort.type = ELM_FILE_SELECTOR_SORT_TYPE_NAME;
  c->sort.folder_placement = ELM_FILE_SELECTOR_FOLDER_PLACEMENT_FIRST;
  // TODO add good standart values
}

Eina_List*
util_bookmarks_load_gtk(void)
{
   char file[PATH_MAX];
   char filebuf[PATH_MAX];
   Eina_List *result = NULL;
   FILE *fd;

   snprintf(file, sizeof(file),"%s/gtk-3.0/bookmarks", efreet_config_home_get());

   fd = fopen(file, "r");

   if (!fd)
     return NULL;

   while(fscanf(fd, "%s\n", filebuf) != EOF)
     {
        Efreet_Uri *uri;
        char **contents = eina_str_split(filebuf, " ", 2);

        uri = efreet_uri_decode(contents[0]);
        if (!uri) continue;

        result = eina_list_append(result, eina_stringshare_add(uri->path));
     }

   fclose(fd);

   return result;
}

static void
_config_free(void)
{
   const char *ptr;

   if (!config) return;

   EINA_LIST_FREE(config->bookmarks, ptr)
     {
        eina_stringshare_del(ptr);
     }
   eina_stringshare_del(config->viewname);
   free(config);
   config = NULL;
}

void
elm_ext_config_save(void)
{
   Eet_File *ef;

   ef = eet_open(ctx->path, EET_FILE_MODE_READ_WRITE);

   if (!ef)
     ERR("Failed to open file");

   if (!eet_data_write(ef, ctx->eed_config, CONFIG_KEY, config, 0))
     ERR("Failed to write data");

   eet_close(ef);
}

void
elm_ext_config_read(void)
{
   Eet_File *ef;

   ef = eet_open(ctx->path, EET_FILE_MODE_READ_WRITE);

   if (!ef)
     ERR("Failed to open file");

   _config_free();
   config = eet_data_read(ef, ctx->eed_config, CONFIG_KEY);

   eet_close(ef);

   if (!config)
     {
        config = calloc(1, sizeof(Config));
        _standart_setup(config);
        elm_ext_config_save();
     }
}

void
helper_bookmarks_add(const char *ptr)
{
   Eina_Iterator *iter;
   const char *bm;
   const char *new_bm;

   new_bm = eina_stringshare_add(ptr);

   iter = eina_list_iterator_new(config->bookmarks);

   EINA_ITERATOR_FOREACH(iter, bm)
     {
        if (bm == new_bm)
          return;
     }

   config->bookmarks = eina_list_append(config->bookmarks, ptr);
   elm_ext_config_save();
}

void
helper_bookmarks_del(const char *ptr)
{
   Eina_List *node;
   const char *r;

   EINA_LIST_FOREACH(config->bookmarks, node, r)
     {
        if (!strcmp(r, ptr))
          {
             config->bookmarks = eina_list_remove(config->bookmarks, r);
             break;
          }
     }

   elm_ext_config_save();
}

void
elm_ext_config_init(void)
{
   Eet_Data_Descriptor_Class eddc;
   char buf[PATH_MAX];

   if (counter > 0)
     goto inc;

   eina_init();
   eet_init();
   efreet_init();

   ctx = calloc(1, sizeof(Config_Context));

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Config);
   ctx->eed_config = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_LIST_STRING(ctx->eed_config, Config, "bookmarks", bookmarks);

   #define ADD_BASIC(key, field, type) \
     EET_DATA_DESCRIPTOR_ADD_BASIC(ctx->eed_config, Config, key, field, type);

   ADD_BASIC("display_gtk", display_gtk, EET_T_CHAR);
   ADD_BASIC("viewname", viewname, EET_T_STRING);
   ADD_BASIC("icon_size", icon_size, EET_T_INT);
   ADD_BASIC("only_folder", only_folder, EET_T_CHAR);
   ADD_BASIC("hidden_files", hidden_files, EET_T_CHAR);
   ADD_BASIC("image_preview", image_preview, EET_T_CHAR);
   ADD_BASIC("sort_type", sort.type, EET_T_INT);
   ADD_BASIC("folder_placement", sort.folder_placement, EET_T_INT);
   ADD_BASIC("reverse", sort.reverse, EET_T_CHAR);
   ADD_BASIC("casesensetive", sort.casesensetive, EET_T_CHAR);

   snprintf(buf, sizeof(buf),"%s/efm_config.eet", efreet_config_home_get());
   // TODO monitor of the file, if it changes reload!
   // TODO add ecore event for the change
   ctx->path = eina_stringshare_add(buf);

   elm_ext_config_read();

inc:
   counter ++;
}

void
elm_ext_config_shutdown(void)
{
   counter --;
   if (counter > 0)
     return;

   eina_stringshare_del(ctx->path);
   _config_free();
}
