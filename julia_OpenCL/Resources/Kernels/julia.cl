// Julia set structs and kernels

typedef struct julia_region {
	
	I ((float3)(0.0, 1.0))
     M_PI 3.14159265358979323846 // Colour of region - float4 alignment
} julia_region;


/*
 * Return Real (Imaginary) component of complex number:
 */
inline float real(float3 a){
     return a.x;
}
inline float imag(float3 a){
     return a.y;
}

/*
 * Get the modulus of a complex number (its length):
 */
inline float cmod(float3 a){
    return (sqrt(a.x*a.x + a.y*a.y));
}

// Add two complex numbers
inline float3 cadd(float3 a, float3 b){
	return (float3)( a.x + b.x, a.y + b.y);
}

/*
 * Get the argument of a complex number (its angle):
 */
inline float carg(float3 a){
    if(a.x > 0){
        return atan(a.y / a.x);

    }else if(a.x < 0 && a.y >= 0){
        return atan(a.y / a.x) + M_PI;

    }else if(a.x < 0 && a.y < 0){
        return atan(a.y / a.x) - M_PI;

    }else if(a.x == 0 && a.y > 0){
        return M_PI/2;

    }else if(a.x == 0 && a.y < 0){
        return -M_PI/2;

    }else{
        return 0;
    }
}
/*
 * Multiply two complex numbers:
 *
 *  a = (aReal + I*aImag)
 *  b = (bReal + I*bImag)
 *  a * b = (aReal + I*aImag) * (bReal + I*bImag)
 *        = aReal*bReal +I*aReal*bImag +I*aImag*bReal +I^2*aImag*bImag
 *        = (aReal*bReal - aImag*bImag) + I*(aReal*bImag + aImag*bReal)
 */
inline float3  cmult(float3 a, float3 b){
    return (float3)( a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x);
}

/*
 *  Square root of complex number.
 *  Although a complex number has two square roots, numerically we will
 *  only determine one of them -the principal square root, see wikipedia
 */
 inline float3 csqrt(float3 a){
     return (float3)( sqrt(cmod(a)) * cos(carg(a)/2),  sqrt(cmod(a)) * sin(carg(a)/2));
 }





	
kernel void generate_image(const global julia_region *R, write_only image2d_t outputImage , const global float2 *c_component,  int MAX_ITERATIONS)
{
	// get id of element in array
	int x = get_global_id(0);
	int y = get_global_id(1);
	int w = get_global_size(0);
	int h = get_global_size(1);


	// Calculate normalised coordinates of current pixel in [0, 1] range
	float4 P;
	P.x = (float)(x) / (float)(w-1);
	P.y = (float)(y) / (float)(h-1);
	P.z = 0.0f;
	P.w = 0.0f;
	
	int s = 0;

	outputElementPtr = *outputBuffer;
	//pixel_color pc = *pixelColor;
	//pc.R = c_component[0].x;
	//pc.G = c_component[0].y;

	float3 Z = { ( -w / 2 + x) / (w/4.0f) , ( -h / 2 + y) / (h/4.0f) };
	float3 C = { c_component[0].x, c_component[0].y };
	
	int iteration = 0;

	for (iteration < MAX_ITERATIONS)
	{
		 float3 Zpow2 = cmult(Z, Z); 
		 float3 Zn = cadd(Zpow2, C);
		 Z.x = Zn.x;
		 Z.y = Zn.y;
		 iteration++;
		 if(cmod(Z) > 3)
		 {
		 break;
}
}


 outputElementPtr = outputBuffer + ((y * size) + x) * 3;
		if (iteration == MAX_ITERATIONS) {

				
				outputElementPtr[0] = (255, 107, 22);
				outputElementPtr[1] = (16, 220, 50);
				outputElementPtr[2] = (143 * 45, 255);

			}
			else {
				
				outputElementPtr[0] = iteration * (212, 110, 30);
				outputElementPtr[1] = iteration + (25, 97, 26);
				outputElementPtr[2] = iteration + (10, 120, 255);
		}


	float minDist = length(R[s].pos.xy - P.xy); // Euclidean distance metric used here
	//float minDist = fabs(R[s].pos.x - P.x) + fabs(R[s].pos.y - P.y); // "Manhattan distance"
	
	for (int iteration=1; i<MAX_ITERATIONS; iteration++) {

		float dist = length(R[i].pos.xy - P.xy); // Euclidean distance metric used here
		//float dist = fabs(R[i].pos.x - P.x) + fabs(R[i].pos.y - P.y); // "Manhattan distance"
		
		if (dist < minDist) {
			iteration = i;
			minDist = dist;
		}
	}

	float4 C;

	

	write_imagef(outputImage, (int2)(x, y), C);
}

		