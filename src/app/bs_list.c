#include "bs_list.h"
#include "bs_mem.h"

bs_list_factory_t*
bs_list_factory_create()
{
  bs_list_factory_t *pFactory; 

  BS_MEM_ALLOC_TYPE(pFactory, bs_list_factory_t, 1); 

  pFactory->pNodePool = NULL; 
  pFactory->pListPool = NULL; 

  return pFactory;   
}

void 
bs_list_factory_destroy(bs_list_factory_t *pFactory)
{
  bs_list_pooled_element_t *pCur, *pNext; 
  
  pCur = pFactory->pListPool; 
  while(pCur != NULL)
  {
    pNext = pCur->pNext; 
    BS_MEM_FREE(pCur); 
    pCur = pNext; 
  }

  pCur = pFactory->pNodePool; 
  while(pCur != NULL)
  {
    pNext = pCur->pNext; 
    BS_MEM_FREE(pCur); 
    pCur = pNext; 
  }
  
  BS_ASSERT(pFactory != NULL);
  BS_MEM_FREE(pFactory);   
}

static bs_list_node_t*
bs_list_factory_get_node(bs_list_factory_t *pFactory)
{
  bs_list_node_t *pNode; 
  
  BS_ASSERT(pFactory != NULL);

  if(pFactory->pNodePool == NULL)
  {
    BS_MEM_ALLOC_TYPE(pNode, bs_list_node_t, 1); 
    return pNode; 
  }  
  
  pNode = (bs_list_node_t*) pFactory->pNodePool; 
  pFactory->pNodePool = pFactory->pNodePool->pNext; 
  
  return pNode; 
}

static void
bs_list_factory_give_node(bs_list_factory_t *pFactory, 
                          bs_list_node_t *pNode)
{
  bs_list_pooled_element_t *pPooled; 

  BS_ASSERT(pFactory != NULL);
  BS_ASSERT(pNode != NULL);

  if(pNode->destructor != NULL)
  {
    pNode->destructor(pNode->pUserData); 
  }

  pPooled = (bs_list_pooled_element_t*) pNode; 
  pPooled->pNext = pFactory->pListPool; 
  pFactory->pListPool = pPooled; 
}

bs_list_t* 
bs_list_factory_get_list(bs_list_factory_t *pFactory)
{
  bs_list_t *pList; 
  
  BS_ASSERT(pFactory != NULL);   
  
  if(pFactory->pListPool == NULL)
  {
    BS_MEM_ALLOC_TYPE(pList, bs_list_t, 1);    
    pList->pFactory = pFactory;  
    pList->pHead = NULL; 
    pList->pTail = NULL;
    pList->length = 0; 
    return pList; 
  }
  
  pList = (bs_list_t*) pFactory->pListPool; 
  pFactory->pListPool = pFactory->pListPool->pNext; 
  
  pList->pFactory = pFactory; 
  pList->pHead = NULL; 
  pList->pTail = NULL;
  pList->length = 0; 
    
  return pList; 
}

void 
bs_list_factory_give_list(bs_list_factory_t *pFactory, 
                          bs_list_t *pList)
{
  bs_list_node_t *pCur, *pNext; 
  bs_list_pooled_element_t *pPooled; 
  
  BS_ASSERT(pFactory != NULL);   
  BS_ASSERT(pList != NULL);   
  
  pCur = pList->pHead; 
  while(pCur != NULL)
  {
    pNext = pCur->pNext; 
    bs_list_factory_give_node(pFactory, pCur); 
    pCur = pNext;   
  }
  
  pPooled = (bs_list_pooled_element_t*) pList; 
  pPooled->pNext = pFactory->pListPool; 
  pFactory->pListPool = pPooled; 
}

bs_list_factory_t *pFactory = NULL; 

bs_list_t* 
bs_list_create()
{
  bs_list_t *pList; 
  
  if(pFactory == NULL)
  {
    pFactory = bs_list_factory_create(); 
  }
  
  pList = bs_list_factory_get_list(pFactory); 
  
  return pList;
}

void 
bs_list_destroy(bs_list_t *pList)
{
  BS_ASSERT(pList != NULL);
  BS_ASSERT(pFactory != NULL);
  bs_list_factory_give_list(pFactory, pList);   
}

void 
bs_list_add_element_to_front(bs_list_t *pList, 
                             void *pUserData, 
                             bs_list_element_destuctor_t destructor)
{
  bs_list_node_t *pNode; 
  
  BS_ASSERT(pList != NULL);
  
  pNode = bs_list_factory_get_node(pList->pFactory); 
  pNode->pPrev = NULL; 
  pNode->pNext = pList->pHead; 
  pNode->pUserData = pUserData; 
  pNode->destructor = destructor; 
  
  pList->length++; 
  if(pList->pHead != NULL)
  {
    pList->pHead->pPrev = pNode;     
  }
  pList->pHead = pNode; 
}

void 
bs_list_add_element_to_back(bs_list_t *pList, 
                            void *pUserData, 
                            bs_list_element_destuctor_t destructor)
{
  bs_list_node_t *pNode; 
  
  BS_ASSERT(pList != NULL);
  
  pNode = bs_list_factory_get_node(pList->pFactory); 
  pNode->pPrev = pList->pTail; 
  pNode->pNext = NULL; 
  pNode->pUserData = pUserData; 
  pNode->destructor = destructor; 
  
  pList->length++; 
  if(pList->pTail != NULL)
  {
    pList->pTail->pNext = pNode; 
  }
  pList->pTail = pNode;     
}

void* 
bs_list_get_first_element(bs_list_t *pList)
{
  BS_ASSERT(pList != NULL);
  if(pList->pHead == NULL)
  {
    return NULL; 
  }
  return pList->pHead->pUserData;   
}

void* 
bs_list_get_last_element(bs_list_t *pList)
{
  BS_ASSERT(pList != NULL);
  if(pList->pTail == NULL)
  {
    return NULL; 
  }
  return pList->pTail->pUserData;     
}

bs_int32_t 
bs_list_get_length(bs_list_t *pList)
{
  BS_ASSERT(pList != NULL);
  
  return pList->length;   
}

void 
bs_list_map(bs_list_t *pList, 
            bs_list_map_t map)
{
  bs_list_node_t *pCur; 
  
  BS_ASSERT(pList != NULL);
  BS_ASSERT(map != NULL);

  pCur = pList->pHead; 
  while(pCur != NULL)
  {
    map(pCur->pUserData); 
    pCur = pCur->pNext;     
  }    
}

void
bs_list_map_f(bs_list_t *pList, 
              bs_list_map_f_t fMap, 
              bs_float_t floatArg)
{
  bs_list_node_t *pCur; 
  
  BS_ASSERT(pList != NULL);
  BS_ASSERT(fMap != NULL);

  pCur = pList->pHead; 
  while(pCur != NULL)
  {
    fMap(pCur->pUserData, floatArg); 
    pCur = pCur->pNext;     
  }      
}

void
bs_list_map_i(bs_list_t *pList, 
              bs_list_map_i_t iMap, 
              bs_int32_t intArg)
{
  bs_list_node_t *pCur; 
  
  BS_ASSERT(pList != NULL);
  BS_ASSERT(iMap != NULL);

  pCur = pList->pHead; 
  while(pCur != NULL)
  {
    iMap(pCur->pUserData, intArg); 
    pCur = pCur->pNext;     
  }        
}

void
bs_list_map_p(bs_list_t *pList, 
              bs_list_map_p_t pMap, 
              void *pDataArg)
{  
  bs_list_node_t *pCur; 
  
  BS_ASSERT(pList != NULL);
  BS_ASSERT(pMap != NULL);

  pCur = pList->pHead; 
  while(pCur != NULL)
  {
    pMap(pCur->pUserData, pDataArg); 
    pCur = pCur->pNext;     
  }        
}

void
bs_list_map_pp(bs_list_t *pList, 
               bs_list_map_pp_t ppMap, 
               void *pDataArgA, 
               void *pDataArgB)
{
  bs_list_node_t *pCur; 
  
  BS_ASSERT(pList != NULL);
  BS_ASSERT(ppMap != NULL);

  pCur = pList->pHead; 
  while(pCur != NULL)
  {
    ppMap(pCur->pUserData, pDataArgA, pDataArgB); 
    pCur = pCur->pNext;     
  }          
}

void
bs_list_map_p_cont(bs_list_t *pList, 
                   bs_list_map_p_cont_t pMapCont, 
                   void *pDataArg)
{
  bs_list_node_t *pCur; 
  
  BS_ASSERT(pList != NULL);
  BS_ASSERT(pMapCont != NULL);

  pCur = pList->pHead; 
  while(pCur != NULL)
  {
    if(pMapCont(pCur->pUserData, pDataArg) == FALSE)
    {
      break;   
    }
    pCur = pCur->pNext;     
  }        
}

void
bs_list_map_pp_cont(bs_list_t *pList, 
                    bs_list_map_pp_cont_t ppMapCont, 
                    void *pDataArgA, 
                    void *pDataArgB)
{
  bs_list_node_t *pCur; 
  
  BS_ASSERT(pList != NULL);
  BS_ASSERT(ppMapCont != NULL);

  pCur = pList->pHead; 
  while(pCur != NULL)
  {
    if(ppMapCont(pCur->pUserData, pDataArgA, pDataArgB) == FALSE)
    {
      break;  
    }
    pCur = pCur->pNext;     
  }            
}

void
bs_list_map_fp(bs_list_t *pList, 
               bs_list_map_fp_t fpMap,
               bs_float_t floatArg,                
               void *pDataArg)
{
  bs_list_node_t *pCur; 
  
  BS_ASSERT(pList != NULL);
  BS_ASSERT(fpMap != NULL);

  pCur = pList->pHead; 
  while(pCur != NULL)
  {
    fpMap(pCur->pUserData, floatArg, pDataArg); 
    pCur = pCur->pNext;     
  }          
}

void
bs_list_map_ip(bs_list_t *pList, 
               bs_list_map_ip_t ipMap,
               bs_int32_t intArg,                
               void *pDataArg)
{
  bs_list_node_t *pCur; 

  BS_ASSERT(pList != NULL);
  BS_ASSERT(ipMap != NULL);

  pCur = pList->pHead; 
  while(pCur != NULL)
  {
    ipMap(pCur->pUserData, intArg, pDataArg); 
    pCur = pCur->pNext; 
  }    
}

void 
bs_list_map_cont(bs_list_t *pList, 
                 bs_list_map_cont_t mapCont)
{
  bs_list_node_t *pCur; 
  
  BS_ASSERT(pList != NULL);
  BS_ASSERT(mapCont != NULL);

  pCur = pList->pHead; 
  while(pCur != NULL)
  {
    if(mapCont(pCur->pUserData) == FALSE)
    {
      break;   
    }
    pCur = pCur->pNext;     
  }      
}

void 
bs_list_map_acc(bs_list_t *pList, 
                bs_list_map_acc_t mapAcc, 
                void *pAccumulator)
{
  bs_list_node_t *pCur; 
  
  BS_ASSERT(pList != NULL);
  BS_ASSERT(mapAcc != NULL);

  pCur = pList->pHead; 
  while(pCur != NULL)
  {
    mapAcc(pCur->pUserData, pAccumulator); 
    pCur = pCur->pNext;     
  }      
}

void 
bs_list_map_acc_cont(bs_list_t *pList, 
                     bs_list_map_acc_cont_t mapAccCont, 
                     void *pAccumulator)
{
  bs_list_node_t *pCur; 
  
  BS_ASSERT(pList != NULL);
  BS_ASSERT(mapAccCont != NULL);

  pCur = pList->pHead; 
  while(pCur != NULL)
  {
    if(mapAccCont(pCur->pUserData, pAccumulator) == FALSE)
    {
      break;   
    }
    pCur = pCur->pNext;     
  }      
}

static void 
bs_list_unlink_element(bs_list_t *pList, 
                       bs_list_node_t *pNode)
{
  bs_list_node_t *pPrev, *pNext; 
  
  BS_ASSERT(pList != NULL); 
  BS_ASSERT(pNode != NULL); 
  
  pPrev = pNode->pPrev; 
  pNext = pNode->pNext; 
  
  bs_list_factory_give_node(pList->pFactory, pNode); 
  
  if(pPrev == NULL && pNext == NULL)
  {
    pList->pHead = NULL; 
    pList->pTail = NULL; 
  }
  else if(pPrev == NULL)
  {
    pList->pHead = pNext; 
    pNext->pPrev = NULL; 
  }
  else if(pNext == NULL)
  {
    pList->pTail = pPrev; 
    pPrev->pNext = NULL; 
  }
  else 
  {
    pPrev->pNext = pNext; 
    pNext->pPrev = pPrev; 
  }

  pList->length--; 
}

void 
bs_list_unlink(bs_list_t *pList, 
               bs_list_unlink_t unlink)
{
  bs_list_node_t *pCur, *pNext; 
  
  BS_ASSERT(pList != NULL);
  BS_ASSERT(unlink != NULL);

  pCur = pList->pHead;  
  while(pCur != NULL)
  {
    pNext = pCur->pNext; 
    if(unlink(pCur->pUserData) == TRUE)
    {
      bs_list_unlink_element(pList, pCur); 
    }
    pCur = pNext; 
  }      
}

void 
bs_list_unlink_i(bs_list_t *pList, 
                 bs_list_unlink_i_t iUnlink, 
                 bs_int32_t intArg)
{
  bs_list_node_t *pCur, *pNext; 
  
  BS_ASSERT(pList != NULL);
  BS_ASSERT(iUnlink != NULL);

  pCur = pList->pHead;  
  while(pCur != NULL)
  {
    pNext = pCur->pNext; 
    if(iUnlink(pCur->pUserData, intArg) == TRUE)
    {
      bs_list_unlink_element(pList, pCur); 
    }
    pCur = pNext; 
  }        
}

void 
bs_list_unlink_p(bs_list_t *pList, 
                 bs_list_unlink_p_t pUnlink, 
                 void *pDataArg)
{
  bs_list_node_t *pCur, *pNext; 
  
  BS_ASSERT(pList != NULL);
  BS_ASSERT(pUnlink != NULL);

  pCur = pList->pHead;  
  while(pCur != NULL)
  {
    pNext = pCur->pNext; 
    if(pUnlink(pCur->pUserData, pDataArg) == TRUE)
    {
      bs_list_unlink_element(pList, pCur); 
    }
    pCur = pNext; 
  }        
  
}

void 
bs_list_unlink_i_cont(bs_list_t *pList, 
                      bs_list_unlink_i_cont_t iContUnlink, 
                      bs_int32_t intArg)
{
  bs_list_node_t *pCur, *pNext; 
  bs_bool_t cont; 
  
  BS_ASSERT(pList != NULL);
  BS_ASSERT(iContUnlink != NULL);

  cont = TRUE; 
  pCur = pList->pHead;  
  while(pCur != NULL && cont == TRUE)
  {
    pNext = pCur->pNext; 
    if(iContUnlink(pCur->pUserData, intArg, &cont) == TRUE)
    {
      bs_list_unlink_element(pList, pCur); 
    }
    pCur = pNext; 
  }          
}

void 
bs_list_unlink_p_cont(bs_list_t *pList, 
                      bs_list_unlink_p_cont_t pContUnlink, 
                      void *pDataArg)
{
  bs_list_node_t *pCur, *pNext; 
  bs_bool_t cont; 
  
  BS_ASSERT(pList != NULL);
  BS_ASSERT(pContUnlink != NULL);

  cont = TRUE; 
  pCur = pList->pHead;  
  while(pCur != NULL && cont == TRUE)
  {
    pNext = pCur->pNext; 
    if(pContUnlink(pCur->pUserData, pDataArg, &cont) == TRUE)
    {
      bs_list_unlink_element(pList, pCur); 
    }
    pCur = pNext; 
  }            
}
