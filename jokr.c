#include "jokr.h"

Expr *NIL_VALUE;

typedef struct {
  FILE *input;
  Token lookahead;
  bool has_lookahead;
} Lexer;

Expr *alloc_expr(ExprType type) {
  Expr *expr = malloc(sizeof(Expr));
  assert(expr);
  expr->type = type;
  return expr;
}

Expr *make_cons(Expr *car, Expr *cdr) {
  Expr *expr = alloc_expr(EXPR_CONS);
  expr->car = car;
  expr->cdr = cdr;
  return expr;
}

Expr *make_symbol(const char *symbol) {
  Expr *expr = alloc_expr(EXPR_SYMBOL);
  expr->symbol = strdup(symbol);
  return expr;
}

Expr *make_number(double num) {
  Expr *expr = alloc_expr(EXPR_NUMBER);
  expr->number = num;
  return expr;
}

Expr *make_nil(void) {
  return NIL_VALUE;
}

void init(void) {
  NIL_VALUE = alloc_expr(EXPR_NIL);
}

void print_expr(Expr *expr);

void print_list(Expr *expr) {
  putchar('(');

  while (expr->type == EXPR_CONS) {
    print_expr(expr->car);
    expr = expr->cdr;

    if (expr->type == EXPR_CONS)
      putchar(' ');
  }

  if (expr->type != EXPR_NIL) {
    printf(" . ");
    print_expr(expr);
  }

  putchar(')');
}

void print_expr(Expr *expr) {
  switch (expr->type) {
    case EXPR_NIL:
      printf("nil");
      break;

    case EXPR_SYMBOL:
      printf("%s", expr->symbol);
      break;

    case EXPR_NUMBER:
      printf("%g", expr->number);
      break;

    case EXPR_CONS:
      print_list(expr);
      break;
  }
}

void unread(int ch, FILE *f) {
  if (ch != EOF) ungetc(ch, f);
}

void skip_whitespace(Lexer *lex) {
  int ch;
  while ((ch = fgetc(lex->input)) != EOF) {
    if (!isspace(ch)) {
      unread(ch, lex->input);
      break;
    }
  }
}

void read_token_chars(Lexer *lex, char *buf) {
  int ch, i = 0;

  while ((ch = fgetc(lex->input)) != EOF &&
         !isspace(ch) && ch != '(' && ch != ')') {
    buf[i++] = ch;
  }

  buf[i] = '\0';
  unread(ch, lex->input);
}

Token next_token(Lexer *lex) {
  Token tok;
  tok.type = TOK_EOF;

  char buf[256];

  skip_whitespace(lex);

  int ch = fgetc(lex->input);
  if (ch == EOF) return tok;

  if (ch == '(') {
    tok.type = TOK_LPAREN;
  } else if (ch == ')') {
    tok.type = TOK_RPAREN;
  } else {
    unread(ch, lex->input);
    read_token_chars(lex, buf);

    char *end;
    double num = strtod(buf, &end);

    if (*end == '\0' && buf[0] != '\0') {
      tok.type = TOK_NUMBER;
      tok.number = num;
    } else {
      tok.type = TOK_SYMBOL;
      tok.symbol = strdup(buf);
    }
  }

  return tok;
}

Token peek_token(Lexer *lex) {
  if (!lex->has_lookahead) {
    lex->lookahead = next_token(lex);
    lex->has_lookahead = true;
  }
  return lex->lookahead;
}

Token consume_token(Lexer *lex) {
  if (lex->has_lookahead) {
    lex->has_lookahead = false;
    return lex->lookahead;
  }
  return next_token(lex);
}

int main(void) {
  init();

  Expr *n = make_number(42.2839);
  Expr *s = make_symbol("FOO");

  print_expr(n);
  printf("\n");

  print_expr(s);
  printf("\n");

  Expr *list =
    make_cons(make_number(1),
    make_cons(make_number(2),
    make_cons(make_number(3),
    make_cons(make_number(4),
    make_cons(make_number(5),
    make_nil())))));

  Expr *pair = make_cons(make_symbol("a"), make_symbol("b"));

  print_expr(pair);
  printf("\n");

  print_expr(list);
  printf("\n");

  return 0;
}
