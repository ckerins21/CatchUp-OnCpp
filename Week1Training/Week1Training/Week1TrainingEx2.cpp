#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
/*
static createArraystack()
{
	std::cout << "[Stack] Attempting to create array of size 1000000..." << std::endl;
	int arr[1000000]; // This may cause a stack overflow if the array is too large 4mb
	arr[0] = 1; // Just to use the array and prevent optimization
	std::cout << "[Stack] Array created successfully." << std::endl;

}*/

void createArrayheap()
{
	std::cout << "[Heap] Attempting to create array of size 1000000..." << std::endl;
	std::vector<int> arr(1000000); // This will allocate memory on the heap, which can handle larger sizes
	arr[0] = 1; // Just to use the array and prevent optimization
	std::cout << "[Heap] Array created successfully." << std::endl;
}
/*
int main()
{
	std::cout << "Creating array on stack..." << std::endl;
	//createArraystack();
	//std::cout << "Creating array on heap..." << std::endl;
	//createArrayheap();
	return 0;
}*/