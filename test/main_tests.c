#include <glib.h>

static void test_stuff(){
    g_assert (1 == 1); //Say
}

int main (int argc, char **argv){
    g_test_init (&argc, &argv);
    g_test_add_func ("/TestTest", test_stuff);

    return g_test_run();
}
