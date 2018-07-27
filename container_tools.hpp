#ifndef _CONTAINER_TOOLS
#define _CONTAINER_TOOLS

#include <algorithm>
#include <iterator>
#include <tuple>

namespace Yee{

// remove some elements in __second according to the elements in __first with criterion in __Predicate
// This function will move some elements into the front of the container, depends on the __Predicate functor of __first container,
// without modifiying the original order. Note this function won't erase any elements
// For example:
// std::vector<bool> flags({1,0,0,0,1,1,1,1});
// std::vector<int> _array;
// for(int i = 0 ; i < flags.size(); ++i)
// {
//    _array.push_back(i);    
// }
// auto last_iter =  remove_according_to(flags.cbegin(), flags.cend(), [](const bool& flag){return flag; } /* criterion you will take reference to*/
//                                       ,_array.begin());  // the container you want to remove something
// _array.erase(last_iter, _array.end());
// =======
// Will output :
// _array : 1,5,6,7,8

template <typename _ForwardIterator2, typename _ForwardIterator, typename _Predicate>
inline _ForwardIterator2
remove_according_to(_ForwardIterator __first, _ForwardIterator __last, _Predicate __pred, _ForwardIterator2 __second)
{
    _ForwardIterator __start = std::find_if(__first, __last, __pred);
    if (__start == __last)  // if not found, return the begin of second container.
        return __second;
    typename std::iterator_traits<_ForwardIterator>::difference_type _n = std::distance(__first, __start);
    _ForwardIterator2 __result = std::next(__second, _n);
    // ++__start;
    for (; __start != __last; ++__start)
        if (!__pred(*__start))
        {
            typename std::iterator_traits<_ForwardIterator>::difference_type _n = std::distance(__first, __start);
            _ForwardIterator2 __i = std::next(__second, _n);
            *__result = std::move(*__i);
            ++__result;
        }
    return __result;
}

template <typename _ForwardIterator, typename _Predicate, typename... _ForwardIterator2>
inline std::tuple<_ForwardIterator2...>
remove_according_recursive(const _ForwardIterator __first,const _ForwardIterator __last, _Predicate __pred, _ForwardIterator2... __second)
{ 
    return std::make_tuple(remove_according_to( __first,  __last,  __pred, __second)...);
}

template <typename _container, typename _Predicate, typename _criterion_container>
inline void keep_according_to(const _criterion_container& __flags,  _Predicate __pred, _container& __second)
{
    typename _container::iterator __ite_second = remove_according_to(__flags.cbegin(), __flags.cend(), __pred, __second.begin());
    __second.erase(__ite_second, __second.end());
}

template <typename _criterion_container, typename _Predicate, typename _container2, typename... _container>
inline void keep_according_to(const _criterion_container& __flags,  _Predicate __pred, _container2& __second, _container&... __rest)
{
    // recursive calling....
    keep_according_to(__flags, __pred, __second);
    keep_according_to(__flags, __pred, __rest...);
}

// adptor for value compare
template <typename _Value>
struct _Value_equals_val
{
    const _Value &_M_value;

    _Value_equals_val(const _Value &__value)
        : _M_value(__value)
    {
    }

    bool operator()(const _Value& __it)
    {
        return __it == _M_value;
    }
};

template <typename _ForwardIterator2, typename _ForwardIterator, typename _Tp>
inline _ForwardIterator2
remove_according_value(_ForwardIterator __first, _ForwardIterator __last, const _Tp& __value, _ForwardIterator2 __second)
{
    _Value_equals_val<_Tp> __value_adaptor(__value);
    return remove_according_to( __first,  __last,  __value_adaptor,  __second);
}

}
#endif