// Julia set structs and kernels

typedef struct julia_region {
	
	I ((cfloat)(0.0, 1.0))
     M_PI 3.14159265358979323846 // Colour of region - float4 alignment
} julia_region;


/*
 * Return Real (Imaginary) component of complex number:
 */
inline float real(cfloat a){
     return a.x;
}
inline float imag(cfloat a){
     return a.y;
}

/*
 * Get the modulus of a complex number (its length):
 */
inline float cmod(cfloat a){
    return (sqrt(a.x*a.x + a.y*a.y));
}

// Add two complex numbers
inline cfloat cadd(cfloat a, cfloat b){
	return (cfloat)( a.x + b.x, a.y + b.y);
}

/*
 * Get the argument of a complex number (its angle):
 */
inline float carg(cfloat a){
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
inline cfloat  cmult(cfloat a, cfloat b){
    return (cfloat)( a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x);
}

/*
 *  Square root of complex number.
 *  Although a complex number has two square roots, numerically we will
 *  only determine one of them -the principal square root, see wikipedia
 */
 inline cfloat csqrt(cfloat a){
     return (cfloat)( sqrt(cmod(a)) * cos(carg(a)/2),  sqrt(cmod(a)) * sin(carg(a)/2));
 }





	
kernel void generate_image(const global julia_region *R, write_only image2d_t outputImage , const global float2 *c_component,  int MAX_ITERATIONS)
{
	// get id of element in array
	int x = get_global_id(0);
	int y = get_global_id(1);
	int w = get_global_size(0);
	int h = get_global_size(1);

	//pixel_color pc = *pixelColor;
	//pc.R = c_component[0].x;
	//pc.G = c_component[0].y;

	cfloat Z = { ( -w / 2 + x) / (w/4.0f) , ( -h / 2 + y) / (h/4.0f) };
	cfloat C = { c_component[0].x, c_component[0].y };
	
	int iteration = 0;

	for (iteration < MAX_ITERATIONS)
	{
		 cfloat Zpow2 = cmult(Z, Z); 
		 cfloat Zn = cadd(Zpow2, C);
		 Z.x = Zn.x;
		 Z.y = Zn.y;
		 iteration++;
		 if(cmod(Z) > 3)
		 {
		 break;
		}
	}
}



			if (iteration == MAX_ITERATIONS) {

				
		outputElementPtr[0] = 0.0f;
		outputElementPtr[1] = 0.0f;
		outputElementPtr[2] = 0.0f;
	}
	else
			
		outputElementPtr[0] = 0.0f;
		outputElementPtr[1] = 0.0f;
		outputElementPtr[2] = 1.0f;

	float minDist = length(R[s].pos.xy - P.xy); // Euclidean distance metric used here
	//float minDist = fabs(R[s].pos.x - P.x) + fabs(R[s].pos.y - P.y); // "Manhattan distance"
	
	for (int iteration=1; i<MAX_ITERSTIONS; iteration++) {

		float dist = length(R[i].pos.xy - P.xy); // Euclidean distance metric used here
		//float dist = fabs(R[i].pos.x - P.x) + fabs(R[i].pos.y - P.y); // "Manhattan distance"
		
		if (dist < minDist) {
			iteration = i;
			minDist = dist;
		}
	}

	float4 C;

	C.x = R[s].colour.x;
	C.y = R[s].colour.y;
	C.z = R[s].colour.z;
	C.w = 1.0;

	write_imagef(outputImage, (int2)(x, y), C);
}

		}
	}

	float4 color = (float4)(pc.R, pc.G, pc.B, pc.A);

	write_imagef(outputImage, (int2)(x, y), colour);
}



