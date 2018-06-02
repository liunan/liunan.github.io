#include "bs_util.h"

#include <stdlib.h>

bs_float_t 
bs_util_random_float()
{
  return (bs_float_t) random() / (bs_float_t) RAND_MAX;     
}
