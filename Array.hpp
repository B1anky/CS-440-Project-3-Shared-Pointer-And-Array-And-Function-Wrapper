#ifndef ARRAY_HPP
#define ARRAY_HPP

#include "MyInt.hpp"
#include <ostream>
#include <initializer_list>
#include <chrono>
#include <random>

namespace cs540{
	class Array{
		public:
		Array(std::initializer_list<MyInt> init){
			arr = new MyInt[init.size()];
			size = 0;
			for(auto num: init){
				insert(num);
			}
		}

		Array(){
			arr = nullptr;
			size = 0;
		}

		Array(const Array& other){
			arr = new MyInt[other.size];
			size = 0;
			for(size_t i = 0; i < other.size; i++){
				insert(other.arr[i]);
			}
		}

		Array(Array &&arrIn){
			arr = std::move(arrIn.arr);
			size = arrIn.size;
			arrIn.arr = nullptr;
			arrIn.size = 0;
		}

		Array &operator=(const Array &other){
			if(&other == this || arr== other.arr){
				return *this;
			}
			size = 0;
			delete[] arr;
			arr = new MyInt[other.size];
			for(size_t i = 0; i < other.size; i++){
				insert(other.arr[i]);
			}
			return *this;
		}

		Array &operator=(Array &&arrIn){
			if(arr == arrIn.arr){
				return *this;
			}
			delete[] arr;
			arr = std::move(arrIn.arr);
			size = arrIn.size;
			arrIn.arr = nullptr;
			arrIn.size = 0;
			return *this;
		}

		~Array(){
			delete[]  arr;
			arr = nullptr;
		}

		void insert(MyInt num){
			arr[size] = num;
			size++;
		}

		friend std::ostream& operator<<(std::ostream& os, const Array& arr); 

		void static move_performance_test(){
			auto seed = std::chrono::system_clock::now().time_since_epoch().count();
			std::default_random_engine gen(seed);
			std::uniform_int_distribution<unsigned int> dist(0, 10000);

			using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
			using Milli = std::chrono::duration<double, std::ratio<1,1000>>;
			using namespace std::chrono;

			int eleSize = 1000000;

			TimePoint start1, end1;
			TimePoint start2, end2;
			TimePoint start3, end3;
			TimePoint start4, end4;

			Array arr1;
			arr1.arr = new MyInt[eleSize];

			for(int i = 0; i < eleSize; ++i) {
				MyInt g = dist(gen);
				arr1.insert(g);
			}

			//Copy constructorb test
			start1 = system_clock::now();
			Array arr2{arr1};			
			end1 = system_clock::now();
			Milli elapsed1 = end1 - start1;
		
			//Move construtcor test
			start2 = system_clock::now();
			Array arr3{std::move(arr1)};
			end2 = system_clock::now();
			Milli elapsed2 = end2 - start2;

			//Assignment operator test
			start3 = system_clock::now();
			Array arr4 = arr2;			
			end3 = system_clock::now();
			Milli elapsed3 = end3 - start3;

			//Move assignment operator test
			start4 = system_clock::now();
			Array arr5 = std::move(arr2);			
			end4 = system_clock::now();
			Milli elapsed4 = end4 - start4;

			std::cerr << "Time to do copy constructor on " << eleSize << " elements: "  << elapsed1.count() << " milliseconds" << std::endl;
			std::cerr << "Time to do move constructor on " << eleSize << " elements: "  << elapsed2.count() << " milliseconds" << std::endl;
			std::cerr << "Time to do assignment operator on " << eleSize << " elements: "  << elapsed3.count() << " milliseconds" << std::endl;
			std::cerr << "Time to do move assignment operator on " << eleSize << " elements: "  << elapsed4.count() << " milliseconds" << std::endl;
		}

		MyInt * arr;
		size_t size;				
	};

	std::ostream& operator<<(std::ostream& os, const Array& arrIn)  {  
	for(size_t i = 0; i < arrIn.size; i++){
		os << arrIn.arr[i];
		if(i != arrIn.size - 1){
			os << ", ";
		}else{
			os << " ";
		}
	}
	return os;  
	}
}

#endif