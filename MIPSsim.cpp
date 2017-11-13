#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <bitset>
#include <queue>
#include <functional>
#include <assert.h>
//#include "test.h"
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
		void Reset() {_bFinished = false;}
		void SetDstAddr(int dstAddr){_dstAddr = dstAddr;}
		int  GetDstAddr()const{return _dstAddr;}
		template <size_t N>
		int  GetSignedNumber(const std::string& inst);
		void PrintDataFormat();
		inline bool Finished()const {return _bFinished;} 
		inline bool Jump() const {return _bNeedJmp;}
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
		void SRL();
		void SRA();
		void NOP();
		void JR();

	private:
		std::ofstream             _disassemblefile;
		std::ofstream             _simulatefile;
		std::string               _InstruBuf;
		std::bitset<InstLen>      _Instruction;
		std::bitset<RegLen>       _Reg;
		std::queue<std::string>   _OpFormatQueue;  //now can be useless
		std::queue<std::string>   _DataFormatQueue;
		std::queue<std::string>   _DataInstruction; // store 32 bit Data Instruction
		typedef  int  DataValue;
		std::vector<DataValue>    _vecRegister;
		size_t                    _startPos;  
		bool                      _bFinished;
		bool                      _bNeedJmp;
		int                       _currentAddr;
		int                       _dstAddr;
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
							{"SW",std::bind(&MemParser::SW,this)},{"BREAK",std::bind(&MemParser::BREAK,this)},
							{"SRL",std::bind(&MemParser::SRL,this)},{"SRA", std::bind(&MemParser::SRA,this)},
							{"NOP",std::bind(&MemParser::NOP,this)},{"JR",std::bind(&MemParser::JR)},
							{"AND",std::bind(&MemParser::AND,this)}};
		typedef std::map<int,int> DATAMAP;	
		DATAMAP                   _dataMap;      // store data of memory
		typedef std::map<int,std::string> OPMAP; //key: addr  value:instruction
		OPMAP                     _opMap;


};
MemParser::MemParser()
	:_bFinished(false),
	_bNeedJmp(false),
	_currentAddr(g_startAddress),
	_dstAddr(-1),
	_vecRegister(32)  //32 registers
{}
MemParser::~MemParser()
{
}

void MemParser::InitDataMap()
{
	//cout<<"data instruction's size: "<<_DataInstruction.size()<<endl;
	while (!_DataInstruction.empty())
	{
		//push to dataformat queue for final output data
		_DataFormatQueue.push(Complement(_currentAddr,_DataInstruction.front()));
		_currentAddr += 4;
		_DataInstruction.pop();
	}
}

void MemParser::PrintDataFormat()
{
	while (!_DataFormatQueue.empty())
	{
		_disassemblefile<<_DataFormatQueue.front()<<endl;
		_DataFormatQueue.pop();
	}
}

void MemParser::MemParse()
{
	_disassemblefile.open("disassembly.txt");
	_simulatefile.open("simulation.txt");
	InitDataMap(); //data: addr 340 ~ addr 432
	ParseInstruction();
	_simulatefile.close();
	//print Data instruction to disassembly.txt
	PrintDataFormat();
	_disassemblefile.close();
}

void MemParser::ParseInstruction()
{
	Reset(); //at the end of BREAK() set _Finished
	auto it = _opMap.begin();
	int cycle = 1; // for debug
	while (it != _opMap.end() && !Finished())
	{
		if (Jump())
		{
			int dstAddr = GetDstAddr();
			it = _opMap.find(dstAddr);
		}
		g_startAddress = it->first;
		_Instruction = getBitset<InstLen>(it->second);
		_InstruBuf = _Instruction.to_string();
		assert(InstLen == _Instruction.size() && InstLen == _InstruBuf.length());
		ParseEachInstruction();
		it ++;
		cycle ++;
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
	int count = 0;
	static int cycle = 1;
	for (int i = 0; i<20;i++)
		_simulatefile<<"-";
	_simulatefile<<endl;
	std::ostringstream oss;
	oss<<"Cycle:"<<cycle;
	if (cycle < 10) oss<<" "<<"\t"<<format<<endl<<endl;
	else oss<<"\t"<<format<<endl<<endl;
	_simulatefile<<oss.str();
	_simulatefile<<"Registers";
	for (auto it = _vecRegister.begin(); it != _vecRegister.end();++it)
	{
		if (count % 8 == 0)
		{
			_simulatefile<<endl<<"R";
			_simulatefile.setf(std::ios::right);
			_simulatefile.fill('0');
			_simulatefile.width(2);
			_simulatefile<<count<<":\t";
		}
		_simulatefile<<*it<<"\t";
		count++;
	}
	_simulatefile<<endl;
	count = 0;
	_simulatefile<<"Data";
	for (auto it = _dataMap.begin(); it != _dataMap.end(); ++it)
	{
		if (count % 8 == 0)
		{
			_simulatefile<<endl<<it->first<<":\t";
		}
		_simulatefile<<it->second<<"\t";
		count++;
	}
	_simulatefile<<endl;
	cycle ++;
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
	_disassemblefile<<oss.str()<<endl;
	//(rs) +(rt) -> (rd)
	_vecRegister[idxD] = _vecRegister[idxS] + _vecRegister[idxT];
	Output2File(format.str());
	g_startAddress += 4;
	_bNeedJmp = false;
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
	_disassemblefile<<oss.str()<<endl;
	//rs - rt -> rd
	_vecRegister[idxD] = _vecRegister[idxS] - _vecRegister[idxT];
	Output2File(format.str());
	g_startAddress += 4;
	_bNeedJmp = false;
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
	_disassemblefile<<oss.str()<<endl;
	int immed = GetSignedNumber<16>(_InstruBuf.substr(_startPos+2*RegLen,16));
	//rs + immediate -> rt
	_vecRegister[idxT] = _vecRegister[idxS] + immed;
	Output2File(format.str());
	g_startAddress += 4;
	_bNeedJmp = false;
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
	offset = GetSignedNumber<18>(Offset);
	sprintf(immediate, "#%d",offset);
	format<<g_startAddress<<"\t"<<"BEQ "<<regS<<", "<<regT<<", "<<immediate;
	_OpFormatQueue.push(format.str());
	oss<<_InstruBuf<<"\t"<<format.str();
	//if rs = rt then branch
	if (_vecRegister[idxS] == _vecRegister[idxT])
	{
		SetDstAddr(g_startAddress + offset + 4);
		_bNeedJmp = true; // modify pc
	}else{
		_bNeedJmp = false;
		g_startAddress += 4;
	}
	Output2File(format.str());
//	cout<<oss.str()<<endl;
	_disassemblefile<<oss.str()<<endl;
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
	offset = GetSignedNumber<18>(Offset); 
	sprintf(regOffset,"#%d",offset);
	format<<g_startAddress<<"\t"<<"BLTZ "<<regS<<", "<<regOffset;
	_OpFormatQueue.push(format.str());
	oss<<_InstruBuf<<"\t"<<format.str();
//	cout<<oss.str()<<endl;
	_disassemblefile<<oss.str()<<endl;
	// if rs < 0 , then  branch
	if (_vecRegister[idxS] < 0)
	{
		SetDstAddr(g_startAddress + offset + 4);	
		_bNeedJmp = true; //modify pc
	}
	else{
		_bNeedJmp = false;
		g_startAddress += 4;
	}
	Output2File(format.str());
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
	//because pc addr can not be a negative
	std::bitset<28> InstrSet = getBitset<28>(indexStr);
	sprintf(instr_index,"#%lu",InstrSet.to_ulong());
	format<<g_startAddress<<"\t"<<"J "<<instr_index;
	_OpFormatQueue.push(format.str());
	oss<<_InstruBuf<<"\t"<<format.str();
//	cout<<oss.str()<<endl;
	_disassemblefile<<oss.str()<<endl;
	//offset -> pc
	SetDstAddr(InstrSet.to_ulong()); // modify pc
	Output2File(format.str());
	_bNeedJmp = true;
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
	offset = GetSignedNumber<18>(Offset);
	sprintf(regOffset, "#%d", offset);
	format<<g_startAddress<<"\t"<<"BGTZ "<<regS<<", "<<regOffset;
	_OpFormatQueue.push(format.str());
	oss<<_InstruBuf<<"\t"<<format.str();
//	cout<<oss.str()<<endl;
	_disassemblefile<<oss.str()<<endl;
	//if rs > 0 then branch
	if (_vecRegister[idxS] > 0)
	{
		SetDstAddr(g_startAddress + offset + 4); //modify pc
		_bNeedJmp = true;
	}else{
		_bNeedJmp = false;
		g_startAddress += 4;	
	}
	Output2File(format.str());
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
//	cout<<output<<endl;
	_disassemblefile<<oss.str()<<endl;
	//rt << sa -> rd
	_vecRegister[idxD] = (_vecRegister[idxT] << sa);
	Output2File(format.str());
	g_startAddress += 4;
	_bNeedJmp = false;
}

void MemParser::SRL() //logical shift right
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
	//  cout<<output<<endl;
	_disassemblefile<<oss.str()<<endl;
	//rt >> sa ->rd
	unsigned int tmp = _vecRegister[idxT];
	_vecRegister[idxD] = (tmp >> sa);
	Output2File(format.str());
	g_startAddress += 4;
	_bNeedJmp = false;
}

void MemParser::SRA() //arighmetic shift right
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
	//  cout<<output<<endl;
	_disassemblefile<<oss.str()<<endl;
	//rt >> sa ->rd
	_vecRegister[idxD] = ( _vecRegister[idxT] >> sa);
	Output2File(format.str());
	g_startAddress += 4;
	_bNeedJmp = false;
}

void MemParser::NOP()
{
	std::stringstream oss, format;
	format<<g_startAddress<<"\t"<<"NOP";
	_OpFormatQueue.push(format.str());
	oss<<_InstruBuf<<"\t"<<format.str();
//	cout<<oss.str()<<endl;
	_disassemblefile<<oss.str()<<endl;
	Output2File(format.str());
	g_startAddress += 4;
}

void MemParser::JR()
{

}

void MemParser::AND()
{
	_startPos += LenOfOpcode;
	char regS[8], regT[8], regD[8];
	int idxS, idxT, idxD;
	std::ostringstream oss,format;
	_Reg = getBitset<RegLen>(_InstruBuf.substr(_startPos,LenOfRegS));
	idxS = _Reg.to_ulong();

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
	sprintf(regBase, "R%d", base);
	offset = GetSignedNumber<16>(_InstruBuf.substr(_startPos+2*RegLen,16));
	sprintf(regOffset,"%d",offset);
	format<<g_startAddress<<"\t"<<"LW "<<regT<<", "<<offset<<"("<<regBase<<")";
	_OpFormatQueue.push(format.str());
	oss<<_InstruBuf<<"\t"<<format.str();
//	cout<<oss.str()<<endl;
	_disassemblefile<<oss.str()<<endl;
	//memory[base+offset] ->rt
	auto it = _dataMap.find(_vecRegister[base]+offset);
	if (it != _dataMap.end())
	{
		_vecRegister[idxT] = it->second;
	}
	Output2File(format.str());
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
	_disassemblefile<<oss.str()<<endl;
	//rs * rt -> rd
	_vecRegister[idxD] = _vecRegister[idxS] * _vecRegister[idxT];
	Output2File(format.str());
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
	idxOffset = GetSignedNumber<16>(_InstruBuf.substr(_startPos+2*RegLen,16));
	sprintf(regOffset,"%d",idxOffset);
	format<<g_startAddress<<"\t"<<"SW "<<regT<<", "<<regOffset<<"("<<regBase<<")";
	_OpFormatQueue.push(format.str());
	oss<<_InstruBuf<<"\t"<<format.str();
//	cout<<oss.str()<<endl;
	_disassemblefile<<oss.str()<<endl;
	//rt -> memory[base+offset]
	auto it = _dataMap.find(_vecRegister[idxBase] + idxOffset);
	if (it != _dataMap.end())
	{
		it->second = _vecRegister[idxT];
	}
	_bNeedJmp = false;
	Output2File(format.str());
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
	_disassemblefile<<oss.str()<<endl;
	g_startAddress += 4;
	Output2File(format.str());
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
template <size_t N>
int MemParser::GetSignedNumber(const std::string& offset)
{
	bool bPositive = false;
	std::bitset<N> instset(offset);
	int num;
	if (instset.test(N-1))
	{
		bPositive = false;//negative
		std::bitset<N> tmpset(instset);
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
		for (int i = 1;i<N;i++)
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
		num = tmpset.to_ulong() * (-1);
	}
	else //positive
	{
		bPositive = true;
		num = instset.to_ulong();
	}
	return num;
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
		_opMap.insert(std::make_pair(_currentAddr,str));
		_currentAddr += 4;
	}
	else
	{
		_DataInstruction.push(str);
	}
}

void MemParser::output() // for debug
{
	for (auto it = _opMap.begin(); it != _opMap.end();++it)
	{
		cout<<it->first<<":  "<<it->second<<endl;
	}
	cout<<endl;
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

};
MIPSsim::MIPSsim(const char* filename)
{
	_infile.open(filename, std::ios::binary);
}
MIPSsim::~MIPSsim()
{
}
int MIPSsim::Parse()
{
	std::string buf;
	while (getline(_infile,buf))
	{
		_memParser.GetInstructions(buf);
	}
	_infile.close();
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
	return 0;
}
