#include "jesus.h"

typedef struct {
  const char *split; //What will be splitted
  char* (*replacement)(Eina_List *f);
} Command_Split;

static char*
filename(Eina_List *files)
{
   Efm_File *f;
   const char *filename;

   f = eina_list_data_get(files);

   return strdup(eo_do_ret(f, filename, efm_file_path_get()));
}

static char*
filelist(Eina_List *files)
{
  Eina_Strbuf *result;
  Eina_List *node;
  Efm_File *f;

  result = eina_strbuf_new();

  EINA_LIST_FOREACH(files, node, f)
    {
       const char *filename;

       eo_do(f, filename = efm_file_path_get());

       eina_strbuf_append(result, filename);
       eina_strbuf_append(result, " ");
    }
   {
     char *res;

     res = eina_strbuf_string_steal(result);
     eina_strbuf_free(result);

     return res;
   }
}

static char*
pathname(Eina_List *files)
{
   char result[PATH_MAX];
   const char *filename;
   Efm_File *f;

   f = eina_list_data_get(files);
   eo_do(f, filename = efm_file_path_get());

   snprintf(result, sizeof(result), "file:///%s", filename);

   return strdup(result);
}

static char*
pathlist(Eina_List *files)
{
  Eina_Strbuf *result;
  Eina_List *node;
  Efm_File *f;

  result = eina_strbuf_new();

  EINA_LIST_FOREACH(files, node, f)
    {
       const char *filename;
       char path[PATH_MAX];

       eo_do(f, filename = efm_file_path_get());
       snprintf(path, sizeof(path), "file:///%s ", filename);

       eina_strbuf_append(result, path);
    }
   {
     char *res;

     res = eina_strbuf_string_steal(result);
     eina_strbuf_free(result);

     return res;
   }
}

static char*
exec_run_cmd_gen(const char *cmd, Eina_List *files)
{
   int i = 0, x = 0;
   char *tmpcmd = strdup(cmd);
   Command_Split splits[] = {
     {"%f", filename},
     {"%F", filelist},
     {"%u", pathname},
     {"%U", pathlist},
     {NULL, NULL}
   };

   for(x = 0; splits[x].split; x++)
     {
        Eina_Strbuf *effective;
        Command_Split split = splits[x];
        char **parts;
        char *name;
        unsigned int c;

        parts = eina_str_split_full(tmpcmd, split.split, 0, &c);

        //the split was not in the cmd
        if (c < 2)
          continue;

        effective = eina_strbuf_new();

        name = split.replacement(files);

        if (!name)
          continue;

        //now reassamble our command
        for(i = 0; parts[i]; i ++)
          {
             eina_strbuf_append(effective, parts[i]);
             if (parts[i + 1])
               eina_strbuf_append(effective, name);
          }

        free(name);
        free(tmpcmd);
        tmpcmd = eina_strbuf_string_steal(effective);
        eina_strbuf_free(effective);
     }
   return tmpcmd;
}

void
exec_run(const char *cmd, Efm_File *f)
{
  const char *effcmd;
  Eina_List *lst = NULL;

  lst = eina_list_append(lst, f);

  effcmd = exec_run_cmd_gen(cmd, lst);

  ecore_exe_run(effcmd, NULL);
}