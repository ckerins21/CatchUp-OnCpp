#include <iostream>
#include <random>
#include <fstream>

int main(){
	std::mt19937_64 rng(42); //seede - reproducible runs

	// price centred around 1000 ticks, std dev 20

	std::normal_distribution<> price_dist(1000.0, 20.0);
	std::uniform_int_distribution<> qty_dist(1, 100);
	std::bernoulli_distribution side_dist(0.5); //50 /50 buy/sell

	std::ofstream out("orders.csv");

	for (uint16_t id = 1; id <= 1'000'000; ++id) {
		int price = std::max(1, (int)price_dist(rng));
		int qty = qty_dist(rng);
		char side = side_dist(rng) ? 'B' : 'S';
		out << id << ' ' << side << ' ' << price << ' ' << qty << '\n';
	}
}