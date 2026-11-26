#include <stddef.h>
#include <stdint.h>
void *memcpy(void *dst,const void *src,size_t n){
 unsigned char *d=dst;const unsigned char *s=src;
 if(!(((uintptr_t)d|(uintptr_t)s)&3u)){
  typedef uint32_t Word __attribute__((may_alias));
  while(n>=4){*(Word*)d=*(const Word*)s;d+=4;s+=4;n-=4;}
 }
 while(n--)*d++=*s++;
 return dst;
}
void *memset(void *dst,int c,size_t n){
 unsigned char *d=dst;unsigned char k=(unsigned char)c;
 while(n&&((uintptr_t)d&3u)){*d++=k;n--;}
 typedef uint32_t Word __attribute__((may_alias));
 Word w=(uint32_t)k*0x01010101u;
 while(n>=4){*(Word*)d=w;d+=4;n-=4;}
 while(n--)*d++=k;
 return dst;
}
void *memmove(void *dst,const void *src,size_t n){
 unsigned char *d=dst;const unsigned char *s=src;
 if((uintptr_t)d<=(uintptr_t)s)return memcpy(dst,src,n);
 while(n){n--;d[n]=s[n];}return dst;
}
int memcmp(const void *aa,const void *bb,size_t n){
 const unsigned char *a=aa,*b=bb;
 while(n--){if(*a!=*b)return (int)*a-(int)*b;a++;b++;}return 0;
}
size_t strlen(const char *s){const char *p=s;while(*p)p++;return (size_t)(p-s);}
