cmake_minimum_required(VERSION 3.16) 
project(lab03) 
set(CMAKE_CXX_STANDARD 14) 
add_compile_options(-pedantic) 
# add_compile_options(-fsanitize=address) 
# add_link_options(-fsanitize=address) 
add_executable(Compilerlab3
	utils.cpp
	lexer.cpp
	parser.cpp
	compile3.cpp
) 
target_compile_features(Compilerlab3 PRIVATE cxx_std_14) 