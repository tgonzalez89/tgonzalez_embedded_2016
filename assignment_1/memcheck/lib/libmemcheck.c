//gcc -shared -fPIC -Wl,--no-as-needed -ldl -o libmemcheck.so libmemcheck.c

#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>

int mallocs = 0;
int frees = 0;

static void* (*real_malloc)(size_t) = NULL;

static void malloc_init() {
    real_malloc = dlsym(RTLD_NEXT, "malloc");
    if (real_malloc == NULL)
        fprintf(stderr, "Error in 'dlsym': %s\n", dlerror());
}

void* malloc(size_t size) {
    if (real_malloc == NULL)
        malloc_init();

    void* p = NULL;
    p = real_malloc(size);
    mallocs++;
    fprintf(stderr, "%p = malloc(%ld), mallocs: %d frees: %d diff: %d\n", p, size, mallocs, frees, mallocs-frees);
    return p;
}

static void (*real_free)(void*) = NULL;

static void free_init() {
    real_free = dlsym(RTLD_NEXT, "free");
    if (real_free == NULL)
        fprintf(stderr, "Error in 'dlsym': %s\n", dlerror());
}

void free(void* p) {
    if (real_free == NULL)
        free_init();

    real_free(p);
    frees++;
    fprintf(stderr, "free(%p), mallocs: %d frees: %d diff: %d\n", p, mallocs, frees, mallocs-frees);
    return;
}
