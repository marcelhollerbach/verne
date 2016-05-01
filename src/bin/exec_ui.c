#define _GNU_SOURCE
#include "jesus.h"

typedef struct  {
  Open_With2_Main_Win_Widgets *wid;
  Eina_List *entrys;
  Efm_File *file;
  Cmd_Choosen choosen;
  Elm_Object_Item *normal;
  Elm_Object_Item *recommented;
  Elm_Object_Item *last_recommend;
  Eina_List *desktops;
  Elm_Genlist_Item_Class *klass;
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
// open with dialog things
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
    efl_file_set(result, desk->icon, NULL);
  else
    elm_icon_standard_set(result, desk->icon); // FIXME this is slowing everything down ... find a better solution

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

static char*
_group_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part EINA_UNUSED)
{
   return strdup(data);
}

static int
_compare(const void *a, const void *b)
{
   Efreet_Desktop *desk_a = elm_object_item_data_get(a);
   Efreet_Desktop *desk_b = elm_object_item_data_get(b);
   int i = 0;

   while ((desk_a && desk_a->name[i] != '\0')
          && (desk_b && desk_b->name[i] != '\0'))
     {
        if (desk_a->name[i] > desk_b->name[i])
          return 1;
        else if (desk_a->name[i] < desk_b->name[i])
          return -1;
        i ++;
     }
   return 0;
}

static Eina_Bool
_transform(void *data)
{
   Open_With_Ui *ui;
   Efreet_Desktop *desk;
   const char *mime_type;
   Elm_Object_Item *it;

   ui = data;
   mime_type = efm_file_mimetype_get(ui->file);
   desk = eina_list_data_get(ui->desktops);

   ui->desktops = eina_list_remove(ui->desktops, desk);

   if (_list_compare(desk->mime_types, mime_type))
     {
        // this is a recommended app
        ui->last_recommend = elm_genlist_item_sorted_insert(ui->wid->elm_genlist1, ui->klass, desk, ui->recommented, 0, _compare, NULL, desk);
     }
   it = elm_genlist_item_sorted_insert(ui->wid->elm_genlist1, ui->klass, desk, ui->normal, 0, _compare, NULL, desk);
   //allways add the normal app
   ui->entrys = eina_list_append(ui->entrys, it);

   ui->desktops = eina_list_remove(ui->desktops, desk);

   if (!ui->desktops)
     {
        elm_genlist_item_class_free(ui->klass);
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static void
_genlist_fill(Open_With_Ui *ui)
{
   Elm_Genlist_Item_Class *gklass;

   ui->klass = elm_genlist_item_class_new();
   ui->klass->item_style = "default";
   ui->klass->func.text_get = _text_get;
   ui->klass->func.content_get = _content_get;

   gklass = elm_genlist_item_class_new();
   gklass->item_style = "group_index";
   gklass->func.text_get = _group_text_get;

   // initial headlines for recommented and not recommented apps
   ui->recommented = ui->last_recommend = elm_genlist_item_append(ui->wid->elm_genlist1, gklass, "Recommended apps", NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
   ui->normal = elm_genlist_item_append(ui->wid->elm_genlist1, gklass, "Normal apps", NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
   ui->desktops = efreet_util_desktop_name_glob_list("*");

   ecore_idler_add(_transform, ui);

   //remove recommented if there are no apps
   elm_genlist_item_class_free(gklass);
}

static void
_label_fill(Open_With_Ui *ui)
{
    Efreet_Desktop *desk = NULL;
    const char *mime_type;
    const char *name;

    mime_type = efm_file_mimetype_get(ui->file);

    if (!mime_type)
      return;

    name = eina_hash_find(config->mime_type_open, mime_type);

    if (name)
      desk = efreet_util_desktop_name_find(name);

    if (desk)
      elm_object_text_set(ui->wid->current_app, desk->name);
    else
      elm_object_text_set(ui->wid->current_app, "No Default");
}

static void
_search_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   Open_With_Ui *ui = data;
   Elm_Object_Item *searched = NULL, *it;
   Eina_List *node;
   const char *search;
   int min = -1;

   search = elm_object_text_get(ui->wid->search);

   EINA_LIST_FOREACH(ui->entrys, node, it)
     {
        Efreet_Desktop *desk;
        char *f;

        desk = elm_object_item_data_get(it);
        if ((f = strcasestr(desk->name, search)))
          {
             int tmin = f - desk->name;
             if (min == -1)
               min = tmin;

             if (tmin > min)
               continue;
             min = tmin;
             searched = it;
          }
     }
   elm_genlist_item_selected_set(searched, EINA_TRUE);
   elm_genlist_item_bring_in(searched, ELM_GENLIST_ITEM_SCROLLTO_MIDDLE);
   //XXX: HAAACK
   elm_object_focus_set(ui->wid->search, EINA_TRUE);
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

   evas_object_del(req->wid->main_win);

   if (req->choosen)
     req->choosen(req->file, desk);
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

   if (!it) return;

   desk = elm_object_item_data_get(it);

   if (!desk) return;

   mime_type = efm_file_mimetype_get(req->file);

   if (eina_hash_find(config->mime_type_open, mime_type))
     eina_hash_del(config->mime_type_open, mime_type, NULL);
   if (!eina_hash_add(config->mime_type_open, mime_type, desk->name))
     printf("Error, adding mimetype failed");

   config_flush();

   elm_genlist_clear(req->wid->elm_genlist1);
   eina_list_free(req->entrys);
   req->entrys = NULL;

   _genlist_fill(req);
   _label_fill(req);
}

void
exec_ui_open_with(Efm_File *file, Cmd_Choosen choosen)
{
    Open_With2_Main_Win_Widgets *wid;
    Open_With_Ui *ui;

    if (!config)
      return;


    ui = calloc(1, sizeof(Open_With_Ui));

    ui->file = file;
    ui->choosen = choosen;
    ui->wid = wid = open_with2_main_win_create(win);

    // filling the ui
    _genlist_fill(ui);

    evas_object_smart_callback_add(wid->open, "clicked", _open_cb, ui);
    evas_object_smart_callback_add(wid->asdefault, "clicked", _as_default_cb, ui);

    //set current app
    _label_fill(ui);

    //listen for changes in the entry
    evas_object_smart_callback_add(wid->search, "changed", _search_cb, ui);
}