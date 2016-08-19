#include "kvasir.h"

void
exec_execute(Efm_File *f)
{
    const char *name, *mime_type;
    Eina_List *mime_types;
    Efreet_Desktop *icon;

    mime_type = efm_file_mimetype_get(f);

    // first check in config for a "special" open wish
    name = eina_hash_find(config->mime_type_open, mime_type);

    if (!name)
      {
         mime_types = efreet_util_desktop_mime_list(mime_type);
         if (!mime_types)
           return;

         icon = eina_list_data_get(mime_types);
      }
    else
      icon = efreet_util_desktop_name_find(name);
    exec_run(icon, f);
}

void
exec_run(Efreet_Desktop *desk, Efm_File *f)
{
  Eina_List *lst = NULL;
  const char *file;

  file = efm_file_path_get(f);
  lst = eina_list_append(lst, file);

  efreet_desktop_exec(desk, lst, NULL);
}