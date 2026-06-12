#include <iostream>
#include <memory>
#include <vector>

// ============================================================================
// IDIOM 1: SMART POINTER LAB (Zero-Overhead Abstractions)
// ============================================================================

struct Data {
    int x;
    int y;
};

// Custom minimal unique_ptr simulation from Week 1
template<typename T>
class MyUniquePtr {
private:
    T* ptr;
public:
    explicit MyUniquePtr(T* p) : ptr(p) {}

    // Destructor to clean up heap memory automatically
    ~MyUniquePtr() {
        delete ptr;
    }

    // Dereference operator
    T& operator*() {
        return *ptr;
    }

    // Arrow operator for member access
    T* operator->() {
        return ptr;
    }
};

// Compare these two in Godbolt at -O2. The assembly will look identical.
int test_standard_ptr() {
    std::unique_ptr<Data> p(new Data{ 10, 20 });
    return p->x + p->y;
}

int test_custom_ptr() {
    MyUniquePtr<Data> p(new Data{ 10, 20 });
    return p->x + p->y;
}


// ============================================================================
// IDIOM 2: THE 16-BYTE RULE (Pass by Value vs. Const Reference)
// ============================================================================

struct Packet {
    long price;   // 8 bytes
    long volume;  // 8 bytes
}; // Total size = 16 bytes (Fits perfectly into two hardware registers)

// FAST: Registers are used directly, bypassing memory entirely
long fast_total_by_value(Packet p) {
    return p.price + p.volume;
}

// SLOWER: Passes a pointer memory address, forcing data lookup overhead
long slow_total_by_reference(const Packet& p) {
    return p.price + p.volume;
}


// ============================================================================
// IDIOM 3: LOOP BOUNDS (Fixed Loop Unrolling vs. Runtime Size)
// ============================================================================

// FIXED SIZE: Compiler knows the limit is 4. It rips the loop structure out.
void fixed_loop(int* __restrict arr) {
    for (int i = 0; i < 4; ++i) {
        arr[i] *= 2;
    }
}

// RUNTIME SIZE: Compiler must generate loop control code & branch logic.
void runtime_loop(int* __restrict arr, int size) {
    for (int i = 0; i < size; ++i) {
        arr[i] *= 2;
    }
}


// ============================================================================
// MAIN FUNCTION (To tie it together for local compilation)
// ============================================================================
int main() {
    // 1. Smart Pointer Execution
    std::cout << "Standard pointer result: " << test_standard_ptr() << "\n";
    std::cout << "Custom pointer result: " << test_custom_ptr() << "\n";

    // 2. Pass-by-Value vs Reference Execution
    Packet pack{ 150, 500 };
    std::cout << "Value sum: " << fast_total_by_value(pack) << "\n";
    std::cout << "Reference sum: " << slow_total_by_reference(pack) << "\n";

    // 3. Loop Execution
    int myArray[4] = { 1, 2, 3, 4 };
    fixed_loop(myArray);
    runtime_loop(myArray, 4);

    return 0;
}