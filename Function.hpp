#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include <cstddef>
#include <string>
#include <stdexcept>

//function pointer for free function
//object for lambda and functor

namespace cs540{

	//Done
	class BadFunctionCall : std::runtime_error {
        public:
            explicit BadFunctionCall(const std::string& s) : std::runtime_error(s){ }
            explicit BadFunctionCall(const char* s) : std::runtime_error(s){ }
    };

    //Done
	template<typename ResultType, typename ... Args>
	struct Functor{
		virtual ResultType operator()(Args... args) = 0;
		virtual Functor<ResultType, Args...>* clone() = 0;
		virtual ~Functor() = default;
	};

	template <typename>
	class Function;

	template <typename ResultType, typename ... Args>
	class Function<ResultType(Args...)> {

		//Done
		template<typename T>
		struct FuncFunctor : public Functor<ResultType, Args...>{
				T obj;
				FuncFunctor(const T& obj) : obj(obj){};
				virtual ~FuncFunctor() = default;
				virtual ResultType operator()(Args...args){
					return obj(args...);
				}
				virtual Functor<ResultType, Args...>* clone(){
					return new FuncFunctor<T>(obj);
				}
		};


		public:
			//Done
			Function() : funcPtr(nullptr), functor(nullptr){}

			template <typename FunctionType>
			Function(FunctionType type){
				funcPtr = nullptr;
				functor = new FuncFunctor<FunctionType>(type);
				//functor = ;
			}

			//Done
			Function(const Function &other) : funcPtr(other.funcPtr){
				if(other.functor){
					functor = other.functor->clone();
				}else{
					functor = nullptr;
				}
			}

			//Done
			Function &operator=(const Function &other){
				if(this != &other){
					if(functor){
						delete functor;
					}
					funcPtr = other.funcPtr;
					if(other.functor){
						functor = other.functor->clone();
					}else{
						functor = nullptr;
					}
				}
				return *this;
			}

			//Done
			~Function(){
				if(functor){
					delete functor;
					functor = nullptr;
				}
			}

			//Done
			ResultType operator()(Args... args){
				if(!funcPtr && !functor){
					throw BadFunctionCall("Bad function call");
				}
				if(funcPtr){
					return funcPtr(args...);
				}else{
					return (*functor)(args...);
				}
			}

			//Done
			explicit operator bool() const {
				return funcPtr != nullptr || functor!= nullptr;
			}

		private:
			ResultType (*funcPtr)(Args...);
			Functor<ResultType, Args...> *functor;
	};

	template <typename ResultType, typename... Args>
	bool operator==(const Function<ResultType(Args...)> &f, std::nullptr_t){
		return !f;
	}

	template <typename ResultType, typename... Args>
	bool operator==(std::nullptr_t, const Function<ResultType(Args...)> & f){
		return !f;
	}

	template <typename ResultType, typename... Args>
	bool operator!=(const Function<ResultType(Args...)> &f, std::nullptr_t){
		return bool(f);
	}

	template <typename ResultType, typename... Args>
	bool operator!=(std::nullptr_t, const Function<ResultType(Args...)> & f){
		return bool(f);
	}
}

#endif