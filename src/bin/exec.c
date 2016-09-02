#include "main.h"

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

static Efreet_Desktop *
_util_default_terminal_get(const char *defaults_list)
{
   Efreet_Desktop *tdesktop = NULL;
   Efreet_Ini *ini;
   const char *s;

   ini = efreet_ini_new(defaults_list);
   if ((ini) && (ini->data) &&
       (efreet_ini_section_set(ini, "Default Applications")) &&
       (ini->section))
     {
        s = efreet_ini_string_get(ini, "x-scheme-handler/terminal");
        if (s) tdesktop = efreet_util_desktop_file_id_find(s);
     }
   if (ini) efreet_ini_free(ini);
   return tdesktop;
}

static Efreet_Desktop *
_util_terminal_desktop_get(void)
{
   const char *terms[] =
   {
      "terminology.desktop",
      "rxvt.desktop",
      "gnome-terminal.desktop",
      "konsole.desktop",
      NULL
   };
   const char *s;
   char buf[PATH_MAX];
   Efreet_Desktop *tdesktop = NULL, *td;
   Eina_List *l;
   int i;

   s = efreet_data_home_get();
   if (s)
     {
        snprintf(buf, sizeof(buf), "%s/applications/defaults.list", s);
        tdesktop = _util_default_terminal_get(buf);
     }
   if (tdesktop) return tdesktop;
   EINA_LIST_FOREACH(efreet_data_dirs_get(), l, s)
     {
        snprintf(buf, sizeof(buf), "%s/applications/defaults.list", s);
        tdesktop = _util_default_terminal_get(buf);
        if (tdesktop) return tdesktop;
     }

   for (i = 0; terms[i]; i++)
     {
        tdesktop = efreet_util_desktop_file_id_find(terms[i]);
        if (tdesktop) return tdesktop;
     }
   if (!tdesktop)
     {
        l = efreet_util_desktop_category_list("TerminalEmulator");
        if (l)
          {
             // just take first one since above list doesn't work.
             tdesktop = l->data;
             EINA_LIST_FREE(l, td)
               {
                  // free/unref the desktops we are not going to use
                  if (td != tdesktop) efreet_desktop_free(td);
               }
          }
     }
   return tdesktop;
}

void
exec_terminal(Efm_File *f)
{
   Efreet_Desktop *desk;
   char path[PATH_MAX];

   if (!efm_file_is_type(f, EFM_FILE_TYPE_DIRECTORY)) return;

   desk = _util_terminal_desktop_get();

   getcwd(path, sizeof(path));
   chdir(efm_file_path_get(f));

   exec_run(desk, f);

   chdir(path);
}
