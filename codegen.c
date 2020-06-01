#include    "mcc.h"

// -------------------------------------------------------------
// 関数定義

// トークンを読みすすめる
bool consume(char *op)
{
	// 着目しているトークンが、期待している記号かどうか？
	// 例えば、"+"を引数に渡して、次は足し算か？を判定するなどに使う
	// （そして、足し算の処理をする）

	// 記号の処理をしたいので、記号じゃなかったり、期待する記号じゃなかったら何もしない。
	if(token->kind != TK_RESERVED || token->len != strlen(op) || memcmp(token->str, op, token->len))
		return false;
	token = token->next;
	return true;
}

// トークンが変数なら読みすすめる
Token* consume_ident()
{
    if(token->kind != TK_IDENT)
        return NULL;
    Token* ret = token;
    token = token->next;
    return ret;
}

// トークンを読みすすめる
void expect(char *op)
{
	if (token->kind != TK_RESERVED || token->len != strlen(op) || memcmp(token->str, op, token->len))
		error_at(token->str,"'%c'ではありません",op);
	token = token->next;
}

// トークンが数値か？見て、数値ならトークンを返す。
int expect_number()
{
	if(token->kind != TK_NUM)
	{
    	error_at(token->str, "数値ではありません");
    }
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
Token *new_token(TokenKind kind, Token *cur, char*str, int len)
{
	Token *tok = (Token*)calloc(1,sizeof(Token));
	tok->kind = kind;
	tok->str = str;
    tok->len = len;
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

        // 次の文字が変数の場合
        if('a' <= *p && *p <= 'z')
        {
            cur = new_token(TK_IDENT, cur, p++, 1);
            cur->len = 1;
            continue;
        }

        // 次の文字が１文字の記号の場合
        if(strncmp(p, ">=", 2) == 0 || strncmp(p, "<=", 2) == 0 || strncmp(p, "==", 2) == 0 || strncmp(p, "!=", 2) == 0 )
        {
            cur = new_token(TK_RESERVED, cur, p++, 2);
            p++;    // ２文字からなる記号なので、ポインタは２つすすめる。
            continue;
        }

		// 次の文字が１文字の記号の場合
		if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '<' || *p == '>' || *p == ';' || *p == '=')
		{
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}

		// 次の文字が数値の場合    
		if (isdigit(*p))
		{
    
			cur = new_token(TK_NUM, cur, p, 0);
			cur->val = strtol(p, &p, 10);
			continue;
		}

		error_at(p, "トークナイズできません");
	}

	new_token(TK_EOF, cur, p, 1);
	return head.next;
}
