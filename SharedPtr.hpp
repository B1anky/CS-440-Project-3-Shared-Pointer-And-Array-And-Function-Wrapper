#include <cstddef>
#include <atomic>
#include <typeinfo>
#include <iostream>
#include <pthread.h>

namespace cs540{
	struct spDestructor{
		virtual void destroy() = 0; 
		virtual ~spDestructor() = default;
	};

	template <typename T>
	struct tspDestructor: public spDestructor{
		T* data;
		tspDestructor() : data(nullptr) { }
		tspDestructor(T* ptr) : data(ptr) { }
		void destroy() override{
			delete data;
		}
	};

	struct refCnt{
		std::atomic_int refs;
		spDestructor* destructor;
		pthread_mutex_t refsLock;
		refCnt() : refs(0), destructor(nullptr), refsLock(PTHREAD_MUTEX_INITIALIZER){ }
		template <typename T>
		refCnt(T* ptr) : refs(1), destructor(new tspDestructor<T>(ptr)), refsLock(PTHREAD_MUTEX_INITIALIZER) { }
		~refCnt(){
			if(destructor){
				destructor->destroy();
				delete destructor;
			}
			pthread_mutex_unlock(&refsLock);
		}
	};

	template <typename T>
	class SharedPtr{
		public:
		T* ptrData;
		refCnt* refObj;

		SharedPtr(){
			//std::cout << "Regular constructor called" << std::endl;
			refObj = nullptr;
			ptrData = nullptr;
		}
		
		template <typename U> 
		explicit SharedPtr(U * ptr){
			//std::cout << "Templated constructor called" << std::endl;
			refObj = new refCnt(ptr);
			ptrData = ptr;
		}

		SharedPtr(const SharedPtr &p){
			//std::cout << "Normal copy constructor called" << std::endl;
			refObj = p.refObj;
			increment();
			ptrData = p.ptrData;
		}

		template <typename U> 
		SharedPtr(const SharedPtr<U> &p){
			//std::cout << "Templated copy constructor called" << std::endl;
			refObj = p.refObj;
			increment();
			ptrData = p.ptrData;
		}

		SharedPtr(SharedPtr &&p){
			//std::cout << "Move constructor called" << std::endl;
			refObj = std::move(p.refObj);
			ptrData = std::move(p.ptrData);
			p.refObj = nullptr;
			p.ptrData = nullptr;
		}
		
		template <typename U> 
		SharedPtr(SharedPtr<U> &&p){
			//std::cout << "Templated move constructor called" << std::endl;
			refObj = std::move(p.refObj);
			ptrData = std::move(p.ptrData);
			p.refObj = nullptr;
			p.ptrData = nullptr;
		}

		template <typename U>
		SharedPtr(const SharedPtr<U> &ptr, T* other){
			refObj = ptr.refObj;
			ptrData = other;
			increment();
		}

		SharedPtr &operator=(const SharedPtr &other){
			if(&other == this || this->refObj == other.refObj){
				return *this;
			}
			decrement();
			refObj = other.refObj;
			increment();
			ptrData = other.ptrData;
			return *this;
		}
		
		template <typename U>
		SharedPtr<T> &operator=(const SharedPtr<U> &other){
			if(this->refObj == other.refObj){
				ptrData = other.ptrData;
				return *this;
			}
			decrement();
			refObj = other.refObj;
			increment();
			ptrData = other.ptrData;
			return *this;
		}

		SharedPtr &operator=(SharedPtr &&p){
			if(this->refObj == p.refObj){
				return *this;
			}
			decrement();
			refObj = std::move(p.refObj);
			ptrData = std::move(p.ptrData);
			p.refObj = nullptr;
			p.ptrData = nullptr;
			return *this;
		}

		template <typename U> 
		SharedPtr &operator=(SharedPtr<U> &&p){
			if(this->refObj == p.refObj){
				return *this;
			}
			decrement();
			refObj = std::move(p.refObj);
			ptrData = std::move(p.ptrData);
			p.refObj = nullptr;
			p.ptrData = nullptr;
			return *this;
		}
	
		~SharedPtr(){
			decrement();
		}

		void reset(){
			//std::cout << "reset() called" << std::endl;
			decrement();
			refObj = nullptr;
			ptrData = nullptr;
		}

		template <typename U> 
		void reset(U *p){
			//std::cout << "reset(U *p) called" << std::endl;
			try{
				decrement();
				refObj = new refCnt(p);
				ptrData = p;
			}catch(...){
				throw "An exception occurred in template <typename U> SharedPtr::reset(U *p)\n";
			}
		}

		T *get() const{
			return ptrData;
		}

		T &operator*() const{
			return *ptrData;
		}

		T *operator->() const{
			return ptrData;
		}

		explicit operator bool() const{
			if(ptrData){
				return true;
			}else{
				return false;
			}
		}

	private:
		void increment(){
			if(refObj){
				pthread_mutex_lock(&refObj->refsLock);
				refObj->refs++;
				pthread_mutex_unlock(&refObj->refsLock);
			}	
		}

		void decrement(){
			if(refObj){
				pthread_mutex_lock(&refObj->refsLock);
				//std::cout << "refsLocking decrement() on refObj address: " << refObj << std::endl;
				refObj->refs--;
				if(refObj->refs == 0){
					//pthread_mutex_unlock(&refObj->refsLock);
					delete refObj;
					refObj = nullptr;
					ptrData = nullptr;
					return;
				}
				pthread_mutex_unlock(&refObj->refsLock);
				//std::cout << "unrefsLocking decrement() on refObj address: " << refObj << std::endl;
			}
		}

	};

	template <typename T1, typename T2>
	bool operator==(const SharedPtr<T1> &lhs, const SharedPtr<T2> &rhs){
		return lhs.get() == rhs.get();	
	}

	template <typename T>
	bool operator==(const SharedPtr<T> &lhs, std::nullptr_t rhs){
		return !lhs;
	}

	template <typename T>
	bool operator==(std::nullptr_t lhs, const SharedPtr<T> &rhs){
		return !rhs;
	}

	template <typename T1, typename T2>
	bool operator!=(const SharedPtr<T1>& lhs, const SharedPtr<T2> &rhs){
		return !(lhs == rhs);
	}

	template <typename T>
	bool operator!=(const SharedPtr<T> &lhs, std::nullptr_t rhs){
		return (bool) lhs;
	}

	template <typename T>
	bool operator!=(std::nullptr_t lhs, const SharedPtr<T> &rhs){
		return (bool) rhs;
	}

	template <typename T, typename U>
	SharedPtr<T> static_pointer_cast(const SharedPtr<U> &r) noexcept{
		return SharedPtr<T>(r, static_cast<T*>(r.get()));
	}

	template <typename T, typename U>
	SharedPtr<T> dynamic_pointer_cast(const SharedPtr<U> &sp){
		if (auto p = dynamic_cast<T*>(sp.get())){
			return SharedPtr<T>(sp, p);
		}else {
			return SharedPtr<T>();
		}
	}
}
