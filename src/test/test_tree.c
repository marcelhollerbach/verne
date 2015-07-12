#define EFL_BETA_API_SUPPORT
#define EFL_EO_API_SUPPORT

#include <Eo.h>
#include <Graph.h>
#include <check.h>

static void
_rec_add(Efl_Tree_Base *root, int depth)
{
    Efl_Tree_Base *child;

    if (depth == 0) return;

    for (int i = 0; i < 10; i++)
      {
         child = eo_add(EFL_TREE_BASE_CLASS, NULL);

         _rec_add(child, depth - 1);
         eo_do(root, efl_tree_base_append(child, NULL));
      }
}

START_TEST(tree_creation_time)
{
    eo_init();
    Efl_Tree_Base *root;
    Eina_List *children;
    int i = 0;
    root = eo_add(EFL_TREE_BASE_CLASS, NULL);

    _rec_add(root, 2);

    eo_do(root, children = efl_tree_base_children(EINA_TRUE));
    {
       Eina_List *node;

       EINA_LIST_FOREACH(children, node, root)
         {
            i++;
         }
    }
    ck_assert_int_eq(i, 10+10*10);
    eo_shutdown();
}
END_TEST
START_TEST(next_prev)
{
    eo_init();
    Efl_Tree_Base *root;
    Efl_Tree_Base* child[8];

    root = eo_add(EFL_TREE_BASE_CLASS, NULL);

    for (int i = 0; i < 8; i++)
      {
         child[i] = eo_add(EFL_TREE_BASE_CLASS, root);
         eo_do(root, efl_tree_base_append(child[i], NULL));
      }

    for (int i = 1; i < 7; i++)
      {
         eo_do(child[i],
           ck_assert_ptr_eq(child[i + 1], efl_tree_base_next_get());
           ck_assert_ptr_eq(child[i - 1], efl_tree_base_prev_get());
          );
      }
    eo_do(child[0],
      ck_assert_ptr_eq(child[1], efl_tree_base_next_get());
      ck_assert_ptr_eq(NULL, efl_tree_base_prev_get());
    );
    eo_do(child[7],
      ck_assert_ptr_eq(NULL, efl_tree_base_next_get());
      ck_assert_ptr_eq(child[6], efl_tree_base_prev_get());
    );


    eo_shutdown();
}
END_TEST
Suite * tree_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("tree");

    /* Core test case */
    tc_core = tcase_create("tree");

    tcase_set_timeout(tc_core, 7);
    tcase_add_test(tc_core, tree_creation_time);
    tcase_add_test(tc_core, next_prev);

    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = tree_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
 }