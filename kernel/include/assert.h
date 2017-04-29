#pragma once

void halt();
void _assert_backend(char *exp, char *file, int line);

#ifndef NDEBUG
#define assert(exp) if (!(exp)) _assert_backend(#exp, __FILE__, __LINE__)
#else
#define assert(exp) ((void)0)
#endif
