#ifndef __BS_LIST_H__
#define __BS_LIST_H__

#include "bs_types.h"

typedef struct bs_list_factory_st bs_list_factory_t; 
typedef struct bs_list_st bs_list_t; 
typedef struct bs_list_node_st bs_list_node_t; 
typedef struct bs_list_pooled_element_st bs_list_pooled_element_t; 

typedef void (*bs_list_element_destuctor_t)(void*); 

typedef void (*bs_list_map_t)(void*); 
typedef void (*bs_list_map_f_t)(void*, bs_float_t); 
typedef void (*bs_list_map_i_t)(void*, bs_int32_t); 
typedef void (*bs_list_map_p_t)(void*, void*); 
typedef void (*bs_list_map_pp_t)(void*, void*, void*); 
typedef bs_bool_t (*bs_list_map_p_cont_t)(void*, void*); 
typedef bs_bool_t (*bs_list_map_pp_cont_t)(void*, void*, void*); 
typedef void (*bs_list_map_fp_t)(void*, bs_float_t, void*); 
typedef void (*bs_list_map_ip_t)(void*, bs_int32_t, void*);
typedef bs_bool_t (*bs_list_map_cont_t)(void*); 
typedef void (*bs_list_map_acc_t)(void*, void*); 
typedef bs_bool_t (*bs_list_map_acc_cont_t)(void*, void*);
typedef bs_bool_t (*bs_list_unlink_t)(void*);
typedef bs_bool_t (*bs_list_unlink_i_t)(void*, bs_int32_t);
typedef bs_bool_t (*bs_list_unlink_p_t)(void*, void*);
typedef bs_bool_t (*bs_list_unlink_i_cont_t)(void*, bs_int32_t, bs_bool_t*);
typedef bs_bool_t (*bs_list_unlink_p_cont_t)(void*, void*, bs_bool_t*);

struct bs_list_pooled_element_st 
{
  bs_list_pooled_element_t *pNext; 
}; 

struct bs_list_factory_st
{
  bs_list_pooled_element_t *pNodePool;   
  bs_list_pooled_element_t *pListPool;
}; 

bs_list_factory_t*
bs_list_factory_create(); 

void 
bs_list_factory_destroy(bs_list_factory_t *pFactory); 

bs_list_t* 
bs_list_factory_get_list(bs_list_factory_t *pFactory); 

void 
bs_list_factory_give_list(bs_list_factory_t *pFactory, 
                          bs_list_t *pList); 

struct bs_list_st 
{
  bs_list_node_t *pHead;
  bs_list_node_t *pTail; 
  bs_list_factory_t *pFactory;   
  bs_int32_t length; 
}; 

struct bs_list_node_st 
{
  bs_list_node_t *pNext; 
  bs_list_node_t *pPrev;
  void *pUserData;
  bs_list_element_destuctor_t destructor; 
}; 

#define BS_LIST_GET_LENGTH(LIST) \
  ((LIST)->length)

bs_list_t* 
bs_list_create();

void 
bs_list_destroy(bs_list_t *pList); 

void 
bs_list_add_element_to_front(bs_list_t *pList, 
                             void *pUserData, 
                             bs_list_element_destuctor_t destructor); 

void 
bs_list_add_element_to_back(bs_list_t *pList, 
                            void *pUserData, 
                            bs_list_element_destuctor_t destructor); 

void* 
bs_list_get_first_element(bs_list_t *pList); 

void* 
bs_list_get_last_element(bs_list_t *pList); 

bs_int32_t 
bs_list_get_length(bs_list_t *pList); 

void 
bs_list_map(bs_list_t *pList, 
            bs_list_map_t map); 

void
bs_list_map_f(bs_list_t *pList, 
              bs_list_map_f_t fMap, 
              bs_float_t floatArg); 

void
bs_list_map_i(bs_list_t *pList, 
              bs_list_map_i_t fMap, 
              bs_int32_t intArg); 

void
bs_list_map_p(bs_list_t *pList, 
              bs_list_map_p_t pMap, 
              void *pDataArg); 
              
void
bs_list_map_pp(bs_list_t *pList, 
               bs_list_map_pp_t ppMap, 
               void *pDataArgA, 
               void *pDataArgB); 

void
bs_list_map_p_cont(bs_list_t *pList, 
                   bs_list_map_p_cont_t pMapCont, 
                   void *pDataArg); 

void
bs_list_map_pp_cont(bs_list_t *pList, 
                    bs_list_map_pp_cont_t ppMapCont, 
                    void *pDataArgA, 
                    void *pDataArgB); 

void
bs_list_map_fp(bs_list_t *pList, 
               bs_list_map_fp_t fpMap,
               bs_float_t floatArg,                
               void *pDataArg); 

void
bs_list_map_ip(bs_list_t *pList, 
               bs_list_map_ip_t ipMap,
               bs_int32_t intArg,                
               void *pDataArg); 

void 
bs_list_map_cont(bs_list_t *pList, 
                 bs_list_map_cont_t mapCont); 

void 
bs_list_map_acc(bs_list_t *pList, 
                bs_list_map_acc_t mapAcc, 
                void *pAccumulator); 

void 
bs_list_map_acc_cont(bs_list_t *pList, 
                     bs_list_map_acc_cont_t mapAccCont, 
                     void *pAccumulator); 

void 
bs_list_unlink(bs_list_t *pList, 
               bs_list_unlink_t unlink); 
                     
void 
bs_list_unlink_i(bs_list_t *pList, 
                 bs_list_unlink_i_t iUnlink, 
                 bs_int32_t intArg); 

void 
bs_list_unlink_p(bs_list_t *pList, 
                 bs_list_unlink_p_t pUnlink, 
                 void *pDataArg); 

void 
bs_list_unlink_i_cont(bs_list_t *pList, 
                      bs_list_unlink_i_cont_t iContUnlink, 
                      bs_int32_t intArg); 

void 
bs_list_unlink_p_cont(bs_list_t *pList, 
                      bs_list_unlink_p_cont_t pContUnlink, 
                      void *pDataArg); 

#endif
