#include "jesus.h"

typedef struct {
    Eio_File *operation; //the _maybe_ running operation
    Evas_Object *object; //< The ui used to display this
    const char *goal; //< To display
    const char *from; //< To display
} Operation;

static Eina_List *operations = NULL;

static void
_progress_cb(void *data, Eio_File *file EINA_UNUSED, const Eio_Progress *prog)
{
    Operation *op = data;
    //update ui
    printf("Progress : %f\n", prog->percent);
}

static Eina_Bool
_filter_cb(void *data EINA_UNUSED, Eio_File *file EINA_UNUSED, const Eina_File_Direct_Info *inf EINA_UNUSED)
{
    //just copy everything
    return EINA_TRUE;
}
static void
_done_cb(void *data, Eio_File *file EINA_UNUSED)
{
    Operation *op = data;

    op->operation = NULL;
}

static void
_error_cb(void *data, Eio_File *file EINA_UNUSED, int errornumb)
{
    Operation *op = data;

    printf("Copy Failed : %s\n", strerror(errornumb));
    op->operation = NULL;
}

static void
_operation_convert(Eina_List *files, const char *goal, void (*job)(Operation *op))
{
   Eina_List *node;
   const char *source;

   EINA_LIST_FOREACH(files, node, source)
     {
        Operation *operation;
        char path[PATH_MAX];
        const char *filename;

        filename = ecore_file_file_get(source);

        operation = calloc(1, sizeof(Operation));
        operation->from = source;
        if (goal)
          {
             snprintf(path, sizeof(path), "%s/%s", goal, filename);

             if (ecore_file_exists(path))
               {
                  char orig[PATH_MAX];
                  char *name;
                  char *fileending;
                  int i = 0;

                  snprintf(orig, sizeof(orig), "%s", path);

                  fileending = strrchr(orig, '.');

                  if (fileending)
                    fileending += 1;

                  name = ecore_file_strip_ext(orig);

                  while (ecore_file_exists(path))
                    {
                       i++;
                       if (fileending)
                         snprintf(path, sizeof(path), "%s_%d.%s", name, i, fileending);
                       else
                         snprintf(path, sizeof(path), "%s_%d", name, i);
                    }
               }
             operation->goal = strdup(path);
          }

        job(operation);

        operations = eina_list_append(operations, operation);

     }
}

static void
_move_job(Operation *op)
{
    if (ecore_file_is_dir(op->from))
      op->operation = eio_dir_move(op->from, op->goal, _filter_cb,
                                   _progress_cb, _done_cb,
                                   _error_cb, op);
    else
      op->operation = eio_file_move(op->from, op->goal,
                                    _progress_cb, _done_cb,
                                    _error_cb, op);
}

static void
_copy_job(Operation *op)
{
    if (ecore_file_is_dir(op->from))
      op->operation = eio_dir_copy(op->from, op->goal, _filter_cb,
                                   _progress_cb, _done_cb,
                                   _error_cb, op);
    else
      op->operation = eio_file_copy(op->from, op->goal,
                                    _progress_cb, _done_cb,
                                    _error_cb, op);
}

static void
_del_job(Operation *op)
{
    Efreet_Uri *uri;
    char path[PATH_MAX];


    snprintf(path, sizeof(path), "file:///%s", op->from);
    uri = efreet_uri_decode(path);

    efreet_trash_delete_uri(uri, 0);
    efreet_uri_free(uri);
}

void
fs_operations_move(Eina_List *files, const char *goal)
{
   _operation_convert(files, goal, _move_job);
}

void
fs_operations_copy(Eina_List *files, const char *goal)
{
   _operation_convert(files, goal, _copy_job);
}

void
fs_operations_delete(Eina_List *files)
{
    _operation_convert(files, NULL, _del_job);
}

void
fs_operations_init(void)
{

}