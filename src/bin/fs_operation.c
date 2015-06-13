#include "jesus.h"

typedef enum {
  OP_COPY,
  OP_MOVE,
  OP_REMOVE
} Operation_Type;

typedef struct {
    Eio_File *operation; //the _maybe_ running operation
    struct {
      Evas_Object *layout; //< The ui used to display this
      Evas_Object *progress; //< the progressbar object
      float progress_value; //< the progress from 0.0 to 1.0.
    } ui;
    const char *goal; //< To display
    const char *from; //< To display
    Operation_Type type; //< the type of this operation

} Operation;

static Eina_List *operations = NULL;
static Eina_Bool time_block;

static Eina_Bool
_timer_idler(void *data)
{
  *((Eina_Bool*)data) = EINA_FALSE;
  return EINA_FALSE;
}

static void
_progress_cb(void *data, Eio_File *file EINA_UNUSED, const Eio_Progress *prog)
{
    Operation *op = data;
    //update ui
    if (!op->ui.layout)
      return;

    op->ui.progress_value = (float)(prog->percent/100.0f);
    if (time_block)
      return;

    ecore_timer_add(0.1, _timer_idler, &time_block);

    time_block = EINA_TRUE;

    elm_progressbar_value_set(op->ui.progress,op->ui.progress_value);
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
    op->ui.progress_value = 1.0f;
    if (op->ui.progress)
      elm_progressbar_value_set(op->ui.progress, op->ui.progress_value);
}

static void
_error_cb(void *data, Eio_File *file EINA_UNUSED, int errornumb)
{
    Operation *op = data;

    printf("Copy Failed : %s\n", strerror(errornumb));
    //TODO this should be moved to a popup
    op->operation = NULL;
}

static void
_operation_convert(Eina_List *files, const char *goal, void (*job)(Operation *op))
{
   Eina_List *node;
   const char *source;

   //converts a list of strings into a list bunch of operations which will be added to the operationsarray
   EINA_LIST_FOREACH(files, node, source)
     {
        Operation *operation;
        char path[PATH_MAX];
        const char *filename;

        filename = ecore_file_file_get(source);

        operation = calloc(1, sizeof(Operation));
        operation->from = source;

        //if there is a goal directory, build a not existing path
        if (goal)
          {
             //build the path
             snprintf(path, sizeof(path), "%s/%s", goal, filename);

             //check if the path allready exists
             if (ecore_file_exists(path))
               {
                  char orig[PATH_MAX];
                  char *name;
                  char *fileending;
                  int i = 0;

                  //copy string to orig
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
    op->type = OP_MOVE;
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
    op->type = OP_COPY;
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

    op->type = OP_REMOVE;
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

static void
_dismissed_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
    evas_object_hide(obj);
    evas_object_del(obj);
}

static void
_popup_cb(void *data EINA_UNUSED, Evas_Object *obj, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
    Evas_Object *popup, *box, *scroller;
    Evas_Coord x, y;
    Operation *operation;
    Eina_List *node;

    if (eina_list_count(operations) == 0)
      return;

    popup = elm_ctxpopup_add(obj);
    elm_ctxpopup_direction_priority_set(popup, ELM_CTXPOPUP_DIRECTION_UP,
                                               ELM_CTXPOPUP_DIRECTION_UP,
                                               ELM_CTXPOPUP_DIRECTION_UP,
                                               ELM_CTXPOPUP_DIRECTION_UP);


    box = elm_box_add(obj);

    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(box, EVAS_HINT_FILL, 0.0);
    EINA_LIST_FOREACH(operations, node, operation)
      {
         Evas_Object *layout, *progress;

         layout = eo_add(ELM_LAYOUT_CLASS, obj);
         evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
         evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
         switch(operation->type)
         {
            case OP_COPY:
            case OP_MOVE:
              //set correct file
              if (operation->type == OP_MOVE)
                eo_do(layout, efl_file_set(THEME_PATH"/efm.edc.edj", "jesus.fs_op.move"));
              else
                eo_do(layout, efl_file_set(THEME_PATH"/efm.edc.edj", "jesus.fs_op.copy"));

              //add progressbar
              progress = eo_add(ELM_PROGRESSBAR_CLASS, obj);
              eo_do(progress, eo_wref_add(&operation->ui.progress));
              //set part and text
              elm_object_part_content_set(layout, "jesus.progress", progress);
              elm_object_part_text_set(layout, "jesus.to", operation->goal);
              elm_object_part_text_set(layout, "jesus.from", operation->from);
              //set progress to correct values
              elm_progressbar_value_set(progress, operation->ui.progress_value);
            break;
            case OP_REMOVE:
              eo_do(layout, efl_file_set(THEME_PATH"/efm.edc.edj", "jesus.fs_op.remove"));
              elm_object_part_text_set(layout, "jesus.from", operation->from);
            break;
         }
         evas_object_show(layout);
         elm_layout_sizing_eval(layout);
         elm_box_pack_end(box, layout);

         eo_do(obj, eo_wref_add(&operation->ui.layout));
      }

    scroller = elm_scroller_add(popup);
    elm_scroller_content_min_limit(scroller, EINA_TRUE, EINA_FALSE);
    elm_object_content_set(scroller, box);

    evas_object_show(scroller);

    elm_object_content_set(popup, scroller);
    evas_object_smart_callback_add(popup, "dismissed", _dismissed_cb, NULL);

    evas_pointer_canvas_xy_get(evas_object_evas_get(popup),&x, &y);

    evas_object_move(popup, x, y);
    evas_object_resize(popup, 200, 200);
    evas_object_show(popup);
}

void
fs_operations_init(void)
{
    elm_layout_signal_callback_add(layout, "jesus.fsaction.popup", "theme", _popup_cb, NULL);

    //Only enable for debugging
    #if 0
    Operation *op;
    op = calloc(1, sizeof(Operation));

    op->type = OP_REMOVE;
    op->from = "bluarb";

    operations = eina_list_append(operations, op);

    op = calloc(1, sizeof(Operation));

    op->type = OP_COPY;
    op->from = "bluarb";
    op->goal = "woaaaahm";

    operations = eina_list_append(operations, op);


    for (int i = 0; i < 10; i++)
      {
         op = calloc(1, sizeof(Operation));

         op->type = OP_MOVE;
         op->from = "bluarb";
         op->goal = "woaaaahm";

         operations = eina_list_append(operations, op);
      }
    #endif
}