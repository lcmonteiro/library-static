/// ============================================================================
/// @file      : static.hpp
/// @copyright : 2022 LCMonteiro
///
/// @author    : Luis Monteiro
/// ============================================================================
#include <iostream>
#include "static.hpp"

// =============================================================================
// helpers
// ============================================================================= 
template<typename T>
auto print(const T& val){
  std::cout << "[ " << val << " ]" << std::endl;
}

template<typename...Ts>
auto print(const std::tuple<Ts...>& val){
  std::cout << "{"
      << std::get<0>(val) << ", " 
      << std::get<1>(val)
      << "} "
      << std::endl;
}

template<typename T, std::size_t N>
auto print(const stc::vector<T,N>& vec){
  std::cout << "[ ";
  for(auto& v: vec) {
    std::cout << v << " ";
  }
  std::cout << "]" << std::endl;
}

template<typename...Ts, std::size_t N>
auto print(const stc::vector<std::tuple<Ts...>,N>& vec){
  std::cout << "[ ";
  for(auto& v: vec) {
    //sizeof...(Ts)
    std::cout << "{"
      << std::get<0>(v) << ", " 
      << std::get<1>(v)
      << "} ";
  }
  std::cout << "]" << std::endl;
}  

// =============================================================================
// pipelines
// ============================================================================= 
auto process = [](auto&& step){
  return std::move(step)
    | stc::filter([](auto& d){ return d%2 == 0; })
    | stc::transform([](auto& d){ return 0.1 + (d * d); })
    | stc::transform_scan([](auto& a, auto& b){ return a + b; })
    | stc::transform_adjacent([](auto& a, auto& b){ return a - b; });
};

// ============================================================================
// entry point
// ============================================================================ 
int main() {
  auto scc = true;
  auto vec = stc::vector<int, 10>{1,8,3,4,5,6};
  auto out = vec
    | stc::filter([](auto& d){ return d%2 == 0; })
    | stc::transform([](auto& d){ return 0.1 + (d * d); })
    | stc::transform_scan([](auto& a, auto& b){ return a + b; })
    | stc::transform_adjacent([](auto& a, auto& b){ return a - b; })
    | stc::zip(vec
      | stc::attach(process)
      | stc::head<3>())
    | stc::reduce([](auto& a, auto& b){ 
      return std::make_tuple(
        std::get<0>(a)+std::get<0>(b), 
        std::get<1>(a)+std::get<1>(b)); });
  scc &= std::get<0>(out) == std::get<1>(out);
  print(out); 

  auto res = stc::range(1,10,1)
    | stc::attach(process)
    | stc::enumerate(1)
    | stc::top<2>([](auto& a, auto& b){ 
      return std::get<1>(a) > std::get<1>(b); });
  scc &= res.size() == 2;
  scc &= std::get<1>(res.front()) > std::get<1>(res.back());
  print(res);
    
  return scc ? 0 : -1; 
}
