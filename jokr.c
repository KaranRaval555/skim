#include "jokr.h"
#include <stdio.h>

Expr *NIL_VALUE;

typedef struct {
  FILE *input;
  Token lookahead;
  bool has_lookahead;
} Lexer;


Expr *parse_expr(Lexer *lex);

void parse_error(const char *msg) {
  fprintf(stderr, "Parse error: %s\n", msg);
  exit(1);
}

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

Expr *parse_list(Lexer *lex) {
  Token t = peek_token(lex);

  if(t.type == TOK_EOF) {
      parse_error("Unexpected EOF");
  }
  if(t.type == TOK_RPAREN) {
    consume_token(lex);
    return make_nil();
  }

  Expr *first = parse_expr(lex);
  Expr *rest = parse_list(lex);

  return make_cons(first, rest);
}

Expr *parse_expr(Lexer *lex) {
  Token t = consume_token(lex);
  switch(t.type) {
    case TOK_NUMBER:
      return make_number(t.number);
    case TOK_SYMBOL:
      return make_symbol(t.symbol);
    case TOK_LPAREN:
      return parse_list(lex);
    case TOK_RPAREN:
      parse_error("unexpected ')'");
      break;
    case TOK_EOF:
      parse_error("unexpected EOF");
      break;
    default:
      parse_error("invalid token");  }
      return NULL;
}

Expr *eval(Expr *expr) {
  switch(expr->type) {
    case EXPR_NIL:
    case EXPR_NUMBER:
      return expr;
    default:
      parse_error("cannot evaluate symbol and list yet");
      return NULL;
  }
}

void repl() {
  char line[1024];

  while (1) {
    printf("> ");
    fflush(stdout);

    if (!fgets(line, sizeof(line), stdin)) {
      break;
    }

    FILE *f = fmemopen(line, strlen(line), "r");

    Lexer lex = {
      .input = f,
      .has_lookahead = false
    };

    Expr *expr = parse_expr(&lex);

    Expr *result = eval(expr);

    print_expr(result);
    printf("\n");

    fclose(f);
  }
}

int main(void) {
  init();

  repl();
  return 0;
}
