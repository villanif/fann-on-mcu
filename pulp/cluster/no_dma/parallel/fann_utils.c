//Copyright (c) 2018 ETH Zurich, Xiaying Wang, Ferdinand von Hagen, Lukas Cavigelli, Michele Magno

#include "fann_conf.h"
#include "fann_utils.h"
#include "rt/rt_api.h"
#include "plp_math.h"

/*
  typedef struct{
  int32_t * pSrcA;     // pointer to the first vector
  int32_t * pSrcB;     // pointer to the second vector
  uint32_t blkSizePE;     // number of samples in each vector
  uint32_t deciPoint;    // decimal point for right shift
  uint32_t nPE;        // number of processing units
  int32_t * resBuffer;      // pointer to result vector
  uint32_t activation_function;
  } dot_prod_activations_instance;
  // also parameters for each stepwise pieces, not worthy.
  */

void dot_prod_entry(void * S){

  //uint32_t nPE = ((plp_dot_prod_instance_q32 *)S)->nPE;
  uint32_t blkSizePE = ((plp_dot_prod_instance_q32 *)S)->blkSizePE;
  int core_id = rt_core_id();
  int32_t * pSrcA = (int32_t*)(((plp_dot_prod_instance_q32 *)S)->pSrcA) + blkSizePE * core_id;
  int32_t * pSrcB = (int32_t*)(((plp_dot_prod_instance_q32 *)S)->pSrcB); // + blkSizePE * core_id;
  
  uint32_t deciPoint = ((plp_dot_prod_instance_q32 *)S)->deciPoint;
  
  int32_t * resBufferPE = &(((plp_dot_prod_instance_q32 *)S)->resBuffer[core_id]);


  //rt_team_barrier();

  plp_dot_prod_q32s_xpulpv2(pSrcA, pSrcB, blkSizePE, deciPoint, resBufferPE);

  //printf("coreid %d, pSrcA %d, pSrcB %d, resbufferpe %d\n", rt_core_id(), pSrcA, pSrcB, resBufferPE[0]);


}


void compute_per_layer_parallel(int32_t * pSrcA, int32_t * pSrcB, uint32_t blockSize, uint32_t deciPoint, uint32_t nPE, int32_t * resBuffer){

  plp_dot_prod_instance_q32 S;
  S.pSrcA = pSrcA;
  S.pSrcB = pSrcB;
  S.blkSizePE = blockSize;
  S.deciPoint = deciPoint;
  //S.nPE = nPE;
  S.resBuffer = resBuffer;

  rt_team_fork(nPE, dot_prod_entry, (void *) &S);

  //rt_team_barrier();
}


void dot_prod_entry_f32(void * S){

  uint32_t blkSizePE = ((plp_dot_prod_instance_f32 *)S)->blkSizePE;
  int core_id = rt_core_id();
  float32_t * pSrcA = (int32_t*)(((plp_dot_prod_instance_f32 *)S)->pSrcA) + blkSizePE * core_id;
  float32_t * pSrcB = (int32_t*)(((plp_dot_prod_instance_f32 *)S)->pSrcB);
  
  float32_t * resBufferPE = &(((plp_dot_prod_instance_f32 *)S)->resBuffer[core_id]);

  plp_dot_prod_f32s_xpulpv2(pSrcA, pSrcB, blkSizePE, resBufferPE);

}

void compute_per_layer_parallel_f32(float32_t * pSrcA, float32_t * pSrcB, uint32_t blockSize, uint32_t nPE, float32_t * resBuffer)
{

  plp_dot_prod_instance_f32 S;
  S.pSrcA = pSrcA;
  S.pSrcB = pSrcB;
  S.blkSizePE = blockSize;
  //S.nPE = nPE;
  S.resBuffer = resBuffer;

  rt_team_fork(nPE, dot_prod_entry_f32, (void *) &S);

  //rt_team_barrier();
}

/*
  compute_per_layer_parallel_activations(int32_t * pSrcA, int32_t * pSrcB, uint32_t blockSize, uint32_t deciPoint, uint32_t nPE, int32_t * resBuffer, uint32_t activation_function){

  plp_dot_prod_instance_q32 S;
  S.pSrcA = pSrcA;
  S.pSrcB = pSrcB;
  S.blkSizePE = blockSize;
  S.deciPoint = deciPoint;
  //S.nPE = nPE;
  S.resBuffer = resBuffer;
  S.activation_function = activation_function;


  rt_team_fork(nPE, dot_prod_entry, (void *) &S);
  }
  // too many parameters to be passed, not worthy.
  */



