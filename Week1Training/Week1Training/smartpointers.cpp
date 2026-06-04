#include <iostream>


template <typename T>
class myUniquePtr {
	T* ptr_;
public:
	explicit myUniquePtr(T* p = nullptr) noexcept : ptr_(p) {
		std::cout << "myUniquePtr constructor called. Resource acquired." << std::endl;
	}
	~myUniquePtr() {
		delete ptr_;
		std::cout << "myUniquePtr destructor called. Resource released." << std::endl;
	}
	myUniquePtr(const myUniquePtr&) = delete; //delete copy constructor
	myUniquePtr& operator=(const myUniquePtr&) = delete; //delete copy assignment operator

	myUniquePtr(myUniquePtr&& other) noexcept : ptr_(other.ptr_) {
		other.ptr_ = nullptr; //transfer ownership
		std::cout << "myUniquePtr move constructor called. Resource ownership transferred." << std::endl;
	}

	myUniquePtr& operator=(myUniquePtr&& other) noexcept {
		if (this != &other) {
			delete ptr_; //release current resource
			ptr_ = other.ptr_; //transfer ownership
			other.ptr_ = nullptr; //nullify the source pointer

		}

		return *this;

	}

	T* get() const noexcept { return ptr_; } //access the raw pointer
	T& operator*() const noexcept { return *ptr_; } //dereference operator
	T* operator->() const noexcept { return ptr_; } //member access operator
	explicit operator bool() const noexcept { return ptr_ != nullptr; } //bool conversion operator


	T* release() noexcept {
		T* temp = ptr_;
		ptr_ = nullptr;
		return temp;
	}
	void reset(T* p = nullptr) noexcept {
		if (ptr_ != p) {
			delete ptr_;
			ptr_ = p;
			std::cout << "myUniquePtr reset called. Resource ownership updated." << std::endl;
		}
	}
};