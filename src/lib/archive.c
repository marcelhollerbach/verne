#include "efm_priv.h"

#include <archive.h>
#include <archive_entry.h>


const char* FILEENDINGS[] = {"tar", "zip", "tar.gz", NULL};

const char *lasterror;

Eina_Bool
efm_archive_file_supported(const char *fileending)
{
   int i = 0;

   if (!fileending) return EINA_FALSE;

   for(; FILEENDINGS[i]; i++)
     {
        if (!strcmp(fileending, FILEENDINGS[i]))
          return EINA_TRUE;
     }
   return EINA_FALSE;
}

Eina_Bool
efm_archive_file_extract(const char *file, const char *goal)
{
   struct archive *a;
   struct archive *out;
   struct archive_entry *entry;

   a = archive_read_new();
   out = archive_write_disk_new();

   if (!a)
     return EINA_FALSE;

   archive_read_support_filter_all(a);
   archive_read_support_format_all(a);

   if (archive_read_open_filename(a, file, 2048) != ARCHIVE_OK)
     {
        lasterror = archive_error_string(a);
        return EINA_FALSE;
     }

   while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
     {
         const void *buff;
         size_t size;
         int64_t offset;
         int r;
         char path[PATH_MAX];

         snprintf(path, sizeof(path), "%s/%s", goal,archive_entry_pathname(entry));

         archive_entry_set_pathname(entry, path);

         if (archive_write_header(out, entry)
              != ARCHIVE_OK)
           {
              lasterror = archive_error_string(a);
              continue;
           }

         for (;;)
            {
               r = archive_read_data_block(a, &buff, &size, &offset);
               if (r == ARCHIVE_EOF)
                 break;

               if (r != ARCHIVE_OK)
                 break;

               if (archive_write_data_block(out, buff, size, offset)
                     != ARCHIVE_OK)
                 {
                    lasterror = archive_error_string(out);
                    return EINA_FALSE;
                 }
            }
         archive_write_finish_entry(out);
     }

   archive_read_close(a);
   archive_read_free(a);
   return EINA_TRUE;
}

const char*
efm_archive_lasterror_get(void)
{
   return lasterror;
}