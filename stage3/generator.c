/*
    Register related functions
*/
#define XMAGIC 0
#define ENTRY_POINT 2056
#define TEXT_SIZE 0
#define DATA_SIZE 0
#define HEAP_SIZE 0
#define STACK_SIZE 0
#define LIBRARY_FLAG 0
#define UNUSED 0

int IP = 0;             // Instruction Pointer
int lastRegistor = -1;  // Last used Registor
int lastUsedLabel = -1; // Last Used label
int labelAddress[1020]; // address of labels

int getReg()
{
    if (lastRegistor == 19)
    {
        printf("Out of Registors");
        exit(1);
    }
    else
        return ++lastRegistor;
}

void freeReg()
{
    --lastRegistor;
}

void initGenerator(FILE *targetFile)
{
    // XMAGIC, Entry Point, Text Size, Data Size, Heap Size, Stack Size, Library Flag, Unused
    fprintf(targetFile, "%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n", XMAGIC, ENTRY_POINT, TEXT_SIZE, DATA_SIZE, HEAP_SIZE, STACK_SIZE, LIBRARY_FLAG, UNUSED);
    fprintf(targetFile, "MOV SP, %d\n", 4095 + 26);
    ++IP;
    for (int i = 1; i <= 26; i++)
    {
        fprintf(targetFile, "MOV [%d], 0\n", 4095 + i);
        ++IP;
    }
}

void pushUsedRegisters(FILE *targetFile)
{
    for (int i = 0; i <= lastRegistor; i++)
    {
        fprintf(targetFile, "PUSH R%d\n", i);
        ++IP;
    }
}

void popUsedRegisters(FILE *targetFile)
{
    for (int i = lastRegistor; i >= 0; i--)
    {
        fprintf(targetFile, "POP R%d\n", i);
        ++IP;
    }
}

void read(int address, FILE *targetFile)
{
    pushUsedRegisters(targetFile);

    fprintf(targetFile, "MOV R0, %d\n", address);
    fprintf(targetFile, "MOV R2, 7\n");
    fprintf(targetFile, "PUSH R2\n"); // Pushed System Call number
    fprintf(targetFile, "MOV R2, -1\n");
    fprintf(targetFile, "PUSH R2\n"); // Pushed argument 1
    fprintf(targetFile, "PUSH R0\n"); // Pushed argument 2
    fprintf(targetFile, "PUSH R0\n"); // Pushed argument 3
    fprintf(targetFile, "PUSH R0\n"); // Pushed return value
    fprintf(targetFile, "INT 6\n");   // System Call
    fprintf(targetFile, "POP R0\n");
    fprintf(targetFile, "POP R0\n");
    fprintf(targetFile, "POP R0\n");
    fprintf(targetFile, "POP R2\n");
    fprintf(targetFile, "POP R2\n");
    IP += 14;

    popUsedRegisters(targetFile);
}

void print(int registerIndex, FILE *targetFile)
{
    pushUsedRegisters(targetFile);

    fprintf(targetFile, "MOV R0, R%d\n", registerIndex);
    fprintf(targetFile, "MOV R2, 5\n");
    fprintf(targetFile, "PUSH R2\n"); // Pushed System Call number
    fprintf(targetFile, "MOV R2, -2\n");
    fprintf(targetFile, "PUSH R2\n"); // Pushed argument 1
    fprintf(targetFile, "PUSH R0\n"); // Pushed argument 2
    fprintf(targetFile, "PUSH R0\n"); // Pushed argument 3
    fprintf(targetFile, "PUSH R0\n"); // Pushed return value
    fprintf(targetFile, "INT 7\n");   // System Call
    fprintf(targetFile, "POP R0\n");
    fprintf(targetFile, "POP R0\n");
    fprintf(targetFile, "POP R0\n");
    fprintf(targetFile, "POP R2\n");
    fprintf(targetFile, "POP R2\n");
    IP += 14;

    popUsedRegisters(targetFile);
}

void codeExit(FILE *targetFile)
{
    fprintf(targetFile, "MOV R2, 10\n");
    fprintf(targetFile, "PUSH R2\n"); // Pushed System Call Number
    fprintf(targetFile, "PUSH R2\n"); // Pushed argument 1
    fprintf(targetFile, "PUSH R2\n"); // Pushed argument 2
    fprintf(targetFile, "PUSH R2\n"); // Pushed argument 3
    fprintf(targetFile, "PUSH R2\n"); // Pushed return value
    fprintf(targetFile, "INT 10\n");  // Interrupt Routine Number
    IP += 7;
}

int getAddress(char id)
{
    int address = 4096 + id - 'a';
    return address;
}

int getLabel()
{
    return ++lastUsedLabel;
}
/*
    Parse tree maker
*/

struct tnode *makeLeafVar(char var)
{
    struct tnode *temp = (struct tnode *)malloc(sizeof(struct tnode));
    temp->varName = (char *)malloc(sizeof(char));
    temp->val = 0;
    *(temp->varName) = var;
    temp->nodeType = NTLEAF;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

struct tnode *makeLeafNum(int n)
{
    struct tnode *temp = (struct tnode *)malloc(sizeof(struct tnode));
    temp->varName = NULL;
    temp->val = n;
    temp->nodeType = NTLEAF;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

struct tnode *makeNonLeafNode(char c, enum NODE_TYPE t, struct tnode *l, struct tnode *r)
{
    struct tnode *temp = (struct tnode *)malloc(sizeof(struct tnode));
    temp->varName = (char *)malloc(sizeof(char));
    *(temp->varName) = c;
    temp->val = 0;
    temp->nodeType = t;
    temp->left = l;
    temp->right = r;
    return temp;
}

int evaluate(struct tnode *t, FILE *targetFile)
{
    int i, j;
    if (t->nodeType == NTLEAF)
    {
        if (t->varName == NULL) // Number
        {
            i = getReg();
            fprintf(targetFile, "MOV R%d, %d\n", i, t->val);
            ++IP;
            return i;
        }
        else // Variable
        {
            int address = getAddress(*(t->varName));
            i = getReg();
            fprintf(targetFile, "MOV R%d, [%d]\n", i, address);
            ++IP;
            return i;
        }
    }
    else if (t->nodeType == NTCONNECTOR)
    {
        i = evaluate(t->left, targetFile);
        j = evaluate(t->right, targetFile);
    }
    else if (t->nodeType == NTREAD)
    {
        int address = getAddress(*(t->left->varName));
        read(address, targetFile);
    }
    else if (t->nodeType == NTWRITE)
    {
        if (t->left->nodeType == NTOPERATOR) //  expression
        {
            i = evaluate(t->left, targetFile);
            print(i, targetFile);
            freeReg();
        }
        else if (t->left->varName == NULL) // Raw integer value
        {
            i = getReg();
            fprintf(targetFile, "MOV R%d, %d\n", i, t->left->val);
            ++IP;
            print(i, targetFile);
            freeReg();
        }
        else // Variable name
        {
            int address = getAddress(*(t->left->varName));
            i = getReg();
            fprintf(targetFile, "MOV R%d, [%d]\n", i, address);
            ++IP;
            print(i, targetFile);
            freeReg();
        }
    }
    else if (t->nodeType == NTIF)
    {
        i = evaluate(t->left, targetFile);

        if (t->right->nodeType != NTELSE)
        {
            int label1 = getLabel();
            fprintf(targetFile, "JZ R%d, L%d\n", i, label1);
            ++IP;
            j = evaluate(t->right, targetFile);
            fprintf(targetFile, "L%d:\n", label1);
            labelAddress[label1] = IP;
        }
        else
        {
            int label1 = getLabel();
            int label2 = getLabel();
            fprintf(targetFile, "JZ R%d, L%d\n", i, label1);
            ++IP;
            j = evaluate(t->right->left, targetFile);
            fprintf(targetFile, "JMP L%d\n", label2);
            ++IP;
            fprintf(targetFile, "L%d:\n", label1);
            labelAddress[label1] = IP;
            j = evaluate(t->right->right, targetFile);
            fprintf(targetFile, "L%d:\n", label2);
            labelAddress[label2] = IP;
        }
        freeReg(); // Freeing i
    }
    else if (t->nodeType == NTWHILE)
    {
        int label = getLabel();
        fprintf(targetFile, "L%d:\n", label);
        labelAddress[label] = IP;
        i = evaluate(t->left, targetFile);
        j = evaluate(t->right, targetFile);
        fprintf(targetFile, "JNZ R%d, L%d\n", j, label);
        ++IP;
        freeReg();
    }
    else
    {
        if (*(t->varName) != '=')
            i = evaluate(t->left, targetFile);
        j = evaluate(t->right, targetFile);
        switch (*(t->varName))
        {
        case '=':
        {
            int address = getAddress(*(t->left->varName));
            fprintf(targetFile, "MOV [%d], R%d\n", address, j);
            i = -1;
            break;
        }
        case '+':
        {
            fprintf(targetFile, "ADD R%d, R%d\n", i, j);
            break;
        }
        case '-':
        {
            fprintf(targetFile, "SUB R%d, R%d\n", i, j);
            break;
        }
        case '*':
        {
            fprintf(targetFile, "MUL R%d, R%d\n", i, j);
            break;
        }
        case '<':
        {
            fprintf(targetFile, "LE R%d, R%d\n", i, j);
            break;
        }
        case '>':
        {
            fprintf(targetFile, "GE R%d, R%d\n", i, j);
            break;
        }
        case 'e':
        {
            fprintf(targetFile, "EQ R%d, R%d\n", i, j);
            break;
        }
        case 'n':
        {
            fprintf(targetFile, "NE R%d, R%d\n", i, j);
            break;
        }
        }
        ++IP;
        freeReg();
        return i;
    }
    return -1;
}

void linker(FILE *from, FILE *to)
{
    char buffer[32];
    while (!feof(from))
    {
        fgets(buffer, 32, from);
        if (buffer[0] == 'L') // Skip labels
            continue;
        if (buffer[0] != 'J' && buffer[0] != 'C') // not jump and call
        {
            fprintf(to, "%s", buffer);
            continue;
        }
        char *pos = strchr(buffer, 'L');
        if (pos == NULL)
            fprintf(to, "%s", buffer);
        else
        {
            int label = 0;
            ++pos;
            while (*pos != '\n')
            {
                label = label * 10 + (*pos - '0');
                ++pos;
            }
            for (int i = 0; buffer[i] != 'L'; i++)
                fprintf(to, "%c", buffer[i]);
            fprintf(to, "%d\n", 2056 + 2 * labelAddress[label]);
        }
    }
}