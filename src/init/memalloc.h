#pragma once
#define MEMALLOC_H

void* dbmalloc(size_t size);
void dbfree(void* data);
void* dbrealloc(void* data, size_t size);