#include    "mcc.h"

// --------------------------------
// read token 
// --------------------------------
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

// --------------------------------
// read token if it is identifer
// --------------------------------
Token* consume_ident()
{
    if(token->kind != TK_IDENT)
        return NULL;
    Token* ret = token;
    token = token->next;
    return ret;
}

// --------------------------------------
// read token if it is match TOKEN_TYPE
// --------------------------------------
bool consume_type(TokenKind kind) {
    if(token->kind != kind)
        return false;
    token = token->next;
    return true;
}

// --------------------------------
// read token if it is expect
// --------------------------------
void expect(char *op)
{
	if (token->kind != TK_RESERVED || token->len != strlen(op) || memcmp(token->str, op, token->len))
		error_at(token->str,"'%c'ではありません",op);
	token = token->next;
}

// -------------------------------------------------
// トークンが数値か？見て、数値ならトークンを返す。
// -------------------------------------------------
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

// -----------------------
// エラー箇所を報告する
// -----------------------
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

// ------------------
//　エラー報告関数
// ------------------
void error(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// ------------------
//　デバッグ情報出力関数
// ------------------
void debug_print(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
}

// ------------------------
// ファイルの終端か判定する
// ------------------------
bool at_eof()
{
	return token->kind == TK_EOF;
}

// --------------
// 新トークン生成
// --------------
Token *new_token(TokenKind kind, Token *cur, char*str, int len)
{
	Token *tok = (Token*)calloc(1,sizeof(Token));
	tok->kind = kind;
	tok->str = str;
    tok->len = len;
	cur->next = tok;
	return tok;
}

static bool is_alpha(char c)
{
    return ('a' <= c && c <= 'z') || c == '_' || ('A' <= c && c <= 'Z');
}

static bool is_alnum(char c)
{
    return is_alpha(c) || ('0' <= c && c <= '9');
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

        // 次がreturnかどうか
        if(strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            cur = new_token(TK_RETURN, cur, p, 6);
            p += 6;
            continue;
        }

        // 次がifかどうか
        if(strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
            cur = new_token(TK_IF, cur, p, 2);
            p += 2;
            continue;
        }

        // 次がwhileかどうか
        if(strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
            cur = new_token(TK_WHILE, cur, p, 5);
            p += 5;
            continue;
        }

        // 次がelseかどうか
        if(strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
            cur = new_token(TK_ELSE, cur, p, 4);
            p+= 4;
            continue;
        }

        // 次がforかどうか
        if(strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
            cur = new_token(TK_FOR, cur, p, 3);
            p+= 3;
            continue;
        }

        // 次の文字が変数の場合
        if(is_alpha(*p))
        {
            char *c = p++;
            while(is_alnum(*p))
                p++;

            cur = new_token(TK_IDENT, cur, c, p - c);
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
		if (ispunct(*p)) {
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
