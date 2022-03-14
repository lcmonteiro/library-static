
/// =====================================================================================
/// @file      : static/vector.hpp
/// @copyright : 2022 LCMonteiro
///
/// @author    : Luis Monteiro
/// =====================================================================================
#pragma once

#include <array>
#include <tuple>
#include <algorithm>

namespace stc {

// ======================================================================================
// static vector
// ======================================================================================
  template<typename T, std::size_t N>
  class vector {
    // ==================================================================================
    // type wrapper 
    // ==================================================================================
    struct type_wrapper {
      constexpr type_wrapper() = default;
      template <typename... A>
      constexpr type_wrapper(A&&... as) { 
        emplace(std::forward<A>(as)...); 
      }
      template <typename... A> 
      constexpr void emplace(A&&... as) { 
        new(static_cast<void*>(&data_))T(std::forward<A>(as)...);
      }
      constexpr void clear() const {
        get().~T();
      }    
      constexpr const T& get() const {
        return *static_cast<const T*>(static_cast<const void*>(&data_));
      }
      constexpr T& get() {
        return *static_cast<T*>(static_cast<void*>(&data_));
      }    
    private:
      std::aligned_storage_t<sizeof(T), alignof(T)> data_;
    };
    
    // ==================================================================================
    // iterator wrapper 
    // ==================================================================================
    template<typename Base>
    struct iterator_wrapper {
      using iterator_category = std::random_access_iterator_tag;
      using difference_type   = std::ptrdiff_t;
      using reference         = decltype(std::declval<Base>()->get());
      using value_type        = std::remove_reference_t<reference>;
      using pointer           = value_type*;
      
      iterator_wrapper(Base&& base): base_{base}{}
      
      reference operator* () const { return  base_->get(); }
      pointer   operator->()       { return &base_->get(); }
      reference operator[](difference_type n) const {return base_[n].get();}
      
      iterator_wrapper& operator++() {++base_; return *this;}
      iterator_wrapper& operator--() {--base_; return *this;}
      
      iterator_wrapper operator++(int) {iterator_wrapper tmp(*this); ++base_; return tmp;}
      iterator_wrapper operator--(int) {iterator_wrapper tmp(*this); --base_; return tmp;}
      
      iterator_wrapper& operator+=(difference_type n) {base_+=n; return *this;}
      iterator_wrapper& operator-=(difference_type n) {base_-=n; return *this;}

      iterator_wrapper operator+(difference_type n) const {return iterator_wrapper(base_+n);}
      iterator_wrapper operator-(difference_type n) const {return iterator_wrapper(base_-n);}

      difference_type operator-(const iterator_wrapper& i) const {return base_-i.base_;}
      
      bool operator==(const iterator_wrapper& i) const {return base_ == i.base_;}
      bool operator!=(const iterator_wrapper& i) const {return base_ != i.base_;}
      bool operator> (const iterator_wrapper& i) const {return base_ >  i.base_;}
      bool operator< (const iterator_wrapper& i) const {return base_ <  i.base_;}
      bool operator>=(const iterator_wrapper& i) const {return base_ >= i.base_;}
      bool operator<=(const iterator_wrapper& i) const {return base_ <= i.base_;}

      friend iterator_wrapper operator+(difference_type n, const iterator_wrapper& i) {
        return iterator_wrapper(n+i.base_);
      }
      friend iterator_wrapper operator-(difference_type n, const iterator_wrapper& i) {
        return iterator_wrapper(n-i.base_);
      }
    private:
      Base base_;
    };
    
    // ==================================================================================
    // vector alises and tags
    // ==================================================================================
    using base_type              = std::array<type_wrapper,N>; 
    using main_type              = std::array<T,N>;
  public:
    using difference_type        = typename main_type::difference_type;
    using size_type              = typename main_type::size_type;
    using value_type             = typename main_type::value_type;
    using const_reference        = typename main_type::const_reference;
    using reference              = typename main_type::reference;
    using const_pointer          = typename main_type::const_pointer;
    using pointer                = typename main_type::pointer;
    using const_iterator         = iterator_wrapper<typename base_type::const_iterator>;
    using iterator               = iterator_wrapper<typename base_type::iterator>;
    using const_reverse_iterator = iterator_wrapper<typename base_type::const_reverse_iterator>;
    using reserse_iterator       = iterator_wrapper<typename base_type::reverse_iterator>;

    // ==================================================================================
    // vector interfaces
    // ==================================================================================

    template<typename ...As>
    explicit vector(As&&...as):
      end_{sizeof...(As)},
      arr_{{std::forward<As>(as)...}}{  
    }
        
    ~vector() {
      std::for_each(
        std::begin(arr_), 
        std::next(std::begin(arr_), end_), 
        [](auto& v){ v.clear();});
    }
    
    auto& operator[](size_type i) const { return arr_[i].get(); }
    
    auto& front()   const { return arr_[     0].get(); }
    auto& back()    const { return arr_[end_-1].get(); }

    auto max_size() const { return arr_.max_size(); }
    auto empty()    const { return 0==end_; }
    auto full()     const { return N==end_; }
    auto size()     const { return end_;    }

    auto  cbegin() const { return const_iterator{arr_.cbegin()}; }
    auto   begin() const { return const_iterator{arr_.begin()}; }
    auto   begin()       { return iterator      {arr_.begin()}; }    
    auto    cend() const { return std::next(cbegin(), end_); }
    auto     end() const { return std::next(begin() , end_); }
    auto     end()       { return std::next(begin() , end_); }

    auto crbegin() const { return std::prev(crend(), end_); }
    auto  rbegin() const { return std::next(rend() , end_); }
    auto  rbegin()       { return std::next(rend() , end_); }    
    auto   crend() const { return       reserse_iterator{arr_.crend()}; }
    auto    rend() const { return const_reverse_iterator{arr_.rend() }; }
    auto    rend()       { return const_reverse_iterator{arr_.rend() }; }

    template <typename... A> 
    void push_back(A&&... as) {
      if(N==end_) pop_back();
      arr_[end_++].emplace(std::forward<A>(as)...);
    }
    void pop_back() {
      if(0==end_) return;
      arr_[--end_].clear();
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
}