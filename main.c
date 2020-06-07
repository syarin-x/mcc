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
    Function *prog = program();

    // analyze lvar with prog
    int offset = 0;
    for(LVar *var = prog->locals; var; var = var->next) {
        offset += 8;
        var->offset = offset;
    }
    prog->stack_size = offset;

    codegen(prog);

    return 0;
}