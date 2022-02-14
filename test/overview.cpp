// ========================================================================
// helpers
// ======================================================================== 
template<typename T>
auto print(const T& vec){
  std::cout << "[ ";
  for(auto& v: vec) {
    std::cout << v << " ";
  }
  std::cout << "]" << std::endl;
};  

template<typename...Ts, std::size_t N>
auto print(
  const stc::vector<std::tuple<Ts...>,N>& vec){
  std::cout << "[ ";
  for(auto& v: vec) {
    //sizeof...(Ts)
    std::cout << "{"
      << std::get<0>(v) << ", " 
      << std::get<1>(v)
      << "} ";
  }
  std::cout << "]" << std::endl;
};  

// ========================================================================
// pipelines
// ======================================================================== 
auto process = [](auto&& step){
  return std::move(step)
    | stc::filter([](auto& d){ return d%2==0; })
    | stc::transform([](auto& d){ return 0.1+d*d; })
    | stc::transform_scan([](auto& a, auto& b){ return a+b; })
    | stc::transform_adjacent([](auto& a, auto& b){ return a-b; });
};

// ========================================================================
// entry point
// ======================================================================== 
int main() {
  auto vec = stc::vector<int, 10>{1,8,3,4,5,6};
  auto out = vec
    | stc::filter([](auto& d){ return d%2==0; })
    | stc::transform([](auto& d){ return 0.1+d*d; })
    | stc::transform_scan([](auto& a, auto& b){ return a+b; })
    | stc::transform_adjacent([](auto& a, auto& b){ return a-b; })
   // | stc::reduce([](auto& a, auto& b){ return a+b; });
    | stc::zip(vec
      | stc::attach(process)
      | stc::head<3>())
    | stc::head<3>();
   // | stc::rank<3>([](auto& a, auto&b){ return a>b; });
  print(out); 
  //print(stc::make_vector(out));
  
  auto res = stc::range(1,10,1)
    | stc::attach(process)
    | stc::enumerate(1)
    | stc::head<3>();
  //auto v = process_y(vec)
  // | stc::head<3>();
  
  print(res);
    
 // auto o = process_x(vec);
  //print(o); 
}
