#include "main.h"
#include <archive.h>
#include <archive_entry.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static void
_extract(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   char *path, *dir, *_dir;
   char goal[PATH_MAX];
   const char *filename;
   char **filename_with_out_fileending;
   struct archive *a;
   struct archive *out;
   struct archive_entry *entry;

   path = data;
   //create a copy since dirname modifys contents
   _dir = strdup(path);
   dir = dirname(_dir);
   filename = ecore_file_file_get(path);
   filename_with_out_fileending = eina_str_split(filename, ".", 2);

   snprintf(goal, sizeof(goal), "%s/%s", dir, filename_with_out_fileending[0]);
   free(filename_with_out_fileending);
   free(_dir);

   a = archive_read_new();
   out = archive_write_disk_new();

   if (!a) {
     return;
   }

   archive_read_support_filter_all(a);
   archive_read_support_format_all(a);

   if (archive_read_open_filename(a, path, 2048) != ARCHIVE_OK)
     {
        printf("%s\n", archive_error_string(a));
        return;
     }

   while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
     {
         const void *buff;
         size_t size = 0;
         int64_t offset = 0;
         int r = 0;
         char path[PATH_MAX];

         snprintf(path, sizeof(path), "%s/%s", goal, archive_entry_pathname(entry));

         archive_entry_set_pathname(entry, path);

         if (archive_write_header(out, entry)
              != ARCHIVE_OK)
           {
              printf("%s\n", archive_error_string(a));
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
                    printf("%s\n", archive_error_string(out));
                    return;
                 }
            }
         archive_write_finish_entry(out);
     }

   archive_read_close(a);
   archive_read_free(a);
}


void
archive_extract(const char *path)
{
   ecore_thread_run(_extract, NULL, NULL, path);
}

typedef struct {
    char *path;
    char *basename;
    char *basename2;
    int offset;
    Eina_List *lst;
    struct archive *archive;
} Create_Request;

static void
_main_cb(void *data, Eio_File *f EINA_UNUSED, const Eina_File_Direct_Info *ff)
{
    Create_Request *req;
    struct archive_entry *entry;
    struct stat file;
    int fd, len = 0;
    char buff[8192];
    char path[PATH_MAX];

    stat(ff->path, &file);

    if (S_ISDIR(file.st_mode)) return;

    req = data;

    snprintf(path, sizeof(path), "%s/%s", req->basename, req->offset + 1 + ff->path);

    entry = archive_entry_new();
    archive_entry_set_pathname(entry, path);
    archive_entry_set_size(entry, file.st_size);
    archive_entry_set_filetype(entry, AE_IFREG);
    archive_entry_set_perm(entry, 0644);
    archive_write_header(req->archive, entry);
    fd = open(ff->path, O_RDONLY);
    len = read(fd, buff, sizeof(buff));
    while ( len > 0 ) {
        archive_write_data(req->archive, buff, len);
        len = read(fd, buff, sizeof(buff));
    }
    close(fd);
    archive_entry_free(entry);
}

static void
_done_cb(void *data, Eio_File *f EINA_UNUSED)
{
    Create_Request *req;

    req = data;
    archive_write_close(req->archive);
    archive_write_free(req->archive);
    free(req->basename2);
    free(req->path);
    free(req);
}

static void
_error_cb(void *data, Eio_File *f EINA_UNUSED, int reason EINA_UNUSED)
{
    Create_Request *req;

    req = data;

    printf("Creating archive %s failed!\n", req->path);

    archive_write_close(req->archive);
    archive_write_free(req->archive);
    free(req->basename2);
    free(req->path);
    free(req);
}

void
archive_create(const char *path, Archive_Type type)
{
   Create_Request *req;
   char goal[PATH_MAX];
   const char *fileending;
   req = calloc(1, sizeof(Create_Request));

   //create new archive
   req->archive = archive_write_new();
   req->path = strdup(path);
   req->basename2 = strdup(path);
   req->basename = basename(req->basename2);

   switch(type)
     {
        case ARCHIVE_TYPE_XZ:
        fileending = ".xz";
        archive_write_add_filter_xz(req->archive);
        break;
        case ARCHIVE_TYPE_ZIP:
        fileending = ".zip";
        archive_write_set_format_zip(req->archive);
        break;
        case ARCHIVE_TYPE_TAR_XZ:
        fileending = ".tar.xz";
        archive_write_add_filter_xz(req->archive);
        archive_write_set_format_pax_restricted(req->archive);
        break;
        case ARCHIVE_TYPE_TAR_GZ:
        fileending = ".tar.gz";
        archive_write_add_filter_gzip(req->archive);
        archive_write_set_format_pax_restricted(req->archive);
        break;
        case ARCHIVE_TYPE_TAR_BZIP2:
        fileending = ".tar.bzip2";
        archive_write_add_filter_bzip2(req->archive);
        archive_write_set_format_pax_restricted(req->archive);
        break;
     }
   {
      int lenght;
      req->offset = lenght = strlen(req->path);

      if (req->path[lenght - 2] == '/') {
         req->path[lenght - 2] = '\0';
         req->offset -= 1;
      }

   }
   snprintf(goal, sizeof(goal), "%s%s", req->path, fileending);

   archive_write_open_filename(req->archive, goal);

   eio_dir_stat_ls(path, NULL, _main_cb, _done_cb, _error_cb, req);

}