#define getshort() fgetshort(stdin)
#define getnshort(p,n) fgetnshort(stdin,p,n)

short int fgetshort(FILE *fp);
int fgetnshort(FILE *fp, short int *p, int n);
