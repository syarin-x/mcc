#include "mcc.h"

int L_end;
int L_else;
int L_begin;
int L_sequence;

static char *argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

// ----------------------
// code generator main
// ----------------------
void codegen(Function* prog){
    
    // ifラベルの通し番号初期化
    L_end = 1;
    L_else = 1;
    L_begin = 1;
    L_sequence = 1;

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
            int end_no = L_end++;
            int els_no = L_else++;
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
        case ND_WHILE: {
            int begin_no = L_begin++;
            int end_no = L_end++;
            printf(".L.begin.%d:\n",begin_no);
            gen(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax,0\n");
            printf("  je .L.end.%d\n",end_no);
            gen(node->body);
            printf("  jmp .L.begin.%d\n", begin_no);
            printf(".L.end.%d:\n",end_no);
            return;
        }
        case ND_FOR: {
            int begin_no = L_begin++;
            int end_no = L_end++;
            if(node->init)
                gen(node->init);
            printf(".L.begin.%d:\n",begin_no);
            if(node->cond) {
                gen(node->cond);
                printf("  pop rax\n");
                printf("  cmp rax,0\n");
                printf("  je  .L.end.%d\n",end_no);
            }
            gen(node->body);
            gen(node->upload);
            printf("  jmp .L.begin.%d\n",begin_no);
            printf(".L.end.%d:\n",end_no);
            return;
        }
        case ND_BLOCK:
            while(node->next)
            {
                gen(node->next);
                node = node->next;
            }
            return;
        case ND_FUNC: {
            int args = 0;
            for(Node* cur = node->args; cur; cur = cur->next) {
                // 一つずつコンパイルしてスタックに積んでいく
                gen(cur);
                args++;
            }

            for(int i = args - 1; i >= 0 ; --i) {
                printf("  pop %s\n", argreg[i]);
            }

            // スタックポインタを16byte区切りにする
            int seq = L_sequence++;
            printf("  mov rax, rsp\n");             // スタックポインタをraxレジスタにコピー
            printf("  and rax, 15\n");              // andを取って、チェックする。16byteなら、ゼロになっている（ゼロフラグON）
            printf("  jnz .L.call.%d\n", seq);      // ゼロでないときは、ジャンプしてアライメントする
            printf("  mov rax, 0\n");               // 関数コールの前にraxをクリア
            printf("  call %s\n", node->funcname);
            printf("  jmp .L.end.%d\n", seq);       // コールが終わったので、終了まで飛ぶ
            printf(".L.call.%d:\n", seq);
            printf("  sub rsp, 8\n");               // 今は8バイト単位の調整だけでいい。
            printf("  mov rax, 0\n");
            printf("  call %s\n", node->funcname);
            printf("  add rsp, 8\n");               // スタックポインタをもとに戻しておく。
            printf(".L.end.%d:\n", seq);
            printf("  push rax\n");                 // raxに返り値があるのでスタックに積む
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