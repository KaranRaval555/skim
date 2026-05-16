#include "skim.h"

static void *alloc(size_t size) {
  void *p = malloc(size);
  if (!p) { fprintf(stderr, "Out of memory\n"); exit(1); }
  return p;
}

Val *make_number(double n) {
  Val *v = alloc(sizeof(Val));
  v->type = VAL_NUMBER;
  v->number = n;
  return v;
}

Val *make_symbol(const char *s) {
  Val *v = alloc(sizeof(Val));
  v->type = VAL_SYMBOL;
  v->symbol = strdup(s);
  return v;
}

Val *make_cons(Val *car, Val *cdr) {
  Val *v = alloc(sizeof(Val));
  v->type = VAL_CONS;
  v->car = car;
  v->cdr = cdr;
  return v;
}

Val *make_nil(void) {
  Val *v = alloc(sizeof(Val));
  v->type = VAL_NIL;
  return v;
}

Val *make_closure(Val *params, Val *body, Val *env) {
  Val *v = alloc(sizeof(Val));
  v->type = VAL_CLOSURE;
  v->params = params;
  v->body = body;
  v->env = env;
  return v;
}

Val *make_primitive(const char *name) {
  Val *v = alloc(sizeof(Val));
  v->type = VAL_PRIMITIVE;
  v->name = strdup(name);
  return v;
}

int is_nil(Val *v)      { return v && v->type == VAL_NIL; }
int is_number(Val *v)  { return v && v->type == VAL_NUMBER; }
int is_symbol(Val *v)  { return v && v->type == VAL_SYMBOL; }
int is_cons(Val *v)     { return v && v->type == VAL_CONS; }
int is_closure(Val *v) { return v && v->type == VAL_CLOSURE; }
int is_primitive(Val *v){ return v && v->type == VAL_PRIMITIVE; }
int is_true(Val *v) { return !(is_symbol(v) && strcmp(v->symbol, "#f") == 0); }

static void print_list(Val *v) {
  printf("(");
  while (is_cons(v)) {
    print_val(v->car);
    v = v->cdr;
    if (is_cons(v)) {
      printf(" ");
    } else if (!is_nil(v)) {
      printf(" . ");
      print_val(v);
      break;
    }
  }
  if (is_nil(v)) printf(")");
  else if (!is_cons(v)) printf(")");
}

void print_val(Val *v) {
  if (!v) { printf("()"); return; }
  
  switch (v->type) {
    case VAL_NUMBER:  printf("%g", v->number); break;
    case VAL_SYMBOL:  printf("%s", v->symbol); break;
    case VAL_NIL:     printf("()"); break;
    case VAL_CONS: print_list(v); break;
    case VAL_CLOSURE:  printf("#<closure>"); break;
    case VAL_PRIMITIVE: printf("#<primitive:%s>", v->name); break;
  }
}

/* Environment is a list of frames, each frame is a list of (name . value) */
Val *make_env(Val *parent) {
  return make_cons(make_nil(), parent);
}

Val *lookup(Val *env, char *name) {
  while (!is_nil(env)) {
    Val *frame = env->car;
    while (is_cons(frame)) {
      if (strcmp(frame->car->car->symbol, name) == 0) {
        return frame->car->cdr;
      }
      frame = frame->cdr;
    }
    env = env->cdr;
  }
  fprintf(stderr, "Error: undefined: %s\n", name);
  exit(1);
}

void define(Val *env, char *name, Val *value) {
  Val *binding = make_cons(make_symbol(name), value);
  env->car = make_cons(binding, env->car);
}

static const char *p;
static int ch;

static void error(const char *msg) {
  fprintf(stderr, "Error: %s\n", msg);
  /* Don't exit in test mode - allows more tests to run */
#ifndef SKIM_TEST
  exit(1);
#endif
}

static void skip_space(void) {
  while (*p && isspace(*p)) p++;
}

static Val *read_expr(void);

static Val *read_list(void) {
  skip_space();
  
  if (*p == ')') { p++; return make_nil(); }
  
  Val *first = read_expr();
  skip_space();
  Val *rest = read_list();
  return make_cons(first, rest);
}

static Val *read_word(void) {
  const char *start = p;
  while (*p && !isspace(*p) && *p != '(' && *p != ')') p++;
  
  char buf[256];
  int len = p - start;
  if (len > 255) len = 255;
  strncpy(buf, start, len);
  buf[len] = '\0';
  
  char *end;
  double n = strtod(buf, &end);
  if (*end == '\0' && buf[0] != '\0') {
    return make_number(n);
  }
  return make_symbol(buf);
}

static Val *read_expr(void) {
  skip_space();
  if (!*p) error("unexpected EOF");
  
  if (*p == '(') { p++; return read_list(); }
  if (*p == '\'') { p++; Val *q = read_expr(); return make_cons(make_symbol("quote"), make_cons(q, make_nil())); }
  
  return read_word();
}

Val *read_val(const char *s) {
  p = s;
  ch = *p;
  return read_expr();
}

static Val *apply_primitive(char *name, Val *args) {
  /* Unary - args is (arg), so we get arg via args->car */
  if (strcmp(name, "car") == 0) {
    if (!is_cons(args->car)) error("car needs pair");
    return args->car->car;
  }
  if (strcmp(name, "cdr") == 0) {
    if (!is_cons(args->car)) error("cdr needs pair");
    return args->car->cdr;
  }
  if (strcmp(name, "null?") == 0) {
    return is_nil(args->car) ? make_symbol("#t") : make_symbol("#f");
  }
  if (strcmp(name, "not") == 0) {
    return is_true(args->car) ? make_symbol("#f") : make_symbol("#t");
  }
  if (strcmp(name, "pair?") == 0) {
    return is_cons(args->car) ? make_symbol("#t") : make_symbol("#f");
  }
  if (strcmp(name, "eq?") == 0) {
    Val *a = args->car;
    Val *b = args->cdr->car;
    int result = (a == b);
    return result ? make_symbol("#t") : make_symbol("#f");
  }

  /* Binary - extract arguments safely */
  if (!is_cons(args) || !is_cons(args->cdr)) error("not enough arguments");

  Val *a = args->car;
  Val *b = args->cdr->car;

  /* Type check for arithmetic */
  if ((strcmp(name, "+") == 0 || strcmp(name, "-") == 0 ||
       strcmp(name, "*") == 0 || strcmp(name, "/") == 0 ||
       strcmp(name, "=") == 0 || strcmp(name, "<") == 0 ||
       strcmp(name, ">") == 0)) {
    if (!is_number(a) || !is_number(b)) error("expected number");
  }

  if (strcmp(name, "+") == 0) {
    double s = 0;
    while (is_cons(args)) {
      s += args->car->number;
      args = args->cdr;
    }
    return make_number(s);
  }
  if (strcmp(name, "-") == 0) {
    double r = a->number;
    args = args->cdr;
    while (is_cons(args)) {
      r -= args->car->number;
      args = args->cdr;
    }
    return make_number(r);
  }
  if (strcmp(name, "*") == 0) {
    double r = 1;
    while (is_cons(args)) {
      r *= args->car->number;
      args = args->cdr;
    }
    return make_number(r);
  }
  if (strcmp(name, "/") == 0) {
    double r = a->number;
    args = args->cdr;
    while (is_cons(args)) {
      r /= args->car->number;
      args = args->cdr;
    }
    return make_number(r);
  }
  if (strcmp(name, "=") == 0) {
    return a->number == b->number ? make_symbol("#t") : make_symbol("#f");
  }
  if (strcmp(name, "<") == 0) {
    return a->number < b->number ? make_symbol("#t") : make_symbol("#f");
  }
  if (strcmp(name, ">") == 0) {
    return a->number > b->number ? make_symbol("#t") : make_symbol("#f");
  }
  if (strcmp(name, "cons") == 0) {
    return make_cons(a, b);
  }
  if (strcmp(name, "list") == 0) {
    return args;  /* args is already a list */
  }

  error("unknown primitive");
  return NULL;
}

static Val *eval_list(Val *list, Val *env);

Val *eval(Val *expr, Val *env) {
  if (is_number(expr)) return expr;
  if (is_nil(expr)) return expr;
  
  if (is_symbol(expr)) {
    return lookup(env, expr->symbol);
  }
  
  /* Function call */
  if (is_cons(expr)) {
    Val *first = expr->car;
    Val *rest = expr->cdr;

    /* Special forms - check BEFORE evaluating operator */
    if (is_symbol(first)) {
      char *name = first->symbol;

      if (strcmp(name, "quote") == 0) return rest->car;

      if (strcmp(name, "if") == 0) {
        Val *cond = eval(rest->car, env);
        return is_true(cond) ? eval(rest->cdr->car, env) : eval(rest->cdr->cdr->car, env);
      }

      if (strcmp(name, "define") == 0) {
        char *var = rest->car->symbol;
        Val *val = eval(rest->cdr->car, env);
        define(env, var, val);
        return make_symbol("ok");
      }

      if (strcmp(name, "lambda") == 0) {
        return make_closure(rest->car, rest->cdr->car, env);
      }
    }

    /* For regular function calls, evaluate operator and arguments */
    Val *func = eval(first, env);
    Val *args = eval_list(rest, env);

    if (is_primitive(func)) return apply_primitive(func->name, args);
    if (is_closure(func)) {
      Val *new_env = make_env(func->env);
      Val *params = func->params;
      while (is_cons(params) && is_cons(args)) {
        define(new_env, params->car->symbol, args->car);
        params = params->cdr;
        args = args->cdr;
      }
      return eval(func->body, new_env);
    }
    error("not a function");
  }

  error("cannot evaluate");
  return NULL;
}

static Val *eval_list(Val *list, Val *env) {
  if (is_nil(list)) return make_nil();
  return make_cons(eval(list->car, env), eval_list(list->cdr, env));
}

void repl(void) {
  /* Create global environment with primitives */
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

  char line[1024];
  while (printf("> "), fflush(stdout), fgets(line, 1024, stdin)) {
    Val *expr = read_val(line);
    Val *result = eval(expr, env);
    print_val(result);
    printf("\n");
  }
}

#ifndef SKIM_TEST
int main(void) {
  repl();
  return 0;
}
#endif
