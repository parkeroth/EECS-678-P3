#ifndef STDLIB_H
#define STDLIB_H

#define NULL ((void*)0)
#define EOF  (-1)

typedef unsigned int size_t;

extern int atoi (const char *s);
extern char *gets (char *s);
extern char *itoa (int num, char *buffer);
extern char *strcat (char *s1, const char *s2);
extern char *strchr (const char *s1, int n);
extern int strcmp (const char *s1, const char *s2);
extern char *strcpy (char *s1, const char *s2);
extern size_t strlen (const char *ptr);
extern char *strncat (char *s1, const char *s2, size_t n);
extern int strncmp (const char *s1, const char *s2, size_t n);
extern char *strncpy (char *s1, const char *s2, size_t n);
extern char *strrchr (const char *s1, int n);
extern char *strstr (const char *s1, const char *s2);

#endif /* STDLIB_H */
