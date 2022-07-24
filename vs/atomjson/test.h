#pragma once
#include<iostream>

class test
{
public:
	test(int a,int b):a(a),b(b){}
	int a;
	int b;
	void print(){ std::cout << a << " " << b << std::endl; }
};

