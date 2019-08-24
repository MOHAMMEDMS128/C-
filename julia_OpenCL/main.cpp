#include <cstdio>
#include <iostream>
#include <random>
#include <exception>
#include <complex>
#include <chrono>
#include<ctime>
#include <malloc.h>
#include <CL\opencl.h>
#include "setup_cl.h"
#include <FreeImage\FreeImagePlus.h>

using namespace std;
using namespace std::chrono;

#pragma warning(disable : 4996)

#pragma region Supporting structures to reflect vector types in OpenCL

struct float2 {

	float x, y;

	float2(const float _x, const float _y) : x(_x), y(_y) {}
};

struct float3 {

	float x, y, z;

	float3(const float _x, const float _y, const float _z) : x(_x), y(_y), z(_z) {}
};

struct float4 {

	float x, y, z, w;

	float4(const float _x, const float _y, const float _z, const float _w) : x(_x), y(_y), z(_z), w(_w) {}
};
#pragma endregion
// Custom sturct to model a julia_region
__declspec(align(16)) struct julia_region {

	// Position of region (in normalised image coordinates [0, 1]) - float4 (16 byte) alignment
	__declspec(align(16)) float2	pos;

	// Colour of region - float4 (16 byte) alignment
	__declspec(align(16)) float3	colour;
};

/*
__declspec(align(16)) struct pixel_color {

	// Position of region (in normalised image coordinates [0, 1]) - float4 (16 byte) alignment
	__declspec(align(16)) float2	pos;

	// Colour of region - float4 (16 byte) alignment
	__declspec(align(16)) float3	colour;
};


struct pixel_color {
	float R;
	float G;
	float B;
	float A;
};

pixel_color colors[16];

void InitPixelColors()
{
	pixel_color black = { 0.0f, 0.0f, 0.0f, 255.0f };
	pixel_color blue = { 9.0f, 26.0f, 236.0f, 255.0f };
	pixel_color brown = { 163.0f, 107.0f, 7.0f, 255.0f };
	pixel_color green = { 8.0f, 240.0f, 6.0f, 255.0f };
	pixel_color magenta = { 250.0f, 32.0f, 130.0f, 255.0f };
	pixel_color orange = { 250.0f, 107.0f, 6.0f, 255.0f };
	pixel_color red = { 250.0f, 0.0f, 0.0f, 255.0f };
	pixel_color darkGrey = { 50.0f, 50.0f, 50.0f, 255.0f };
	pixel_color lightBlue = { 50.0f, 170.0f, 200.0f, 255.0f };
	pixel_color lightGreen = { 50.0f, 240.0f, 175.0f, 255.0f };
	pixel_color lightYellow = { 245.0f, 250.0f, 140.0f, 255.0f };
	pixel_color violetRed = { 245.0f, 150.0f, 180.0f, 255.0f };
	pixel_color ivory = { 255.0f, 250.0f, 240.0f, 255.0f };
	pixel_color yellow = { 235.0f, 255.0f, 15.0f, 255.0f };
	pixel_color cyan = { 0.0f, 240.0f, 240.0f, 255.0f };
	pixel_color lime = { 191.0f, 255.0f, 0.0f, 255.0f };

	colors[0] = black;
	colors[1] = blue;
	colors[2] = brown;
	colors[3] = green;
	colors[4] = magenta;
	colors[5] = orange;
	colors[6] = red;
	colors[7] = darkGrey;
	colors[8] = lightBlue;
	colors[9] = lightGreen;
	colors[10] = lightYellow;
	colors[11] = violetRed;
	colors[12] = ivory;
	colors[13] = yellow;
	colors[14] = cyan;
	colors[15] = lime;
}

pixel_color getPixelColor(int val) {

	return colors[val];
}
*/
void julia_set(
	cl_context context,
	cl_device_id device,
	cl_command_queue commandQueue,
	cl_program program,
	float c_component_real_part = -0.7269f,
	float c_component_imaginary_part = 0.1889f,
	int imageWidth = 1024,
	int imageHeight = 1024,
	int MAX_ITERATIONS = 1000)
{


	cl_int err;
	cl_kernel kernel = 0;
	cl_mem outputImage;
	cl_mem				regionBuffer = 0;
	fipImage result;
	julia_region		*vRegions = nullptr;
	
	//cl_mem pixelColorBuffer;

	//pixel_color pixelColor;
	//pixelColor.R = 1.0;
	//pixelColor.G = 1.0;
	//pixelColor.B = 0.0;
	//pixelColor.A = 1.0;
	//cl_mem pixelColorBuffer;

	float2 C_component(c_component_real_part, c_component_imaginary_part);
	cl_mem c_componentBuffer;
	try
	{
		/*const int imageWidth = 1024;
		const int imageHeight = 1024;
		const cl_int MAX_ITERATIONS = 300;
		*/

		// Setup random number engine
		random_device rd;
		mt19937 mt(rd());
		auto D = uniform_real_distribution<float>(0.0f, 1.0f);

		// Create region array in host memory.  Setup seed points for julia graph generation.
		vRegions = (julia_region*)_aligned_malloc(MAX_ITERATIONS * sizeof(julia_region), 16);

		if (!vRegions)
			throw exception("Cannot create voronoi region array in host memory");






		// Create and validate the OpenCL context
		context = createContext();

		if (!context)
			throw exception("Cannot create OpenCL context");


		// 2. Get a list of devices associated with the context
		device = getDeviceForContext(context);

		if (!device)
			throw exception("Cannot obtain valid device ID");


		// Create the command queue
		commandQueue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, 0);

		if (!commandQueue)
			throw exception("Cannot create command queue");



		// Create the program object based on julia_kernels.cl
		cl_program program = createProgram(context, device, "Resources\\Kernels\\julia.cl");

		if (!program)
			throw exception("Cannot create program object");


		// Get the generate_image kernel from program object created above
		kernel = clCreateKernel(program, "julia", 0);

		if (!kernel)
			throw exception("Could not create kernel");



		// Create buffer corresponding to the region array
		//regionBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, MAX_ITERATIONS * sizeof(pixel_color), jRegions, 0);

		//if (!regionBuffer)
		//	throw exception("Cannot create region buffer");



		// Setup output image
		cl_image_format outputFormat;

		outputFormat.image_channel_order = CL_BGRA; // store a BGRA image
		outputFormat.image_channel_data_type = CL_UNORM_INT8;// Each component is 8 bits in the range [0, 1]

		outputImage = clCreateImage2D(context, CL_MEM_WRITE_ONLY, &outputFormat, imageWidth, imageHeight, 0, 0, &err);


		if (!regionBuffer) {
			throw(string("pixel buffer failed"));
		}

		clSetKernelArg(kernel, 0, sizeof(cl_mem), &regionBuffer);
		clSetKernelArg(kernel, 1, sizeof(cl_mem), &outputImage);
		clSetKernelArg(kernel, 3, sizeof(int), &MAX_ITERATIONS);


		// setup worksize arrays
		size_t globalWorkSize[2] = { imageWidth, imageHeight };


		// Setup event (for profiling)
		cl_event juliaEvent;

		// enqueue kernel
		err = clEnqueueNDRangeKernel(commandQueue, kernel, 2, 0, globalWorkSize, 0, 0, 0, &juliaEvent);

		// Block until voronoi kernel finishes and report time taken to run the kernel
		clWaitForEvents(1, &juliaEvent);

		cl_ulong startTime = (cl_ulong)0;
		cl_ulong endTime = (cl_ulong)0;

		clGetEventProfilingInfo(juliaEvent, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &startTime, 0);
		clGetEventProfilingInfo(juliaEvent, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endTime, 0);

		double tdelta = (double)(endTime - startTime);

		std::cout << "Time taken (in seconds) to create the Julia Set = " << (tdelta * 1.0e-9) << endl;


		// Extract the resulting julia set from OpenCL
		result = fipImage(FREE_IMAGE_TYPE::FIT_BITMAP, imageWidth, imageHeight, 32);

		if (!result.isValid())
			throw exception("Cannot create the output image");


		size_t origin[3] = { 0, 0, 0 };
		size_t region[3] = { imageWidth, imageHeight, 1 };

		// err = clEnqueueReadImage(commandQueue, outputImage, CL_TRUE, origin, region, 0, 0, result, 0, 0, 0);

		err = clEnqueueReadImage(commandQueue, outputImage, CL_TRUE, origin, region, 0, 0, result.accessPixels(), 0, 0, 0);
		cout << "Saving image...\n";
		result.convertTo24Bits();
		BOOL saved = result.save("Julia_SetOpenCL.jpg");

		if (!saved)
			throw exception("Cannot save Julia Set");
	}


	catch (exception& err)
	{
		// Output the exception message to the console
		cout << err.what() << endl;

	}

}

void Sequential(int imageWidth, int imageHeight, int MAX_ITERATIONS, double c_component_real_part, double c_component_imaginary_part)
{
	// Get the starting timepoint
	auto start = high_resolution_clock::now();

	cout << "\n\Processing\n" << endl;

	fipImage outputImage;
	outputImage = fipImage(FIT_BITMAP, imageWidth, imageHeight, 24);
	UINT bytesPerElement = 3;
	BYTE* outputBuffer = outputImage.accessPixels();
	const float limit = 2.0f;
	complex<double> Z;
	complex<double> C(c_component_real_part, c_component_imaginary_part);


	for (int i = 0; i < imageHeight; i++)
	{
		for (int f = 0; f < imageWidth; f++)
		{
			//Z = complex<double>((-imageWidth / 2 + j) / (imageWidth / 4.0), (-imageHeight / 2 + i) / (imageHeight / 4.0));
			Z = complex<float>(-limit + 2.0f * limit / float(imageHeight) * f, -limit + 2.0f * limit / float(imageWidth) * i);
			int iteration = 0;

			for (int k = 0; k < MAX_ITERATIONS; k++)
			{
				Z = Z * Z + C;
				iteration++;
				if (abs(Z) > 2)
				{
					break;
				}
			}


			auto outputElementPtr = outputBuffer + ((i * imageWidth) + f) * 3;

			if (iteration == MAX_ITERATIONS) {


				outputElementPtr[0] = (255, 117, 23);
				outputElementPtr[1] = (106, 240, 40);
				outputElementPtr[2] = (14 , 45, 255);

			}
			else {
				
				outputElementPtr[0] = iteration * (212, 110, 30);
				outputElementPtr[1] = iteration + (215, 97, 26);
				outputElementPtr[2] = iteration + (100, 120, 255);
			}

		}
	}

	cout << "Saving image...\n";
	outputImage.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
	outputImage.convertTo24Bits();
	outputImage.save("Julia_Set.png");

	cout << "...done\n\n";
	// Get ending timepoint
	auto stop = high_resolution_clock::now();
	// Get the duration, Substart (start - stop) timepoints  
	auto duration = duration_cast<seconds>(stop - start);

	cout << "Duration of the Julia set function: "
		<< duration.count() << " Seconds" << endl;

}



int main(int argc, char **argv)
{






	int imageWidth = 1024;
	int imageHeight = 1024;

	float c_component_real_part = -0.7269f;

	float c_component_imaginary_part = 0.1889f;

	int MAX_ITERATIONS = 255;


	Sequential(imageWidth, imageHeight, MAX_ITERATIONS, c_component_real_part, c_component_imaginary_part);

	//done


	return 0;
}



/*
// Deallocate resources
clReleaseMemObject(pixelColorBuffer);
clReleaseMemObject(outputImage);
clReleaseMemObject(c_componentBuffer);
clReleaseKernel(kernel);
clReleaseProgram(program);
clReleaseCommandQueue(commandQueue);
clReleaseContext(context);





	return 0;
}
//} */
/*catch (exception& err)
{
	// Output the exception message to the console
	cout << err.what() << endl;

	// Dispose of resources
	clReleaseMemObject(pixelColorBuffer);
	//clReleaseMemObject(outputImage);
	//clReleaseMemObject(c_componentBuffer);
	//clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(commandQueue);
	clReleaseContext(context);


	// Done - report error
	return 1;
}
}*/
