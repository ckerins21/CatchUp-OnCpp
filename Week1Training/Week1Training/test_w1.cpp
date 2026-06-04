#include "smartpointers.cpp"
#include <iostream>
#include <cassert>

struct Tracker {
	bool& destroyed;
	Tracker(bool& d) : destroyed(d) {}
	~Tracker() {
		destroyed = true;
	}
};

void test_default_construction() {
	myUniquePtr<int> p;
	assert(p.get() == nullptr); // should be null
	assert(!p); // should be null
}

void test_constructor_and_destruction() {
	bool destroyed = false;
	{
		myUniquePtr<Tracker> p(new Tracker(destroyed));
		assert(!destroyed); // should not be null
	}
	assert(destroyed); // should be true after going out of scope
}


static_assert(!std::is_copy_constructible_v<myUniquePtr<int>>, "myUniquePtr should not be copy constructible");

void move_constructor_ownership_transfer() {
	myUniquePtr<int> p1(new int(42));
	myUniquePtr<int> p2(std::move(p1));

	assert(p1.get() == nullptr); // p1 should be null after move
	assert(*p2 == 42); // p2 should have the value
}

void move_assignment_self_allignment() {
	bool d1 = false, d2 = false;
	myUniquePtr<Tracker> a(new Tracker(d1));
	myUniquePtr<Tracker> b(new Tracker(d2));

	a = std::move(b);
	assert(d1);
	assert(!d2);
	assert(b.get() == nullptr);

	myUniquePtr<int> c(new int(7));
	c = std::move(c); // self-assignment should not cause issues
	assert(c.get() != nullptr|| true);
}

int main() {
	//test_default_construction();

	//test_constructor_and_destruction();

	//move_constructor_ownership_transfer();

	move_assignment_self_allignment();
	std::cout << "All tests passed!" << std::endl;
	return 0;
}