#include <stdio.h>
#include <stdlib.h>

#include <CL/cl.h>

/* OpenCL kernel */
const char* programSource =
"__kernel                                   \n"
"void vecadd(__global int *A,               \n"
"            __global int *B,               \n"
"            __global int *C)               \n"
"{                                          \n"
"   int idx = get_global_id(0);             \n"
"                                           \n"                    
"   C[idx] = A[idx] + B[idx];               \n" 
"}                                          \n"
;

int main()
{
    int *A=NULL, *B=NULL, *C=NULL;

    const int elements=2048;

    size_t datasize = sizeof(int)*elements;

    A = (int *)malloc(datasize);
    B = (int *)malloc(datasize);
    C = (int *)malloc(datasize);

    int i;
    for(i = 0; i < elements; i++) {
        A[i] = i;
        B[i] = i;
    }

    cl_int status;

    cl_uint numPlatforms = 0;
    status = clGetPlatformIDs(0, NULL, &numPlatforms);

    cl_platform_id *platforms = NULL;
    platforms = (cl_platform_id *)malloc(numPlatforms * sizeof(cl_platform_id));

    status = clGetPlatformIDs(numPlatforms, platforms, NULL);

    cl_uint numDevices = 0;
    status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices);

    cl_device_id *devices;
    devices = (cl_device_id *)malloc(numDevices * sizeof(cl_device_id));

    status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, numDevices, devices, NULL);

    /* Create a context and associate it with the devices */
    cl_context context;
    context = clCreateContext(NULL, numDevices, devices, NULL, NULL, &status);

    /* Create a command queue and associate it with the device */
    cl_command_queue cmdQueue;
    cmdQueue = clCreateCommandQueue(context, devices[0], 0, &status);
    
    cl_mem bufA;
    bufA = clCreateBuffer(context, CL_MEM_READ_ONLY, datasize, NULL, &status);

    cl_mem bufB;
    bufB = clCreateBuffer(context, CL_MEM_READ_ONLY, datasize, NULL, &status);

    cl_mem bufC;
    bufC = clCreateBuffer(context, CL_MEM_WRITE_ONLY, datasize, NULL, &status);

    status = clEnqueueWriteBuffer(cmdQueue, bufA, CL_FALSE, 0, datasize, A, 0, NULL, NULL);
    status = clEnqueueWriteBuffer(cmdQueue, bufB, CL_FALSE, 0, datasize, B, 0, NULL, NULL);
    
    /* Create a program */
    cl_program program = clCreateProgramWithSource(context, 1,
        (const char **)&programSource, NULL, &status);

    status = clBuildProgram(program, numDevices, devices, NULL, NULL, NULL);

    cl_kernel kernel;
    kernel = clCreateKernel(program, "vecadd", &status);

    /* Associate the buffers with the kernel */
    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufA);
    status = clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufB);
    status = clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufC);

    /* Define an index space of work items */
    size_t globalWorkSize[1];
    globalWorkSize[0] = elements;

    /* Execute the kernel */
    status = clEnqueueNDRangeKernel(cmdQueue, kernel, 1, NULL,
        globalWorkSize, NULL, 0, NULL, NULL);

    /* Read the output */
    clEnqueueReadBuffer(cmdQueue, bufC, CL_TRUE, 0,
        datasize, C, 0, NULL, NULL);

    int result = 1;
    for (i = 0; i < elements; i++) {
        if (C[i] != i+i) {
            result = 0;
            break;
        }
    }

    if (result) {
        printf("Output is correct\n");
    } else {
        printf("Output is incorrect\n");
    }

    /* Free OpenCL resources */
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(cmdQueue);
    clReleaseMemObject(bufA);
    clReleaseMemObject(bufB);
    clReleaseMemObject(bufC);
    clReleaseContext(context);

    /* Free host resources */
    free(A);
    free(B);
    free(C);
    free(platforms);
    free(devices);

    return 0;
}
