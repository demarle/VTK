#ifndef __vtkGenericIterator_h__
#define __vtkGenericIterator_h__

template<class T>
struct vtkGenericIterator
  {
    vtkGenericIterator() : _id(0){;}
    vtkGenericIterator(T id) : _id(id){;}

    vtkGenericIterator<T>& operator++()
      {this->_id++; return *this;}
    vtkGenericIterator<T>& operator--()
      {this->_id++; return *this;}

    vtkGenericIterator<T>& operator+=(const vtkGenericIterator<T>& it)
      {this->_id += it._id; return *this;}
    vtkGenericIterator<T>& operator-=(const vtkGenericIterator<T>& it)
      {this->_id -= it._id; return *this;}

    vtkGenericIterator<T> operator+(T range) const
      {return vtkGenericIterator<T>(this->_id + range);}
    vtkGenericIterator<T> operator-(T range) const
      {return vtkGenericIterator<T>(this->_id - range);}

    bool operator != (const vtkGenericIterator<T>& it) const
      {return this->_id != it._id;}

    T operator*() const
      {return this->_id;}

  private :
    T _id;
  };

template<class T>
inline T operator- (const vtkGenericIterator<T>& it0, const vtkGenericIterator<T>& it1)
  {
  return *it0 - *it1;
  }

#endif //__vtkGenericIterator_h__
