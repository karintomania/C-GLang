#define UNITY_BUILD

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include "lexer.c"
#include "parser.c"
#include "typechecker.c"
#include "interpreter.c"

#include "tests/test_util.c"

#include "tests/lexer_test.c"
#include "tests/parser_test.c"
#include "tests/typechecker_test.c"
#include "tests/interpreter_test.c"


/*---------------------
 Testing framework
------------------------*/
#define MAX_TESTS 1000

typedef struct {
    // TODO: remove return int
    int (*ptr)(void);
    const char *name;
} TestFunc;

#define FUNC_DEF(func) {func, #func}

int main(void) {
  TestFunc tests[] = {
    FUNC_DEF(test_run_lexer),
    FUNC_DEF(test_run_parser),
    FUNC_DEF(test_run_typechecker),
    FUNC_DEF(test_run_interpreter),
    {NULL, ""}, // null termination
  };

  for (int i=0; i < MAX_TESTS; i++) {
    TestFunc test = tests[i];

    if (test.ptr != NULL) {
      printf("\n%s started...\n", test.name);

      test.ptr();

      printf("%s passed!\n", test.name);
    } else {
      break;
    }
  }

  printf("All tests passed.\n");
  return 0;
}

