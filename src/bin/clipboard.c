#include "jesus.h"

typedef struct
{
    EINA_INLIST;
    const char *file;
} Clipboard_Item;

static Eina_Inlist *clipboard = NULL;
static Clipboard_Mode mode;
static Eina_Trash *trash;

void
clipboard_paste(const char *paste)
{
    Clipboard_Item *item;
    Eina_List *files = NULL;

    EINA_INLIST_FOREACH(clipboard, item)
      {
         if (item->file)
           files = eina_list_append(files, item->file);
      }

    if (mode == COPY)
      fs_operations_copy(files, paste);
    else
      fs_operations_move(files, paste);
}

void
clipboard_set(Clipboard_Mode m, Eina_List *list)
{
    Efm_File *file;
    Eina_List *node;
    Clipboard_Item *item;

    //free existing clipboard
    EINA_INLIST_FOREACH(clipboard, item)
      {
         eina_stringshare_del(item->file);
         eina_trash_push(&trash, item);
      }
    clipboard = NULL;

    mode = m;
    EINA_LIST_FOREACH(list, node, file)
      {
         const char *path;

         item = eina_trash_pop(&trash);

         if (!item)
           item = malloc(sizeof(Clipboard_Item));

         eo_do(file, path = efm_file_path_get());

         item->file = eina_stringshare_add(path);

         clipboard = eina_inlist_append(clipboard, EINA_INLIST_GET(item));
      }
}

Eina_Bool
clipboard_something_in(void)
{
    return (!clipboard) ? EINA_TRUE : EINA_FALSE;
}

void
clipboard_init(void)
{
   eina_trash_init(&trash);
}

void
clipboard_shutdown(void)
{
   Clipboard_Item *item;

   EINA_TRASH_CLEAN(&trash, item)
     {
        free(item);
     }
}