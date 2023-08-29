#include <stdio.h>
#include </home/nvidia/users/mini/CudaFunc.h>
#include <cuda_runtime.h>
#include <iostream>
#include <chrono>

int root=3 ;
int framefactor=2 ;
int square=9 ;	
int height=4;
int width=2;

__global__ void multi_add(char* d_i,char* d_o,int width,int height,int numpartitions){
		int with = width ;
		int hith = height ;
		int local = 0 ;

		for(int i=0;i<2*with;i++){
			local=0 ;
			for(int k=0;k<numpartitions;k++){
				local+=(int)d_i[blockIdx.x*hith*with*numpartitions+k*hith*with+2*with*threadIdx.x+i]  ;
				//printf("%d ",(int)d_i[blockIdx.x*hith*with*numpartitions+k*hith*with+2*with*threadIdx.x+i]) ;
			}
			local/=numpartitions ;
			d_o[(blockIdx.x*hith*with)+2*with*threadIdx.x+i]=(char)local ;
		}
		__syncthreads();
}

void AvgCalGpu(int aheight,int awidth,int aroot,int aframefactor, char *converted_arr[],char sum_arr[]){
	root = aroot ;
	square= root*root ;
	framefactor = aframefactor ;
	height = aheight ;
	width = awidth ;
	
	char temp_converted_arr[framefactor*root*height*width] ;

	char *d_i;
	char *d_o;
	char *d_s;
	
	cudaMalloc((void **)&d_i, height*width*sizeof(char)*square);
	cudaMalloc((void **)&d_o, height*width*sizeof(char)*root);
	for(int r=0;r<framefactor;r++){
		char* lp = converted_arr[r] ;
		cudaMemcpy(d_i,lp,height*width*sizeof(char)*square,cudaMemcpyHostToDevice);
		multi_add<<<root,height/2>>>(d_i,d_o,width,height,root);
		cudaMemcpy(temp_converted_arr+root*height*width*r,d_o,height*width*sizeof(char)*root,cudaMemcpyDeviceToHost);
	}

	cudaFree(d_i) ;
	cudaFree(d_o) ;
	cudaMalloc((void **)&d_i, height*width*sizeof(char)*root*framefactor);	
	cudaMalloc((void **)&d_o, height*width*sizeof(char)*1*framefactor);
	
	cudaMemcpy(d_i,temp_converted_arr,height*width*sizeof(char)*root*framefactor,cudaMemcpyHostToDevice);	
	multi_add<<<framefactor,height/2>>>(d_i,d_o,width,height,root);
	cudaFree(d_i) ;
	
	cudaMalloc((void **)&d_s, height*width*sizeof(char)*1);
	multi_add<<<1,height/2>>>(d_o,d_s,width,height,framefactor);	
	cudaMemcpy(sum_arr,d_s,height*width*sizeof(char)*1,cudaMemcpyDeviceToHost);
	cudaFree(d_s) ;
	cudaFree(d_o) ;

}
