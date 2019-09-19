#ifndef utils_h__
#define utils_h__

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <Windows.h>
#include <conio.h>
#include <io.h>

#define HAVE_KBHIT 1
#define HAVE_PEEKNAMEDPIPE 1

int read_key();

void term_init(void);
void term_exit(void);


bool set_debug_privilege();

#endif // utils_h__
