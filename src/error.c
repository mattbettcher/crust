#include "error.h"

static void print_error(Error *error) {
    switch (error->kind) {
    case ERROR_NONE:
        // Do nothing!
        break;
    case ERROR_EXPECTED:
        
        break;
    
    default:
        printf("Unhandled error!");
        break;
    }
}
