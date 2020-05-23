#include    "inc.h"

// -------------------------------------------------------------
// 型定義
// トークンの種類
typedef enum {
	TK_RESERVED,
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
};

// 現在分析中のトークン
Token *token;

// 入力プログラム
char *user_input;

// -------------------------------------------------------------
// プロトタイプ宣言
bool consume(char op);
void expect(char op);
int expect_number(void);
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char*str);
Token *tokenize();



// -------------------------------------------------------------
// 関数定義

// トークンを読みすすめる
bool consume(char op)
{
	// 着目しているトークンが、期待している記号かどうか？
	// 例えば、"+"を引数に渡して、次は足し算か？を判定するなどに使う
	// （そして、足し算の処理をする）

	// 記号の処理をしたいので、記号じゃなかったり、期待する記号じゃなかったら何もしない。
	if(token->kind != TK_RESERVED || token->str[0] != op)
		return false;
	token = token->next;
	return true;
}

// トークンを読みすすめる
void expect(char op)
{
	if (token->kind != TK_RESERVED || token->str[0] != op)
		error_at(token->str,"'%c'ではありません",op);
	token = token->next;
}

// トークンが数値か？見て、数値ならトークンを返す。
int expect_number()
{
	if(token->kind != TK_NUM)
		error_at(token->str, "数値ではありません");
	int val = token->val;
	token = token->next;
	return val;
}

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...)
{
	va_list ap;
	va_start(ap,fmt);

	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, "");
	fprintf(stderr, "^");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

 //　エラー報告関数
void error(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// ファイルの終端か判定する
bool at_eof()
{
	return token->kind == TK_EOF;
}


// 新トークン生成
Token *new_token(TokenKind kind, Token *cur, char*str)
{
	Token *tok = (Token*)calloc(1,sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	cur->next = tok;
	return tok;
}

// トークナイズ関数
Token *tokenize()
{
	char *p = user_input;
	Token head;
	head.next = NULL;
	Token *cur = &head;

	while(*p) {

		// 次の文字がスペースの場合
		if(isspace(*p))
		{
			p++;
			continue;
		}

		// 次の文字が記号の場合
		if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')')
		{
			cur = new_token(TK_RESERVED, cur, p++);
			continue;
		}

		// 次の文字が数値の場合    
		if (isdigit(*p))
		{
    
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue;
		}

		error_at(p, "トークナイズできません");
	}

	new_token(TK_EOF, cur, p);
	return head.next;
}