#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

 //　エラー報告関数
void error(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

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
		error("'%c'ではありません",op);
	token = token->next;
}

// トークンが数値か？見て、数値ならトークンを返す。
int expect_number()
{
	if(token->kind != TK_NUM)
		error("数値ではありません");
	int val = token->val;
	token = token->next;
	return val;
}

// ファイルの終端か判定する
bool at_eof()
{
	return token->kind == TK_EOF;
}

// 新トークン生成
Token *new_token(TokenKind kind, Token *cur, char*str)
{
	Token *tok = calloc(1,sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	cur->next = tok;
	return tok;
}

// トークナイズ関数
Token *tokenize(char *p)
{
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
		if (*p == '+' || *p == '-')
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

		error("トークナイズできません");
	}

	new_token(TK_EOF, cur, p);
	return head.next;
}

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        fprintf(stderr,"引数の個数が正しくありません\n");
        return 1;
    }

	// トークナイズする
	token = tokenize(argv[1]);

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");


	// 一番最初のトークンは数値でないといけない
	// これ以降は、カレントのトークンは記号じゃないといけない
	printf("  mov rax, %d\n", expect_number());

	// トークン終端まで分析していく
	while(!at_eof())
	{
		// '+'記号なら、足し合わせのアセンブリを出力
		if(consume('+'))
		{
			// 次の数値がトークンかはチェックすること
			printf("  add rax, %d\n", expect_number());
			continue;
		}

		// 次のトークンがマイナスじゃなかったらエラーにする
		// このタイミングでは、次のトークンは数値でも、プラス記号でもない
		expect('-');
		printf("  sub rax, %d\n", expect_number());
	}

    printf("  ret\n");
    return 0;
}