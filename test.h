#ifndef __TEST_H__
#define __TEST_H__

#include <iostream>
#include <string>
#include <map>
#include <functional>
using std::cout;
using std::endl;
class Test
{
	public:
		void Add(int);
		void Sub(int);
		void mul(int);
		void div(int);
		typedef std::map<std::string,std::function<void(int)>> STR2FUNC;
		STR2FUNC _str2Func = {{"Add",std::bind(&Test::Add,this,std::placeholders::_1)},
							{"Sub",std::bind(&Test::Sub,this,std::placeholders::_1)},
							{"mul",std::bind(&Test::mul,this,std::placeholders::_1)},
							{"div",std::bind(&Test::div,this,std::placeholders::_1)}};
};

void Test::Add(int a)
{
	cout<<"a: "<<a<<endl;
}
void Test::Sub(int a)
{
	cout<<"a: "<<a<<endl;
}
void Test::mul(int a)
{
	cout<<"a: "<<a<<endl;
}
void Test::div(int a)
{
	cout<<"a: "<<a<<endl;
}






#endif
