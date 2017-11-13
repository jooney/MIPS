#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>
#include <map>
//#include <vector>
#include <bitset>
#include <queue>
#include <functional>
#include <assert.h>
#include "test.h"
using std::endl;
using std::cout;

const int STARTADDRESS = 256;
int g_startAddress = 256;
const size_t InstLen = 32;
const size_t RegLen  = 5;

class MemParser
{
	public:
		MemParser();
		~MemParser();
		void   GetInstructions(const std::string& buf);
		void   MemParse();
		void   SimulateTrace();
		void   output(); // for debug
		template <size_t N>
		std::bitset<N> getBitset(const std::string& buf);
	private:
		bool IsCategory1(const std::string& buf) const;
		bool IsCategory2(const std::string& buf) const;
		std::string Complement(int startAddr,const std::string& inst);
		void InitDataMap();
		void ParseInstruction();
		void ParseEachInstruction();
		void JudgeBreak(const std::string& inst);
		void Output2File(const std::string& format); // param:file handler
		inline bool Finished()const {return _bFinished;} 
		void ADD();
		void SUB();
		void BEQ();
		void MUL();
		void AND();
		void OR();
		void XOR();
		void NOR();
		void SLT();
		void ADDI();
		void ANDI();
		void ORI();
		void XORI();
		void SLL();
		void LW();
		void BGTZ();
		void J();
		void BLTZ();
		void SW();
		void BREAK();

	private:
		std::string               _InstruBuf;
		std::bitset<InstLen>      _Instruction;
		std::bitset<RegLen>       _Reg;
		std::queue<std::string>   _OpFormatQueue;  //now can be useless
		std::queue<std::string>   _DataFormatQueue;
		std::queue<std::string>   _OpInstruction;  //store 32 bit Op Instruction
		std::queue<std::string>   _DataInstruction; // store 32 bit Data Instruction
		typedef  int  DataValue;
		std::vector<DataValue>    _vecRegister;
		size_t                    _startPos;  //340
		bool                      _bFinished;
		int                       _currentDataAddr;
		const size_t  LenOfOpcode     = 4;
		const size_t  LenOfRegS       = 5;
		const size_t  LenOfRegT       = 5;
		const size_t  LenOfRegD       = 5;
		const size_t  LenOfType       = 2;
		//key: Identification bits value:Instruction
		typedef std::map<std::string,std::string> OPCODE1; 
		OPCODE1 _Opcode1 = {{"0000","J"},{"0001","JR"},{"0010","BEQ"},{"0011","BLTZ"},{"0100","BGTZ"},
						  {"0101","BREAK"},{"0110","SW"},{"0111","LW"},{"1000","SLL"},{"1001","SRL"},
				          {"1010","SRA"},{"1011","NOP"}};
		typedef std::map<std::string,std::string> OPCODE2;
		OPCODE2 _Opcode2 = {{"0000","ADD"},{"0001","SUB"},{"0010","MUL"},{"0011","AND"},{"0100","OR"},
                          {"0101","XOR"},{"0110","NOR"},{"0111","SLT"},{"1000","ADDI"},{"1001","ANDI"},
                          {"1010","ORI"},{"1011","XORI"}};
		typedef std::map<std::string,std::function<void()>> OP2FUNC;
		OP2FUNC  _Op2Func = {{"ADD",std::bind(&MemParser::ADD,this)},{"SUB",std::bind(&MemParser::SUB,this)},
		                    {"ADDI",std::bind(&MemParser::ADDI,this)},{"BEQ",std::bind(&MemParser::BEQ,this)},
							{"SLL",std::bind(&MemParser::SLL,this)},{"LW",std::bind(&MemParser::LW,this)},
							{"MUL",std::bind(&MemParser::MUL,this)},{"BGTZ",std::bind(&MemParser::BGTZ,this)},
							{"J",std::bind(&MemParser::J,this)},{"BLTZ",std::bind(&MemParser::BLTZ,this)},
							{"SW",std::bind(&MemParser::SW,this)},{"BREAK",std::bind(&MemParser::BREAK,this)}};
		typedef std::map<int,int> DATAMAP;	
		DATAMAP                   _dataMap;


};
MemParser::MemParser()
	:_bFinished(false),
	_currentDataAddr(g_startAddress),
	_vecRegister(32)  //32 registers
{}
MemParser::~MemParser()
{
}

void MemParser::InitDataMap()
{
	cout<<_DataInstruction.size()<<endl;
	while (!_DataInstruction.empty())
	{
		//push to dataformat queue for final output data
		_DataFormatQueue.push(Complement(_currentDataAddr,_DataInstruction.front()));
		_currentDataAddr += 4;
		_DataInstruction.pop();
	}
}

void MemParser::MemParse()
{
	InitDataMap(); //data: addr 340 ~ addr 432
	ParseInstruction();
	//print Data instruction to disassembly.txt
}

void MemParser::ParseInstruction()
{
	while (!_OpInstruction.empty())
	{
		_Instruction = getBitset<InstLen>(_OpInstruction.front());
		_InstruBuf = _Instruction.to_string();
		assert(InstLen == _Instruction.size() && InstLen == _InstruBuf.length());
		ParseEachInstruction();
		_OpInstruction.pop();
	}
}

void MemParser::ParseEachInstruction()
{
	_startPos = 0;
	_startPos += LenOfType ;
	std::string Opcode(_InstruBuf.substr(_startPos,LenOfOpcode));
	if (IsCategory1(_InstruBuf))
	{
		OPCODE1::iterator it = _Opcode1.find(Opcode);
		if (it != _Opcode1.end())
		{
			OP2FUNC::iterator iter = _Op2Func.find(it->second);
			if (iter != _Op2Func.end())
				(iter->second)();
		}
		else
		{
			//for debug
			cout<<"opcode2 can't be found"<<endl;
		}
	}
	else if (IsCategory2(_InstruBuf))
	{
		OPCODE2::iterator it = _Opcode2.find(Opcode);
		if (it != _Opcode2.end())
		{
			OP2FUNC::iterator iter = _Op2Func.find(it->second);
			if (iter != _Op2Func.end())
				(iter->second)();
		}
		else
		{
			//for debug
			cout<<"opcode2 can't be found"<<endl;
		}
	}
	//cout<<_Instruction<<endl;
}

template <size_t N>
std::bitset<N> MemParser::getBitset(const std::string& buf)
{
	return std::bitset<N>(buf);
}
bool MemParser::IsCategory1(const std::string& buf) const
{
	if (buf[0] == '0' && buf[1] == '1')
		return true;
	return false;
}
bool MemParser::IsCategory2(const std::string& buf) const
{
	if (buf[0] == '1' && buf[1] == '1')
		return true;
	return false;
}

void MemParser::Output2File(const std::string& format)
{
	int count = 0,cycle;
	for (int i = 0; i<20;i++)
		cout<<"-";
	cout<<endl;
	cycle = (g_startAddress - STARTADDRESS)/4 +1;
	std::ostringstream oss;
	oss<<"Cycle:"<<cycle;
	if (cycle < 10) oss<<" "<<"\t"<<format<<endl<<endl;
	else oss<<"\t"<<format<<endl<<endl;
	cout<<oss.str();
//	cout<<"Registers";
//	for (auto it = _vecRegister.begin(); it != _vecRegister.end();++it)
//	{
//		if (count % 8 == 0)
//		{
//			cout<<endl<<"R";
//			cout.setf(std::ios::right);
//			cout.fill('0');
//			cout.width(2);
//			cout<<count<<":\t";
//		}
//		cout<<*it<<"\t";
//		count++;
//	}
//	cout<<endl;
//	count = 0;
	cout<<"Data";
	for (auto it = _dataMap.begin(); it != _dataMap.end(); ++it)
	{
		if (count % 8 == 0)
		{
			cout<<endl<<it->first<<":\t";
		}
		cout<<it->second<<"\t";
		count++;
	}
	cout<<endl;
}

void MemParser::ADD()
{
	_startPos += LenOfOpcode;
	_Reg = getBitset<RegLen>(_InstruBuf.substr(_startPos+2*RegLen,LenOfRegD)); 
	char regS[8] = {0},regT[8] = {0},regD[8] = {0};
	int idxS, idxT, idxD, cycle;
	std::ostringstream oss, format;
	idxD = _Reg.to_ulong(); //rd
	sprintf(regD, "R%d", idxD);
	_Reg = getBitset<RegLen>(_InstruBuf.substr(_startPos,LenOfRegS));
	idxS = _Reg.to_ulong();//rs
	sprintf(regS, "R%d", idxS);
	_Reg = getBitset<RegLen>(_InstruBuf.substr(_startPos+RegLen,LenOfRegT));
	idxT = _Reg.to_ulong();//rt
	sprintf(regT, "R%d", idxT);
	format<<g_startAddress<<"\t"<<"ADD "<<regD<<", "<<regS<<", "<<regT;
	_OpFormatQueue.push(format.str());
	oss<<_InstruBuf<<"\t"<<format.str(); 
//	cout<<oss.str()<<endl;
	//(rs) +(rt) -> (rd)
	_vecRegister[idxD] = _vecRegister[idxS] + _vecRegister[idxT];
	Output2File(format.str());
	g_startAddress += 4;
}

void MemParser::SUB()
{
	_startPos += LenOfOpcode;
	_Reg = getBitset<RegLen>(_InstruBuf.substr(_startPos+2*RegLen,LenOfRegD));
	char regS[8] = {0},regT[8] = {0},regD[8] = {0};
	int idxS, idxT, idxD;
	std::ostringstream oss, format;
	idxD = _Reg.to_ulong(); //rd
	sprintf(regD, "R%d", idxD);
	_Reg = getBitset<RegLen>(_InstruBuf.substr(_startPos,LenOfRegS));
	idxS = _Reg.to_ulong();//rs
	sprintf(regS, "R%d", idxS);
	_Reg = getBitset<RegLen>(_InstruBuf.substr(_startPos+RegLen,LenOfRegT));
	idxT = _Reg.to_ulong();//rt
	sprintf(regT, "R%d", idxT);
	format<<g_startAddress<<"\t"<<"SUB "<<regD<<", "<<regS<<", "<<regT;
	_OpFormatQueue.push(format.str());
	oss<<_InstruBuf<<"\t"<<format.str();
//	cout<<oss.str()<<endl;
	g_startAddress += 4;
}

void MemParser::ADDI()
{
	_startPos += LenOfOpcode;
	char regS[8] = {0}, regT[8] = {0}, immediate[16] = {0};
	_Reg = getBitset<RegLen>(_InstruBuf.substr(_startPos+RegLen,LenOfRegT));
	int idxS, idxT;
	std::ostringstream oss,format;
	idxT = _Reg.to_ulong(); //rt
	sprintf(regT, "R%d", idxT);
	_Reg = getBitset<RegLen>(_InstruBuf.substr(_startPos,LenOfRegS));
	idxS = _Reg.to_ulong();//rs
	sprintf(regS, "R%d", idxS);
	std::bitset<16> immset;
	immset = getBitset<16>(_InstruBuf.substr(_startPos+2*RegLen,16));//immediate
	sprintf(immediate,"#%lu",immset.to_ulong());
	format<<g_startAddress<<"\t"<<"ADDI "<<regT<<", "<<regS<<", "<<immediate;
	_OpFormatQueue.push(format.str());
	oss<<_InstruBuf<<"\t"<<format.str();
//	cout<<oss.str()<<endl;
	//rs + i
	Output2File(format.str());
	g_startAddress += 4;
}

void MemParser::BEQ()
{
	_startPos += LenOfOpcode;
	char regS[8] = {0}, regT[8] = {0},immediate[18] = {0};
	_Reg = getBitset<RegLen>(_InstruBuf.substr(_startPos+RegLen,LenOfRegT));
	int idxS, idxT, offset;
	std::ostringstream oss,format;
	idxT = _Reg.to_ulong(); //rt
	sprintf(regT, "R%d", idxT);
	_Reg = getBitset<RegLen>(_InstruBuf.substr(_startPos,LenOfRegS));
	idxS = _Reg.to_ulong();//rs
	sprintf(regS, "R%d", idxS);
	std::bitset<16> immset ;
	immset = getBitset<16>(_InstruBuf.substr(_startPos+2*RegLen,16));
	std::string Offset = immset.to_string(),output;
	Offset += "00";
	std::bitset<18> Immediate = getBitset<18>(Offset); //<<2  fixme shift 2 bit 
	sprintf(immediate,"#%lu",Immediate.to_ulong());
	format<<g_startAddress<<"\t"<<"BEQ "<<regS<<", "<<regT<<", "<<immediate;
	_OpFormatQueue.push(format.str());
	oss<<_InstruBuf<<"\t"<<format.str();
//	cout<<oss.str()<<endl;
	g_startAddress += 4;
}

void MemParser::BLTZ()
{
	_startPos += LenOfOpcode;
	char regS[8] = {0},regOffset[18] = {0};
	int idxS, offset;
	std::ostringstream oss,format;
	_Reg = getBitset<RegLen>(_InstruBuf.substr(_startPos,LenOfRegS));
	idxS = _Reg.to_ulong(); //rs
	sprintf(regS, "R%d", idxS);
	std::bitset<16> offsetset;
	offsetset = getBitset<16>(_InstruBuf.substr(_startPos+2*RegLen,16));
	std::string Offset = offsetset.to_string();
	Offset += "00";
	std::bitset<18> OffsetSet = getBitset<18>(Offset); //<<2  fixme shift 2 bit 
	sprintf(regOffset,"#%lu",OffsetSet.to_ulong());
	format<<g_startAddress<<"\t"<<"BLTZ "<<regS<<", "<<regOffset;
	_OpFormatQueue.push(format.str());
	oss<<_InstruBuf<<"\t"<<format.str();
//	cout<<oss.str()<<endl;
	g_startAddress += 4;
}

void MemParser::J()
{
	_startPos += LenOfOpcode;
	char instr_index[28] = {0};
	int index;
	std::stringstream oss,format;
	std::bitset<26> instrset = getBitset<26>(_InstruBuf.substr(_startPos,26));
	std::string indexStr = instrset.to_string();
	indexStr += "00";
	std::bitset<28> InstrSet = getBitset<28>(indexStr);//<<2 fixme shift 2 bit
	sprintf(instr_index,"#%lu",InstrSet.to_ulong());
	format<<g_startAddress<<"\t"<<"J "<<instr_index;
	_OpFormatQueue.push(format.str());
	oss<<_InstruBuf<<"\t"<<format.str();
//	cout<<oss.str()<<endl;
	g_startAddress += 4;
}

void MemParser::BGTZ()
{
	_startPos += LenOfOpcode;
	char regS[8] = {0}, regOffset[18] = {0};
	int idxS ,offset;
	std::stringstream oss,format;
	_Reg = getBitset<RegLen>(_InstruBuf.substr(_startPos,LenOfRegS));
	idxS = _Reg.to_ulong();//rs
	sprintf(regS, "R%d",idxS);
	std::bitset<16> offsetset ;
	offsetset = getBitset<16>(_InstruBuf.substr(_startPos+2*RegLen,16));
	std::string Offset = offsetset.to_string();
	Offset += "00";
	std::bitset<18> offbitset= getBitset<18>(Offset);//<<2  fixme shift 2 bit
	sprintf(regOffset, "#%lu", offbitset.to_ulong());
	format<<g_startAddress<<"\t"<<"BGTZ "<<regS<<", "<<regOffset;
	_OpFormatQueue.push(format.str());
	oss<<_InstruBuf<<"\t"<<format.str();
//	cout<<oss.str()<<endl;
	g_startAddress += 4;	
}

void MemParser::SLL()
{
	_startPos += LenOfOpcode;
	char regD[8] = {0}, regT[8] = {0},regSA[8] = {0};
	_Reg = getBitset<RegLen>(_InstruBuf.substr(_startPos+2*RegLen,LenOfRegD));
	int idxD, idxT, sa;
	std::ostringstream oss,format;
	idxD = _Reg.to_ulong(); //rd
	sprintf(regD, "R%d", idxD);
	_Reg = getBitset<RegLen>(_InstruBuf.substr(_startPos+RegLen,LenOfRegT));
	idxT = _Reg.to_ulong(); //rt
	sprintf(regT,"R%d", idxT);
	_Reg = getBitset<RegLen>(_InstruBuf.substr(_startPos+3*RegLen,RegLen));
	sa = _Reg.to_ulong();
	sprintf(regSA,"#%d",sa);
	format<<g_startAddress<<"\t"<<"SLL "<<regD<<", "<<regT<<", "<<regSA;
	_OpFormatQueue.push(format.str());
	oss<<_InstruBuf<<"\t"<<format.str();
	std::string output = oss.str();
//	cout<<output<<endl;
	g_startAddress += 4;
}

void MemParser::LW()
{
	_startPos += LenOfOpcode;
	char regT[8] = {0}, regBase[8] = {0}, regOffset[16] = {0};
	int idxT, base, offset;
	std::ostringstream oss,format;
	_Reg = getBitset<RegLen>(_InstruBuf.substr(_startPos+RegLen,LenOfRegT));
	idxT = _Reg.to_ulong();
	sprintf(regT, "R%d", idxT);//rt
	_Reg = getBitset<RegLen>(_InstruBuf.substr(_startPos,RegLen));
	base = _Reg.to_ulong();
	sprintf(regBase, "%d", base);
	std::bitset<16> offsetset;
	offsetset = getBitset<16>(_InstruBuf.substr(_startPos+2*RegLen,16));
	offset = offsetset.to_ulong();
	sprintf(regOffset,"%d",offset);
	format<<g_startAddress<<"\t"<<"LW "<<regT<<", "<<offset<<"("<<regBase<<")";
	_OpFormatQueue.push(format.str());
	oss<<_InstruBuf<<"\t"<<format.str();
//	cout<<oss.str()<<endl;
	g_startAddress += 4;
}

void MemParser::MUL()
{
	_startPos += LenOfOpcode;
	char regS[8] = {0}, regT[8] = {0}, regD[8] = {0};
	int idxS, idxT, idxD;
	std::stringstream oss,format;
	_Reg = getBitset<RegLen>(_InstruBuf.substr(_startPos+2*RegLen,LenOfRegD));
	idxD = _Reg.to_ulong();
	sprintf(regD,"R%d",idxD); //rd
	_Reg = getBitset<RegLen>(_InstruBuf.substr(_startPos,LenOfRegS));
	idxS = _Reg.to_ulong();
	sprintf(regS,"R%d",idxS);//rs
	_Reg = getBitset<RegLen>(_InstruBuf.substr(_startPos+RegLen,LenOfRegT));
	idxT = _Reg.to_ulong();
	sprintf(regT,"R%d",idxT);//rt
	format<<g_startAddress<<"\t"<<"MUL "<<regD<<", "<<regS<<", "<<regT;
	_OpFormatQueue.push(format.str());
	oss<<_InstruBuf<<"\t"<<format.str();
//	cout<<oss.str()<<endl;
	g_startAddress += 4;
}

void MemParser::SW()
{
	_startPos += LenOfOpcode;
	char regBase[8] = {0}, regT[8] = {0},regOffset[16] = {0};
	std::stringstream oss,format;
	int idxBase, idxT, idxOffset;
	_Reg = getBitset<RegLen>(_InstruBuf.substr(_startPos+RegLen,LenOfRegT));
	idxT = _Reg.to_ulong();
	sprintf(regT,"R%d",idxT); //rt
	_Reg = getBitset<RegLen>(_InstruBuf.substr(_startPos,RegLen));
	idxBase = _Reg.to_ulong();
	sprintf(regBase,"R%d",idxBase);//rbase
	std::bitset<16> offsetset;
	offsetset = getBitset<16>(_InstruBuf.substr(_startPos+2*RegLen,16));
	idxOffset = offsetset.to_ulong();
	sprintf(regOffset,"%d",idxOffset);
	format<<g_startAddress<<"\t"<<"SW "<<regT<<", "<<regOffset<<"("<<regBase<<")";
	_OpFormatQueue.push(format.str());
	oss<<_InstruBuf<<"\t"<<format.str();
//	cout<<oss.str()<<endl;
	g_startAddress += 4;
}

void MemParser::BREAK()
{
	_startPos += LenOfOpcode;
	std::stringstream oss,format;
	format<<g_startAddress<<"\t"<<"BREAK";
	_OpFormatQueue.push(format.str());
	oss<<_InstruBuf<<"\t"<<format.str();
//	cout<<oss.str()<<endl;
	g_startAddress += 4;
	_bFinished = true;
}

std::string MemParser::Complement(int startAddr, const std::string& inst)
{
	//positive or negative
	bool bPositive = false;
	std::bitset<InstLen> instset(inst);
	int num;
	if (instset.test(31)) 
	{
		bPositive = false; //negative
		std::bitset<InstLen> tmpset(instset); //= inst;
		tmpset.flip();
		bool bCarry = false;
		//diwei add 1
		if (tmpset.test(0)) 
		{
			tmpset.reset(0);
			bCarry = true;
		}
		else
		{
			tmpset.set(0);
			bCarry = false;
		}
		for (int i = 1;i<InstLen;i++)
		{
			if (bCarry && !tmpset.test(i))
			{
				tmpset.set(i);//set current pos 1
				bCarry = false;
			}
			else if (bCarry && tmpset.test(i))
			{
				tmpset.reset(i);//set current pos 0
				bCarry = true;
			}
			else
				bCarry = false;
		}
		num = tmpset.to_ulong();
	}
	else //positive
	{
		bPositive = true;
		std::bitset<InstLen> bit(inst);
		num = bit.to_ulong();
	}
	std::ostringstream oss;
	if (!bPositive)
	{
		_dataMap.insert(std::make_pair(startAddr,num*-1));
		oss<<inst<<"\t"<<startAddr<<"\t"<<"-"<<num;
	}else{
		_dataMap.insert(std::make_pair(startAddr,num));
		oss<<inst<<"\t"<<startAddr<<"\t"<<num;
	}
//	cout<<oss.str()<<endl; 
	return oss.str();
}

void MemParser::SimulateTrace()
{
	int count = 0;
	while (!_OpFormatQueue.empty())
	{
		for (int i = 0;i<20;i++)
			cout<<"-";
		cout<<endl;
	//	cout<<"Cycle:"<<++count<<"\t"<<
	}
}

void MemParser::JudgeBreak(const std::string& inst)
{
	assert(inst.length() == InstLen);
	if (inst[0] == '0' && inst[1] == '1' && inst[2] == '0' && inst[3] == '1' 
		&& inst[4] == '0' && inst[5] == '1')
	{
		_bFinished = true;
	}
}

void MemParser::GetInstructions(const std::string& buf)
{
	std::bitset<InstLen> inst = getBitset<InstLen>(buf);	
	std::string str = inst.to_string();
	if (!Finished())
	{
		JudgeBreak(str);
		_currentDataAddr += 4;
		_OpInstruction.push(str);
	}
	else
	{
		_DataInstruction.push(str);
	}
}

void MemParser::output()
{
	while (!_OpInstruction.empty())
	{
		cout<<_OpInstruction.front()<<endl;
		_OpInstruction.pop();
	}
	while (!_DataInstruction.empty())
	{
		cout<<_DataInstruction.front()<<endl;
		_DataInstruction.pop();
	}
}

class MIPSsim
{
	public:
		explicit MIPSsim(const char* filename);
		~MIPSsim();
		int Parse();
	private:
		MemParser            _memParser;
		std::ifstream        _infile;
		std::ofstream        _disassemblefile;
		std::ofstream        _simulatefile;

};
MIPSsim::MIPSsim(const char* filename)
{
	_infile.open(filename, std::ios::binary);
}
MIPSsim::~MIPSsim()
{
	_infile.close();
	_disassemblefile.close();
	_simulatefile.close();
}
int MIPSsim::Parse()
{
	std::string buf;
	while (getline(_infile,buf))
	{
		_memParser.GetInstructions(buf);
	}
//	_memParser.output(); //for debug
	_memParser.MemParse();
	return 0;
}
int main(int argc, char*argv[])
{
	if (argc < 2){
		cout<<"file param lost"<<endl;
		return -1;
	}
	MIPSsim mipssim(argv[1]);
	mipssim.Parse();

//	std::ifstream fin("binary.txt",std::ios::binary);
//	char* txt = "11000000000000000000100000000000";
//	char* txt1 = "11100000000000100000000000000011";
//	std::string buf;
//	char identification[4],rs[5],rt[5],rd[5],lasteleven[11];
//	while (getline(fin,buf))
//	{
//		if (buf[0] == '1' && buf[1] == '1')
//			cout<<"11 ";
//		memcpy(identification,&buf[2],4);
//		memcpy(rs,&buf[5],5);
//		memcpy(rt,&buf[11],5);
//		memcpy(rd,&buf[16],5);
//		memcpy(lasteleven,&buf[21],11);
//		cout<<"rs: "<<rs<<" rt: "<<rt<<" rd: "<<rd<<" last: "<<lasteleven<<endl;
//
////		cout<<buf<<endl;
//	}
//	std::string str("11010");
//	std::bitset<5> bitvec(str); //= from_string<5>(str);
//	if (bitvec.test(0))
//	{
//		cout<<"pos 1: "<<1<<endl;
//	}
	//std::string ss(bitvec,2,2);
//	std::string ss = str.substr(2,3);
//	cout<<ss<<endl;
	//char buf[] = "110101";
	//char bf[10] = {0};
	//memset(bf,'1',10);
	//memcpy(bf,&buf[3],3);
	//cout<<bf<<endl;
//	std::vector<int> vec;
//	vec.reserve(10);
//	vec[0] = 10;vec[1] = 3;vec[2] = 5;
//	cout<<"size: "<<vec.size()<<endl;
//	cout<<"capacity: "<<vec.capacity()<<endl;
//	cout<<vec[0]<<" "<<vec[1]<<" "<<vec[2]<<endl;
	//std::vector<int>::iterator it;
	//for (it = vec.begin(); it != vec.end(); ++it)
	//{
	//	cout<<*it<<" ";
	//}
//	for (auto it = vec.begin(); it != vec.end(); ++it)
//	{
//		cout<<*it<<" ";
//	}
//	vec[0] = 10;
	//int cycle = 10;
	//std::ostringstream oss;
	//oss<<"hello"<<"\t"<<"world";
	//cout<<"Cycle:"<<cycle<<"\t"<<oss.str()<<endl;
	//cout<<"Cycle:"<<cycle<<"\t"<"hello"<<"\t"<<"world"<<endl;
	return 0;
}
