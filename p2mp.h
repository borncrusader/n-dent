#ifndef __P2MP_H
#define __P2MP_H

#define P2MP_ZERO(p) memset(&(p), 0, sizeof(p))

void die(const char *s, int cause);
void warn(const char *s, int cause);

#endif
