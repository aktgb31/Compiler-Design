/*
    Register related functions
*/
#define REGISTER_COUNT 20
int registerStates[REGISTER_COUNT];

int getReg()
{
    for (int i = 0; i < REGISTER_COUNT; i++)
    {
        if (registerStates[i] == 0)
        {
            registerStates[i] = 1;
            return i;
        }
    }
    printf("Out of Registors");
    exit(1);
}

void freeReg()
{
    for (int i = REGISTER_COUNT - 1; i >= 0; i--)
    {
        if (registerStates[i] == 1)
        {
            registerStates[i] = 0;
            return;
        }
    }
}

void initializeCodeGenerator()
{
    for (int i = 0; i < REGISTER_COUNT; i++)
        registerStates[i] = 0;

    fprintf(targetFile, "%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n", 0, 2056, 0, 0, 0, 0, 0, 0);
    fprintf(targetFile, "BRKP\n");
}

/*
    Parse tree maker
*/

struct tnode *makeLeafNode(int n)
{
    struct tnode *temp;
    temp = (struct tnode *)malloc(sizeof(struct tnode));
    temp->op = NULL;
    temp->val = n;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

struct tnode *makeOperatorNode(char c, struct tnode *l, struct tnode *r)
{
    struct tnode *temp;
    temp = (struct tnode *)malloc(sizeof(struct tnode));
    temp->op = malloc(sizeof(char));
    *(temp->op) = c;
    temp->left = l;
    temp->right = r;
    return temp;
}

int evaluate(struct tnode *t, FILE *targetFile)
{
    if (t->op == NULL)
    {
        int i = getReg();
        fprintf(targetFile, "MOV R%d, %d\n", i, t->val);
        return i;
    }
    else
    {
        switch (*(t->op))
        {
        case '+':
        {
            int i = evaluate(t->left, targetFile);
            int j = evaluate(t->right, targetFile);
            fprintf(targetFile, "ADD R%d, R%d\n", i, j);
            freeReg();
            return i;
            break;
        }
        case '-':
        {
            int i = evaluate(t->left, targetFile);
            int j = evaluate(t->right, targetFile);
            fprintf(targetFile, "SUB R%d, R%d\n", i, j);
            freeReg();
            return i;
            break;
        }
        case '*':
        {
            int i = evaluate(t->left, targetFile);
            int j = evaluate(t->right, targetFile);
            fprintf(targetFile, "MUL R%d, R%d\n", i, j);
            freeReg();
            return i;
            break;
        }
        case '/':
        {
            int i = evaluate(t->left, targetFile);
            int j = evaluate(t->right, targetFile);
            fprintf(targetFile, "DIV R%d, R%d\n", i, j);
            freeReg();
            return i;
            break;
        }
        }
    }
}