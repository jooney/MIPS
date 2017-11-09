#include <iostream>
#include <fstream>
#include <string>
using namespace std;
class MIPSsim
{
	public:
		explicit MIPSsim(const char* filename);
};
int main(int argc, char*argv[])
{
//	ofstream fout;
//	fout.open("output.txt");
//	fout<<"Here is a number: "<<150<<endl;
//	fout<<"Now here is a string: "<<"aaa"<<endl;
	ifstream infile;
	if (argc > 1){
		infile.open(argv[1],ios::binary);
	}
	else{
		std::cout<<"open file: "<<argv[1]<<" failed"<<std::endl;
		return -1;
	}
	string str;
	while (getline(infile,str))
	{
		cout<<str<<endl;;
	}
	infile.close();
	return 0;
}
