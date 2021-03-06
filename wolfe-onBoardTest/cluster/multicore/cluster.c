#include "rt/rt_api.h"
#include "stdio.h"
#include "plp_math.h"
#include "fann.h"
#include "test_data.h"
#include "fann_conf.h"


#define PERF_COUNTER

// for taking measurements on board
#define NUM_REPEAT 5
#define WARM_CACHE 2

RT_CL_DATA fann_type local_data_buffer[2][NUM_INPUT];
int buff_index = 0;

void cluster_entry(void *arg){

  //printf("(%d, %d) Hello! Cluster entered\n", rt_cluster_id(), rt_core_id());


    fann_type *calc_out;

    int sum_cycles = 0;
    int sum_instr = 0;

    // Transfer NUM_INPUT data to L1
    rt_dma_copy_t id_in;
    int datasize = sizeof(fann_type);

    // Prologue for the loop, we need to fetch one buffer
    rt_dma_memcpy((int)test_data_input, (int)local_data_buffer[buff_index], NUM_INPUT*datasize, RT_DMA_DIR_EXT2LOC, 0, &id_in);
//printf("local_data_buffer[2][NUM_INPUT] %x, NUM_INPUT*datasize %d, id_in %d\n", &local_data_buffer[buff_index], NUM_INPUT*datasize, id_in);
    buff_index ^= 1;

#ifdef NUM_REPEAT
    buff_index ^= 1;
    rt_dma_wait(&id_in);
#endif

    int corr = 0;
    int i = 0;
#ifdef NUM_REPEAT
    for(i = 0; i < NUM_REPEAT; ++i) {
#else
    for(i = 0; i < NUM_TESTS; ++i) {
			  // note: the test data has been rescaled offline. For a real application don't forget to scale the input data by MULTIPLIER!

      // Wait for previous iteration input transfer. This is supposed to be already finished if processing is long enough
      rt_dma_wait(&id_in);

      // Enqueue the input buffer transfer for the next iteration so that the DMA transfers it while we do computation
      rt_dma_memcpy((int)(test_data_input + (NUM_INPUT * (i+1))), (int)local_data_buffer[buff_index], NUM_INPUT*datasize, RT_DMA_DIR_EXT2LOC, 0, &id_in);
      buff_index ^= 1;
#endif

#ifdef PERF_COUNTER

      rt_perf_t perf;
      rt_perf_init(&perf);

//  do_bench_0(&perf, (1<<RT_PERF_CYCLES) | (1<<RT_PERF_INSTR));

      rt_perf_conf(&perf, (1<<RT_PERF_CYCLES) | (1<<RT_PERF_INSTR));
  //rt_perf_conf(&perf, (1<<RT_PERF_IMISS)); //75
  //rt_perf_conf(&perf, (1<<RT_PERF_TCDM_CONT));
  //rt_perf_conf(&perf, (1<<RT_PERF_INSTR));
  //rt_perf_conf(&perf, (1<<RT_PERF_ACTIVE_CYCLES));

  rt_perf_reset(&perf);
  rt_perf_start(&perf);

#endif

  calc_out = fann_run(local_data_buffer[buff_index]);


#ifdef PERF_COUNTER

  rt_perf_stop(&perf);


//  printf("Total cycles: %d\n", rt_perf_read(RT_PERF_CYCLES));
//  printf("Instructions: %d\n", rt_perf_read(RT_PERF_INSTR));

  if (i >= WARM_CACHE) {
    sum_cycles += rt_perf_read(RT_PERF_CYCLES);
    sum_instr += rt_perf_read(RT_PERF_INSTR);
  }

  //printf("imiss stalls: %d\n", rt_perf_read(RT_PERF_IMISS));
  //printf("imiss stalls: %d\n", rt_perf_read(RT_PERF_TCDM_CONT));

#endif

#ifndef NUM_REPEAT
int cla = 0;
	if ((calc_out[0] > calc_out[1]) && (calc_out[0] > calc_out[2])) {
	    cla = 0;
	} else
	if ((calc_out[1] > calc_out[0]) && (calc_out[1] > calc_out[2])) {
	    cla = 1;
	} else {
	    cla = 2;
	}

        if (cla == test_data_output[i]) {
            ++corr;
        }
#endif
        
    }

#ifdef NUM_REPEAT
    printf("#### NUM_INPUT_multiriscy %d\n", NUM_INPUT);
    printf("#### NUM_OUTPUT_multiriscy %d\n", NUM_OUTPUT);
    printf("#### mean_cycles_multiriscy %d\n", sum_cycles/(NUM_REPEAT-WARM_CACHE));
    printf("#### mean_instr_multiriscy %d\n", sum_instr/(NUM_REPEAT-WARM_CACHE));

#else
    // Wait for previous iteration input transfer. This is supposed to be already finished if processing is long enough
    rt_dma_wait(&id_in);

    //printf("mean cycles over num test is %d, mean instr is %d\n", sum_cycles/NUM_TESTS, sum_instr/NUM_TESTS);

    //printf("#### run on multiriscy\n");
    printf("#### NUM_INPUT_multiriscy %d\n", NUM_INPUT);
    printf("#### NUM_OUTPUT_multiriscy %d\n", NUM_OUTPUT);
    printf("#### mean_cycles_multiriscy %d\n", sum_cycles/(NUM_TESTS-WARM_CACHE));
    printf("#### mean_instr_multiriscy %d\n", sum_instr/(NUM_TESTS-WARM_CACHE));

    //printf("correct: %d out of %d\n", corr, NUM_TESTS);
#endif


}


