#include "jesus.h"

typedef struct  {
  Executorui_Open_With_Widgets *wid;
  Efm_File *file;
  Cmd_Choosen choosen;
} Open_With_Ui;

static Eina_Bool
_list_compare(Eina_List *list, const char *mime_type)
{
   Eina_List *node;
   const char *mime_type2;

   EINA_LIST_FOREACH(list, node, mime_type2)
   {
      if (!strcmp(mime_type2, mime_type))
        return EINA_TRUE;
   }
   return EINA_FALSE;
}

//====================================================
//open with dialog things
//====================================================

static char*
_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   Efreet_Desktop *desk;

   desk = data;

   return strdup(desk->name);
}

static Evas_Object*
_gen_icon(Efreet_Desktop *desk, Evas_Object *obj)
{
  Evas_Object *result;

  result = elm_icon_add(obj);

  if (ecore_file_exists(desk->icon))
    eo_do(result, efl_file_set(desk->icon, NULL));
  else
    elm_icon_standard_set(result, desk->icon); //FIXME this is slowing everything down ... find a better solution

  return result;
}

static Evas_Object*
_content_get(void *data, Evas_Object *obj, const char *part)
{
   Evas_Object *result = NULL;

   if (!strcmp(part, "elm.swallow.icon"))
     {
        result = _gen_icon(data, obj);
     }
   return result;
}

static Evas_Object*
_default_content_get(void *data, Evas_Object *obj, const char *part)
{
   Evas_Object *result = NULL;

   if (!strcmp(part, "elm.swallow.icon"))
     {
        result = _gen_icon(data, obj);
     }
   else if (!strcmp(part, "elm.swallow.end"))
     {
        result = elm_layout_add(obj);
        eo_do(result, efl_file_set(THEME_PATH"/efm.edc.edj", "default_indicator"););
     }
   return result;
}

static char*
_group_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   return strdup(data);
}

static void
_genlist_fill(Open_With_Ui *ui, const char *cmd)
{
  Eina_List *cats;
  Elm_Genlist_Item_Class *klass, *klassdefault, *gklass;
  Elm_Object_Item *last_recommend = NULL;
  const char *cat;
  const char *mime_type;

  eo_do(ui->file, mime_type = efm_file_obj_mimetype_get());

  klass = elm_genlist_item_class_new();
  klass->item_style = "default";
  klass->func.text_get = _text_get;
  klass->func.content_get = _content_get;

  //yeah we are creating a seperated item class for the default item
  //if we dont do this we have to pass a tuple or something as item data
  //which results in a allocation of memory, which is very very slow
  //this is much faster
  klassdefault = elm_genlist_item_class_new();
  klassdefault->item_style = "default";
  klassdefault->func.text_get = _text_get;
  klassdefault->func.content_get = _default_content_get;

  gklass = elm_genlist_item_class_new();
  gklass->item_style = "group_index";
  gklass->func.text_get = _group_text_get;

  //initial headlines for recommented and not recommented apps
  last_recommend = elm_genlist_item_append(ui->wid->elm_genlist1, gklass, "Recommended apps", NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
  elm_genlist_item_append(ui->wid->elm_genlist1, gklass, "Normal apps", NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
  cats = efreet_util_desktop_categories_list();

  EINA_LIST_FREE(cats, cat)
    {
       Eina_List *entrys;
       Efreet_Desktop *desk;

       entrys = efreet_util_desktop_category_list(cat);
       EINA_LIST_FREE(entrys, desk)
         {
            Elm_Genlist_Item_Class *choosen;

            if (cmd && !strcmp(cmd, desk->exec))
              choosen = klassdefault;
            else
              choosen = klass;

            if (_list_compare(desk->mime_types, mime_type))
              {
                 //this is a recommended app
                 last_recommend = elm_genlist_item_insert_after(ui->wid->elm_genlist1, choosen, desk, NULL, last_recommend, 0, NULL, NULL);
              }
            else
              {
                 //this is not
                 elm_genlist_item_append(ui->wid->elm_genlist1, choosen, desk, NULL, 0, NULL, NULL);
              }
         }
    }
  elm_genlist_item_class_free(gklass);
  elm_genlist_item_class_free(klassdefault);
  elm_genlist_item_class_free(klass);
}

static void
_open_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   Open_With_Ui *req;
   Efreet_Desktop *desk;
   Elm_Object_Item *it;

   req = data;
   it = elm_genlist_selected_item_get(req->wid->elm_genlist1);
   desk = elm_object_item_data_get(it);

   evas_object_del(req->wid->open_with);

   if (req->choosen)
     req->choosen(req->file, desk->exec);
}

static void
_as_default_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   Open_With_Ui *req;
   Efreet_Desktop *desk;
   Elm_Object_Item *it;
   const char *mime_type;

   req = data;
   it = elm_genlist_selected_item_get(req->wid->elm_genlist1);
   desk = elm_object_item_data_get(it);

   eo_do(req->file, mime_type = efm_file_obj_mimetype_get());

   eina_hash_del(config->mime_type_open, mime_type, NULL);
   eina_hash_add(config->mime_type_open, mime_type, desk->exec);

   config_flush();

   elm_genlist_clear(req->wid->elm_genlist1);

   _genlist_fill(req, desk->exec);
}

void
exec_ui_open_with(Efm_File *file, Cmd_Choosen choosen)
{
    const char *mime_type;
    const char *cmd;
    Executorui_Open_With_Widgets *wid;
    Open_With_Ui *ui;

    if (!config)
      return;

    eo_do(file, mime_type = efm_file_obj_mimetype_get());
    if (!mime_type)
      return;

    cmd = eina_hash_find(config->mime_type_open, mime_type);

    ui = calloc(1, sizeof(Open_With_Ui));

    ui->file = file;
    ui->choosen = choosen;
    ui->wid = wid = executorui_open_with_create(win);

    //filling the ui
    _genlist_fill(ui, cmd);

    evas_object_smart_callback_add(wid->open, "clicked", _open_cb, ui);
    evas_object_smart_callback_add(wid->set_as_default, "clicked", _as_default_cb, ui);

    evas_object_show(wid->open_with);
}