
/// ============================================================================
/// @file      : static.hpp
/// @copyright : 2022 LCMonteiro
/// 
/// @author    : Luis Monteiro                                                                     
/// ============================================================================
#pragma once

#include <array>
#include <algorithm>

namespace stc {
  
// =============================================================================
// static vector
// =============================================================================
  template<typename T, std::size_t N>
  class vector {
    using base_type = std::array<T,N>;
  public:
    using difference_type        = typename base_type::difference_type;
    using size_type              = typename base_type::size_type;
    using value_type             = typename base_type::value_type;
    using const_reference        = typename base_type::const_reference;
    using reference              = typename base_type::reference;
    using const_pointer          = typename base_type::const_pointer;
    using pointer                = typename base_type::pointer;
    using const_iterator         = typename base_type::const_iterator;
    using iterator               = typename base_type::iterator;
    using const_reverse_iterator = typename base_type::const_reverse_iterator;
    using reserse_iterator       = typename base_type::reverse_iterator;
    
    template<typename ...A>
    vector(A&&...as):
      end_{sizeof...(A)},
      arr_{{std::forward<A>(as)...}}{
      std::for_each(
        end(),
        arr_.end(),
        [](auto& e){ e.~T(); });
    }
    
    auto& operator[](size_type i) const { 
      return arr_[i]; 
    }
    
    auto& front()   const { return arr_[0]; }
    auto& back()    const { return arr_[end_-1];}
    
    auto data()     const { return arr_.data(); }
    auto size()     const { return end_; }
   
    auto max_size() const { return arr_.max_size(); }
    auto empty()    const { return 0==end_; }
    auto full()     const { return N==end_; }
    
    auto  cbegin() const { return arr_.cbegin(); }
    auto   begin() const { return arr_.begin(); }
    auto   begin()       { return arr_.begin(); }    
    auto    cend() const { return std::next(arr_.cbegin() , end_); }
    auto     end() const { return std::next(arr_.begin()  , end_); }
    auto     end()       { return std::next(arr_.begin()  , end_); }    
   
    auto crbegin() const { return std::next(arr_.crbegin(), N-end_); }
    auto  rbegin() const { return std::next(arr_.rbegin() , N-end_); }
    auto  rbegin()       { return std::next(arr_.rbegin() , N-end_); }    
    auto   crend() const { return arr_.crend(); }
    auto    rend() const { return arr_.rend(); }
    auto    rend()       { return arr_.rend(); }    
    
    template <typename... A> 
    auto& push_back(A&&... as) { 
      if(N==end_) pop_back();
      return *new(&arr_[end_++])T(std::forward<A>(as)...); 
    }
    auto pop_back() {
      if(0==end_) return false;
      arr_[--end_].~T();
      return true;
    }
  private:
    size_type end_;
    base_type arr_;
  };
  template<typename T, typename...Ts>
  auto make_vector(T v, Ts...vs) {
    return vector<T, 1 + sizeof...(Ts)>{
      std::forward<T>(v), 
      std::forward<Ts>(vs)...
    };
  }

// =============================================================================
// static function
// ============================================================================= 
  template<typename T, typename F>
  struct function: F {
    function(F&& fn): F{std::forward<F>(fn)}{}
  };
  
// =============================================================================
// traits
// =============================================================================
  template<typename T>
  struct identity {
    using type = T;
  };
  template<typename...>
  struct checker: identity<int>{};

// =============================================================================
// static helpers
// ============================================================================= 
  namespace details {
    template<typename T>
    struct process {
      template<typename F>
      using type = function<T,F>;
      template<typename F>
      static auto make(F&& fn) { 
        return type<F>{std::forward<F>(fn)};
      }   
    };
    struct step:process<step>{};
    struct close:process<close>{};
    
    template<typename T>
    using check = typename checker<
      decltype(std::declval<T>().cbegin()),
      decltype(std::declval<T>().cend())
    >::type;
    
    template<typename T>
    constexpr auto declval()->T{ return {}; }
    template <>
    constexpr auto declval()->void{}
  }

// =============================================================================
// static algorithms
// ============================================================================= 
  template<typename T>
  constexpr auto load(T&& in) {
    return stc::details::step::make([
      &in
    ](auto&& nxt) mutable {
      for(auto& v : in)
        nxt(v);
      return stc::details::declval<
        decltype(nxt(in.front()))>();
    });
  }
  
  template<typename F>
  constexpr auto transform(F fnc) {
    return stc::details::step::make([
      fnc=std::move(fnc)
    ](auto&& nxt) mutable {
      return [
        fnc=std::move(fnc),
        nxt=std::move(nxt)
      ](auto&& val) mutable { 
        nxt(fnc(val)); 
        return stc::details::declval<
          decltype(nxt(fnc(val)))>();
      }; 
    });
  }
  
  template<typename F>
  constexpr auto filter(F fnc){
    return stc::details::step::make([
      fnc=std::move(fnc)
    ](auto&& nxt) mutable {
      return [
        fnc=std::move(fnc),
        nxt=std::move(nxt)
      ](auto&& val) mutable { 
        if(fnc(val)) nxt(val); 
        return stc::details::declval<
          decltype(nxt(val))>();
      };
    });
  }
  
  template<std::size_t N, typename F>
  constexpr auto sort(F&& fnc){
    return stc::details::close::make([
      fnc = std::move(fnc)
    ](auto&& beg) mutable {
      auto aux = [](auto a){ return a; };
      auto res = stc::vector<decltype(beg(aux)),N+1>{};
      auto end = [&res, &fnc](auto&& v) {
        res.push_back(v);
        std::rotate(
            std::lower_bound(std::begin(res), std::end(res), v, fnc), 
            std::prev(std::end(res)), 
            std::end(res));
      };
      beg(end);
      if(res.max_size()<res.size())
        res.pop_back();
      return res;
    });
  }
}

// =============================================================================
// pipeline links
// ============================================================================= 
template<typename A, typename B, stc::details::check<A> = 0>
constexpr auto operator|(
  A&& a,
  stc::details::step::type<B>&& b) {
  return stc::load(std::forward<A>(a)) | std::move(b);
}
template<typename A,typename B>
constexpr auto operator|(
  stc::details::step::type<A>&& a,
  stc::details::step::type<B>&& b) {
  return stc::details::step::make([
    a = std::move(a),
    b = std::move(b)
  ](auto&& c) mutable { 
    return a(b(c)); 
  });
}
template<typename A,typename B>
constexpr auto operator|(
  stc::details::step::type<A>&& a,
  stc::details::close::type<B>&& b) {
  return b(a);
}
