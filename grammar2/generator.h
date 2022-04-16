enum NODE_TYPE
{
    NTLEAF,
    NTREAD,
    NTWRITE,
    NTIF,
    NTELSE,
    NTFUNDECL,
    NTFUNCALL,
    NTRETURN,
    NTCONNECTOR,
    NTOPERATOR
};

struct symbol
{
    char *name;
    int binding;
    struct symbol *next;
};

struct tnode
{
    int val;
    char *varName;
    enum NODE_TYPE nodeType;
    struct symbol *entry;
    struct tnode *left, *right;
};

int localScope = 0;

struct tnode *makeLeafVar(char *var);

struct tnode *makeLeafNum(int n);

struct tnode *makeNonLeafNode(char c, enum NODE_TYPE t, struct tnode *l, struct tnode *r);

int evaluate(struct tnode *t, FILE *targetFile);

void initGenerator(FILE *targetFile);

void codeExit(FILE *targetFile);

void linker(FILE *from, FILE *to);
