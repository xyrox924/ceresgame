#ifndef CVECTOR_H
#define CVECTOR_H

#define VECTOR_DEFAULT_CAPACITY 4

#define vector(T) struct \
{ \
    T *data; \
    size_t size; \
    size_t capacity; \
}

#define vector_init(v) do \
{ \
    (v).data = malloc(VECTOR_DEFAULT_CAPACITY * sizeof(*(v).data)); \
    if (!(v).data) \
    { \
        perror("Failed to allocate memory for vector"); \
        exit(-1); \
    } \
    (v).size = 0; \
    (v).capacity = VECTOR_DEFAULT_CAPACITY; \
} while (0)

#define vector_push_back(v, value) do \
{ \
    if ((v).size >= (v).capacity) \
    { \
        vector_resize(v, (v).capacity * 2); \
    } \
    (v).data[(v).size++] = (value); \
} while (0)

#define vector_resize(v, new_capacity) do \
{ \
    (v).capacity = (new_capacity); \
    (v).data = realloc((v).data, (v).capacity * sizeof(*(v).data)); \
    if (!(v).data) \
    { \
        perror("Failed to allocate memory for vector"); \
        exit(-1); \
    } \
} while (0)

#define vector_free(v) do \
{ \
    free((v).data); \
    (v).data = NULL; \
    (v).size = 0; \
    (v).capacity = 0; \
} while (0)

#define vector_at(v, index) (v).data[index]

#endif /* CVECTOR_H */