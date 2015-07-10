#include "Graph.h"

typedef struct {
    Eina_List *children;
} Efl_Tree_Base_Data;

#undef EAPI

#ifdef _WIN32
# ifdef EFL_ECORE_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ECORE_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

static Eina_Bool
_child_removed(void *data, Eo *obj, const Eo_Event_Description2 *event EINA_UNUSED, void *event_info EINA_UNUSED)
{
    eo_do(data, efl_tree_base_remove(obj));
    return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_child_changes(void *data, Eo *obj, const Eo_Event_Description2 *event, void *event_info)
{
    Efl_Tree_Update ev;

    if (event == EFL_TREE_BASE_EVENT_CHILDREN_DEL_DIRECT)
      {
         ev.parent = obj;
         ev.child = event_info;
         eo_do(data, eo_event_callback_call(EFL_TREE_BASE_EVENT_CHILDREN_DEL_RECURSIVE, &ev));

      }
    else if (event == EFL_TREE_BASE_EVENT_CHILDREN_ADD_DIRECT)
      {
         ev.parent = obj;
         ev.child = event_info;
         eo_do(data, eo_event_callback_call(EFL_TREE_BASE_EVENT_CHILDREN_ADD_RECURSIVE, &ev));

      }
    else if (event == EFL_TREE_BASE_EVENT_CHILDREN_ADD_RECURSIVE || event == EFL_TREE_BASE_EVENT_CHILDREN_DEL_RECURSIVE)
      {
         eo_do(data, eo_event_callback_call(event, event_info));
      }
    return EO_CALLBACK_CONTINUE;
}

static void
_child_subscribe(Eo *obj, Efl_Tree_Base *item)
{
   eo_do(item, eo_event_callback_add(EO_BASE_EVENT_DEL, _child_removed, obj);
               eo_event_callback_add(EFL_TREE_BASE_EVENT_CHILDREN_ADD_RECURSIVE, _child_changes, obj);
               eo_event_callback_add(EFL_TREE_BASE_EVENT_CHILDREN_ADD_DIRECT, _child_changes, obj);
               eo_event_callback_add(EFL_TREE_BASE_EVENT_CHILDREN_DEL_RECURSIVE, _child_changes, obj);
               eo_event_callback_add(EFL_TREE_BASE_EVENT_CHILDREN_DEL_DIRECT, _child_changes, obj);
               );
}
static void
_child_unsubscribe(Eo *obj, Efl_Tree_Base *item)
{
   eo_do(item, eo_event_callback_del(EO_BASE_EVENT_DEL, _child_removed, obj);
               eo_event_callback_del(EFL_TREE_BASE_EVENT_CHILDREN_ADD_RECURSIVE, _child_changes, obj);
               eo_event_callback_del(EFL_TREE_BASE_EVENT_CHILDREN_ADD_DIRECT, _child_changes, obj);
               eo_event_callback_del(EFL_TREE_BASE_EVENT_CHILDREN_DEL_RECURSIVE, _child_changes, obj);
               eo_event_callback_del(EFL_TREE_BASE_EVENT_CHILDREN_DEL_DIRECT, _child_changes, obj);
               );
}
EOLIAN static Eina_List *
_efl_tree_base_children(Eo *obj EINA_UNUSED, Efl_Tree_Base_Data *pd, Eina_Bool recursive)
{
   if (!recursive)
     {
        return pd->children;
     }
   else
     {
        Eina_List *node, *node2, *result = NULL;
        Efl_Tree_Base *it, *child;

        EINA_LIST_FOREACH(pd->children, node, it)
          {
             Eina_List *child_children;

             eo_do(it, child_children = efl_tree_base_children(EINA_TRUE));

             //merge child_children into result
             EINA_LIST_FOREACH(child_children, node2, child)
               {
                  //
                  result = eina_list_append(result, child);
               }
             result = eina_list_append(result, it);
          }
        return result;
     }
}
EOLIAN static void
_efl_tree_base_remove(Eo *obj, Efl_Tree_Base_Data *pd, Efl_Tree_Base *item)
{
   _child_unsubscribe(obj, item);
   pd->children = eina_list_remove(pd->children, item);
   eo_do(obj, eo_event_callback_call(EFL_TREE_BASE_EVENT_CHILDREN_DEL_DIRECT, item));
}

EOLIAN static void
_efl_tree_base_append(Eo *obj, Efl_Tree_Base_Data *pd, Efl_Tree_Base *item, Efl_Tree_Base *relative)
{
   _child_subscribe(obj, item);
   if (relative)
     {
        Eina_List *rel;

        rel = eina_list_data_find(pd->children, relative);
        if (!rel)
          return;
        pd->children = eina_list_append_relative_list(pd->children, item, rel);
     }
   else
     {
        pd->children = eina_list_append(pd->children, item);
     }
   eo_do(obj, eo_event_callback_call(EFL_TREE_BASE_EVENT_CHILDREN_ADD_DIRECT, item));
}

EOLIAN static void
_efl_tree_base_prepend(Eo *obj EINA_UNUSED, Efl_Tree_Base_Data *pd, Efl_Tree_Base *item, Efl_Tree_Base *relative)
{
   _child_subscribe(obj, item);
   if (relative)
     {
        Eina_List *rel;

        rel = eina_list_data_find(pd->children, relative);
        if (!rel)
          return;
        pd->children = eina_list_prepend_relative_list(pd->children, item, rel);
     }
   else
     {
        pd->children = eina_list_prepend(pd->children, item);
     }
   eo_do(obj, eo_event_callback_call(EFL_TREE_BASE_EVENT_CHILDREN_ADD_DIRECT, item));
}

static int
_compare_func(const void *data1, const void *data2)
{
   int res;
   eo_do(data1, res = efl_compare(data2));

   return res;
}

EOLIAN static void
_efl_tree_base_insert_sorted(Eo *obj EINA_UNUSED, Efl_Tree_Base_Data *pd, Efl_Tree_Base *item)
{
   pd->children = eina_list_sorted_insert(pd->children, _compare_func, item);
}

#include "efl_tree_base.eo.x"