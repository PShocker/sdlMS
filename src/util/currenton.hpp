#ifndef HEADER_UTIL_CURRENTON_HPP
#define HEADER_UTIL_CURRENTON_HPP

/**
 *   A 'Currenton' allows access to the currently active instance of a
 *   class via the static current() function. It is kind of like a
 *   singleton, but without handling the object construction itself or
 *   in other words its a glorified global variable that points to the
 *   current instance of a class.
 */
template<class C>
class Currenton
{
private:
  static Currenton<C>* s_current;

protected:
  Currenton()
  {
    s_current = this;
  }

  virtual ~Currenton()
  {
    if (s_current == this)
    {
      s_current = nullptr;
    }
  }

public:
  static C* current() { return static_cast<C*>(s_current); }
};

template<class C>
Currenton<C>* Currenton<C>::s_current = nullptr;

#endif

/* EOF */
