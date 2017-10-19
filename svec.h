#ifndef SVEC_H
#define SVEC_H

// Original Author: Nat Tucl
// Modified By: Me

// Defines a String Vector.
typedef struct svec
{
    int size;
    int cap;
    char** data;
} svec;

// Create and return a new svec..
svec* make_svec();

// Destroy the given svec.
void free_svec(svec* sv);

// Get the item at the given index in the svec.
char* svec_get(svec* sv, int ii);

// Add the given item to the end of the svec.
void svec_push(svec* sv, const char* item);

// Return and remove the last item of the svec.
char* svec_pop(svec* sv);

// Find the index of the given item in the svec.
int svec_find(svec* sv, const char* item);

// Does the svec contain the given item?
int svec_contains(svec* sv, const char* item);

// Returns a new svec containing the data of the svec up to the given item.
svec* svec_up_to(svec* sv, const char* item);

// Returns a new svec containing the data of the svec up from the given item.
svec* svec_up_from(svec* sv, const char* item);

#endif
