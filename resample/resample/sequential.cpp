#include "sequential.h"
#include "utils.h"
#include "stdlib.h"
#include "control.h"
#include "profiler.h"
#include "groups.h"
#include "CL/cl.h"

////////////////// RESAMPLE USING POLYNOMIAL APPROXIMATION /////////////////
int exSeq_Resample(ResultsStruct* results)
{
	return 0;
}

// Calculate Q and R matrixes for QR Decomposition of matrix
void QR(cl_float* R, cl_float* Q, cl_uint arrayWidth, cl_uint arrayHeight)
{
	cl_float a;
	cl_float b;
	cl_float c;
	cl_float s;
	cl_float r;
	cl_float Rnew1[2048];
	cl_float Rnew2[2048];
	cl_float Qnew1[2048];
	cl_float Qnew2[2048];
	for (int j = 0; j < arrayWidth; j++)
	{
		for (int i = arrayHeight - 1; i > j; i--)
		{
			// Calculate Givens rotations
			a = R[arrayWidth * (i - 1) + j];
			b = R[arrayWidth * i + j];
			r = sqrt(a * a + b * b);
			c = a / r;
			s = -b / r;
			// Zero out elements in R matrix
			for (int k = j; k < arrayWidth; k++)
			{
				Rnew1[k] = R[arrayWidth * (i - 1) + k] * c - R[arrayWidth * i + k] * s;
				Rnew2[k] = R[arrayWidth * (i - 1) + k] * s + R[arrayWidth * i + k] * c;
			}
			// Copy new values back to R matrix
			for (int k = j; k < arrayWidth; k++)
			{
				R[arrayWidth * (i - 1) + k] = Rnew1[k];
				R[arrayWidth * i + k] = Rnew2[k];
			}
			// Update Q matrix
			for (int k = 0; k < arrayHeight; k++)
			{
				Qnew1[k] = Q[arrayHeight * (i - 1) + k] * c + Q[arrayHeight * i + k] * s;
				Qnew2[k] = -Q[arrayHeight * (i - 1) + k] * s + Q[arrayHeight * i + k] * c;
			}
			for (int k = 0; k < arrayHeight; k++)
			{
				Q[arrayHeight * (i - 1) + k] = Qnew1[k];
				Q[arrayHeight * i + k] = Qnew2[k];
			}
		}
	}
}

int exSeq_QRD(ResultsStruct* results)
{
	const cl_uint arrayWidth = 3;
	const cl_uint arrayHeight = 5;
	cl_uint numIter = 10000; // Number of iterations for runtime averaging

							 // allocate working buffers. 
							 // the buffer should be aligned with 4K page and size should fit 64-byte cached line
	cl_uint optimizedSize = ((sizeof(cl_float) * arrayWidth * arrayHeight - 1) / 64 + 1) * 64;
	cl_float* A = (cl_float*)_aligned_malloc(optimizedSize, 4096);

	optimizedSize = ((sizeof(cl_float) * arrayHeight * arrayHeight - 1) / 64 + 1) * 64;
	cl_float* Q = (cl_float*)_aligned_malloc(optimizedSize, 4096);

	cl_float Atmp[] = { 0.8147, 0.0975, 0.1576,
		0.9058, 0.2785, 0.9706,
		0.1270, 0.5469, 0.9572,
		0.9134, 0.9575, 0.4854,
		0.6324, 0.9649, 0.8003 };

	cl_float Qtmp[] = { 1.0,   0,   0,   0,   0,
		0, 1.0,   0,   0,   0,
		0,   0, 1.0,   0,   0,
		0,   0,   0, 1.0,   0,
		0,   0,   0,   0, 1.0 };

	if (NULL == A)
	{
		LogError("Error: _aligned_malloc failed to allocate buffers.\n");
		return -1;
	}

	// Initialize A
	for (int i = 0; i < arrayWidth * arrayHeight; i++)
	{
		A[i] = Atmp[i];
	}

	// Initialize Q
	for (int i = 0; i < arrayHeight * arrayHeight; i++)
	{
		Q[i] = Qtmp[i];
	}

	// add
	ProfilerStruct profiler;
	profiler.Start();

	QR(A, Q, arrayWidth, arrayHeight);

	profiler.Stop();
	float runTime = profiler.Log();

	_aligned_free(A);

	results->WindowsRunTime = (double)runTime;
	results->HasWindowsRunTime = true;
	return 0;
}

// Solves for coefficients in equation R*x=Q*b where
// R and Q are from QR Decomposition
// x is desired coefficients
// b is column vector of sampled data
// @param[in] R upper triangular matrix of size MxN (from QR Decomposition)
// @param[in] Qtb Q*b column vector of size Mx1
// @param[in] Dim coefficients to solve for - should match N columns size of R
// @param[out] Result is where x is stored
void BackSub(cl_float* R, cl_float* Qtb, size_t Dim, cl_float* Result)
{
	// Start at the Nth row in the right-triangular matrix R where we effectively have R[n][n] * x[n] = Qtb[n]
	// Work our way up solving for each x[n] value in reverse, allowing us to solve for each row as we work our way up
	for (size_t resultIdx = Dim - 1; resultIdx >= 0; --resultIdx)
	{
		const size_t RRowStartIdx = resultIdx*Dim; // since R is indexed as an array instead of a matrix

		// sum up product of remaining row of R with known x values
		cl_float subtractSum = 0;
		for (size_t sumIdx = resultIdx + 1; sumIdx < Dim; sumIdx++)
			subtractSum += R[RRowStartIdx + sumIdx] * Result[sumIdx];

		// calculate final result eg: if ax + b = c, then x = (c-b)/a
		if(Result)
			Result[resultIdx] = (Qtb[resultIdx] - subtractSum) / R[RRowStartIdx + resultIdx];
	}
}

// Tests BackSub
int Test_BackSub(ResultsStruct* results)
{
	// get R matrix
	// get column vector product of Q matrix and b column vector
	// run BackSub function to solve for coefficients
	// store coefficients in ControlObject
	return -1;
}

// Sequential evaluation of polynomial Test
// Evauluates data in ControlObject 
int exSeq_PolyEval(ResultsStruct* results)
{
	// get coeffs from ControlObject
	// get sample_rate_output_
	// calculate output data
	// write to output file
	return -1;
}
