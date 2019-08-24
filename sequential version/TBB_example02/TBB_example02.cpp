
#include <windows.h>
#include <string>
#include <iostream>
#include <random>
#include <cmath>
#include <vector>
#include<chrono>
#include <complex>
#include <tbb/tbb.h>
#include <FreeImage/FreeImagePlus.h>


using namespace std;
using namespace std::chrono;
using namespace tbb;


void sequential_julia
(const complex<float>  C,
	const UINT imageHeight = 2500,
	const UINT imageWidth = 2500,
	const UINT MAX_ITERATIONS = 1000,
	const float limit = 2.0f) {

	// Get the starting timepoint
	auto start = high_resolution_clock::now();


	// Setup output image
	fipImage outputImage;
	outputImage = fipImage(FIT_BITMAP, imageHeight, imageWidth, 24);

	UINT bytesPerElement = 3;
	BYTE* outputBuffer = outputImage.accessPixels();

	complex<float> Z;

	std::cout << "Processing...\n";
	//loop through every pixel y and x
	for (int y = 0; y < imageHeight; y++) {

		cout.flush();

		for (int x = 0; x < imageWidth; x++) {

			//calculate the initial real and imaginary part of z, based on the pixel location and zoom and position values
			Z = complex<float>(-limit + 2.0f * limit / float(imageWidth) * x, -limit + 2.0f * limit / float(imageHeight) * y);

			int i;
			//start the iteration process , i will represent the number of iterations
			for (i = 0; i < MAX_ITERATIONS; i++) {
				Z = Z * Z + C;
				//if the point is outside the circle with radius 2: stop
				if (abs(Z) > 2.0)
					break;


			}
			// START COLOURING 
			auto outputElementPtr = outputBuffer + ((y * imageHeight) + x) * 3;

			if (i == MAX_ITERATIONS) {


				outputElementPtr[0] = (85, 0, 130);
				outputElementPtr[1] = (0, 0, 225);
				outputElementPtr[2] = (143, 0, 225);
			}
			else {

				outputElementPtr[0] = i % (225, 127, 0);
				outputElementPtr[1] = i + (25, 17, 26);
				outputElementPtr[2] = i + (237, 57, 100);


			}


		}
	}

	cout << "Saving image...\n";
	outputImage.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
	outputImage.convertTo24Bits();
	outputImage.save("Sequential_Version.png");

	cout << "...done\n\n";
	// Get ending timepoint
	auto stop = high_resolution_clock::now();
	// Get the duration, Substart (start - stop) timepoints  
	auto duration = duration_cast<seconds>(stop - start);

	cout << "Duration of the Julia set function: "
		<< duration.count() << " Seconds" << endl;

}
int main(void) {



	std::complex<long double> c(-0.7269f, 0.1889f);
	sequential_julia(c);
}
