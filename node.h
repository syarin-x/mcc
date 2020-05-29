#include    "token.h"
#include    "inc.h"

// -------------------------------------------------------------
// 型定義
typedef enum{
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_EQA, // ==
    ND_NEQ, // !=
    ND_GTH, // <
    ND_GTE, // <=
    ND_NUM, // num
} NodeKind;

typedef struct Node Node;

struct Node{
    NodeKind kind;
    Node* lhs;
    Node* rhs;
    int val;
};

// -------------------------------------------------------------
// プロトタイプ宣言
Node* new_node(NodeKind kind, Node* lhs, Node* rhs);
Node* new_node_num(int val);
Node* expr();
Node* equality();
Node* relational();
Node* add();
Node *mul();
Node *primary();
Node *unary();
void gen(Node * node);


// -------------------------------------------------------------
// 関数定義

// nodeの作成
Node* new_node(NodeKind kind, Node* lhs, Node* rhs){
    Node* node = (Node*)calloc(1,sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// node(数値)の作成
Node* new_node_num(int val)
{
    Node* node = (Node*)calloc(1,sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

Node* expr()
{
    return equality();
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

void gen(Node * node)
{
    if(node->kind == ND_NUM)
    {
        printf("  push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch(node->kind)
    {
        case ND_ADD:
            printf("  add rax, rdi\n");
            break;
        case ND_SUB:
            printf("  sub rax, rdi\n");
            break;
        case ND_MUL:
            printf("  imul rax, rdi\n");
            break;
        case ND_DIV:
            // idiv命令は、raxレジスタとrdxレジスタを足し合わせたものを、idiv命令の引数で割って、
            // 商をraxレジスタに、余りをrdxレジスタにセットする仕様。
            // cqo命令は、raxレジスタの64bit値を128bitに引き伸ばしてraxとrdxに入れる。
            // o + raxをrdiで割る、といった実装になっている。
            printf("  cqo\n");
            printf("  idiv rdi\n");
            break;
        case ND_EQA:
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzb rax,al\n");
            break;
        case ND_NEQ:
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzb rax,al\n");
            break;
        case ND_GTH:
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzb rax,al\n");
            break;
        case ND_GTE:
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzb rax,al\n");
            break;
        default:
            break;
    }
    
    printf("  push rax\n");
}