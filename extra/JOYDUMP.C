#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <time.h>
#include <conio.h> 


int JOY_get_count(void);
int JOY_get_tsc(void);
int JOY_cli(void);
int JOY_sti(void);



int dump(int l, int f)
{
  int n, c = 0, s, b = 0, q = 0, r = 0, j = 0, o = 0, ret = 0;
  int m = 3200000L;
  unsigned char * d = malloc(m);
  unsigned short * t = malloc(m * sizeof(unsigned short));
  unsigned * tsc = malloc(m * sizeof(unsigned));

  outp(0x201, 0);
  JOY_cli();
  s = n = 0xffff&JOY_get_count();
  while(c < 2)
  {
    int i = 0;
  
    if(n > s && !b) b = !b;
    if(n <= s && b)
    {
      b = !b;
      c += 1;
    }
    if(c >= 2) continue;
    if(j >= m) goto ret1;    
    if(n <= 0x4000 && q && r && !c) goto ret1;
    if(n > 0x4000 && q && !r) r = !r;
    if(n <= 0x4000 && !q) q = !q;

    d[j] = inp(0x201);
    tsc[j] = JOY_get_tsc();
    t[j++] = n;

    if(f && j > 0 && (!(d[j-1]&0x3) || !(d[j-1]&0x0c))) break;

    for(i = 0 ; i < l ; i++);
    n = 0xffff&JOY_get_count();    
  }  

  printf( "           #           == AX == AY == BX == BY == A0 == A1 == B0 == B1 ==\n");

  for(m = j, j = 0 ; j < m ; j ++)
  {
    int i;
    printf("%010u#%05u{%05u} ", tsc[j], t[j], 0xffff&(s-t[j]));

    c = d[j];    
    for(i = 0; i < 8 ; i += 1)
    {
      printf(" ");
      if(o && (c & (1 << i)) != (b & (1 << i))) printf("****");
      else if(c & (1 << i)) printf("   *");
      else printf("*   ");
    }    
    b = c;
    o = 1;

    printf("\n");
  }
  

ret0:

  free(d);
  free(t);
  free(tsc);

  JOY_sti();

  return ret;

ret1:
  ret = 1;
  goto ret0;
}

void main(int argc, char ** argv)
{
  int i = 0;

  srand((int)time(0));

  for(i = 0 ; i < (argc > 1 ? atoi(argv[1]) : 3) ; i++)
  { 
    int j, r = rand();
    printf("  -- Preparing --");
    for(j = 0 ; j < (r%100 + 1000) ; j++) {
      int k = 0;      
      printf("\b");
      for(k = 0 ; k < 1000000L ; k++);
      printf("%c", "\\|/-"[j%4]);
    }
    printf("\n");
    if(dump(argc > 2 ? atoi(argv[2]) : 100, argc > 3)) i--;
  }

}
