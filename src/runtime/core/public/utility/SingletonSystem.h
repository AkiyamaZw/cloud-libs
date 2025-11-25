#pragma  once


namespace cloud
{


struct SingletonSystem
{
    virtual ~SingletonSystem() = default;
    virtual void Init() = 0;
    virtual void Exit() = 0 ;
};

#define DEFINE_SINGLE_SYSTEM(Type) static Type* Inst();

}
