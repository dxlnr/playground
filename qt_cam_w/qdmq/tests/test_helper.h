#ifndef TEST_MSGQ_H
#define TEST_MSGQ_H

#include <stdio.h>
#include <stdlib.h>

#define ANSI_COLOR_RED    "\x1b[31m"
#define ANSI_COLOR_GREEN  "\x1b[32m"
#define ANSI_COLOR_RESET  "\x1b[0m"

enum { MAX_TEST_NAME_LEN = 48 };

extern int total;
extern int fails;

#define TEST_ASSERT(expr)                                                    \
  do {                                                                       \
    const char *test_name = __func__;                                        \
    if (!(expr)) {                                                           \
      fprintf(stderr,                                                        \
              "  %-*s : " ANSI_COLOR_RED "FAILED" ANSI_COLOR_RESET " (%s)\n",\
              MAX_TEST_NAME_LEN, test_name,                                  \
              #expr);                                                        \
      ++fails;                                                               \
    } else {                                                                 \
      fprintf(stdout,                                                        \
              "  %-*s : " ANSI_COLOR_GREEN "OK" ANSI_COLOR_RESET "\n",       \
              MAX_TEST_NAME_LEN, test_name);                                 \
    }                                                                        \
    ++total;                                                                 \
    break;                                                                   \
  } while (0)

#endif
