// Week1Training.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <cstdio>
#include <string>


class FileHandle {

    private:
		FILE* file;

    public:

        //constructor
		FileHandle(const char* filename) {
			file = fopen(filename, "w");
			std::cout << "File opened: " << filename << std::endl;

			if (!file) {
				std::cerr << "Error opening file: " << filename << std::endl;
				// Handle error as needed, e.g., throw an exception
			}
		}

		//Destructor
		~FileHandle() {
			if (file) {
				fclose(file);
				std::cout << "File closed." << std::endl;
			}
			else
			{
				std::cout << "No file to close." << std::endl;
			}
		}

		//Copy constructor
		FileHandle(const FileHandle&) = delete;
		//Copy assignment operator
		FileHandle& operator=(const FileHandle&) = delete;

		// We do the copy constructor and copy assignment operator 
		// as deleted because we don't want to allow copying of the 
		// FileHandle class. This is because the class manages a resource
		// (a file handle) that cannot be safely copied. If we allowed 
		// copying, it could lead to issues such as double closing of 
		// the file handle when both the original and the copy go out of 
		// scope, which would result in undefined behavior. By deleting 
		// these functions, we ensure that each instance of FileHandle is 
		// unique and manages its own resource without any risk of 
		// unintended side effects from copying.

};

void processFile(bool shouldExitEarly) {

	std::cout << "Starting to process file..." << std::endl;

	FileHandle file("test.txt");
	if (shouldExitEarly) {
		std::cout << "Exiting early from processFile." << std::endl;
		return; // Early exit, file will be closed by the destructor
	}


	std::cout << "Processing file... finished normally" << std::endl;
	// Simulate some processing here
}

int main()
{
    std::cout << "Hello World!\n";

	processFile(true); // Test with early exit
	processFile(false); // Test with normal completion
	return 0;


}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

