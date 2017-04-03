#include <stdio.h>

#include <CL/cl.h>

#define MAX_NAME_LEN    1000
#define MEM_SIZE        128
#define MAX_SOURCE_SIZE 0x100000

void print_platform_device();


int main(int argc, char **argv)
{
    print_platform_device();

    cl_uint plat_count, dev_count;
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_program program;
    cl_kernel kernel;
    cl_command_queue command_queue;
    cl_mem memobj;
    cl_int ret;

    char string[MEM_SIZE];

    FILE *fp;
    char fileName[] = "./main.cl";
    char *source_str;
    size_t source_size;

    /* Load the source code containing the kernel*/
    fp = fopen(fileName, "r");
    if (!fp) {  
        fprintf(stderr, "Failed to load kernel.\n");  
        exit(1);  
    }
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);

    /* Get platform and device */
    clGetPlatformIDs(1, &platform, &plat_count);
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, &dev_count);

    /* Create OpenCL context */
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &ret);
    
    /* Create Command Queue */
    command_queue = clCreateCommandQueue(context, device, 0, &ret);

    /* Create Kernel Program from the source */
    program = clCreateProgramWithSource(context, 1, 
        (const char **)&source_str,(const size_t *)&source_size, &ret); 
    
    /* Build Kernel Program */
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    
    /* Create OpenCL Kernel */
    kernel = clCreateKernel(program, "hello", &ret);

    /* Create Memory Buffer */
    memobj = clCreateBuffer(context, CL_MEM_READ_WRITE, MEM_SIZE * sizeof(char), 0, &ret);
    
    /* Set OpenCL Kernel Parameters */  
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&memobj);  
    
    /* Execute OpenCL Kernel */  
    ret = clEnqueueTask(command_queue, kernel, 0, NULL,NULL);  
    
    /* Copy results from the memory buffer */
    ret = clEnqueueReadBuffer(command_queue, memobj, CL_TRUE, 0,  
            MEM_SIZE * sizeof(char),string, 0, NULL, NULL);

    /* Display Result */  
    puts(string); 

    /* Finalization */  
    ret = clFlush(command_queue);  
    ret = clFinish(command_queue);  
    ret = clReleaseKernel(kernel);  
    ret = clReleaseProgram(program);  
    ret = clReleaseMemObject(memobj);  
    ret = clReleaseCommandQueue(command_queue);  
    ret = clReleaseContext(context);  
                                 
    free(source_str);  
                                        
	return 0;
}


void print_platform_device()
{
    cl_uint plat_count;
    clGetPlatformIDs(0, NULL, &plat_count);

    cl_platform_id *platforms = 
        (cl_platform_id *) malloc(plat_count*sizeof(cl_platform_id));
    clGetPlatformIDs(plat_count, platforms, NULL);
    
    for (cl_uint i = 0; i < plat_count; ++i)
    {
        char buf[MAX_NAME_LEN];
        clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, 
            sizeof(buf), buf, NULL);
        printf("platform %d: vendor '%s'\n", i, buf);
    
        // get devices in platform
        cl_uint dev_count;
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 
            0, NULL, &dev_count);
        
        cl_device_id *devices = 
            (cl_device_id *) malloc(dev_count*sizeof(cl_device_id));
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL,
            dev_count, devices, NULL);

        for (cl_uint j = 0; j < dev_count; ++j)
        {
            char buf[MAX_NAME_LEN];
            clGetDeviceInfo(devices[j], CL_DEVICE_NAME,
                sizeof(buf), buf, NULL);
            printf("  device %d: '%s'\n", j, buf);
        }

        free(devices);
    }
    
    free(platforms);
}
