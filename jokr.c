#include "jokr.h"

FILE *source;
Atom tokens[256];
int token_count = 0;
Expr *NIL_VALUE;

typedef enum {
  OK,
  ERROR
} Error;

Expr *alloc_expr(Token_Type type) {
  Expr *expr = malloc(sizeof(Expr));
  assert(expr);
  expr->type = type;
  return expr;
}

Expr *make_pair(Expr *car, Expr *cdr) {
  Expr *expr = alloc_expr(CONS);
  CAR(expr) = car;
  CDR(expr) = cdr;
  return expr;
}

Expr *make_symbol(const char *symbol) {
  Expr *expr = alloc_expr(SYMBOL);
  expr->symbol = strdup(symbol);
  return expr;
}

Expr *make_num(double num) {
  Expr *expr = alloc_expr(NUM);
  expr->num = num;
  return expr;
}

Expr *make_nil() {
  return NIL_VALUE;
}

void init() {
  NIL_VALUE = alloc_expr(NIL);
}

char *token_to_string(Atom token) {
  switch(token.type) {
    case NIL:
      return "NIL";
    case LPAREN: 
      return "LPAREN";
    case RPAREN:
      return "RPAREN";
    case SYMBOL:
      return "SYMBOL";
    case NUM:
      return "NUM";
    case CONS:
      return "CONS";
    case STRING:
      return "STRING";
    default:
      return "Invalid token";
  }
}


void print_list(Expr *expr) {
  putchar('(');
  print_expr(CAR(expr));
  expr = CDR(expr);
  while (!is_nil(expr)) {
    if (is_pair(expr)) {
      putchar(' ');
      print_expr(CAR(expr));
      expr = CDR(expr);
    }
    else {
      printf(" . ");
      print_expr(expr);
      break;
    }
  }
  putchar(')');
}

void print_expr(Expr *expr) {
  switch (expr->type) {
    case NIL:
      printf("nil");
      break;
    case SYMBOL:
      printf("%s", expr->symbol);
      break;
    case NUM:
      printf("%g", expr->num);
      break;
    case CONS:
      print_list(expr);
      break;
    default:
      break;
  }
}

void go_back(int ch, FILE *source) {
  if (ch != EOF) ungetc(ch, source); 
}

int peek_token(FILE *f) {
  int ch = fgetc(f);
  go_back(ch, source);
  return ch;
}

void skip_whitespaces() {
  int ch = fgetc(source);
  while(isspace(ch)) {
    ch = fgetc(source);
  }
  go_back(ch, source);
}

void print_tokens(Atom tokens[256]) {
  for (int i = 0; i < token_count; i++) {
    const Atom token = tokens[i];
    printf("%s ",token_to_string(token));
    if(token.type == NUM) {
      printf("%g", token.num);
    }
    else if(token.type == LPAREN) {
      putchar('(');
    }
    else if(token.type == RPAREN) {
      putchar(')');
    }
    else {
      printf("%s", token.symbol);
    }
    printf("\n");
  }
}

void consume_subsequent_chars(char buffer[256], int *ch, int *i) {
  while(*ch != EOF && !isspace(*ch) && !is_paren(*ch)) {
      buffer[(*i)++] = *ch;
      *ch = fgetc(source);
  }
  buffer[*i] = '\0';
  go_back(*ch, source);
}

bool is_valid_num(char buffer[256], char *end) {
  return *end == '\0' && buffer[0] != '\0';
}

Atom *lex() {
  while(1) {
    char buffer[256];
    int i = 0;
    skip_whitespaces();
    int ch = fgetc(source);
    if(ch == EOF) break;

    if(ch == '(') {
      tokens[token_count++].type = LPAREN;
    }
    else if(ch == ')') {
      tokens[token_count++].type = RPAREN;
    }
    else if(ch == '\'') {
      consume_subsequent_chars(buffer, &ch, &i);
      tokens[token_count].type = STRING;
      tokens[token_count++].symbol = strdup(buffer);
    }
    else {
      consume_subsequent_chars(buffer, &ch, &i);
      char *end;
      double d = strtod(buffer, &end);
      if(is_valid_num(buffer, end)) { 
        tokens[token_count].type = NUM;
        tokens[token_count].num = d;
      }
      else {
        tokens[token_count].type = SYMBOL;
        tokens[token_count].symbol = strdup(buffer);
      }
      token_count++;
    }
  }
  printf("reached\n");
  print_tokens(tokens);
  return tokens;
}

char *read(char* input) { return input; }
char *eval(char* input) { return input; }
char *print(char* input) { return input; }
char *repl(char* input) {
  return print(input);
}

int main() {
  init();
  source = fopen("./test.scm", "r");
  // source = stdin;

  Expr *n = make_num(42.2839);
  Expr *s = make_symbol("FOO");

  print_expr(n);
  printf("\n");

  print_expr(s);
  printf("\n");

  Expr *list_nums = make_pair(
    make_num(1.000),
      make_pair(
        make_num(2.23),
          make_pair(
            make_num(3.5),
              make_pair(
                make_num(4),
                  make_pair(
                    make_num(5.283849),
                      make_nil())))));

  Expr *pair = make_pair(make_symbol("a"), make_symbol("b"));

  print_expr(pair);
  printf("\n");

  print_expr(list_nums);
  printf("\n");

  lex();
  fclose(source);
  return 0;
}
