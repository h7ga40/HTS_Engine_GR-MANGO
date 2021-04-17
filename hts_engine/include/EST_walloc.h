#ifndef EST_WALLOC_H
#define EST_WALLOC_H

void *safe_wcalloc(size_t size);
void wfree(void *mem);
char *wstrdup(const char *string);

#endif // EST_WALLOC_H
