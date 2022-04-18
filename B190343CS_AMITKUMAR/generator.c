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

int SP = 0;             // Stack Pointer
int BP_OFFSET = 0;      // Base Pointer offset for functions
int IP = 0;             // Instruction Pointer
int lastRegistor = -1;  // Last used Registor
int lastUsedLabel = -1; // Last Used label
int labelAddress[1020]; // address of labels
int addressOfFUN = -1;  // address of function FUN

int localScope = 0;

struct symbol *GSymbolTable = NULL;
struct symbol *LSymbolTable = NULL;
struct symbol *findSymbol(struct symbol *table, char *name)
{
    struct symbol *p = table;
    while (p != NULL)
    {
        if (strcmp(p->name, name) == 0)
        {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

void addGlobalSymbol(char *name)
{
    struct symbol *newSymbol = (struct symbol *)malloc(sizeof(struct symbol));
    newSymbol->name = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(newSymbol->name, name);
    newSymbol->binding = ++SP;
    newSymbol->next = GSymbolTable;
    GSymbolTable = newSymbol;
}

void addLocalSymbol(char *name)
{
    struct symbol *newSymbol = (struct symbol *)malloc(sizeof(struct symbol));
    newSymbol->name = malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(newSymbol->name, name);
    if (strcmp(name, "argc") == 0)
        newSymbol->binding = -2;
    else
        newSymbol->binding = ++BP_OFFSET;
    newSymbol->next = LSymbolTable;
    LSymbolTable = newSymbol;
}

void printTable(struct symbol *table)
{
    struct symbol *p = table;
    while (p != NULL)
    {
        printf("%s %d\n", p->name, p->binding);
        p = p->next;
    }
}

void clearTable(struct symbol *table, int *pointer)
{
    struct symbol *p = table;
    while (p != NULL)
    {
        struct symbol *temp = p;
        p = p->next;
        --(*pointer);
        free(temp);
    }
}

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

void pushUsedRegistors(FILE *targetFile)
{
    for (int i = 0; i <= lastRegistor; i++)
    {
        fprintf(targetFile, "PUSH R%d\n", i);
        ++IP;
    }
}

void popUsedRegistors(FILE *targetFile)
{
    for (int i = lastRegistor; i >= 0; i--)
    {
        fprintf(targetFile, "POP R%d\n", i);
        ++IP;
    }
}

void read(int address, FILE *targetFile)
{
    pushUsedRegistors(targetFile);

    fprintf(targetFile, "MOV R0, R%d\n", address);
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

    popUsedRegistors(targetFile);
}

void print(int registerIndex, FILE *targetFile)
{
    pushUsedRegistors(targetFile);

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

    popUsedRegistors(targetFile);
}

void codeExit(FILE *targetFile)
{
    fprintf(targetFile, "MOV R2, 10\n");
    fprintf(targetFile, "PUSH R2\n"); // Pushed System Call Number
    fprintf(targetFile, "PUSH R2\n"); // Pushed argument 1
    fprintf(targetFile, "PUSH R2\n"); // Pushed argument 2
    fprintf(targetFile, "PUSH R2\n"); // Pushed argument 3
    fprintf(targetFile, "PUSH R2\n"); // Pushed return value
    fprintf(targetFile, "INT 10");    // Interrupt Routine Number
    IP += 7;
}

int getLabel()
{
    return ++lastUsedLabel;
}

//
//
//  Code Generator
//
//
void initGenerator(FILE *targetFile)
{
    // XMAGIC, Entry Point, Text Size, Data Size, Heap Size, Stack Size, Library Flag, Unused
    fprintf(targetFile, "%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n", XMAGIC, ENTRY_POINT, TEXT_SIZE, DATA_SIZE, HEAP_SIZE, STACK_SIZE, LIBRARY_FLAG, UNUSED);
    fprintf(targetFile, "MOV SP, %d\n", 4096);
    ++IP;
    fprintf(targetFile, "MOV BP, 4095\n");
    ++IP;
}

/*
    Parse tree maker
*/

struct tnode *makeLeafVar(char *var)
{
    struct tnode *temp = (struct tnode *)malloc(sizeof(struct tnode));
    temp->val = 0;
    temp->varName = (char *)malloc(sizeof(char) * (strlen(var) + 1));
    strcpy(temp->varName, var);
    temp->nodeType = NTLEAF;
    temp->left = NULL;
    temp->right = NULL;

    temp->entry = findSymbol((localScope == 0 ? GSymbolTable : LSymbolTable), temp->varName);
    if (temp->entry == NULL)
    {
        if (localScope)
        {
            addLocalSymbol(temp->varName);
            temp->entry = LSymbolTable;
        }
        else
        {
            addGlobalSymbol(temp->varName);
            temp->entry = GSymbolTable;
        }
    }
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
    temp->entry = NULL;
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
    temp->entry = NULL;
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
            int address = t->entry->binding;
            i = getReg();
            fprintf(targetFile, "MOV R%d, %d\n", i, address);
            ++IP;
            fprintf(targetFile, "ADD R%d, BP\n", i);
            ++IP;
            fprintf(targetFile, "MOV R%d, [R%d]\n", i, i);
            ++IP;
            return i;
        }
    }
    else if (t->nodeType == NTRETURN)
    {
        i = evaluate(t->left, targetFile);
        j = getReg();
        for (int k = 0; k < BP_OFFSET; k++) // Removing local variables
        {
            fprintf(targetFile, "POP R%d\n", j);
            ++IP;
        }
        fprintf(targetFile, "MOV R%d, -2\n", j); // Moved to return value
        ++IP;
        fprintf(targetFile, "ADD R%d, BP\n", j); // Add base pointer to it
        ++IP;
        fprintf(targetFile, "MOV [R%d], R%d\n", j, i); // store return value
        ++IP;
        fprintf(targetFile, "MOV BP, [BP]\n");
        ++IP;
        fprintf(targetFile, "POP R%d\n", j); // Popped base pointer stored in stack
        ++IP;
        freeReg();
        freeReg();
        fprintf(targetFile, "RET\n");
        ++IP;
    }
    else if (t->nodeType == NTFUNDECL)
    {
        int label = getLabel();
        fprintf(targetFile, "JMP L%d\n", label);
        ++IP;
        addressOfFUN = 2056 + 2 * IP;

        fprintf(targetFile, "PUSH BP\n");
        ++IP;
        fprintf(targetFile, "MOV BP, SP\n");
        ++IP;

        for (int k = 0; k < BP_OFFSET; k++)
        {
            fprintf(targetFile, "PUSH R0\n");
            ++IP;
        }

        i = evaluate(t->left, targetFile);
        fprintf(targetFile, "L%d:\n", label);
        labelAddress[label] = IP;
    }
    else if (t->nodeType == NTFUNCALL)
    {
        int usedRegistors = lastRegistor;
        pushUsedRegistors(targetFile);
        lastRegistor = -1;

        i = evaluate(t->left, targetFile);

        fprintf(targetFile, "PUSH R%d\n", i); // Pushed arguments
        ++IP;

        fprintf(targetFile, "PUSH R%d\n", i); // Pushed return value
        ++IP;

        freeReg();

        fprintf(targetFile, "CALL %d\n", addressOfFUN); // Call function FUN
        ++IP;

        lastRegistor = usedRegistors;

        int k = getReg();
        fprintf(targetFile, "POP R%d\n", k); // Pop return value
        ++IP;
        j = getReg();
        fprintf(targetFile, "POP R%d\n", j); // Pop arguments
        ++IP;
        freeReg();
        freeReg(); // Freed the newly allocated registor this one contains return value
        popUsedRegistors(targetFile);

        i = getReg();
        if (i != k)
        {
            printf("Error Internal Logic Failed %d, %d\n", i, j);
            exit(1);
        }
        return i;
    }
    else if (t->nodeType == NTCONNECTOR)
    {
        i = evaluate(t->left, targetFile);
        j = evaluate(t->right, targetFile);
    }
    else if (t->nodeType == NTREAD)
    {
        int address = t->left->entry->binding;

        i = getReg();
        fprintf(targetFile, "MOV R%d, %d\n", i, address);
        ++IP;
        fprintf(targetFile, "ADD R%d,BP\n", i);
        ++IP;
        read(i, targetFile);
        freeReg();
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
            int address = t->left->entry->binding;
            i = getReg();
            fprintf(targetFile, "MOV R%d, %d\n", i, address);
            ++IP;
            fprintf(targetFile, "ADD R%d, BP\n", i);
            ++IP;
            fprintf(targetFile, "MOV R%d, [R%d]\n", i, i);
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
    else
    {
        if (*(t->varName) != '=')
            i = evaluate(t->left, targetFile);
        j = evaluate(t->right, targetFile);
        switch (*(t->varName))
        {
        case '=':
        {
            int address = t->left->entry->binding;
            i = getReg();
            fprintf(targetFile, "MOV R%d, %d\n", i, address);
            ++IP;
            fprintf(targetFile, "ADD R%d, BP\n", i);
            ++IP;
            fprintf(targetFile, "MOV [R%d], R%d\n", i, j);
            ++IP;
            freeReg();
            i = -1;
            break;
        }
        case '+':
        {
            fprintf(targetFile, "ADD R%d, R%d\n", i, j);
            ++IP;
            break;
        }
        case '-':
        {
            fprintf(targetFile, "SUB R%d, R%d\n", i, j);
            ++IP;
            break;
        }
        case '*':
        {
            fprintf(targetFile, "MUL R%d, R%d\n", i, j);
            ++IP;
            break;
        }
        case 'L':
        {
            fprintf(targetFile, "LE R%d, R%d\n", i, j);
            ++IP;
            break;
        }
        case 'G':
        {
            fprintf(targetFile, "GE R%d, R%d\n", i, j);
            ++IP;
            break;
        }
        case '<':
        {
            fprintf(targetFile, "LT R%d, R%d\n", i, j);
            ++IP;
            break;
        }
        case '>':
        {
            fprintf(targetFile, "GT R%d, R%d\n", i, j);
            ++IP;
            break;
        }
        case 'e':
        {
            fprintf(targetFile, "EQ R%d, R%d\n", i, j);
            ++IP;
            break;
        }
        case 'n':
        {
            fprintf(targetFile, "NE R%d, R%d\n", i, j);
            ++IP;
            break;
        }
        default:
        {
            printf("Error : Unrecognised Symbol\n");
            break;
        }
        }
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
        if (buffer[strlen(buffer) - 2] == ':') // Skip labels
            continue;
        if (buffer[0] != 'J') // not jump and call
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