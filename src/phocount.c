/*
 * Copyright (c) 2014, Brookhaven Science Associates, Brookhaven        
 * National Laboratory. All rights reserved.                            
 *                                                                      
 * Redistribution and use in source and binary forms, with or without   
 * modification, are permitted provided that the following conditions   
 * are met:                                                             
 *                                                                      
 * * Redistributions of source code must retain the above copyright     
 *   notice, this list of conditions and the following disclaimer.      
 *                                                                      
 * * Redistributions in binary form must reproduce the above copyright  
 *   notice this list of conditions and the following disclaimer in     
 *   the documentation and/or other materials provided with the         
 *   distribution.                                                      
 *                                                                      
 * * Neither the name of the Brookhaven Science Associates, Brookhaven  
 *   National Laboratory nor the names of its contributors may be used  
 *   to endorse or promote products derived from this software without  
 *   specific prior written permission.                                 
 *                                                                      
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS  
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT    
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS    
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE       
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,           
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES   
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR   
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)   
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,  
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OTHERWISE) ARISING   
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE   
 * POSSIBILITY OF SUCH DAMAGE.                                          
 *
 */

#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

#include "phocount.h"

int count(data_t *in, data_t *out, data_t *stddev, 
          int ndims, index_t *dims, data_t *thresh, int sum_max){
  index_t nimages = dims[0];
  index_t M = dims[ndims-1];
  index_t N = dims[ndims-2];
  index_t imsize = N*M;

  int x;
  for(x=1;x<(ndims-2);x++){
    nimages = nimages * dims[x];
  }   

  index_t i;
#pragma omp parallel shared(in, out, stddev) private(i)
  for(i=0;i<nimages;i++){
    // Find the start pointers of the image
    data_t *inp = in + (i*imsize);
    data_t *outp = out + (i*imsize);
    data_t *stddevp = stddev + (i*imsize);

    index_t j;
    index_t start = M + 1;
    index_t stop = imsize - start;
    data_t pixel[9];

    // Clear out the parts of the output array we don't use
    
    for(j=0;j<start;j++){
      *outp = 0;
      *stddevp = 0;
      inp++;
      outp++;
      stddevp++;
    }

    // Now start the search
    for(j=start;j<stop;j++){
      if((*inp >= thresh[0]) && (*inp < thresh[1])){
        // The pixel is above thresh
        // Now get the surrounding 9 pixels. 
        pixel[0] = *inp;
        pixel[1] = *(inp - M - 1);
        pixel[2] = *(inp - M);
        pixel[3] = *(inp - M + 1);
        pixel[4] = *(inp - 1);
        pixel[5] = *(inp + 1);
        pixel[6] = *(inp + M - 1);
        pixel[7] = *(inp + M);
        pixel[8] = *(inp + M + 1);

        // Is this the brightest pixel?
        
        int n;
        int flag = 0;
        for(n=1;n<9;n++){
          if(pixel[n] > pixel[0]){
            flag = 1;
            break;
          }
        }
       
        if(!flag){
          // Sort the array
          sort(pixel, 9);

          data_t sum = 0;
          for(n=0;n<sum_max;n++){
            sum += pixel[n];
          }
          *outp = sum;

          // Now calculate the varience
       
          data_t mean = 0;
          data_t var = 0;
          for(n=0;n<9;n++){
            mean += pixel[n];
          }
          mean = mean / 9;
          for(n=0;n<9;n++){
            var += pow(pixel[n] - mean, 2);
          }

          *stddevp = pow(var / 9, 0.5);
        }
      }

      inp++;
      outp++;
      stddevp++;
    }
    for(j=stop;j<imsize;j++){
      *outp = 0;
      *stddevp = 0;
      inp++;
      outp++;
      stddevp++;
    }
  }


  return 0;
}

void sort(data_t *array, int n){
  int c;
  for (c = 1 ; c <= n - 1; c++) {
    int d = c;
    while(d>0 && array[d] > array[d-1]){
      data_t t = array[d];
      array[d] = array[d-1];
      array[d-1] = t;
      d--;
    }
  }
}