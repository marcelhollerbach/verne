#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include "emous_priv.h"
#include <Efreet.h>

int EMOUS_MOUNT_REQUEST_ADD;
int EMOUS_MOUNT_REQUEST_DEL;

/*
 * Eio file which is valid while the scripts are loaded
 */
Eio_File *ls;

/*
 * List of resolver icons
 */
Eina_List *resolver;

Eina_Hash *exes;

EAPI Eina_List*
emous_mount_type_get(void)
{
  return resolver;
}

EAPI Emous_Mount_Type*
emous_mount_type_name_get(const char *name)
{
   Eina_List *node;
   Emous_Mount_Type *res;

   EINA_LIST_FOREACH(resolver, node, res)
     {
        if (!strcmp(res->type, name))
          break;
     }
   return res;
}


Emous_Mount_Type*
_emous_mount_type_get_or_create(const char *name)
{
   //TODO if NULL create a new
   return emous_mount_type_name_get(name);
}

static Eina_Bool
_data_get(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc  EINA_UNUSED, void *event_info)
{
  Ecore_Exe_Event_Data *del = event_info;
  Emous_Mount_Request *req = data;
  char buf[PATH_MAX];

  if (req->buf)
    {
       snprintf(buf, sizeof(buf), "%s\n%s", req->buf, del->data);
       free(req->buf);
       req->buf = strdup(buf);
    }
  else
    req->buf = strdup(del->data);

  return EINA_FALSE;
}

EAPI Eina_Bool
emous_request_mount(Emous_Mount_Type *fs, const char *source, const char *mountpoint)
{
   Ecore_Exe *exe;
   Emous_Mount_Request *req;
   char exe_path[PATH_MAX];
   const char *_source, *param;


   req = calloc(1, sizeof(Emous_Mount_Request));

   _source = strdup(source);
   param = strdup(mountpoint);

   snprintf(exe_path, sizeof(exe_path), "%s %s %s 2>&1", fs->script_path, _source, param);

   exe = ecore_exe_pipe_run(exe_path, ECORE_EXE_PIPE_WRITE | ECORE_EXE_PIPE_READ | ECORE_EXE_PIPE_READ_LINE_BUFFERED, NULL);

   eo_do(exe, eo_event_callback_add(ECORE_EXE_EVENT_DATA_GET, _data_get, req));

   if (!exe)
     {
        ERR("Exe failed!");
        free(req);
        return EINA_FALSE;
     }

   req->source = _source;
   req->mount_point = param;
   req->type = fs;

   eina_hash_add(exes, &exe, req);

   return EINA_TRUE;
}

static Eina_Bool
_filter_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, const char *file)
{
   if (!ecore_file_is_dir(file))
     return EINA_FALSE;

   return EINA_TRUE;
}

static void
_main_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, const char *file)
{
   char ini_path[PATH_MAX];
   char script[PATH_MAX];
   Efreet_Ini *ini;
   Emous_Mount_Type *res;
   const char *name, *script_path, *desc;

   snprintf(ini_path, sizeof(ini_path), "%s/resolve.desktop", file);

   if (!ecore_file_exists(ini_path))
     {
         ERR("File does not exist!! %s", ini_path);
         return;
     }

   ini = efreet_ini_new(ini_path);

   if (!ini)
     {
        ERR("Ini file %s could not be loaded", ini_path);
        efreet_ini_free(ini);

        return;
     }

   efreet_ini_section_set(ini, "resolve");

   name = efreet_ini_string_get(ini, "name");
   script_path = efreet_ini_string_get(ini, "script");
   desc = efreet_ini_string_get(ini, "description");

   if (!name || !script_path || !desc)
     {
        ERR("Name script_path and description need to be set!");
        return;
     }

   snprintf(script, sizeof(script), "%s/%s", file, script_path);

   if (!ecore_file_exists(script))
     {
        ERR("Script file %s does not exists", script);
        efreet_ini_free(ini);

        return;
     }

   res = calloc(1, sizeof(Emous_Mount_Type));

   res->type = eina_stringshare_add(name);
   res->script_path = eina_stringshare_add(script);
   res->description = eina_stringshare_add(desc);

   efreet_ini_free(ini);

   INF("Loaded resolver %s", res->type);

   resolver = eina_list_append(resolver, res);
}

static void
_done_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED)
{
  ls = NULL;
}

static void
_error_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, int error EINA_UNUSED)
{
   ls = NULL;
   ERR("A error occoured, not sure which resolvers are loaded!\n");
}

static void
_event_free(void *usr EINA_UNUSED, void *event)
{
   Emous_Mount_Request_End *e;

   e = event;

   free(e->req);
   free(e);
}

static Eina_Bool
_child_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Exe_Event_Del *del = event;

   Emous_Mount_Request *req;
   Eina_Bool success = EINA_TRUE;
   req = eina_hash_find(exes, &del->exe);

   if (!req)
     {
        INF("Not for us!");
        return EINA_FALSE;
     }

   if (del->exit_code != 0)
     {
        //ERR("Mount reqest dropped! content: \n%s\n content_end", req->buf);
        success = EINA_FALSE;
     }

   Emous_Mount_Request_End *e;

   e = calloc(1, sizeof(Emous_Mount_Request_End));

   e->req = req;
   e->success = success;

   ecore_event_add(EMOUS_MOUNT_REQUEST_DEL, e, _event_free, NULL);
   return EINA_FALSE;
}

int
mount_init()
{
   EMOUS_MOUNT_REQUEST_DEL = ecore_event_type_new();
   EMOUS_MOUNT_REQUEST_ADD = ecore_event_type_new();

   ls = eio_file_ls(RESOLVER_DIR, _filter_cb,
                    _main_cb, _done_cb, _error_cb,
                    NULL);

   exes = eina_hash_pointer_new(NULL);

   ecore_event_handler_add(ECORE_EXE_EVENT_DEL, _child_del, NULL);

   return 1;
}

void
mount_shutdown()
{
   Emous_Mount_Type *res;

   EINA_LIST_FREE(resolver, res)
     {
        eina_stringshare_del(res->type);
        eina_stringshare_del(res->description);
        eina_stringshare_del(res->script_path);
        free(res);
     }
}