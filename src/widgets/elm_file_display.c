#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

/*
 * Clicks above the view:
 *
 * simple click:
 *    is left up and down on the same pixel
 * complex click:
 *    is a left down and then a move
 *
 * simple click:
 *  - If a item is under the click pass this event to the view
 *  - If no item is lower ... do nothing
 * complex click:
 *  - If (item is lower)/(items are selected) drag and drop this/those
 *  - If no item is lower init rect
 * rightclick:
 *  - Context menu depending on the
 */
#include "elm_file_display_priv.h"

#define W_DATA(w) Elm_File_Display_Data *pd = eo_data_scope_get(w, ELM_FILE_DISPLAY_CLASS);

typedef struct
{
  const char *group;
  const char *file;
} Drag_Icon;

/* struct to pass the last mouse coords to the drag cb */
typedef struct
{
   int x,y;
   Eina_List *icons; //< the list of 3 or less Drag_Icons;
} Drag_Pass;

/* struct to work with the animation */
typedef struct
{
   Eina_Bool anim_mode; //< true if animation mode is choosen
   Eina_List *icons; //< a list of 3 icons which are the later sample
   const char *ptr; //the string
   Evas_Object *obj; //< the object to start the drag on
   struct {
      Eina_List *icons; //< the animated icons
      Ecore_Animator *timer; //the ecore_animator for the animation
   } anim;
} Anim_Struct;

/* a icon of the animation */
typedef struct
{
  int sx, sy, sw, sh;
  int gr;
  int fw, fh;
  Evas_Object *icon;
} Anim_Icon;

#define STEP_SIZE 30
#define ICON_SIZE_INF 80
#define ICON_SIZE_SUP 80+6*30

static Eina_Hash *views = NULL;
static void _event_rect_mouse_move(void *data, Evas *e, Evas_Object *obj, void *event);
static void _event_rect_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event);

/*
 *======================================
 * View pool stuff
 *======================================
 */

static void
_views_standart_init()
{
   views = eina_hash_string_small_new(NULL);
   eo_do(ELM_FILE_DISPLAY_CLASS,
      elm_obj_file_display_view_pool_add(ELM_FILE_DISPLAY_VIEW_GRID_CLASS);
      elm_obj_file_display_view_pool_add(ELM_FILE_DISPLAY_VIEW_DEBUG_CLASS);
    );
}

static void
_elm_file_display_view_pool_add(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, const Eo_Class *view)
{
   const char *name;

   if (!views)
     _views_standart_init();

   eo_do(view, name = elm_file_display_view_name_get());

   eina_hash_direct_add(views, name, view);
}

static void
_elm_file_display_view_pool_del(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, const Eo_Class *view)
{
   const char *name;

   if (!view)
     return;

   eo_do(view, name = elm_file_display_view_name_get());
   eina_hash_del(views, name, NULL);
}


/*
 *======================================
 * Menu stuff
 *======================================

 The menu:

 ***PRE HOOK***
 _____________________________
      View        >...
      Icon Size   >[o]100-200
                  >...
                  >[o]200
      Sort        >[o]name
                   [o]mtime
                   [o]size
                   [o]ext
                   [x] Folderfirst
                   [x] Folderlast
                   [x] reverse
 ______________________________
 ***LAST HOOK***
 */
static void
_ctx_view_sel(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   Evas_Object *w = evas_object_data_get(obj, "__w");

   eo_do(w, elm_obj_file_display_view_set(data));
}

static void
_ctx_icon_size(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   Evas_Object *w = evas_object_data_get(obj, "__w");
   int size = (int)(uintptr_t)data;

   eo_do(w, elm_obj_file_display_show_icon_size_set(size));
}

static void
_ctx_sort_type(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   Evas_Object *w = evas_object_data_get(obj, "__w");
   int sort_type = (int)(uintptr_t)data;

   eo_do(w, elm_obj_file_display_sort_type_set(sort_type));
}

static void
_ctx_folder_placement(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   Evas_Object *w = evas_object_data_get(obj, "__w");
   Elm_File_Display_Folder_Placement p = ((Elm_File_Display_Folder_Placement)data);

   if (p == (Elm_File_Display_Folder_Placement)config->sort.folder_placement)
     p = ELM_FILE_DISPLAY_FOLDER_PLACEMENT_TRIVIAL;

   eo_do(w, elm_obj_file_display_folder_placement_set(p));
}

static void
_ctx_reverse(void *data EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   Evas_Object *w = evas_object_data_get(obj, "__w");
   Eina_Bool p;

   if (config->sort.reverse != 0)
     p = EINA_FALSE;
   else
     p = EINA_TRUE;
   eo_do(w, elm_obj_file_display_reverse_sort_set(p));
}

static void
_ctx_disable_preview(void *data EINA_UNUSED, Evas_Object *obj, void *event EINA_UNUSED)
{
   Evas_Object *w = evas_object_data_get(obj, "__w");
   Eina_Bool b;

   eo_do(w, b = elm_obj_file_display_filepreview_show_get();
            elm_obj_file_display_filepreview_show_set(!b));
}

static void
_ctx_menu_open(Eo* obj, int x, int y, Efm_File *file)
{
   Evas_Object *menu;
   Elm_Object_Item *it, *it2;
   Elm_File_Display_Menu_Hook ev;

   menu = elm_menu_add(elm_object_top_widget_get(obj));
   evas_object_data_set(menu, "__w", obj);

   ev.menu = menu;
   ev.file = file;

   eo_do(obj,
         eo_event_callback_call(&_ELM_FILE_DISPLAY_EVENT_HOOK_MENU_SELECTOR_START,
                                &ev));
   elm_menu_item_separator_add(menu, NULL);

   /*
    * Views
    */
   it = elm_menu_item_add(menu, NULL, NULL, "Views", NULL, NULL);
   const char *name;

   if (views)
     {
        Eina_Iterator *iter;
        const Eo_Class *klass;

        iter = eina_hash_iterator_data_new(views);

        EINA_ITERATOR_FOREACH(iter, klass);
          {
             eo_do(klass, name = elm_file_display_view_name_get());
             it2 = elm_menu_item_add(menu, it, NULL, name, _ctx_view_sel, klass);
          }
     }

   /*
    * File preview
    */
   {
      Evas_Object *ch;
      Eina_Bool v;

      ch = elm_check_add(menu);
      elm_object_text_set(ch, "Filepreview");

      eo_do(obj, v = elm_obj_file_display_filepreview_show_get());

      elm_check_state_set(ch, v);

      it = elm_menu_item_add(menu, NULL, NULL, NULL, _ctx_disable_preview, ch);
      elm_object_item_content_set(it, ch);
   }

   /*
    * Icon size
    */
   it = elm_menu_item_add(menu, NULL, NULL, "Icon Size", NULL, NULL);
   {
      Evas_Object *rad;
      int c;
      char buf[PATH_MAX];
      //make the menu for the icons

      for (c = ICON_SIZE_INF; c < ICON_SIZE_SUP; c += STEP_SIZE)
        {
           snprintf(buf, sizeof(buf), "%d", c);

           rad = elm_check_add(menu);
           elm_object_text_set(rad, buf);

           if (config->icon_size == c)
             elm_check_state_set(rad, EINA_TRUE);

           it2 = elm_menu_item_add(menu, it, NULL, NULL, _ctx_icon_size, (void *)(uintptr_t)c);
           elm_object_item_content_set(it2, rad);
        }
   }

   /*
    * Sort Settings
    */
   it = elm_menu_item_add(menu, NULL, NULL, "Sort", NULL, NULL);
   {
      Evas_Object *gr, *rad, *ck;

      //SORT_TYPE_SIZE
      it2 = elm_menu_item_add(menu, it, NULL, NULL, _ctx_sort_type, (void *)(uintptr_t)ELM_FILE_DISPLAY_SORT_TYPE_SIZE);
      gr = rad = elm_radio_add(menu);
      elm_object_text_set(rad, "Size");
      elm_radio_state_value_set(rad, 0);
      elm_object_item_content_set(it2, rad);

      //SORT_TYPE_DATE
      it2 = elm_menu_item_add(menu, it, NULL, NULL, _ctx_sort_type, (void *)(uintptr_t)ELM_FILE_DISPLAY_SORT_TYPE_DATE);
      rad = elm_radio_add(menu);
      elm_radio_group_add(rad, gr);
      elm_object_text_set(rad, "Date");
      elm_radio_state_value_set(rad, 1);
      elm_object_item_content_set(it2, rad);

      //SORT_TYPE_NAME
      it2 = elm_menu_item_add(menu, it, NULL, NULL, _ctx_sort_type, (void *)(uintptr_t)ELM_FILE_DISPLAY_SORT_TYPE_NAME);
      rad = elm_radio_add(menu);
      elm_radio_group_add(rad, gr);
      elm_object_text_set(rad, "Name");
      elm_radio_state_value_set(rad, 2);
      elm_object_item_content_set(it2, rad);

      //SORT_TYPE_EXTENSION
      it2 = elm_menu_item_add(menu, it, NULL, NULL, _ctx_sort_type, (void *)(uintptr_t)ELM_FILE_DISPLAY_SORT_TYPE_EXTENSION);
      rad = elm_radio_add(menu);
      elm_radio_group_add(rad, gr);
      elm_object_text_set(rad, "Extension");
      elm_radio_state_value_set(rad, 3);
      elm_object_item_content_set(it2, rad);
      elm_radio_value_set(gr, config->sort.type);

      elm_menu_item_separator_add(menu, it);

      //FOLDER_FIRST
      it2 = elm_menu_item_add(menu, it, NULL, NULL, _ctx_folder_placement, (void *)(uintptr_t)ELM_FILE_DISPLAY_FOLDER_PLACEMENT_FIRST);
      ck = elm_check_add(menu);
      if (config->sort.folder_placement == ELM_FILE_DISPLAY_FOLDER_PLACEMENT_FIRST)
        elm_check_state_set(ck, EINA_TRUE);
      elm_object_text_set(ck, "Folder first");
      elm_object_item_content_set(it2, ck);

      it2 = elm_menu_item_add(menu, it, NULL, NULL, _ctx_folder_placement, (void *)(uintptr_t)ELM_FILE_DISPLAY_FOLDER_PLACEMENT_LAST);
      ck = elm_check_add(menu);
      if (config->sort.folder_placement == ELM_FILE_DISPLAY_FOLDER_PLACEMENT_LAST)
        elm_check_state_set(ck, EINA_TRUE);
      elm_object_text_set(ck, "Folder last");
      elm_object_item_content_set(it2, ck);

      elm_menu_item_separator_add(menu, it);

      it2 = elm_menu_item_add(menu, it, NULL, NULL, _ctx_reverse, NULL);
      ck = elm_check_add(menu);
      if (config->sort.reverse != 0)
        elm_check_state_set(ck, EINA_TRUE);
      elm_object_text_set(ck, "Reverse");
      elm_object_item_content_set(it2, ck);
   }
   elm_menu_item_separator_add(menu, NULL);
   eo_do(obj,
         eo_event_callback_call(&_ELM_FILE_DISPLAY_EVENT_HOOK_MENU_SELECTOR_END,
                                &ev));
   elm_menu_move(menu, x, y);
   evas_object_show(menu);
}


/*
 *======================================
 * ANIMATION STUFF
 *======================================
 * Once the user pressed the left button and continues
 * moving the mouse, the items returned by the selection_get
 * will be used to create the samples.
 * After that the drag_animation will be started.
 * if this is finished, "real" dnd with elm will be started
 * if the mousebutton is left before that everything will be freed.
 */

/* Hide the icons of a animation*/
static void
_drag_anim_hide(Anim_Struct *as)
{
   Anim_Icon *sti;

   if (!as->anim.icons) return;

   EINA_LIST_FREE(as->anim.icons, sti)
     {
        evas_object_hide(sti->icon);
        free(sti);
     }

   as->anim.icons = NULL;
}

/* free all ressources of a animation */
static void
_drag_anim_free(Anim_Struct *as)
{
   _drag_anim_hide(as);

   if (as->anim.timer)
     ecore_animator_del(as->anim.timer);

   free(as);
}

static Evas_Object*
_drag_icon_create(void *data, Evas_Object *win, Evas_Coord *xoff, Evas_Coord *yoff)
{
   Evas_Object *icon, *res, *rai;
   Eina_List *node;
   Drag_Pass *p = data;
   Drag_Icon *ic;
   char buf[PATH_MAX];
   int i = 0;

   res = elm_layout_add(win);
   if (!elm_layout_theme_set(res, "file_display", "drag_icon", "default"))
     ERR("Failed to set selection theme, selection is invisible ...");

   evas_object_move(res, 0,0);
   evas_object_resize(res, 60, 60);

   EINA_LIST_FOREACH(p->icons, node, ic)
     {
        i++;

        icon = elm_image_add(win);
        evas_object_show(icon);

        elm_image_file_set(icon,ic->file, ic->group);

        if (i == 1)
          rai = icon;

        snprintf(buf, sizeof(buf), "img%d", i);
        elm_object_part_content_set(res, buf, icon);
     }

   evas_object_raise(rai);
   evas_object_show(res);

   if (xoff) *xoff = p->x - 30;
   if (yoff) *yoff = p->y - 30;

   free(p);
   return res;
}

static const char *
_list_to_char(Eina_List *items)
{
   const char *drag_data = NULL;
   Eina_List *l;
   Elm_File_Display_View_DndFile *it;
   unsigned int len = 0;

   EINA_LIST_FOREACH(items, l, it)
     {
        const char *filename;
        Efm_File *file;

        eo_do(it->file_icon, file = elm_obj_file_icon_fm_monitor_file_get());
        eo_do(file, filename = efm_file_obj_filename_get());
        len += strlen(filename);
     }

  drag_data = malloc(len + eina_list_count(items) * (FILESEP_LEN + 1) + 1);
  strcpy((char *) drag_data, "");

  /* drag data in form: file://URI1\nfile://URI2\n */
  EINA_LIST_FOREACH(items, l, it)
    {
       const char *filename;
       Efm_File *file;

       eo_do(it->file_icon, file = elm_obj_file_icon_fm_monitor_file_get());
       eo_do(file, filename = efm_file_obj_filename_get());

       strcat((char *) drag_data, FILESEP);
       strcat((char *) drag_data, filename);
       strcat((char *) drag_data, "\n");
    }
  return drag_data;
}

/**
 * Start drag in warp mode
 */
static void
_drag_start(Eina_List *icons, Evas_Object *object, const char* ptr)
{
   Evas_Coord xm, ym;
   Drag_Pass *m;

   evas_pointer_canvas_xy_get(evas_object_evas_get(object), &xm, &ym);
   m = calloc(1, sizeof(Drag_Pass));

   m->x = xm;
   m->y = ym;

   m->icons = icons;

   elm_drag_start(object, ELM_SEL_FORMAT_TARGETS, ptr, ELM_XDND_ACTION_COPY,
                       _drag_icon_create, m, NULL, NULL, NULL, NULL, NULL, NULL);
}

static Eina_Bool
_drag_anim_play(void *data, double pos)
{
   Anim_Struct *as = data;
   Anim_Icon *sti;
   Eina_List *l;
   Evas_Coord xm, ym;

   evas_pointer_canvas_xy_get(evas_object_evas_get(as->obj), &xm, &ym);

   if (pos > 0.99)
     {
        _drag_start(as->icons, as->obj, as->ptr);
        _drag_anim_hide(as);
        as->anim.timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }

   EINA_LIST_FOREACH(as->anim.icons, l, sti)
     {
        int x, y, h, w;
        w = sti->sw + ((sti->fw - sti->sw) * pos);
        h = sti->sh + ((sti->fh - sti->sh) * pos);
        x = sti->sx - (pos * ((sti->sx + (w/2) - xm)));
        y = sti->sy - (pos * ((sti->sy + (h/2) - ym)));
        evas_object_move(sti->icon, x, y);
        evas_object_resize(sti->icon, w, h);
     }
   return ECORE_CALLBACK_RENEW;
}

/**
 * Start drag in animation mode
 */
static Anim_Struct*
_drag_anim_start(Eina_List *icons, Evas_Object *object, const char* ptr)
{
  Eina_List *display_icons = NULL, *node;
  Elm_File_Display_View_DndFile *vd;
  Evas_Object *icon;
  Anim_Icon *ai;
  Anim_Struct *ans;
  Drag_Icon *drag_icon;
  int c = 0;

  ans = calloc(1, sizeof(Anim_Struct));
  ans->ptr = ptr;
  ans->obj = object;

  EINA_LIST_FOREACH(icons, node, vd)
    {
       const char *sample_icon_file;
       const char *sample_icon_group;

       eo_do(vd->file_icon, elm_obj_file_icon_fill_sample(&sample_icon_group, &sample_icon_file));

       /* init end icons */
       if (c < 3)
         {
            drag_icon = calloc(1, sizeof (Drag_Icon));

            drag_icon->group = sample_icon_group;
            drag_icon->file = sample_icon_file;

            ans->icons = eina_list_append(ans->icons, drag_icon);
            c++;
         }
       ai = calloc(1, sizeof(Anim_Icon));

       ai->sx = vd->x;
       ai->sy = vd->y;
       ai->sw = vd->w;
       ai->sh = vd->h;
       ai->fh = 60;
       ai->fw = 60;

       icon = elm_icon_add(object);
       elm_image_file_set(icon, sample_icon_file, sample_icon_group);
       evas_object_move(icon, vd->x , vd->y);
       evas_object_resize(icon, vd->w, vd->h);
       evas_object_show(icon);

       ai->icon = icon;

       display_icons = eina_list_append(display_icons, ai);
    }

  ans->anim.icons = display_icons;

  ans->anim.timer = ecore_animator_timeline_add(0.2, _drag_anim_play, ans);
  return ans;
}

/*
 *======================================
 * EVENT RECT
 *======================================
 */

static void
_selections_del(Eina_List *list)
{
   Elm_File_Display_View_DndFile *vd;

   EINA_LIST_FREE(list, vd)
     {
        free(vd);
     }
}

static void
_event_rect_mouse_up(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   W_DATA(data)

   /* clear selected */
   if (pd->event.on_item)
     {
        //drop ...
        pd->event.on_item = EINA_FALSE;

        if (pd->drag_data)
          _drag_anim_free(pd->drag_data);
        pd->drag_data = NULL;
     }
   else
     {
        int x, y, w, h;

        /* get geometry */
        evas_object_geometry_get(pd->event.selection, &x, &y, &w, &h);

        /* call for selection*/
        eo_do(pd->cached_view, elm_file_display_view_items_select(x, y, w, h));

        /* check if those really exists */
        evas_object_del(pd->event.selection);
        pd->event.selection = NULL;
     }
   pd->event.mouse_down.x = -1;
   pd->event.mouse_down.y = -1;
   evas_object_event_callback_del_full(pd->event.rect, EVAS_CALLBACK_MOUSE_UP, _event_rect_mouse_up, data);
   evas_object_event_callback_del_full(pd->event.rect, EVAS_CALLBACK_MOUSE_MOVE, _event_rect_mouse_move, data);
}

static void
_event_rect_mouse_down(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
  W_DATA(data)
  Evas_Event_Mouse_Down *ev = event;
  Efm_File *file;

  //check if there is a item under it and save it if it is
  eo_do(pd->cached_view, file = elm_file_display_view_item_get(ev->output.x, ev->output.y));
  if (ev->button == 1)
    {
       if (file)
         {
            pd->event.on_item = EINA_TRUE;
         }
       else //save the coords otherwise
         {
            pd->event.mouse_down.x = ev->output.x;
            pd->event.mouse_down.y = ev->output.y;
         }
       evas_object_event_callback_add(pd->event.rect, EVAS_CALLBACK_MOUSE_UP, _event_rect_mouse_up, data);
       evas_object_event_callback_add(pd->event.rect, EVAS_CALLBACK_MOUSE_MOVE, _event_rect_mouse_move, data);
    }
   else if (ev->button == 3)
    {
       _ctx_menu_open(pd->obj, ev->output.x, ev->output.y, file);
    }
}

static void
_event_rect_mouse_move(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event)
{
   W_DATA(data)
   Evas_Event_Mouse_Move *ev = event;
   /* are we in dnd mode ? */
   if (pd->event.on_item)
     {
        /* start possible dragging */
        const char *ptr;
        Eina_List *dnd_selection;

        if (pd->drag_data) return;

        /* updated selections*/
        eo_do(pd->cached_view, dnd_selection = elm_file_display_view_selection_get());

        if (!dnd_selection)
          return;

        ptr= _list_to_char(dnd_selection);

        pd->drag_data = _drag_anim_start(dnd_selection, elm_object_parent_widget_get(pd->cached_view), ptr);
        _selections_del(dnd_selection);
        return;
     }
   else
     {
       int w, h, x, y, rx, ry, rw, rh;

       if (!pd->event.selection)
         {
            pd->event.selection = elm_layout_add(pd->obj);
            if (!elm_layout_theme_set(pd->event.selection, "file_display", "selection", "default"))
              ERR("Failed to set selection theme, selection is invisible ...");
            evas_object_show(pd->event.selection);
         }

       x = pd->event.mouse_down.x;
       y = pd->event.mouse_down.y;

       w = ev->cur.canvas.x - pd->event.mouse_down.x;
       h = ev->cur.canvas.y - pd->event.mouse_down.y;

       evas_object_geometry_get(pd->event.rect, &rx, &ry, &rw, &rh);

       if (w < 0)
         {
            w *= -1;
            if (x - w > rx)
              {
                 x = x - w;
              }
            else
              {
                 x = rx;
                 w = pd->event.mouse_down.x - rx;
              }
         }

       if (h < 0)
         {
            h *= -1;
            if (y - h > ry)
              {
                 y = y - h;
              }
            else
              {
                 y = ry;
                 h = pd->event.mouse_down.y - ry;
              }
         }

       if (x+w > rx+rw)
         w = (rx+rw)-x;

       if (y+h > ry+rh)
         h = (ry+rh)-y;

       evas_object_move(pd->event.selection, x, y);
       evas_object_resize(pd->event.selection, w, h);
       return;
   }
}
/*
 *======================================
 * VIEW EVENTS
 *======================================
 */
static Eina_Bool
_view_resize_cb(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desk EINA_UNUSED, void *event EINA_UNUSED)
{
  Elm_File_Display_Data *pd;
  int x, y, w, h;

  pd = data;
  eo_do(pd->cached_view, elm_file_display_view_size_get(&x, &y, &w, &h));

  evas_object_resize(pd->event.rect, w, h);
  evas_object_move(pd->event.rect, x, y);

  evas_object_show(pd->event.rect);
  evas_object_layer_set(pd->event.rect, EVAS_LAYER_MAX);

  return EINA_TRUE;
}

Eina_Bool
_item_select_changed(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desk EINA_UNUSED, void *event EINA_UNUSED)
{
   Eo *wid = data;
   Elm_File_Display_Data *pd;

   if (!event)
     return EINA_TRUE;

   pd = eo_data_scope_get(wid, ELM_FILE_DISPLAY_CLASS);

   pd->selection = event;

   eo_do(wid, eo_event_callback_call(ELM_FILE_DISPLAY_EVENT_ITEM_SELECTION_CHANGED, pd->selection));
   return EINA_TRUE;
}
/*
 *======================================
 * WIDGET STUFF
 *======================================
 */
EOLIAN static void
_elm_file_display_view_set(Eo *obj, Elm_File_Display_Data *pd, const Eo_Class *klass)
{
   if (pd->cached_view)
     eo_del(pd->cached_view);

   pd->view_klass = klass;
   pd->cached_view = eo_add(klass, obj);

   elm_object_part_content_set(obj, "content", pd->cached_view);
   evas_object_show(pd->cached_view);
   eo_do(pd->cached_view, eo_event_callback_add(EVAS_OBJECT_EVENT_RESIZE, _view_resize_cb, pd);
                          eo_event_callback_add(ELM_FILE_DISPLAY_VIEW_EVENT_ITEM_SELECT_SIMPLE, _util_item_select_simple, obj);
                          eo_event_callback_add(ELM_FILE_DISPLAY_VIEW_EVENT_ITEM_SELECT_CHOOSEN, _util_item_select_choosen, obj);
                          eo_event_callback_add(ELM_FILE_DISPLAY_VIEW_EVENT_ITEM_SELECT_CHANGED, _item_select_changed, obj);
                          elm_file_display_view_path_set(pd->current_path);
                          );
   _view_resize_cb(pd, NULL, NULL, NULL);
}

EOLIAN static const Eo_Class*
_elm_file_display_view_get(Eo *obj EINA_UNUSED, Elm_File_Display_Data *pd)
{
   return pd->view_klass;
}

EOLIAN Eo*
_elm_file_display_eo_base_constructor(Eo *obj, Elm_File_Display_Data *pd)
{
  Eo *eo;

  efm_init();
  eo_do(EMOUS_CLASS, emous_init());
  config_init();
  if (!views)
    _views_standart_init();

  pd->show_filepreview = EINA_TRUE;
  pd->current_path = eina_stringshare_add("/");
  pd->obj = obj;

  eo_do(ELM_FILE_MIMETYPE_CACHE_CLASS, cache = elm_file_mimetype_cache_generate(config->icon_size));
  return eo_do_super_ret(obj, ELM_FILE_DISPLAY_CLASS, eo, eo_constructor());
}

EOLIAN void
_elm_file_display_eo_base_destructor(Eo *obj, Elm_File_Display_Data *pd EINA_UNUSED)
{
   eo_del(pd->cached_view);
   efm_shutdown();
   emous_shutdown();
   config_shutdown();
   eo_do_super(obj, ELM_FILE_DISPLAY_CLASS, eo_destructor());
}


EOLIAN static void
_elm_file_display_evas_object_smart_add(Eo *obj, Elm_File_Display_Data *pd)
{
   Evas_Object *o;
   const Eo_Class *view;

   eo_do_super(obj, ELM_FILE_DISPLAY_CLASS, evas_obj_smart_add());

   if (!elm_layout_theme_set(obj, "file_display", "base", "default"))
     {
        CRI("Failed to set theme file\n");
     }

   pd->bookmark = o = bookmark_add(obj);
   elm_object_part_content_set(obj, "bookmark", o);

   pd->preview = o = filepreview_add(obj);
   elm_object_part_content_set(obj, "filepreview", o);

   view = eina_hash_find(views, config->viewname);
   if (!view)
     {
        ERR("The saved view cannot be found, falling back to grid");
        view = ELM_FILE_DISPLAY_VIEW_GRID_CLASS;
     }

   eo_do(obj, elm_obj_file_display_view_set(view));

   pd->event.rect = evas_object_rectangle_add(obj);
   evas_object_repeat_events_set(pd->event.rect, EINA_TRUE);
   evas_object_color_set(pd->event.rect, 0, 0, 0, 0);
   evas_object_event_callback_add(pd->event.rect, EVAS_CALLBACK_MOUSE_DOWN, _event_rect_mouse_down, obj);

   pd->event.mouse_down.x = -1;
   pd->event.mouse_down.y = -1;
}

EOLIAN static void
_elm_file_display_show_icon_size_set(Eo *obj, Elm_File_Display_Data *pd, int size)
{
     config->icon_size = size;
     config_save();
     //refresh the url so the view is doing everything new
     eo_do(obj, efl_file_set(pd->current_path, NULL));
     eo_del(cache);
     eo_do(ELM_FILE_MIMETYPE_CACHE_CLASS, cache = elm_file_mimetype_cache_generate(size));
}

EOLIAN static int
_elm_file_display_show_icon_size_get(Eo *obj EINA_UNUSED, Elm_File_Display_Data *pd EINA_UNUSED)
{
     return config->icon_size;
}

EOLIAN static Elm_File_Display_Sort_Type
_elm_file_display_sort_type_get(Eo *obj EINA_UNUSED, Elm_File_Display_Data *pd EINA_UNUSED)
{
   return config->sort.type;
}

EOLIAN void
_elm_file_display_bookmark_show_set(Eo *obj EINA_UNUSED, Elm_File_Display_Data *pd, Eina_Bool b)
{
  char *sig;

  if (b)
    sig = "bookmark,visible";
  else
    sig = "bookmark,invisible";

  pd->show_bookmark = b;

  elm_object_signal_emit(obj, sig, "elm");
}

EOLIAN Eina_Bool
_elm_file_display_bookmark_show_get(Eo *obj EINA_UNUSED, Elm_File_Display_Data *pd)
{
  return pd->show_bookmark;
}

EOLIAN void
_elm_file_display_filepreview_show_set(Eo *obj EINA_UNUSED, Elm_File_Display_Data *pd, Eina_Bool b)
{
  char *sig;

  if (b)
    sig = "filepreview,visible";
  else
    sig = "filepreview,invisible";

  pd->show_filepreview = b;

  elm_object_signal_emit(obj, sig, "elm");
}

EOLIAN Eina_Bool
_elm_file_display_filepreview_show_get(Eo *obj EINA_UNUSED, Elm_File_Display_Data *pd)
{
  return pd->show_filepreview;
}

EOLIAN static void
_elm_file_display_show_hidden_file_set(Eo *obj, Elm_File_Display_Data *pd, Eina_Bool hidden)
{
     config->hidden_files = hidden;
     config_save();
     //refresh the url so the view is doing everything new
     eo_do(obj, efl_file_set(pd->current_path, NULL));
}

EOLIAN static Eina_Bool
_elm_file_display_show_hidden_file_get(Eo *obj EINA_UNUSED, Elm_File_Display_Data *pd EINA_UNUSED)
{
     return config->hidden_files;
}

EOLIAN static void
_elm_file_display_sort_type_set(Eo *obj, Elm_File_Display_Data *pd, Elm_File_Display_Sort_Type t)
{
   config->sort.type = t;
   config_save();
   //refresh the url so the view is doing everything new
   eo_do(obj, efl_file_set(pd->current_path, NULL));
}

EOLIAN void
_elm_file_display_folder_placement_set(Eo *obj, Elm_File_Display_Data *pd, Elm_File_Display_Folder_Placement t)
{
   config->sort.folder_placement = t;
   config_save();
   //refresh the url so the view is doing everything new
   eo_do(obj, efl_file_set(pd->current_path, NULL));
}

EOLIAN Elm_File_Display_Folder_Placement
_elm_file_display_folder_placement_get(Eo *obj EINA_UNUSED, Elm_File_Display_Data *pd EINA_UNUSED)
{
   return config->sort.folder_placement;
}

EOLIAN void
_elm_file_display_reverse_sort_set(Eo *obj, Elm_File_Display_Data *pd, Eina_Bool b)
{
   config->sort.reverse = b;
   config_save();
   //refresh the url so the view is doing everything new
   eo_do(obj, efl_file_set(pd->current_path, NULL));
}

EOLIAN Eina_Bool
_elm_file_display_reverse_sort_get(Eo *obj EINA_UNUSED, Elm_File_Display_Data *pd EINA_UNUSED)
{
   return config->sort.reverse;
}

EOLIAN static Eina_Bool
_elm_file_display_efl_file_file_set(Eo *obj, Elm_File_Display_Data *pd, const char *file, const char *key EINA_UNUSED)
{
   if (!ecore_file_exists(file))
     return EINA_FALSE;
   if (pd->current_path)
     eina_stringshare_del(pd->current_path);

   pd->current_path = eina_stringshare_add(file);
   eo_do(pd->cached_view, elm_file_display_view_path_set(pd->current_path));
   eo_do(obj, eo_event_callback_call(ELM_FILE_DISPLAY_EVENT_PATH_CHANGED_USER, (void*)pd->current_path));
   return EINA_TRUE;
}

EOLIAN static void
_elm_file_display_efl_file_file_get(Eo *obj EINA_UNUSED , Elm_File_Display_Data *pd, const char **file, const char **key)
{
   if (file)
     *file = pd->current_path;
   if (key)
     *key = NULL;
}

EOLIAN void
_elm_file_display_evas_object_smart_show(Eo *obj, Elm_File_Display_Data *pd)
{
   eo_do_super(obj, ELM_FILE_DISPLAY_CLASS, evas_obj_smart_show());
   evas_object_show(pd->event.rect);
}

EOLIAN void
_elm_file_display_evas_object_smart_hide(Eo *obj, Elm_File_Display_Data *pd)
{
  eo_do_super(obj, ELM_FILE_DISPLAY_CLASS, evas_obj_smart_hide());
  evas_object_hide(pd->event.rect);
}

EOLIAN Eina_List*
_elm_file_display_selection_get(Eo *obj EINA_UNUSED, Elm_File_Display_Data *pd)
{
   return pd->selection;
}

#include "elm_file_display.eo.x"
