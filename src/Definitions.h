#ifndef Definitions_h
#define Definitions_h

#define DEBUG

#ifndef DEBUG
#define Assert(n)
#else
#define Assert(n) \
if (!(n)) { \
printf("%s - Failed", #n); \
printf("On %s ", __DATE__); \
printf("At %s ", __TIME__); \
printf("In File %s ", __FILE__); \
printf("At Line %d\n", __LINE__); \
exit(1); \
}
#endif

typedef unsigned long long U64;

#endif