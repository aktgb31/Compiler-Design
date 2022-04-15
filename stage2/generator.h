enum NODE_TYPE
{
    NTLEAF,
    NTREAD,
    NTWRITE,
    NTCONNECTOR,
    NTOPERATOR
};

struct tnode
{
    int val;
    char *varName;
    enum NODE_TYPE nodeType;
    struct tnode *left, *right;
};

struct tnode *makeLeafVar(char var);

struct tnode *makeLeafNum(int n);

struct tnode *makeNonLeafNode(char c, enum NODE_TYPE t, struct tnode *l, struct tnode *r);

int evaluate(struct tnode *t, FILE *targetFile);

int getReg();

void freeReg();

void initGenerator(FILE *targetFile);

void print(int registerIndex, FILE *targetFile);

void codeExit(FILE *targetFile);