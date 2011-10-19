#ifndef __P2MP_H
#define __P2MP_H

#define FILE_NSIZE  100               // File name size
#define MSS         1500              // Maximum segment size of a packet
#define MAX_RECV    10                // Maximum number of receivers

#define BUF_TIMEOUT 2

#define P2MP_ZERO(p) memset(&(p), 0, sizeof(p))

void die(const char *s, int cause);
void warn(const char *s, int cause);

#endif
