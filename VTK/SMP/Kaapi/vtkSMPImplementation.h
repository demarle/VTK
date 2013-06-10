#ifndef __vtkSMPImplementation_h_
#define __vtkSMPImplementation_h_

#include "vtkObject.h"
#include <kaapi++>
#include <utility>
#include <tuple>

class KaapiInit {
//  ka::Community com;
public:
  KaapiInit();
  ~KaapiInit();
};

template<int... N> struct seq {};
template<int N, int... S> struct gens : gens<N-1, N-1, S...> {};
template<int... S> struct gens<0, S...> { typedef seq<S...> type; };

template<class T> class Paire : public std::pair<T,vtkIdType>
{
  public:
    Paire(const T& t) : std::pair<T,vtkIdType>(t,1) {}
};

template<class... Args> class vtkFunctor;

template<class... Args>
class Tuple : public std::tuple<Paire<Args>...>
{
    int offset = 0;
    template<int... S> void pass(seq<S...>) { this->noop(this->ApplyOffset(std::get<S>(*this))...); }
    template<int... S> void cont(const vtkFunctor<Args...>* f, seq<S...>) { this->execute(f, this->PlusPlus(std::get<S>(*this))...); }
    template<class T> T ApplyOffset(Paire<T> p) { p.first += this->offset * p.second; return p.first; }
    template<class T> T PlusPlus(Paire<T> p) { p.first += p.second; return p.first; }
    void execute(const vtkFunctor<Args...>* f, Args... a) { (*f)(a...); }
    void noop(Args...) {}

  public:
    Tuple(Args... a) : std::tuple<Paire<Args>...>(Paire<Args>(a)...) {}
    template<int N> void SetId(vtkIdType i) { std::get<N>(*this).second = i; }
    template<int N> vtkIdType GetId() { return std::get<N>(*this).second; }

    void SetOffset(int o) { offset = o; }
    void apply() { this->pass(typename gens<sizeof...(Args)>::type()); }
    void exec(const vtkFunctor<Args...>* f) { this->cont(f, typename gens<sizeof...(Args)>::type()); }

    template<int N> typename std::tuple_element<N, std::tuple<Args...>>::type Get() { return std::get<N>(*this).first; }
};

#endif //__vtkSMPImplementation_h_
