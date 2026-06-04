#include <iostream>
#include <utility>

template<typename T>
class MyUniquePtr {
private:
	T* ptr = nullptr; //the secret raw pointer

public:
	//constructor
	explicit MyUniquePtr(T* p = nullptr) : ptr(p)
	{
		std::cout << "MyUniquePtr constructor called. Resource acquired." << std::endl;
	}

	//destructor
	~MyUniquePtr()
	{
		delete ptr; //release the resource
		std::cout << "MyUniquePtr destructor called. Resource released." << std::endl;
	}

	MyUniquePtr(const MyUniquePtr&) = delete; //delete copy constructor

	MyUniquePtr& operator=(const MyUniquePtr&) = delete; //delete copy assignment operator

	//Move constructor
	MyUniquePtr(MyUniquePtr&& other) noexcept : ptr(other.release()) //transfer ownership
	{
		std::cout << "[MyUniquePtr] move constructor called. Resource ownership transferred." << std::endl;

	}

	MyUniquePtr& operator=(MyUniquePtr&& other) noexcept //move assignment operator
	{
		std::cout << "[MyUniquePtr] move assignment operator called. Resource ownership transferred." << std::endl;
		if (this != &other) {
			reset(other.release()); //release current resource and transfer ownership
		}
		return *this;
	}

	T* get() const { return ptr; } //access the raw pointer

	T* release() //release ownership of the resource
	{
		T* temp = ptr; //store the current pointer
		ptr = nullptr; //set our pointer to null to indicate we no longer own it
		return temp; //return the raw pointer to the caller
	}

	void reset(T* p = nullptr) //reset the pointer to a new resource
	{
		if (ptr != p) { //only reset if the new pointer is different
			delete ptr; //release current resource
			ptr = p; //take ownership of the new resource
			std::cout << "[MyUniquePtr] reset called. Resource ownership updated." << std::endl;
		}
	}

	T& operator*() const { return *ptr; } //dereference operator
	T* operator->() const { return ptr; } //arrow operator

	explicit operator bool() const { return ptr != nullptr; } //boolean conversion operator


};

struct Test {
	Test() { std::cout << "Test Created\n"; }
	~Test() { std::cout << "Test Destroyed\n"; }
	void hello() { std::cout << "Hello from inside the object!\n"; }
};

int main() {
	{
		std::cout << "--- Creating p1 ---\n";
		MyUniquePtr<Test> p1(new Test());
		p1->hello();

		std::cout << "\n--- Moving p1 to p2 ---\n";
		MyUniquePtr<Test> p2 = std::move(p1); // Transfers ownership

		if (!p1) std::cout << "p1 is now null (as expected).\n";
		p2->hello();

	} // p2 goes out of scope here; Destructor is called!

	std::cout << "\nEnd of main.\n";
	return 0;
}
