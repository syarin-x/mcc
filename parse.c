#include    "mcc.h"

LVar *locals;

// ---------------------------------------------------------------------------
// local function definition
// ---------------------------------------------------------------------------
static Node *stmt(void);
static Node *expr(void);
static Node *assign(void);
static Node *equality(void);
static Node *relational(void);
static Node *add(void);
static Node *mul(void);
static Node *unary(void);
static Node *primary(void);

// ----------------
// 変数の検索
// ----------------
static LVar *find_lvar(Token *tok) {
    for(LVar *var = locals; var; var = var->next)
        if(strlen(var->name) == tok->len && !strncmp(tok->str, var->name, tok->len))
            return var;
    
    return NULL;
}

// ------------
// nodeの作成
// ------------
Node* new_node(NodeKind kind, Node* lhs, Node* rhs){
    Node* node = (Node*)calloc(1,sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// ------------------
// node(数値)の作成
// ------------------
Node* new_node_num(int val)
{
    Node* node = (Node*)calloc(1,sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// ----------------------
// create node(variant)
// ----------------------
LVar* new_lvar(char* name)
{
    LVar *var = calloc(1,sizeof(LVar));
    var->next = locals;
    var->name = name;
    locals = var;
    return var;
}

// -----------------------
// analyze program
// -----------------------
Function *program()
{
    locals = NULL;

    Node head = {};
    Node *cur = &head;

    while(!at_eof())
    {
        cur->next = stmt();
        cur = cur->next;
    }
    
    Function *prog = calloc(1,sizeof(Function));
    prog->node = head.next;
    prog->locals = locals;
    return prog;
}

Node* stmt()
{
    Node* node;
    if(consume_type(TK_RETURN)) {
        node = calloc(1,sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
        expect(";");
        return node;
    } else if(consume_type(TK_IF)) {
        node = calloc(1,sizeof(Node));
        node->kind = ND_IF;
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        if(consume_type(TK_ELSE)) {
            node->els = stmt();
        }
        return node;
    } else if(consume_type(TK_WHILE)) {
        node = calloc(1,sizeof(Node));
        node->kind = ND_WHILE;
        expect("(");
        node->cond = expr();
        expect(")");
        node->body = stmt();
        return node;
    } else if(consume_type(TK_FOR)) {
        node = calloc(1,sizeof(Node));
        node->kind = ND_FOR;
        expect("(");
        if(!consume(";")) {
            node->init = expr();
            expect(";");
        }
        if(!consume(";")) {
            node->cond = expr();
            expect(";");
        }
        if(!consume(")")) {
            node->upload = expr();
            expect(")");
        }
        node->body = stmt();
        return node;
    } else if(consume("{")) {
        // block statement
        node = calloc(1,sizeof(Node));
        node->kind = ND_BLOCK;
        Node* node_list = node;
        while(!consume("}"))
        {
            node_list->next = stmt();
            node_list = node_list->next;
        }
        return node;
    } else {
        node = expr();
    }
        
    expect(";");
    return node;
}

Node* expr()
{
    return assign();
}

Node* assign()
{
    Node* node = equality();

    if(consume("="))
        node = new_node(ND_ASSIGN, node, assign());
    return node;
}

Node* equality()
{
    Node* node = relational();

    for(;;)
    {
        if(consume("=="))
            node = new_node(ND_EQA, node, relational());
        else if(consume("!="))
            node = new_node(ND_NEQ, node, relational());
        else
            return node;
    }
}

// 大小関係
Node* relational()
{
    Node* node = add();

    for(;;)
    {
        if(consume("<"))
            node = new_node(ND_GTH, node, add());
        else if(consume("<="))
            node = new_node(ND_GTE, node, add());

        // >,>=は、抽象構文木の左辺と右辺を入れ替えて、<,<=に読み替えることで対応する
        else if(consume(">"))
            node = new_node(ND_GTH, add(), node);
        else if(consume(">="))
            node = new_node(ND_GTE, add(), node);
        else
            return node;
    }
}

// 足し引きの式
Node* add()
{
    // 左側を調べる
    Node *node=mul();

    // 右側を調べる（ので、かならず記号は + か - のはず。
    for(;;) // 無限ループすることで、続く足し算は全部くっつける
    {
        if(consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if(consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

// 掛け算系
Node *mul()
{
    Node* node = unary();

    for(;;) // 無限ループすることで、続く掛け算はすべてくっつける
    {
        if(consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if(consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

// 数値？
Node *primary()
{
    if(consume("("))
    {
        // カッコが始まったら、続く文字を一つの数式として解析する。
        Node* node = expr();

        // expr()から戻ってきたら、四則演算以外の記号（+ - * / 0-9）になっている。
        expect(")");
        return node;
    }

    // 次のトークンが変数なら、トークンアドレスを受け取る
    Token *tok = consume_ident();
    if(tok)
    {
        if(consume("(")) {  // function
            expect(")");

            Node* node = calloc(1,sizeof(Node));
            node->kind = ND_FUNC;
            node->funcname = strndup(tok->str, tok->len);

            return node;
        }
        else {  // variant
            // find lvar from local variable list
            LVar *lvar = find_lvar(tok);
            if(!lvar) {
                // missing lvar
                lvar = new_lvar(strndup(tok->str, tok->len));
            }

            Node* node = calloc(1,sizeof(Node));
            node->kind = ND_LVAR;
            node->var = lvar;

            return node;
        }
    }

    return new_node_num(expect_number());
}

// 単項プラスと単項マイナス
Node *unary()
{
    if(consume("+"))
        return primary();
    else if(consume("-"))
        return new_node(ND_SUB, new_node_num(0), primary());
    else
        return primary();    
}