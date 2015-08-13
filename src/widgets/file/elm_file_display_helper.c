#include "elm_file_display_priv.h"

static int
_alphabetic_sort(const char *n1, const char *n2)
{
   int c = 0;


   while(n1[c] != '\0' && n2[c] != '\0')
     {
        char c1, c2;

        c1 = n1[c];
        c2 = n2[c];
        if (!config->sort.casesensetive)
          {
             c1 = tolower(c1);
             c2 = tolower(c2);
          }

        if (c1 < c2)
          return -1;
        else if (c1 > c2)
          return 1;
        c ++;
     }
  return 0;
}

static int
_file_name_sort(Efm_File *f1, Efm_File *f2)
{
   const char *n1;
   const char *n2;
   eo_do(f1, n1 = efm_file_filename_get());
   eo_do(f2, n2 = efm_file_filename_get());

   if (n1[0] == '.')
     n1 ++;
   if (n2[0] == '.')
     n2 ++;
   return _alphabetic_sort(n1, n2);
}

static int
sort_name_func(const void *data1, const void *data2)
{
   Efm_File *f1 = ((Efm_File*)data1), *f2 = ((Efm_File*)data2);
   if (config->sort.type == ELM_FILE_DISPLAY_SORT_TYPE_NAME)
     {
        return _file_name_sort(f1, f2);
     }
   else if (config->sort.type == ELM_FILE_DISPLAY_SORT_TYPE_SIZE)
     {
       Efm_File_Stat *st1, *st2;

        eo_do(f1, st1 = efm_file_stat_get());
        eo_do(f2, st2 = efm_file_stat_get());

        if (st1->size > st2->size)
          return 1;
        else
          return -1;
     }
   else if (config->sort.type == ELM_FILE_DISPLAY_SORT_TYPE_DATE)
     {
        Efm_File_Stat *st1, *st2;

        eo_do(f1, st1 = efm_file_stat_get());
        eo_do(f2, st2 = efm_file_stat_get());

        if (st1->mtime > st2->mtime)
          return 1;
        else
          return -1;
     }
   else // if (config->sort.type == SORT_TYPE_EXTENSION)
     {
        const char *ext1;
        const char *ext2;

        eo_do(f1, ext1 = efm_file_fileending_get());
        eo_do(f2, ext2 = efm_file_fileending_get());
        if (!ext1 && !ext2)
          return _file_name_sort(f1, f2);
        else if (!ext1 && ext2)
          return -1;
        else if (ext1 && !ext2)
          return 1;

        int sort = _alphabetic_sort(ext1, ext2);
        if (sort == 0)
          return _file_name_sort(f1, f2);
        else
          return sort;
     }
}

int
sort_func(const void *data1, const void *data2)
{
   Eina_Bool is;
   Efm_File *f1, *f2;
   int mul;

   f1 = elm_object_item_data_get(data1);
   f2 = elm_object_item_data_get(data2);

   if (config->sort.reverse)
     mul = -1;
   else
     mul = 1;

   if (eo_do_ret(f1, is, efm_file_is_type(EFM_FILE_TYPE_DIRECTORY)) &&
       eo_do_ret(f2, is, efm_file_is_type(EFM_FILE_TYPE_DIRECTORY)))
     {
       return sort_name_func(f1, f2) * mul;
     }
   else if (eo_do_ret(f1, is, efm_file_is_type(EFM_FILE_TYPE_DIRECTORY)) &&
            !eo_do_ret(f2, is, efm_file_is_type(EFM_FILE_TYPE_DIRECTORY)))
     {
        if (config->sort.folder_placement == ELM_FILE_DISPLAY_FOLDER_PLACEMENT_FIRST)
          return -1;
        else if (config->sort.folder_placement == ELM_FILE_DISPLAY_FOLDER_PLACEMENT_LAST)
          return 1;
        else
          return sort_name_func(f1, f2) * mul;
     }
   else if (!eo_do_ret(f1, is, efm_file_is_type(EFM_FILE_TYPE_DIRECTORY)) &&
            eo_do_ret(f2, is, efm_file_is_type(EFM_FILE_TYPE_DIRECTORY)))
     {
        if (config->sort.folder_placement == ELM_FILE_DISPLAY_FOLDER_PLACEMENT_FIRST)
          return 1;
        else if (config->sort.folder_placement == ELM_FILE_DISPLAY_FOLDER_PLACEMENT_LAST)
          return -1;
        else
          return sort_name_func(f1, f2)* mul;
     }
   else
     {
        return sort_name_func(f1, f2) * mul;
     }
}