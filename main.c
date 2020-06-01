#include    "mcc.h"

extern char* user_input;
extern Node *code[100];

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        fprintf(stderr,"引数の個数が正しくありません\n");
        return 1;
    }

	// トークナイズ
    user_input = argv[1];
	token = tokenize();
    program();


    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // プロローグ
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");

    // 先頭の式から順番にコード生成
    for(int i = 0; code[i]; i++)
    {
        gen(code[i]);
        printf("  pop rax\n");
    }

    // エピローグ
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return 0;
}