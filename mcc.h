#include "inc.h"

// """"""""""""""""""""""""""""""""""""""""""""""""
// tokenizer
// """"""""""""""""""""""""""""""""""""""""""""""""

// -------------------------------------------------------------
// 型定義
// トークンの種類
typedef enum {
	TK_RESERVED,
    TK_IDENT,
	TK_NUM,         // 数値
    TK_RETURN,      // return
    TK_IF,
    TK_ELSE,
    TK_WHILE,
	TK_EOF,
} TokenKind;

typedef struct Token Token;

// トークンの構造体定義
struct Token {
	TokenKind kind;		// トークンの型
	Token *next;		// 次の入力トークン
	int val;			// kind がTK_NUMのときの数値
	char *str;			// トークンの文字列
    int len;            // トークンの長さ
};

// 現在分析中のトークン
Token *token;


// ローカル変数の型
typedef struct LVar LVar;
struct LVar{
    LVar *next;
    char *name;
    int len;
    int offset;
};

// 入力プログラム
char *user_input;

// -------------------------------------------------------------
// プロトタイプ宣言
bool consume(char *op);
Token *consume_ident(void);
bool consume_type(TokenKind kind);

void expect(char *op);
int expect_number(void);
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);
void debug_print(char *fmt, ...);
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char*str, int len);
Token *tokenize();

// """"""""""""""""""""""""""""""""""""""""""""""""
// parcer
// """"""""""""""""""""""""""""""""""""""""""""""""

// -------------------------------------------------------------
// 型定義
typedef enum{
    ND_ADD,     // +
    ND_SUB,     // -
    ND_MUL,     // *
    ND_DIV,     // /
    ND_EQA,     // ==
    ND_NEQ,     // !=
    ND_GTH,     // <
    ND_GTE,     // <=
    ND_NUM,     // num
    ND_ASSIGN,  // =
    ND_LVAR,    // ローカル変数
    ND_RETURN,  // return
    ND_IF,      // if
    ND_WHILE,   // while
} NodeKind;

typedef struct Node Node;

struct Node{
    NodeKind kind;
    Node* next;

    Node* lhs;
    Node* rhs;
    
    Node* cond; // 条件
    Node* then; // 実行stmtたち
    Node* els;  // else Node -> ifがつながる。
    Node* body; // whileの中の実行部分

    LVar* var;
    int val;
    int offset;
};

Node *code[100];

typedef struct Function Function;
struct Function{
    Node *node;
    LVar *locals;
    int stack_size;
};

// -------------------------------------------------------------
// プロトタイプ宣言
Node* new_node(NodeKind kind, Node* lhs, Node* rhs);
Node* new_node_num(int val);
Function *program(void);
void gen(Node* node);
void gen_lval(Node *node);
void codegen(Function *prog);