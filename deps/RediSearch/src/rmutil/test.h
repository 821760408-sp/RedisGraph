#ifndef __TESTUTIL_H__
#define __TESTUTIL_H__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static int numTests = 0;
static int numAsserts = 0;

#undef MY_STRINGIFY
#define MY_STRINGIFY(x) #x

#define TESTFUNC(f)                            \
  printf("  Testing %s\t\t", MY_STRINGIFY(f)); \
  numTests++;                                  \
  fflush(stdout);                              \
  if (f()) {                                   \
    printf(" %s FAILED!\n", MY_STRINGIFY(f));  \
    exit(1);                                   \
  } else                                       \
    printf("[PASS]\n");

#define ASSERTM(expr, ...)                                                                 \
  if (!(expr)) {                                                                           \
    fprintf(stderr, "%s:%d: Assertion '%s' Failed: " __VA_ARGS__ "\n", __FILE__, __LINE__, \
            MY_STRINGIFY(expr));                                                           \
    return -1;                                                                             \
  }                                                                                        \
  numAsserts++;

#define ASSERT(expr)                                                                          \
  if (!(expr)) {                                                                              \
    fprintf(stderr, "%s:%d Assertion '%s' Failed\n", __FILE__, __LINE__, MY_STRINGIFY(expr)); \
    return -1;                                                                                \
  }                                                                                           \
  numAsserts++;

#define ASSERT_STRING_EQ(s1, s2)                    \
  if (strcmp(s1, s2)) {                             \
    fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); \
    fprintf(stderr, "'%s' != '%s'\n", s1, s2);      \
    return -1;                                      \
  }

#define ASSERT_EQUAL(x, y, ...)                                           \
  if (x != y) {                                                           \
    fprintf(stderr, "%s:%d: ", __FILE__, __LINE__);                       \
    fprintf(stderr, "%g != %g: " __VA_ARGS__ "\n", (double)x, (double)y); \
    return -1;                                                            \
  }                                                                       \
  numAsserts++;

#define FAIL(fmt, ...)                                                            \
  {                                                                               \
    fprintf(stderr, "%s:%d: FAIL: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    return -1;                                                                    \
  }

#define RETURN_TEST_SUCCESS return 0;
#define TEST_CASE(x, block) \
  int x {                   \
    block;                  \
    return 0                \
  }

#define PRINT_TEST_SUMMARY printf("\nTotal: %d tests and %d assertions OK\n", numTests, numAsserts);

#define TEST_MAIN(body)                         \
  int main(int argc, char **argv) {             \
    printf("Starting Test '%s'...\n", argv[0]); \
    body;                                       \
    PRINT_TEST_SUMMARY;                         \
    printf("\n--------------------\n\n");       \
    return 0;                                   \
  }
#endif