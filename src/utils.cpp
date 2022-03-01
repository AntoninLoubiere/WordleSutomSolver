#include "utils.h"
#include <cstdlib>
#include <ctime>
#include <stdlib.h>
bool initialised = false;

int randomInt(int min, int max) {
    d_assert(min < max);
    if (!initialised) {
        srand(time(NULL));
        initialised = true;
    }
    return rand() % (max - min) + min;
}
