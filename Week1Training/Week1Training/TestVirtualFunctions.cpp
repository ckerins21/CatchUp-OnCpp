#include <iostream>
#include <utility>

class Animal {
public:
    virtual void makeNoise() { std::cout << "Generic sound"; }
};

class Dog : public Animal {
public:
    void makeNoise() { std::cout << "Woof!"; }
};

int main() {
    // If you do this:
    Animal* myPet = new Dog();
    myPet->makeNoise(); // Outputs: "Generic sound" :(
}