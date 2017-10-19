#include <stdlib.h>
#include <string.h>

// Original Author: Nat Tuck
// Modified By: Me
#include "svec.h"

// Create and return a new svec.
svec*
make_svec()
{
    // Allocate memory for the svec.
    svec* sv = malloc(sizeof(svec));

    // Initialize the beginning params.
    sv->size = 0;
    sv->cap  = 2;

    // Allocate memory for the svec's data.
    sv->data = malloc(sv->cap * sizeof(char*));

    return sv;
}

// Destroy the given svec.
void
free_svec(svec* sv)
{
    // Free all the Strings.
    for (int ii = 0; ii < sv->size; ++ii) {
        free(sv->data[ii]);
    }

    // Free the rest.
    free(sv->data);
    free(sv);
}

// Get the String at ii in the given svec.
char*
svec_get(svec* sv, int ii)
{
    return sv->data[ii];
}

// Add a copy of the item to the end of the svec,
// and reallocate more space if necessary.
void
svec_push(svec* sv, const char* item)
{
    // Reallocate memory if the size is equal to the capacity.
    if (sv->size >= sv->cap) {
        sv->cap *= 2;
        sv->data = realloc(sv->data, sv->cap * sizeof(char*));
    }

    // Push a copy on.
    sv->data[sv->size] = strdup(item);
    sv->size += 1;
}

// Get the index of the first occurence of the given item in the svec.
// Returns -1 if not found.
int
svec_find(svec* sv, const char* item)
{
    for (int ii = 0; ii < sv->size; ++ii) {
        if (strcmp(sv->data[ii], item) == 0) {
            return ii;
        }
    }

    return -1;
}

// Returns a new svec containing all the data of the svec up
// to, but not including, the given item.
svec*
svec_up_to(svec* sv, const char* item)
{
    svec* result_svec = make_svec();

    int ii = 0;
    while ((ii < sv->size) && (strcmp(item, svec_get(sv, ii)) != 0)) {
        svec_push(result_svec, svec_get(sv, ii));
        ++ii;
    }

    return result_svec;
}

// Returns a new svec containing all the data of the svec up
// from, but not including, the given item.
svec*
svec_up_from(svec* sv, const char* item)
{
    svec* result_svec = make_svec();

    int ii = svec_find(sv, item) + 1;
    for (ii; ii < sv->size; ++ii) {
        svec_push(result_svec, svec_get(sv, ii));
    }

    return result_svec;
}

// Does the svec contain the given item?
int
svec_contains(svec* sv, const char* item)
{
    for (int ii = 0; ii < sv->size; ++ii) {
        if (strcmp(sv->data[ii], item) == 0) {
            return 1;
        }
    }

    return 0;
}
