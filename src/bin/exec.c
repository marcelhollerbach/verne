#include "jesus.h"

typedef struct {
  const char *split; //What will be splitted
  char* (*replacement)(Eina_List *f);
} Command_Split;

#define STRBUF_RETURN(s) \
   { \
     char *res; \
     res = eina_strbuf_string_steal(s); \
     eina_strbuf_free(s); \
     return res; \
   } \

static void
escape_path(const char *path, Eina_Strbuf *buf)
{
   Eina_Strbuf *tmp;

   tmp = eina_strbuf_new();

   eina_strbuf_append(tmp, path);
   eina_strbuf_replace_all(tmp, " ", "\\ ");
   eina_strbuf_replace_all(tmp, "(", "\\(");
   eina_strbuf_replace_all(tmp, ")", "\\)");
   eina_strbuf_replace_all(tmp, "[", "\\[");
   eina_strbuf_replace_all(tmp, "]", "\\]");
   eina_strbuf_replace_all(tmp, "{", "\\{");
   eina_strbuf_replace_all(tmp, "}", "\\}");
   eina_strbuf_append_buffer(buf, tmp);
   eina_strbuf_free(tmp);
}

static char*
filename(Eina_List *files)
{
   Efm_File *f;
   const char *filename;
   Eina_Strbuf *buf;

   buf = eina_strbuf_new();

   f = eina_list_data_get(files);

   eo_do(f, filename = efm_file_path_get());

   escape_path(filename, buf);

   STRBUF_RETURN(buf)
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

        escape_path(filename, result);

        eina_strbuf_append(result, " ");
     }

   STRBUF_RETURN(result);
}

static char*
pathname(Eina_List *files)
{
   Eina_Strbuf *result;
   const char *filename;
   Efm_File *f;

   result = eina_strbuf_new();

   f = eina_list_data_get(files);
   eo_do(f, filename = efm_file_path_get());

   escape_path(filename, result);

   STRBUF_RETURN(result);
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

        eo_do(f, filename = efm_file_path_get());

        eina_strbuf_append(result, "file:///");
        escape_path(filename, result);
     }
   STRBUF_RETURN(result);
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