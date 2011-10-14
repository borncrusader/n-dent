#include <stdio.h>
#include <stdlib.h>

#include "p2mp.h"

void die(const char *s, int cause)
{
  if(!cause) {
    fprintf(stderr, "%s\n", s);
  } else {
    fprintf(stderr, "%s %s\n", s, strerror(cause));
  }
  exit(cause);
}

void warn(const char *s, int cause)
{
  if(!cause) {
    fprintf(stderr, "%s\n", s);
  } else {
    fprintf(stderr, "%s %s\n", s, strerror(cause));
  }
}
