#include <stdlib.h>
#include <stdio.h>

main()
{
	long t1;
	int t2;
	t1 = 30000;
	t2 = 37000;
	if (t1>t2) printf(">");
	if (t1<t2) printf("<");
	if (t1==t2) printf("=");
}
