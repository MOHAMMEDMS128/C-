
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


void TBB_julia(
	const complex<float>  C,
	const UINT imageHeight = 2500,
	const UINT imageWidth = 2500,
	const UINT MAX_ITERATIONS = 1000,
	const float limit = 2.0f) {

	//start the timer (capture the current system time )
	tick_count t0 = tick_count::now();

	// Setup output image array
	fipImage outputImage;
	outputImage = fipImage(FIT_BITMAP, imageHeight, imageWidth, 24);

	UINT bytesPerElement = 3;
	BYTE* outputBuffer = outputImage.accessPixels();

	cout << "Processing...\n";

	parallel_for(

		blocked_range2d<UINT, UINT>(0, imageHeight, 0, imageWidth),

		[=](const blocked_range2d<UINT, UINT>& r) {



		auto y1 = r.rows().begin();
		auto y2 = r.rows().end();
		auto x1 = r.cols().begin();
		auto x2 = r.cols().end();

		for (auto y = y1; y < y2; ++y) {

			for (auto x = x1; x < x2; ++x) {


				complex<float> Z;
				//calculate the initial real and imaginary part of z, based on the pixel location and zoom and position values
				Z = complex<float>(-limit + 2.0f * limit / float(imageWidth) * x, -limit + 2.0f * limit / float(imageHeight) * y);

				int i;
				//start the iteration process , i will represent the number of iterations
				for (i = 0; i < MAX_ITERATIONS; i++) {
					Z = Z * Z + C;
					//if the point is outside the circle with radius 3: stop
					if (abs(Z) > 3) break;


				}
				// START COLOURING 
				auto outputElementPtr = outputBuffer + ((y * imageHeight) + x) * 3;

				if (i == MAX_ITERATIONS) {


					outputElementPtr[0] = (75, 0, 130);
					outputElementPtr[1] = (14, 0, 225);
					outputElementPtr[2] = (143, 0, 225);
				}
				else {

					outputElementPtr[0] = i % (225, 127, 0);
					outputElementPtr[1] = i + (25, 17, 26);
					outputElementPtr[2] = i + (237, 57, 91);

				}


			}
		}
	}
	);


	std::cout << "Saving image...\n";

	outputImage.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
	outputImage.convertTo24Bits();
	outputImage.save("TBB_Version.png");

	std::cout << "...done\n\n";

	//Get system time after task (capture the current system time )
	tick_count t1 = tick_count::now();
	// Convert time to seconds (substract t1 - t0)
	cout << "Duration of the Julia set = " << (t1 - t0).seconds() << " seconds\n";
}

int main(void) {



	std::complex<long double> c(-0.74543, 0.11301f);
	TBB_julia(c);
}
