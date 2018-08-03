#include <iostream>
#include <thread>
#include <iomanip>
#include <cstdlib>
#include <cuda_runtime.h>
// to remove intellisense highlighting
#include <device_launch_parameters.h>
#ifndef __CUDACC__
#define __CUDACC__
#endif
#include <device_functions.h>
#include "BruteFactor.h"

using namespace std;
void brute(const char* a, BruteFactor** LN){
	*LN = new BruteFactor(a);
	(*LN)->getPrimes(); //Brute force
}
void brute2(const char* a, BruteFactor** LN){
	*LN = new BruteFactor(a);
	(*LN)->getPrimesCuda(); //Brute force
}


int main(){
	//Change this to your project path
	//string filename, a_path = "C:\\Users\\Rene\\ReneA-GDrive\\Project\\DPS915\\A3_2\\A3_2\\";
	string filename, a_path = "D:\\ReneA-GDrive\\Project\\DPS915\\A3_2\\A3_2\\";

	do{
		cout << "Enter File Name : ";
		cin >> filename;
		if (filename.compare("exit") != 0){
			//use for small numbers < 20 digits
			BruteFactor *noCuda, *hasCuda;
			string location = (a_path + filename);
			thread /*t1(brute, location.c_str(), &noCuda),*/ t2(brute2, location.c_str(), &hasCuda);
			//t1.join();
			t2.join();
			//noCuda->display();
			hasCuda->display();
		}
	} while (filename.compare("exit"));
	return 0;

}
