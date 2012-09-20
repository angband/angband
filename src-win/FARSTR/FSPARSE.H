#define NOT_IN_STRING 0
#define LEAVING_STRING 1
#define IN_STRING 2
#define IGNORE_STRING 3

#define FALSE 0
#define TRUE 1

int NotWhiteSpace(char);
void SetCommentLevel(char,char,int *,int*);
void SetParenLevel(char,int*);
void SetIOSState(char,char,char *);
void AddStringToTable(char *,FILE *,FILE *);
unsigned int GetHighFromHeader(FILE *);


