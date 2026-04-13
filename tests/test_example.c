#include <stdio.h>

int test_context_functions(void);
int test_parse_functions(void);
int test_string_functions(void);

int main(void)
{
    int failures = 0;

    failures += test_context_functions();
    failures += test_parse_functions();
    failures += test_string_functions();

    if (failures != 0)
    {
        printf("FAIL: %d test assertion(s) failed\n", failures);
        return 1;
    }

    printf("PASS\n");
    return 0;
}
