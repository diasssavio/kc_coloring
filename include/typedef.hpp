#ifndef TYPEDEF_HPP_
#define TYPEDEF_HPP_

#define MAX_DOUBLE 1e+75
#define EPSILON 1e-5

#define LOGS true

#include <sstream>

typedef std::pair < int, int > edge;

template<typename T>
T string_to(const std::string& s){
	std::istringstream i(s);
	T x;
	if (!(i >> x)) return 0;
	return x;
}

template<typename T>
std::string to_string2(const T& t){
  std::stringstream ss;
  if(!(ss << t)) return "";
  return ss.str();
}

#endif /* TYPEDEF_HPP_ */
