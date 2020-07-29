
#ifndef ENUMERATOR_H
#define ENUMERATOR_H

struct Enumerator {
    void* start; // Value 
    int (*hasNext) (void*);
    void* (*peek) (void*);
    void* (*next) (void*);
};
typedef struct Enumerator Enumerator;

#endif // ENUMERATOR_H
