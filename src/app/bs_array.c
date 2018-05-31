#include "bs_array.h"
#include "bs_mem.h"

bs_array_t* 
bs_array_create(bs_uint32_t capacity, 
                bs_array_element_destroy_cb destroyCallback)
{
  bs_array_t *pArray; 

  BS_ASSERT(capacity > 0); 
  
  BS_MEM_ALLOC_TYPE(pArray, bs_array_t, 1); 
  BS_MEM_ALLOC_TYPE(pArray->pArray, void*, capacity); 
  pArray->capacity = capacity; 
  pArray->size = 0; 
  pArray->destroyCallback = destroyCallback; 
  
  return pArray;
}

void 
bs_array_destroy(bs_array_t *pArray)
{
  bs_int32_t i; 
  
  BS_ASSERT(pArray != NULL);
  BS_ASSERT(pArray->pArray != NULL);
  
  if(pArray->destroyCallback != NULL)
  {
    for(i = 0; i < pArray->size; i++)
    {
      pArray->destroyCallback(pArray->pArray[i]);
    }
  }
  BS_MEM_FREE(pArray->pArray);   
  BS_MEM_FREE(pArray);   
}

void 
bs_array_ensure_capacity(bs_array_t *pArray)
{
  BS_ASSERT(pArray != NULL);
  
  if(pArray->size == pArray->capacity)
  {
    pArray->capacity *= 2; 
    BS_MEM_REALLOC_TYPE(pArray->pArray, void*, pArray->capacity); 
  }  
}

void 
bs_array_trim(bs_array_t *pArray)
{
  void **pPtr; 
  bs_uint32_t i; 
  
  BS_ASSERT(pArray != NULL);

  BS_MEM_ALLOC_TYPE(pPtr, void*, pArray->size); 
  for(i = 0; i < pArray->size; i++)
  {
    pPtr[i] = pArray->pArray[i]; 
  }
  BS_MEM_FREE(pArray->pArray); 
  pArray->pArray = pPtr;
  pArray->capacity = pArray->size;   
}
