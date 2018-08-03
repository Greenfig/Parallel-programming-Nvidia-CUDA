#include<iostream>
#include<cstdlib>
#include<cstring>
#include<cstdio>
#include<fstream>
#include<string>
#include<sstream>
#include<chrono>
#include<vector>
#include <cuda_runtime.h>
// to remove intellisense highlighting
#include <device_launch_parameters.h>
#ifndef __CUDACC__
#define __CUDACC__
#endif
#include <device_functions.h>

#define NTPB 1024


using namespace std;

class BruteFactor {
private:
	ifstream fp;
	string Name, digits, sumofdigits;
	unsigned __int64 RSA_NUMBER, p1, p2;
	chrono::high_resolution_clock::time_point start, finish;
	double totalduration;

public:
	BruteFactor();
	BruteFactor(const char* filename);
	void display() const;
	//Calculation Methods
	unsigned __int64 modulus(const unsigned __int64 dividen, const unsigned __int64 divisor);
	void getPrimes();
	//Cuda
	void getPrimesCuda();
	void cudaError(cudaError_t err);
	/////////////////////
	bool isValid(const unsigned __int64 number);
	~BruteFactor();
};

BruteFactor::BruteFactor(){
	cout << endl;
	cout << "No File Loaded...";
}

BruteFactor::BruteFactor(const char* filename){
	string line, File_Name, F_Digits, F_Digit_Sum;
	int counter = 0;


	fp.open(filename, ios::in);
	if (!fp.is_open())
		cerr << '"' << filename << '"' << " Not Loaded..." << endl;
	else{
		cout << "File " << '"' << filename << '"' << " Loaded..." << endl;
		while (getline(fp, line)){
			if (counter == 0){
				stringstream ss;
				ss << atol(line.c_str());
				RSA_NUMBER = stoull(line);
			}
			else if (counter == 1)
				Name = line;
			else if (counter == 2)
				digits = line;
			else if (counter == 3)
				sumofdigits = line;
			counter++;
		}
	}
	p1 = 0;
	p2 = 0;
	cout << endl;

}

//////////////////////////////////////////////////////////////
///////////////////////////Kernel/////////////////////////////
//////////////////////////////////////////////////////////////

__constant__ unsigned __int64 d_RSA_NUMBER[1];	//constant RSA number

__global__ void cudaGetPrimes(unsigned __int64 *a, unsigned __int64 *b, unsigned __int64 start){
	int t_index = threadIdx.x + blockIdx.x *  blockDim.x;	// thread index
	unsigned __int64 num = start + t_index;

	if (num < d_RSA_NUMBER[0]){
		if (d_RSA_NUMBER[0] % num == 0){
			*a = num;
			*b = d_RSA_NUMBER[0] / *a;
		}
	}
	//__syncthreads();
}


unsigned __int64 BruteFactor::modulus(const unsigned __int64 dividen, const unsigned __int64 divisor){
	return dividen%divisor;
}

//////////////////////////////////////////////////////////////
//////////////Check if number is prime factor/////////////////
//////////////////////////////////////////////////////////////

bool BruteFactor::isValid(const unsigned __int64 number){
	int count = 0, i;

	for (i = 2; i < number; i++){
		if (number%i == 0)
			count++;
	}

	if (count > 0)
		return false;
	else
		return true;
}

void BruteFactor::display() const{
	cout << endl << "Brute Factor" << endl;
	cout << Name << endl;
	cout << digits << endl;
	cout << sumofdigits << endl;
	cout << RSA_NUMBER << " = " << p1 << " X " << p2 << endl << endl;
	cout << "Calculation time = " << totalduration << endl;
}

void BruteFactor::getPrimes(){
	//start timer
	start = chrono::high_resolution_clock::now();

	//Brute force method
	for (unsigned __int64 i = 2; i < RSA_NUMBER; i++){
		if (modulus(RSA_NUMBER, i) == 0){
			p1 = i;
			p2 = RSA_NUMBER / p1;
			break;
		}
	}
	//finish timer
	finish = chrono::high_resolution_clock::now();
	totalduration = chrono::duration_cast<chrono::milliseconds>(finish - start).count();

}

void BruteFactor::getPrimesCuda(){
	bool foundprime = false;
	unsigned __int64 i = 2;

	//set rsa number on device
	cudaError(cudaMemcpyToSymbol(d_RSA_NUMBER, &RSA_NUMBER, sizeof(__int64), 0, cudaMemcpyHostToDevice));

	//initialize temporary device variables
	unsigned __int64 *d_a, *d_b;

	cudaError(cudaMalloc((void**)&d_a, sizeof(__int64)));
	cudaError(cudaMalloc((void**)&d_b, sizeof(__int64)));

	//start timer
	start = chrono::high_resolution_clock::now();
	int blks = (RSA_NUMBER / NTPB) < 65535 ? (RSA_NUMBER / NTPB) : 65535;

	//loop until prime is found or max number is reached
	while (!foundprime && i < RSA_NUMBER){
		//starting number for each iteration 
		unsigned __int64 h_a = i;
		//call cuda 
		cudaGetPrimes << <blks, NTPB >> >(d_a, d_b, i);
		//get data from device
		cudaError(cudaMemcpy(&p1, d_a, sizeof(__int64), cudaMemcpyDeviceToHost));
		cudaError(cudaMemcpy(&p2, d_b, sizeof(__int64), cudaMemcpyDeviceToHost));

		if (p1 != 0 && p2 != 0)
			foundprime = true;

		i = i + blks * NTPB;
	}
	//free device variables
	cudaError(cudaFree(d_a));
	cudaError(cudaFree(d_b));

	//finish timer
	finish = chrono::high_resolution_clock::now();
	totalduration = chrono::duration_cast<chrono::milliseconds>(finish - start).count();
}

//Cuda error handling 
void BruteFactor::cudaError(cudaError_t err){
	if (err != cudaSuccess){
		cerr << cudaGetErrorName(err) << endl;
	}
}


BruteFactor::~BruteFactor(){
	if (!fp.is_open())
		return;
	else{
		fp.close();
	}
}
