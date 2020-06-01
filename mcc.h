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
	TK_NUM,
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

// 入力プログラム
char *user_input;

// -------------------------------------------------------------
// プロトタイプ宣言
bool consume(char *op);
Token *consume_ident(void);
void expect(char *op);
int expect_number(void);
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);
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
} NodeKind;

typedef struct Node Node;

struct Node{
    NodeKind kind;
    Node* lhs;
    Node* rhs;
    int val;
    int offset;
};

Node *code[100];

// -------------------------------------------------------------
// プロトタイプ宣言
Node* new_node(NodeKind kind, Node* lhs, Node* rhs);
Node* new_node_num(int val);
void program();
Node* stmt();
Node* expr();
Node* assign();
Node* equality();
Node* relational();
Node* add();
Node *mul();
Node *primary();
Node *unary();
void gen(Node * node);
void gen_lval(Node *node);

