#include "efm_priv.h"
#include <archive.h>
#include <archive_entry.h>

typedef struct {
    char *goal; //< where the archive is extracted
    char *original; //< the archivonst e extracted from
    int ref;
    const char *error;
} Archive_File;

static Eina_Hash *files;

static void
_extract(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Archive_File *f;
   Eina_List *archive_files = NULL;
   f = data;

   struct archive *a;
   struct archive *out;
   struct archive_entry *entry;

   a = archive_read_new();
   out = archive_write_disk_new();

   if (!a) {
     return;
   }

   archive_read_support_filter_all(a);
   archive_read_support_format_all(a);

   if (archive_read_open_filename(a, f->original, 2048) != ARCHIVE_OK)
     {
        f->error = archive_error_string(a);
        return;
     }

   while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
     {
         const void *buff;
         size_t size = 0;
         int64_t offset = 0;
         int r = 0;
         char path[PATH_MAX];

         snprintf(path, sizeof(path), "%s/%s", f->goal, archive_entry_pathname(entry));

         archive_entry_set_pathname(entry, path);

         if (archive_write_header(out, entry)
              != ARCHIVE_OK)
           {
              f->error =  archive_error_string(a);
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
                    f->error = archive_error_string(out);
                    return;
                 }
            }
         archive_write_finish_entry(out);
         archive_files = eina_list_append(archive_files, strdup(path));
     }

   archive_read_close(a);
   archive_read_free(a);
   f->ref --;

   char *path;

   EINA_LIST_FREE(archive_files, path)
     {
        chmod(path, S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
        free(path);
     }
}

static void
_end(void *data, Ecore_Thread *th EINA_UNUSED) {
   Archive_File *f;

   f = data;

   if (f->error)
     ERR("Extracting %s failed, reason ; %s", f->original, f->error);
}

static char*
_archive_path_gen(const char *archive)
{
   const char *file;
   char *dir;
   char buf[PATH_MAX];

   file = ecore_file_file_get(archive);
   snprintf(buf, sizeof(buf), "/tmp/%s/", file);
   dir = strdup(buf);

   return dir;
}

static void
_archive_free(void *data)
{
   Archive_File *file = data;
   free(file->goal);
   free(file->original);
   free(file);
}

int
archive_init(void)
{
   files = eina_hash_string_superfast_new(_archive_free);
   return 1;
}

const char*
archive_access(const char *archive, int direct)
{
   Archive_File *f;

   f = eina_hash_find(files, archive);

   //if the archive has a error problem delete it and try again
   if (f && f->error)
     {
        eina_hash_del_by_key(files, archive);
        f = NULL;
     }

   if (!f)
     {
        f = calloc(1, sizeof(Archive_File));
        f->goal = _archive_path_gen(archive);
        f->original = strdup(archive);
        ecore_file_mkdir(f->goal);
        f->ref = 0;

        eina_hash_add(files, archive, f);
        if (direct)
          {
             _extract(f, NULL);
             _end(f, NULL);
          }
        else
          {
             ecore_thread_run(_extract, _end, NULL, f);
          }
     }

   //increase reference
   f->ref ++;

   return f->goal;
}

int
archive_unref(const char *archive)
{
   Archive_File *f;

   f = eina_hash_find(files, archive);

   if (!f) return -1;

   if (f->ref == 1)
     {
        eina_hash_del(files, archive, f);
        return 0;
     }
   else
     {
        f->ref --;
        return f->ref;
     }
}

int
archive_shutdown(void)
{
   //TODO delete hash
   return 1;
}

int
archive_support(const char *fileending)
{
#define MATCH(v) !strcmp(fileending, v)
   if (!strncmp(fileending, "tar", 3) ||
       MATCH("cpio") ||
       MATCH("zip") ||
       MATCH("shar") ||
       MATCH("iso") ||
       MATCH("ar") || MATCH("a") ||
       MATCH("xar") ||
       MATCH("lzh") || MATCH("lha") ||
       MATCH("rar") ||
       MATCH("cab") ||
       MATCH("lzma") || MATCH(".7z")
       )
     return EINA_TRUE;
   return EINA_FALSE;
#undef MATCH
}