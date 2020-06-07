#include "mcc.h"

int if_end;
int if_else;

// ----------------------
// code generator main
// ----------------------
void codegen(Function* prog){
    
    // ifラベルの通し番号初期化
    if_end = 1;
    if_else = 1;

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // prologue
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", prog->stack_size);

    for(Node *node = prog->node; node; node = node->next)
        gen(node);

    // Epilogue
    printf(".L.return:\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}

static void gen_addr(Node *node)
{
    if(node->kind == ND_LVAR)
    {        
        printf("  lea rax,[rbp-%d]\n", node->var->offset);
        printf("  push rax\n");
        return;
    }

    error("not an lvalue");
}

static void load(void) {
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
}

static void store(void) {
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
}

void gen(Node* node)
{
    switch(node->kind)
    {
        case ND_NUM:
            printf("  push %d\n", node->val);
            return;
        case ND_LVAR:
            gen_addr(node);
            load();
            return;
        case ND_ASSIGN:
            gen_addr(node->lhs);
            gen(node->rhs);
            store();
            return;
        case ND_RETURN:
            gen(node->lhs);
            printf("  pop rax\n");
            printf("  jmp .L.return\n");
            return;
        case ND_IF: {
            int end_no = if_end++;
            int els_no = if_else++;
            if(node->els) {
                gen(node->cond);
                printf("  pop rax\n");
                printf("  cmp rax,0\n");
                printf("  je  .L.else.%d\n",els_no);
                gen(node->then);
                printf("  jmp .L.end.%d\n",end_no);
                printf(".L.else.%d:\n", els_no);
                gen(node->els);
                printf(".L.end.%d:\n",end_no);
            } else {
                gen(node->cond);
                printf("  pop rax\n");
                printf("  cmp rax,0\n");
                printf("  je  .L.end.%d\n",end_no);
                gen(node->then);
                printf(".L.end.%d:\n",end_no);                
            }
            return;
        }
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

void gen_lval(Node* node)
{
    if(node->kind != ND_LVAR)
        error("代入の左辺値が変数ではありません");
    
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}