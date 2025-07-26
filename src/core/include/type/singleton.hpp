#ifndef PLIB_CORE_TYPE_SINGLETON_HPP
#define PLIB_CORE_TYPE_SINGLETON_HPP
/**
    @brief single instance class
*/

namespace plib::core::type{

template<typename T>
class Singleton  {
public:
    static T& Instance() {
        if(Singleton::s_instance==0) {
            Singleton::s_instance = CreateInstance();
        }
        return *(Singleton::s_instance);
    }
    
    static T* GetInstance() {
        if(Singleton::s_instance==0) {
            Singleton::s_instance = CreateInstance();
        }
        return Singleton::s_instance;
    }
    
    static T* getInstance() {
        if(Singleton::s_instance==0) {
            Singleton::s_instance = CreateInstance();
        }
        return Singleton::s_instance;
    }
    
    static void Destroy() {
        if(Singleton::s_instance!=0) {
            DestroyInstance(Singleton::s_instance);
            Singleton::s_instance=0;
        }
    }
    
protected:
    Singleton()	{
        Singleton::s_instance = static_cast<T*>(this);
    }
    
    ~Singleton() {
        Singleton::s_instance = 0;
    }
    
private:
    static T* CreateInstance(){
        return new T();
    }
    
    static void DestroyInstance(T* p) {
        delete p;
    }
    
private:
    static T *s_instance;
    
private:
    explicit Singleton(Singleton const &) { }
    Singleton& operator=(Singleton const&) { return *this; }
};

template<typename T>
T* Singleton<T>::s_instance=0;
}

#endif // PLIB_CORE_TYPE_SINGLETON_HPP