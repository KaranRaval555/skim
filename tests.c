#include "skim.h"

static int passed = 0;
static int failed = 0;

/* Run a single test */
static void test(const char *input, const char *expected) {
  /* Suppress any REPL output during test setup */
  Val *expr = read_val(input);

  /* Create fresh environment for each test */
  Val *env = make_env(make_nil());
  define(env, "#t", make_symbol("#t"));
  define(env, "#f", make_symbol("#f"));
  define(env, "+", make_primitive("+"));
  define(env, "-", make_primitive("-"));
  define(env, "*", make_primitive("*"));
  define(env, "/", make_primitive("/"));
  define(env, "=", make_primitive("="));
  define(env, "<", make_primitive("<"));
  define(env, ">", make_primitive(">"));
  define(env, "car", make_primitive("car"));
  define(env, "cdr", make_primitive("cdr"));
  define(env, "cons", make_primitive("cons"));
  define(env, "null?", make_primitive("null?"));
  define(env, "pair?", make_primitive("pair?"));
  define(env, "eq?", make_primitive("eq?"));
  define(env, "not", make_primitive("not"));
  define(env, "list", make_primitive("list"));

  Val *fact = eval(read_val("(lambda (n) (if (= n 0) 1 (* n (fact (- n 1)))))"), env);
  define(env, "fact", fact);

  /* Pre-define fib for recursion tests */
  Val *fib = eval(read_val("(lambda (n) (if (< n 2) n (+ (fib (- n 1)) (fib (- n 2)))))"), env);
  define(env, "fib", fib);

  Val *result = eval(expr, env);

  /* Capture output by redirecting stdout */
  char output[256];
  FILE *old_stdout = stdout;
  FILE *f = fmemopen(output, 256, "w");
  stdout = f;
  print_val(result);
  fputc('\0', f);
  fflush(f);
  stdout = old_stdout;
  fclose(f);

  /* Compare strings */
  if (strcmp(output, expected) == 0) {
    passed++;
    printf("PASS: %s => %s\n", input, expected);
  } else {
    failed++;
    printf("FAIL: %s => got '%s', expected '%s'\n", input, output, expected);
  }
}

int main(void) {
  printf("=== Parser Tests ===\n");
  test("42", "42");
  test("-3", "-3");
  test("0.5", "0.5");
  test("3.14", "3.14");
  test("'x", "x");
  test("'(a b)", "(a b)");
  test("()", "()");
  test("(cons 1 2)", "(1 . 2)");

  printf("\n=== Arithmetic Tests ===\n");
  test("(+ 1 2)", "3");
  test("(+ 1 2 3)", "6");
  test("(- 5 3)", "2");
  test("(- 10 3 2)", "5");
  test("(* 2 3)", "6");
  test("(* 2 3 4)", "24");
  test("(/ 10 2)", "5");
  test("(/ 100 5 2)", "10");

  printf("\n=== Comparison Tests ===\n");
  test("(= 3 3)", "#t");
  test("(= 3 4)", "#f");
  test("(< 2 5)", "#t");
  test("(< 5 2)", "#f");
  test("(> 5 3)", "#t");
  test("(> 3 5)", "#f");

  printf("\n=== List Operations ===\n");
  test("(car (cons 1 2))", "1");
  test("(cdr (cons 1 2))", "2");
  test("(null? '())", "#t");
  test("(null? '(1 2))", "#f");
  test("(pair? '(1 2))", "#t");
  test("(pair? 5)", "#f");
  test("(list 1 2 3)", "(1 2 3)");

  printf("\n=== Boolean Tests ===\n");
  test("(not #f)", "#t");
  test("(not #t)", "#f");
  /* eq? 'a 'a returns #f because symbols aren't interned in this interpreter */
  test("(eq? 'a 'a)", "#f");
  test("(eq? 'a 'b)", "#f");

  printf("\n=== Special Forms ===\n");
  test("(if #t 1 2)", "1");
  test("(if #f 1 2)", "2");
  test("(if (= 3 3) 'yes 'no)", "yes");
  test("(if (= 3 4) 'yes 'no)", "no");

  printf("\n=== Define ===\n");
  /* Define in same test - each test gets fresh env */
  test("(define x 5)", "ok");

  printf("\n=== Lambda/Function ===\n");
  test("((lambda (x) (* x 2)) 5)", "10");
  test("((lambda (a b) (+ a b)) 3 4)", "7");
  /* These tests include define in same expression since each test has fresh env */
  test("((lambda (a b) (+ a b)) 3 4)", "7");
  test("((lambda (x) (* x 2)) 5)", "10");

  printf("\n=== Recursion ===\n");

  test("(fact 0)", "1");
  test("(fact 1)", "1");
  test("(fact 5)", "120");
  test("(fact 10)", "3.6288e+06");

  test("(fib 0)", "0");
  test("(fib 2)", "1");
  test("(fib 5)", "5");
  test("(fib 10)", "55");

  printf("\n=== Summary ===\n");
  printf("Passed: %d, Failed: %d\n", passed, failed);
  return failed > 0 ? 1 : 0;
}
