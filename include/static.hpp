
/// ============================================================================
/// @file      : static.hpp
/// @copyright : 2022 LCMonteiro
///
/// @author    : Luis Monteiro
/// ============================================================================
#pragma once

#include <array>
#include <tuple>
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
  template<int n>
  struct number {
    const int value{n};
  };
  template<typename...>
  struct checker: identity<int>{};

// =============================================================================
// static helpers
// =============================================================================
  namespace pipe {
    template<typename T, typename F>
    struct part: function<T,F>{
      using function<T,F>::function;
    };
    template<typename T, typename F>
    auto make(F&& fn) {
      return part<T,F>{std::forward<F>(fn)};
    }

    template<typename T>
    using check = typename checker<
      decltype(std::declval<T>().cbegin()),
      decltype(std::declval<T>().cend())
    >::type;

    template<typename T>
    constexpr auto declval()->T{ return {}; }
    template <>
    constexpr auto declval()->void{}
    
    struct reflect{
      template<typename T>
      constexpr auto operator()(T v)->T{ return v; } 
    };
  }

// =============================================================================
// static algorithms
// =============================================================================
  template<typename T>
  constexpr auto load(T&& in) {
    return stc::pipe::make<stc::number<0>>(
      [&in](auto&& nxt) mutable {
        std::for_each(std::cbegin(in), std::cend(in), [&nxt](auto& v){ nxt(v); });
        return pipe::declval<decltype(nxt(*std::cbegin(in)))>();
      });
  }

  template<typename T>
  constexpr auto range(T beg, T end, T step) {
    return stc::pipe::make<stc::number<0>>(
      [=](auto&& nxt) mutable {
        for(; beg<end; beg+=step)
          nxt(beg);
        return pipe::declval<decltype(nxt(beg))>();
      });
  }

  template<typename T , stc::pipe::check<T> = 0>
  constexpr auto zip(const T& in) {
    return stc::pipe::make<stc::number<1>>(
      [&in](auto&& nxt) mutable {
      return [
        cur=std::cbegin(in),
        end=std::cend(in),
        nxt=std::move(nxt)
      ](auto&& val) mutable {
        if(cur!=end)
          nxt(std::make_tuple(val, *cur++));
        return pipe::declval<decltype(
          nxt(std::make_tuple(val,*cur)))>();
      };
    });
  }

  template<typename T = std::size_t>
  constexpr auto enumerate(T init = {}) {
    return stc::pipe::make<stc::number<1>>(
      [init](auto&& nxt) mutable {
      return [
        cur=std::move(init),
        nxt=std::move(nxt)
      ](auto&& val) mutable {
        nxt(std::make_tuple(cur++,val));
        return pipe::declval<decltype(
          nxt(std::make_tuple(cur,val)))>();
      };
    });
  }

  template<typename F>
  constexpr auto transform(F fnc) {
    return stc::pipe::make<stc::number<1>>(
      [fnc=std::move(fnc)](auto&& nxt) mutable {
        return [
          fnc=std::move(fnc),
          nxt=std::move(nxt)
        ](auto&& val) mutable {
          nxt(fnc(val));
          return pipe::declval<decltype(nxt(fnc(val)))>();
        };
      });
  }

  template<typename F>
  constexpr auto transform_scan(F fnc) {
    return stc::pipe::make<stc::number<2>>(
      [fnc=std::move(fnc)](auto&& beg, auto&& nxt) mutable {
        return [
          fnc=std::move(fnc),
          nxt=std::move(nxt),
          acc=decltype(beg(pipe::reflect{})){}
        ](auto&& val) mutable {
          acc = fnc(acc, val);
          nxt(acc);
          return stc::pipe::declval<decltype(nxt(acc))>();
        };
      });
  }

  template<typename F>
  constexpr auto transform_adjacent(F fnc) {
    return stc::pipe::make<stc::number<2>>(
      [fnc=std::move(fnc)](auto&& beg, auto&& nxt) mutable {
        return [
          fnc=std::move(fnc),
          nxt=std::move(nxt),
          acc=decltype(beg(pipe::reflect{})){}
        ](auto&& val) mutable {
          nxt(fnc(val, acc));
          acc = std::move(val);
          return pipe::declval<decltype(nxt(fnc(acc,val)))>();
        };
      });
  }

  template<typename F>
  constexpr auto filter(F fnc){
    return stc::pipe::make<stc::number<1>>(
      [fnc=std::move(fnc)](auto&& nxt) mutable {
        return [
          fnc=std::move(fnc),
          nxt=std::move(nxt)
        ](auto&& val) mutable {
          if(fnc(val)) nxt(val);
          return pipe::declval<decltype(nxt(val))>();
        };
      });
  }

  template<std::size_t N>
  constexpr auto head(){
    return stc::pipe::make<stc::number<9>>(
      [](auto&& beg) mutable {
        auto res = stc::vector<decltype(beg(pipe::reflect{})),N+1>{};
        auto end = [&res](auto&& v){ res.push_back(v); };
        beg(end);
        if(res.max_size()<res.size())
          res.pop_back();
        return res;
      });
  }

  template<std::size_t N, typename F>
  constexpr auto top(F&& fnc){
    return stc::pipe::make<stc::number<9>>([
      fnc = std::move(fnc)
    ](auto&& beg) mutable {
      auto res = stc::vector<decltype(beg(pipe::reflect{})),N+1>{};
      auto end = [&res, &fnc](auto&& v) {
        res.push_back(v);
        std::rotate(
            std::lower_bound(std::begin(res), std::end(res), v, fnc),
            std::prev(std::end(res)),
            std::end(res));
      };
      beg(end);
      if(res.max_size()==res.size())
        res.pop_back();
      return res;
    });
  }

  template<typename F>
  constexpr auto reduce(F&& fnc){
    return stc::pipe::make<stc::number<9>>([
      fnc = std::move(fnc)
    ](auto&& beg) mutable {
      auto res = decltype(beg(pipe::reflect{})){};
      auto end = [&res, &fnc](auto&& val) { res = fnc(res,val); };
      beg(end);
      return res;
    });
  }

  template<typename F>
  constexpr auto attach(F&& fnc){
    return stc::pipe::make<stc::number<9>>([
      fnc = std::move(fnc)
    ](auto&& beg) mutable {
       return fnc(std::move(beg));
    });
  }
}

// =============================================================================
// pipeline links
// =============================================================================
template<typename A, typename B, typename C, stc::pipe::check<A> = 0>
constexpr auto operator|(
  A&& a, stc::pipe::part<B,C>&& b) {
  return stc::load(std::forward<A>(a)) | std::move(b);
}

template<typename A,typename B>
constexpr auto operator|(
  stc::pipe::part<stc::number<0>,A>&& a,
  stc::pipe::part<stc::number<1>,B>&& b) {
  return stc::pipe::make<stc::number<0>>([
    a=std::move(a),
    b=std::move(b)](auto&& c) mutable {
      return a(b(c));
  });
}

template<typename A,typename B>
constexpr auto operator|(
  stc::pipe::part<stc::number<0>,A>&& a,
  stc::pipe::part<stc::number<2>,B>&& b) {
  return stc::pipe::make<stc::number<0>>([
    a=std::move(a),
    b=std::move(b)](auto&& c) mutable {
      return a(b(a,c));
  });
}

template<typename A,typename B>
constexpr auto operator|(
  stc::pipe::part<stc::number<0>,A>&& a,
  stc::pipe::part<stc::number<9>,B>&& b) {
  return b(std::move(a));
}
