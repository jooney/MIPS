/*On my honor, I have neither given nor received unauthorized aid on this assignment*/
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <memory>
#include <bitset>
#include <queue>
#include <functional>
#include <assert.h>
using std::endl;
using std::cout;
using namespace std::placeholders;
class InstParser
{
	public:
		struct IFUnit
		{
			std::string _waitingInst;
			std::string _executedInst;
			bool        _bStalled;  //the fetch unit can be stalled due to a branch instruction
			IFUnit()
				:_waitingInst(""),_executedInst(""),_bStalled(false){}
			bool  IsStalled()const {return _bStalled;}
		};
		struct RegInfo
		{
			int   _val;
			bool  _bWrite;
			bool  _bRead;
			bool  _bTobeWrite;
			bool  _bTobeRead;
			RegInfo()
				:_bWrite(false),_bRead(false),_bTobeWrite(false),_bTobeRead(false){}
		};
	public:
		InstParser(const char* filename);
		~InstParser();
		void ReadInstsFromFile();
		void instParse();
	private:
		void ParseDataInst();
		void ParseInst();
		void ParseEachInst(bool,std::string& ParsedInst);
		void DoPipeLine();
		bool bBreakInst(const std::string& inst);
		void trace2File(const std::string& format);
		inline bool Finished()const {return _bFinished;} 
		inline bool Jump() const {return _bJump;}
		inline bool CanFetchNextInst()const {return !IsPreIssueQuFull() && !_IfUnit.IsStalled();}
		inline bool Branch1(const std::string& buf) const {return ('0' == buf[0] && '1' == buf[1])? true:false;}
		inline bool Branch2(const std::string& buf) const {return ('1' == buf[0] && '1' == buf[1])? true:false;};
		inline bool IsPreIssueQuFull() const {return _preIssuequ.size() >= PreQuSize;};
		template <size_t N>
		inline std::bitset<N> fromStr2Bit(const std::string& buf) const {return std::bitset<N>(buf);}
		inline void SetDstInstAddr(int dstInstAddr){_dstInstAddr = dstInstAddr;}
		inline int  GetDstInstAddr()const{return _dstInstAddr;}
		void ADD(bool,std::string&);
		void SUB(bool,std::string&);
		void BEQ(bool,std::string&);
		void MUL(bool,std::string&);
		void AND(bool,std::string&);
		void OR(bool,std::string&);
		void XOR(bool,std::string&);
		void NOR(bool,std::string&);
		void SLT(bool,std::string&);
		void ADDI(bool,std::string&);
		void ANDI(bool,std::string&);
		void ORI(bool,std::string&);
		void XORI(bool,std::string&);
		void SLL(bool,std::string&);
		void LW(bool,std::string&);
		void BGTZ(bool,std::string&);
		void J(bool,std::string&);
		void BLTZ(bool,std::string&);
		void SW(bool,std::string&);
		void BREAK(bool,std::string&);
		void SRL(bool,std::string&);
		void SRA(bool,std::string&);
		void NOP(bool,std::string&);
		void JR(bool,std::string&);

	private:
		std::ifstream             _infile;
		std::ofstream             _traceFile;
		std::string               _InstStr;
		std::queue<std::string>   _parsedDataqu;
		std::queue<std::string>   _opSequencequ;
		std::queue<std::pair<int,std::string>>   _preIssuequ;
		std::queue<std::pair<int,std::string>>   _preAlu1qu;
		std::queue<std::pair<int,std::string>>   _preMemqu;
		std::queue<std::pair<int,std::string>>   _postMemqu;
		std::queue<std::pair<int,std::string>>   _preAlu2qu;
		std::queue<std::pair<int,std::string>>   _postAlu2qu;
		int                       _InstructionAddr;
		size_t                    _startPos;  
		bool                      _bFinished;
		bool                      _bJump;
		int                       _dstInstAddr;
		IFUnit                    _IfUnit;
		typedef std::map<int,int> ADDR2MEMDATA;
		ADDR2MEMDATA  _addr2MemData;
		std::vector<RegInfo>  _regVec;
		typedef std::map<int,std::string> ADDR2OPINST;
		ADDR2OPINST _addr2OpInst;
		typedef std::map<int,std::string> ADDR2DATAINST;
		ADDR2DATAINST _addr2DataInst;
		typedef std::map<std::string,std::string> OPBRANCH1;
		OPBRANCH1 _opBranch1 = {{"0000","J"},{"0001","JR"},{"0010","BEQ"},{"0011","BLTZ"},{"0100","BGTZ"},
						  {"0101","BREAK"},{"0110","SW"},{"0111","LW"},{"1000","SLL"},{"1001","SRL"},
				          {"1010","SRA"},{"1011","NOP"}};
		typedef std::map<std::string,std::string> OPBRANCH2;
		OPBRANCH2 _opBranch2 = {{"0000","ADD"},{"0001","SUB"},{"0010","MUL"},{"0011","AND"},{"0100","OR"},
                          {"0101","XOR"},{"0110","NOR"},{"0111","SLT"},{"1000","ADDI"},{"1001","ANDI"},
                          {"1010","ORI"},{"1011","XORI"}};
		typedef std::map<std::string,std::function<void(bool,std::string&)>> OP2FUNC;
		OP2FUNC  _Op2Func = {{"ADD",std::bind(&InstParser::ADD,this,_1,_2)},{"SUB",std::bind(&InstParser::SUB,this,_1,_2)},
		                    {"ADDI",std::bind(&InstParser::ADDI,this,_1,_2)},{"BEQ",std::bind(&InstParser::BEQ,this,_1,_2)},
							{"SLL",std::bind(&InstParser::SLL,this,_1,_2)},{"LW",std::bind(&InstParser::LW,this,_1,_2)},
							{"MUL",std::bind(&InstParser::MUL,this,_1,_2)},{"BGTZ",std::bind(&InstParser::BGTZ,this,_1,_2)},
							{"J",std::bind(&InstParser::J,this,_1,_2)},{"BLTZ",std::bind(&InstParser::BLTZ,this,_1,_2)},
							{"SW",std::bind(&InstParser::SW,this,_1,_2)},{"BREAK",std::bind(&InstParser::BREAK,this, _1,_2)},
							{"SRL",std::bind(&InstParser::SRL,this,_1,_2)},{"SRA", std::bind(&InstParser::SRA,this,_1,_2)},
							{"NOP",std::bind(&InstParser::NOP,this,_1,_2)},{"JR",std::bind(&InstParser::JR,this,_1,_2)},
							{"AND",std::bind(&InstParser::AND,this,_1,_2)},{"OR",std::bind(&InstParser::OR,this,_1,_2)},
							{"XOR",std::bind(&InstParser::XOR,this,_1,_2)},{"SLT",std::bind(&InstParser::SLT,this,_1,_2)},
							{"ANDI",std::bind(&InstParser::ANDI,this,_1,_2)},{"ORI",std::bind(&InstParser::ORI,this,_1,_2)},
							{"XORI",std::bind(&InstParser::XORI,this,_1,_2)},{"NOR",std::bind(&InstParser::NOR,this,_1,_2)}};

	private:
		static const size_t PreQuSize = 4;
		static const size_t AluQuSize = 2;
		static const size_t InstLen = 32;
		static const size_t RegLen  = 5;
		static const int    BeginAddr = 256;
		const size_t  OpLen   = 4;
		const size_t  BranchLen  = 2;
};
InstParser::InstParser(const char* filename)
	:_bFinished(false),
	_bJump(false),
	_InstructionAddr(BeginAddr),
	_dstInstAddr(-1),
	_regVec(32)  //32 registers
{
	_infile.open(filename,std::ios::binary);
}
InstParser::~InstParser(){}

void InstParser::instParse()
{
	_traceFile.open("simulation.txt");
	ParseDataInst();
	ParseInst();
	_traceFile.close();
}

void InstParser::ParseInst()
{
	bool bPipe = false;std::string ParsedInst("");
	int nFetchTimes = 0; //each time IFunit has most 2 times to fetch inst
	auto it = _addr2OpInst.begin();
	while (it != _addr2OpInst.end() && !Finished()) //simulationfile
	{
		while (CanFetchNextInst() && nFetchTimes < 2 )
		{
			if (Jump())
			{
				int dstAddr = GetDstInstAddr();
				it = _addr2OpInst.find(dstAddr);
			}
			_InstructionAddr = it->first;
			_InstStr = it->second;
			assert(InstLen == _InstStr.length());
			ParseEachInst(false,ParsedInst);
			nFetchTimes ++;
			it ++;
			_preIssuequ.push(std::make_pair(_InstructionAddr,ParsedInst));
		}
		DoPipeLine();
		nFetchTimes = 0;
	}
}

void InstParser::DoPipeLine()
{
}
void InstParser::ParseEachInst(bool bWB,std::string& ParsedInst)
{
	_startPos = 0;
	_startPos += BranchLen ;
	std::string Opcode(_InstStr.substr(_startPos,OpLen));
	if (Branch1(_InstStr))
	{
		OPBRANCH1::iterator it = _opBranch1.find(Opcode);
		if (it != _opBranch1.end())
		{
			OP2FUNC::iterator iter = _Op2Func.find(it->second);
			if (iter != _Op2Func.end()){
				_startPos += OpLen;
				(iter->second)(bWB,ParsedInst);
			}
		}
	}
	else if (Branch2(_InstStr))
	{
		OPBRANCH2::iterator it = _opBranch2.find(Opcode);
		if (it != _opBranch2.end())
		{
			OP2FUNC::iterator iter = _Op2Func.find(it->second);
			if (iter != _Op2Func.end()){
				_startPos += OpLen;
				(iter->second)(bWB,ParsedInst);
			}
		}
	}
}
void InstParser::ParseDataInst()
{
	std::ostringstream oss;
	int num;
	for (auto it = _addr2DataInst.begin(); it != _addr2DataInst.end(); ++it)
	{
		num = std::strtoul((it->second).c_str(),NULL,2);
		_addr2MemData.insert(std::make_pair(it->first,num));
		oss<<it->second<<"\t"<<it->first<<"\t"<<num;
		_parsedDataqu.push(oss.str());
		oss.str("");
	}
}
void InstParser::trace2File(const std::string& format)
{
	std::queue<std::pair<int,std::string>> tmpqu;
	auto lambda = [&tmpqu]() ->std::string {
		if (!tmpqu.empty()){
			std::string str = tmpqu.front().second;
			tmpqu.pop();
			return str;
		}
		return "";
	};
	int count = 0;
	static int cycle = 1;
	for (int i = 0; i<20;i++)
		_traceFile<<"-";
	_traceFile<<endl;
	std::ostringstream oss;
	oss<<"Cycle:"<<cycle;
	if (cycle < 10) oss<<" "<<"\t"<<format<<endl<<endl;
	else oss<<"\t"<<format<<endl<<endl;
	_traceFile<<oss.str();
	//IF Unit
	_traceFile<<"IF Unit:"<<endl<<"\t"<<"Waiting Instruction: "<<_IfUnit._waitingInst<<endl<<"\t"<<"Executed Instruction: "<<_IfUnit._executedInst<<endl;
	//Pre-Issus Queue
	tmpqu = _preIssuequ;
	_traceFile<<"Pre-Issue Queue:"<<endl<<"\tEntry 0: "<<lambda()<<endl<<"\tEntry 1: "<<lambda()<<endl<<"\tEntry 2: "<<lambda()<<endl<<"\tEntry 3: "<<lambda()<<endl;
	//Pre-Alu1 Queue
	tmpqu = _preAlu1qu;
	_traceFile<<"Pre-ALU1 Queue:"<<endl<<"\tEntry 0: "<<lambda()<<endl<<"\tEntry 1: "<<lambda()<<endl;
	//Pre-MEM Queue
	tmpqu = _preMemqu;
	_traceFile<<"Pre-MEM Queue: "<<lambda()<<endl;
	//Post-MEM Queue
	tmpqu = _postMemqu;
	_traceFile<<"Post-MEM Queue: "<<lambda()<<endl;
	_traceFile<<"Registers";
	for (auto it = _regVec.begin(); it != _regVec.end();++it)
	{
		if (count % 8 == 0)
		{
			_traceFile<<endl<<"R";
			_traceFile.setf(std::ios::right);
			_traceFile.fill('0');
			_traceFile.width(2);
			_traceFile<<count<<":\t";
		}
		_traceFile<<it->_val<<"\t";
		count++;
	}
	_traceFile<<endl<<endl;
	count = 0;
	_traceFile<<"Data";
	for (auto it = _addr2MemData.begin(); it != _addr2MemData.end(); ++it)
	{
		if (count % 8 == 0)
		{
			_traceFile<<endl<<it->first<<":\t";
		}
		_traceFile<<it->second<<"\t";
		count++;
	}
	_traceFile<<endl<<endl;
	cycle ++;
}

void InstParser::ADD(bool bWB,std::string& ParsedInst)//(rs) +(rt) -> (rd)
{
	char regS[8] = {0},regT[8] = {0},regD[8] = {0};
	int idxS, idxT, idxD;
	std::ostringstream oss, format;
	idxD = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+2*RegLen,RegLen)).to_ulong();
	sprintf(regD, "R%d", idxD);
	idxS = fromStr2Bit<RegLen>(_InstStr.substr(_startPos,RegLen)).to_ulong();
	sprintf(regS, "R%d", idxS);
	idxT = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+RegLen,RegLen)).to_ulong();
	sprintf(regT, "R%d", idxT);
	format<<"[ADD "<<regD<<", "<<regS<<", "<<regT<<"]";
	ParsedInst = format.str();
	//format<<_InstructionAddr<<"\t"<<"ADD "<<regD<<", "<<regS<<", "<<regT;
	if (bWB)
		_regVec[idxD]._val = _regVec[idxS]._val + _regVec[idxT]._val;
	_bJump = false;
	//trace2File(format.str());
}

void InstParser::NOR(bool bWB,std::string& ParsedInst)// rs NOR rt -> (rd)
{
	char regS[8] = {0},regT[8] = {0},regD[8] = {0};
	int idxS, idxT, idxD;
	std::ostringstream oss, format;
	idxD = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+2*RegLen,RegLen)).to_ulong();
	sprintf(regD, "R%d", idxD);
	idxS = fromStr2Bit<RegLen>(_InstStr.substr(_startPos,RegLen)).to_ulong();
	sprintf(regS, "R%d", idxS);
	idxT = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+RegLen,RegLen)).to_ulong();
	sprintf(regT, "R%d", idxT);
	format<<"[NOR "<<regD<<", "<<regS<<", "<<regT<<"]";
	ParsedInst = format.str();
	//format<<_InstructionAddr<<"\t"<<"NOR "<<regD<<", "<<regS<<", "<<regT;
	if (bWB)
		_regVec[idxD]._val = ~(_regVec[idxS]._val | _regVec[idxT]._val);
	_bJump = false;
	//trace2File(format.str());
}

void InstParser::SUB(bool bWB,std::string& ParsedInst)//rs - rt -> rd
{
	char regS[8] = {0},regT[8] = {0},regD[8] = {0};
	int idxS, idxT, idxD;
	std::ostringstream oss, format;
	idxD = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+2*RegLen,RegLen)).to_ulong();
	sprintf(regD, "R%d", idxD);
	idxS = fromStr2Bit<RegLen>(_InstStr.substr(_startPos,RegLen)).to_ulong();
	sprintf(regS, "R%d", idxS);
	idxT = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+RegLen,RegLen)).to_ulong();
	sprintf(regT, "R%d", idxT);
	format<<"[SUB "<<regD<<", "<<regS<<", "<<regT<<"]";
	ParsedInst = format.str();
	//format<<_InstructionAddr<<"\t"<<"SUB "<<regD<<", "<<regS<<", "<<regT;
	if (bWB)
		_regVec[idxD]._val = _regVec[idxS]._val - _regVec[idxT]._val;
	_bJump = false;
	//trace2File(format.str());
}

void InstParser::ADDI(bool bWB,std::string& ParsedInst)//rs + immediate -> rt
{
	char regS[8] = {0}, regT[8] = {0}, immediate[16] = {0};
	int idxS, idxT;
	std::ostringstream oss,format;
	idxT = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+RegLen,RegLen)).to_ulong();
	sprintf(regT, "R%d", idxT);
	idxS = fromStr2Bit<RegLen>(_InstStr.substr(_startPos,RegLen)).to_ulong();
	sprintf(regS, "R%d", idxS);
	std::bitset<16> immset;
	sprintf(immediate,"#%lu",fromStr2Bit<16>(_InstStr.substr(_startPos+2*RegLen,16)).to_ulong());
	format<<"[ADDI "<<regT<<", "<<regS<<", "<<immediate<<"]";
	ParsedInst = format.str();
	//format<<_InstructionAddr<<"\t"<<"ADDI "<<regT<<", "<<regS<<", "<<immediate;
	int immed = std::strtoul(_InstStr.substr(_startPos+2*RegLen,16).c_str(),NULL,2);
	if (bWB)
		_regVec[idxT]._val = _regVec[idxS]._val + immed;
	_bJump = false;
//	trace2File(format.str());
}

void InstParser::BEQ(bool bWB,std::string& ParsedInst)//if rs = rt then branch
{
	char regS[8] = {0}, regT[8] = {0},immediate[18] = {0};
	int idxS, idxT, offset;
	std::ostringstream oss,format;
	idxT = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+RegLen,RegLen)).to_ulong(); //rt
	sprintf(regT, "R%d", idxT);
	idxS = fromStr2Bit<RegLen>(_InstStr.substr(_startPos,RegLen)).to_ulong();//rs
	sprintf(regS, "R%d", idxS);
	std::string Offset = fromStr2Bit<16>(_InstStr.substr(_startPos+2*RegLen,16)).to_string(),output;
	Offset += "00";
	offset = std::strtoul(Offset.c_str(),NULL,2);
	sprintf(immediate, "#%d",offset);
	format<<"[BEQ "<<regS<<", "<<regT<<", "<<immediate<<"]";
	ParsedInst = format.str();
	//format<<_InstructionAddr<<"\t"<<"BEQ "<<regS<<", "<<regT<<", "<<immediate;
	if (bWB){
		if (_regVec[idxS]._val == _regVec[idxT]._val)
		{
			SetDstInstAddr(_InstructionAddr + offset + 4);
			_bJump = true; // modify pc
		}else{
			_bJump = false;
		}
	}
//	trace2File(format.str());
}

void InstParser::BLTZ(bool bWB,std::string& ParsedInst)// if rs < 0 , then  branch
{
	char regS[8] = {0},regOffset[18] = {0};
	int idxS, offset;
	std::ostringstream oss,format;
	idxS = fromStr2Bit<RegLen>(_InstStr.substr(_startPos,RegLen)).to_ulong(); //rs
	sprintf(regS, "R%d", idxS);
	std::string Offset = fromStr2Bit<16>(_InstStr.substr(_startPos+2*RegLen,16)).to_string();
	Offset += "00";
	offset = std::strtoul(Offset.c_str(),NULL,2);
	sprintf(regOffset,"#%d",offset);
	format<<"[BLTZ "<<regS<<", "<<regOffset<<"]";
	ParsedInst = format.str();
	//format<<_InstructionAddr<<"\t"<<"BLTZ "<<regS<<", "<<regOffset;
	if (bWB){
		if (_regVec[idxS]._val < 0)
		{
			SetDstInstAddr(_InstructionAddr + offset + 4);
			_bJump = true; //modify pc
		}
		else{
			_bJump = false;
		}
	}
//	trace2File(format.str());
}

void InstParser::J(bool bWB,std::string& ParsedInst)//offset -> pc
{
	char instr_index[28] = {0};
	std::stringstream oss,format;
	std::string indexStr = fromStr2Bit<26>(_InstStr.substr(_startPos,26)).to_string();
	indexStr += "00";
	//because pc addr can not be a negative
	std::bitset<28> InstrSet = fromStr2Bit<28>(indexStr);
	sprintf(instr_index,"#%lu",InstrSet.to_ulong());
	format<<"[J "<<instr_index<<"]";
	ParsedInst = format.str();
	//format<<_InstructionAddr<<"\t"<<"J "<<instr_index;
	SetDstInstAddr(InstrSet.to_ulong()); // modify pc
	//trace2File(format.str());
	_bJump = true;
}

void InstParser::BGTZ(bool bWB,std::string& ParsedInst)//if rs > 0 then branch
{
	char regS[8] = {0}, regOffset[18] = {0};
	int idxS ,offset;
	std::stringstream oss,format;
	idxS = fromStr2Bit<RegLen>(_InstStr.substr(_startPos,RegLen)).to_ulong();//rs
	sprintf(regS, "R%d",idxS);
	std::string Offset = fromStr2Bit<16>(_InstStr.substr(_startPos+2*RegLen,16)).to_string();
	Offset += "00";
	offset = std::strtoul(Offset.c_str(),NULL,2);
	sprintf(regOffset, "#%d", offset);
	format<<"[BGTZ "<<regS<<", "<<regOffset<<"]";
	ParsedInst = format.str();
	//format<<_InstructionAddr<<"\t"<<"BGTZ "<<regS<<", "<<regOffset;
	if (bWB){
	if (_regVec[idxS]._val > 0)
		{
			SetDstInstAddr(_InstructionAddr + offset + 4); //modify pc
			_bJump = true;
		}else{
			_bJump = false;
		}
	}
	//trace2File(format.str());
}

void InstParser::SLL(bool bWB,std::string& ParsedInst)//rt << sa -> rd
{
	char regD[8] = {0}, regT[8] = {0},regSA[8] = {0};
	int idxD, idxT, sa;
	std::ostringstream oss,format;
	idxD = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+2*RegLen,RegLen)).to_ulong(); //rd
	sprintf(regD, "R%d", idxD);
	idxT = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+RegLen,RegLen)).to_ulong(); //rt
	sprintf(regT,"R%d", idxT);
	sa = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+3*RegLen,RegLen)).to_ulong();
	sprintf(regSA,"#%d",sa);
	format<<"[SLL "<<regD<<", "<<regT<<", "<<regSA<<"]";
	ParsedInst = format.str();
	//format<<_InstructionAddr<<"\t"<<"SLL "<<regD<<", "<<regT<<", "<<regSA;
	if (bWB)
		_regVec[idxD]._val = (_regVec[idxT]._val << sa);
	//trace2File(format.str());
	_bJump = false;
}

void InstParser::SRL(bool bWB,std::string& ParsedInst) //logical shift right //rt >> sa ->rd
{
	char regD[8] = {0}, regT[8] = {0},regSA[8] = {0};
	int idxD, idxT, sa;
	std::ostringstream oss,format;
	idxD = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+2*RegLen,RegLen)).to_ulong();
	sprintf(regD, "R%d", idxD);
	idxT = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+RegLen,RegLen)).to_ulong();
	sprintf(regT,"R%d", idxT);
	sa = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+3*RegLen,RegLen)).to_ulong();
	sprintf(regSA,"#%d",sa);
	format<<"[SLL "<<regD<<", "<<regT<<", "<<regSA<<"]";
	ParsedInst = format.str();
	//format<<_InstructionAddr<<"\t"<<"SLL "<<regD<<", "<<regT<<", "<<regSA;
	if (bWB)
	{
		unsigned int tmp = _regVec[idxT]._val;
		_regVec[idxD]._val = (tmp >> sa);
	}
//	trace2File(format.str());
	_bJump = false;
}

void InstParser::SRA(bool bWB,std::string& ParsedInst) //arighmetic shift right //rt >> sa ->rd
{
	char regD[8] = {0}, regT[8] = {0},regSA[8] = {0};
	int idxD, idxT, sa;
	std::ostringstream oss,format;
	idxD = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+2*RegLen,RegLen)).to_ulong();
	sprintf(regD, "R%d", idxD);
	idxT = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+RegLen,RegLen)).to_ulong();
	sprintf(regT,"R%d", idxT);
	sa = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+3*RegLen,RegLen)).to_ulong();
	sprintf(regSA,"#%d",sa);
	format<<"[SLL "<<regD<<", "<<regT<<", "<<regSA<<"]";
	ParsedInst = format.str();
	//format<<_InstructionAddr<<"\t"<<"SLL "<<regD<<", "<<regT<<", "<<regSA;
	if (bWB)
		_regVec[idxD]._val = ( _regVec[idxT]._val >> sa);
	//trace2File(format.str());
	_bJump = false;
}

void InstParser::NOP(bool bWB,std::string& ParsedInst)//no op
{
	std::stringstream oss, format;
	format<<"[NOP]";
	ParsedInst = format.str();
	//format<<_InstructionAddr<<"\t"<<"NOP";
//	trace2File(format.str());
	_bJump = false;
}

void InstParser::JR(bool bWB,std::string& ParsedInst)//rs -> pc
{
	_startPos += OpLen;
	char regS[8];
	int idxS;
	idxS = fromStr2Bit<RegLen>(_InstStr.substr(_startPos,RegLen)).to_ulong();
	sprintf(regS, "R%d",idxS);
	std::stringstream oss, format;
	format<<"[JR "<<regS<<"]";
	ParsedInst = format.str();
	//format<<_InstructionAddr<<"\t"<<"JR "<<regS;
	SetDstInstAddr(_regVec[idxS]._val + 4);
//	trace2File(format.str());
	_bJump = true;
}

void InstParser::AND(bool bWB,std::string& ParsedInst)// rs AND rt -> rd
{
	char regS[8], regT[8], regD[8];
	int idxS, idxT, idxD;
	std::ostringstream oss,format;
	idxS = fromStr2Bit<RegLen>(_InstStr.substr(_startPos,RegLen)).to_ulong();
	sprintf(regS, "R%d",idxS);
	idxT = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+RegLen,RegLen)).to_ulong();
	sprintf(regT, "R%d",idxT);
	idxD = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+2*RegLen, RegLen)).to_ulong();
	sprintf(regD, "R%d", idxD);
	format<<"[AND "<<regD<<", "<<regS<<", "<<regT<<"]";
	ParsedInst = format.str();
	//format<<_InstructionAddr<<"\t"<<"AND "<<regD<<", "<<regS<<", "<<regT;
	if (bWB)
		_regVec[idxD]._val = _regVec[idxS]._val & _regVec[idxT]._val;
//	trace2File(format.str());
	_bJump = false;
}

void InstParser::ANDI(bool bWB,std::string& ParsedInst)// rs AND immediate -> rt
{
	char regS[8], regT[8], immediate[16];
	int idxS, idxT, imme;
	std::ostringstream oss,format;
	idxS = fromStr2Bit<RegLen>(_InstStr.substr(_startPos,RegLen)).to_ulong();
	sprintf(regS, "R%d",idxS);
	idxT = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+RegLen,RegLen)).to_ulong();
	sprintf(regT, "R%d",idxT);
	imme = std::strtoul(_InstStr.substr(_startPos+2*RegLen,16).c_str(),NULL,2);
	sprintf(immediate, "R%d", imme);
	format<<"[ANDI "<<regT<<", "<<regS<<", "<<immediate<<"]";
	ParsedInst = format.str();
	//format<<_InstructionAddr<<"\t"<<"ANDI "<<regT<<", "<<regS<<", "<<immediate;
	if (bWB)
		_regVec[idxT]._val = _regVec[idxS]._val & imme;
//	trace2File(format.str());
	_bJump = false;
}

void InstParser::OR(bool bWB,std::string& ParsedInst)// rs or rt -> rd
{
	char regS[8], regT[8], regD[8];
	int idxS, idxT, idxD;
	std::ostringstream oss,format;
	idxS = fromStr2Bit<RegLen>(_InstStr.substr(_startPos,RegLen)).to_ulong();
	sprintf(regS, "R%d",idxS);
	idxT = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+RegLen,RegLen)).to_ulong();
	sprintf(regT, "R%d",idxT);
	idxD = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+2*RegLen, RegLen)).to_ulong();
	sprintf(regD, "R%d", idxD);
	format<<"[OR "<<regD<<", "<<regS<<", "<<regT<<"]";
	ParsedInst = format.str();
	//format<<_InstructionAddr<<"\t"<<"OR "<<regD<<", "<<regS<<", "<<regT;
	if (bWB)
		_regVec[idxD]._val = _regVec[idxS]._val | _regVec[idxT]._val;
//	trace2File(format.str());
	_bJump = false;
}

void InstParser::ORI(bool bWB,std::string& ParsedInst)// rs or immediate -> rt
{
	char regS[8], regT[8], immediate[16];
	int idxS, idxT, imme;
	std::ostringstream oss,format;
	idxS = fromStr2Bit<RegLen>(_InstStr.substr(_startPos,RegLen)).to_ulong();
	sprintf(regS, "R%d",idxS);
	idxT = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+RegLen,RegLen)).to_ulong();
	sprintf(regT, "R%d",idxT);
	imme = std::strtoul(_InstStr.substr(_startPos+2*RegLen,16).c_str(),NULL,2);
	sprintf(immediate, "R%d", imme);
	format<<"[ORI "<<regT<<", "<<regS<<", "<<immediate<<"]";
	ParsedInst = format.str();
	//format<<_InstructionAddr<<"\t"<<"ORI "<<regT<<", "<<regS<<", "<<immediate;
	if (bWB)
		_regVec[idxT]._val = _regVec[idxS]._val | imme;
//	trace2File(format.str());
	_bJump = false;
}

void InstParser::XOR(bool bWB,std::string& ParsedInst)// rs xor rt -> rd
{
	char regS[8], regT[8], regD[8];
	int idxS, idxT, idxD;
	std::ostringstream oss,format;
	idxS = fromStr2Bit<RegLen>(_InstStr.substr(_startPos,RegLen)).to_ulong();
	sprintf(regS, "R%d",idxS);
	idxT = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+RegLen,RegLen)).to_ulong();
	sprintf(regT, "R%d",idxT);
	idxD = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+2*RegLen, RegLen)).to_ullong();
	sprintf(regD, "R%d", idxD);
	format<<"[XOR "<<regD<<", "<<regS<<", "<<regT<<"]";
	ParsedInst = format.str();
	//format<<_InstructionAddr<<"\t"<<"XOR "<<regD<<", "<<regS<<", "<<regT;
	if (bWB)
		_regVec[idxD]._val = _regVec[idxS]._val ^ _regVec[idxT]._val;
//	trace2File(format.str());
	_bJump = false;
}

void InstParser::XORI(bool bWB,std::string& ParsedInst)// rs xor immediate-> rt
{
	char regS[8], regT[8], immediate[16];
	int idxS, idxT, imme;
	std::ostringstream oss,format;
	idxS = fromStr2Bit<RegLen>(_InstStr.substr(_startPos,RegLen)).to_ulong();
	sprintf(regS, "R%d",idxS);
	idxT = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+RegLen,RegLen)).to_ulong();
	sprintf(regT, "R%d",idxT);
	imme = std::strtoul(_InstStr.substr(_startPos+2*RegLen,16).c_str(),NULL,2);
	sprintf(immediate, "R%d", imme);
	format<<"[XORI "<<regT<<", "<<regS<<", "<<immediate<<"]";
	ParsedInst = format.str();
	//format<<_InstructionAddr<<"\t"<<"XORI "<<regT<<", "<<regS<<", "<<immediate;
	if (bWB)
		_regVec[idxT]._val = _regVec[idxS]._val ^ imme;
//	trace2File(format.str());
	_bJump = false;
}

void InstParser::SLT(bool bWB,std::string& ParsedInst)// (rs < rt ) -> rd
{
	char regS[8], regT[8], regD[8];
	int idxS, idxT, idxD;
	std::ostringstream oss,format;
	idxS = fromStr2Bit<RegLen>(_InstStr.substr(_startPos,RegLen)).to_ulong();
	sprintf(regS, "R%d",idxS);
	idxT = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+RegLen,RegLen)).to_ulong();
	sprintf(regT, "R%d",idxT);
	idxD = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+2*RegLen, RegLen)).to_ulong();
	sprintf(regD, "R%d", idxD);
	format<<"[SLT "<<regD<<", "<<regS<<", "<<regT<<"]";
	ParsedInst = format.str();
	//format<<_InstructionAddr<<"\t"<<"SLT "<<regD<<", "<<regS<<", "<<regT;
	if (bWB)
		_regVec[idxD]._val = (_regVec[idxS]._val < _regVec[idxT]._val)? 1:0;
//	trace2File(format.str());
	_bJump = false;
}

void InstParser::LW(bool bWB,std::string& ParsedInst)//memory[base+offset] ->rt
{
	char regT[8] = {0}, regBase[8] = {0}, regOffset[16] = {0};
	int idxT, base, offset;
	std::ostringstream oss,format;
	idxT = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+RegLen,RegLen)).to_ulong();
	sprintf(regT, "R%d", idxT);
	base = fromStr2Bit<RegLen>(_InstStr.substr(_startPos,RegLen)).to_ulong();
	sprintf(regBase, "R%d", base);
	offset = std::strtoul(_InstStr.substr(_startPos+2*RegLen,16).c_str(),NULL,2);
	sprintf(regOffset,"%d",offset);
	format<<"[LW "<<regT<<", "<<offset<<"("<<regBase<<")"<<"]";
	ParsedInst = format.str();
	//format<<_InstructionAddr<<"\t"<<"LW "<<regT<<", "<<offset<<"("<<regBase<<")";
	if (bWB){
		auto it = _addr2MemData.find(_regVec[base]._val+offset);
		if (it != _addr2MemData.end())
		{
			_regVec[idxT]._val = it->second;
		}
	}
//	trace2File(format.str());
	_bJump = false;
}

void InstParser::MUL(bool bWB,std::string& ParsedInst)//rs * rt -> rd
{
	char regS[8] = {0}, regT[8] = {0}, regD[8] = {0};
	int idxS, idxT, idxD;
	std::stringstream oss,format;
	idxD = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+2*RegLen,RegLen)).to_ulong();
	sprintf(regD,"R%d",idxD);
	idxS = fromStr2Bit<RegLen>(_InstStr.substr(_startPos,RegLen)).to_ulong();
	sprintf(regS,"R%d",idxS);
	idxT = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+RegLen,RegLen)).to_ulong();
	sprintf(regT,"R%d",idxT);
	format<<"[MUL "<<regD<<", "<<regS<<", "<<regT<<"]";
	ParsedInst = format.str();
	//format<<_InstructionAddr<<"\t"<<"MUL "<<regD<<", "<<regS<<", "<<regT;
	if (bWB)
		_regVec[idxD]._val = _regVec[idxS]._val * _regVec[idxT]._val;
//	trace2File(format.str());
	_bJump = false;
}

void InstParser::SW(bool bWB,std::string& ParsedInst)//rt -> memory[base+offset]
{
	char regBase[8] = {0}, regT[8] = {0},regOffset[16] = {0};
	std::stringstream oss,format;
	int idxBase, idxT, idxOffset;
	idxT = fromStr2Bit<RegLen>(_InstStr.substr(_startPos+RegLen,RegLen)).to_ulong();
	sprintf(regT,"R%d",idxT);
	idxBase = fromStr2Bit<RegLen>(_InstStr.substr(_startPos,RegLen)).to_ulong();
	sprintf(regBase,"R%d",idxBase);
	idxOffset = std::strtoul(_InstStr.substr(_startPos+2*RegLen,16).c_str(),NULL,2);
	sprintf(regOffset,"%d",idxOffset);
	format<<"[SW "<<regT<<", "<<regOffset<<"("<<regBase<<")"<<"]";
	ParsedInst = format.str();
	//format<<_InstructionAddr<<"\t"<<"SW "<<regT<<", "<<regOffset<<"("<<regBase<<")";
	if (bWB){
		auto it = _addr2MemData.find(_regVec[idxBase]._val + idxOffset);
		if (it != _addr2MemData.end())
		{
			it->second = _regVec[idxT]._val;
		}
	}
	_bJump = false;
//	trace2File(format.str());
}

void InstParser::BREAK(bool bWB,std::string& ParsedInst)
{
	std::stringstream oss,format;
	format<<"[BREAK]";
	ParsedInst = format.str();
	//format<<_InstructionAddr<<"\t"<<"BREAK";
//	trace2File(format.str());
	_bFinished = true;
}

bool InstParser::bBreakInst(const std::string& inst)
{
	assert(inst.length() == InstLen);
	if (inst[0] == '0' && inst[1] == '1' && inst[2] == '0' && inst[3] == '1' 
		&& inst[4] == '0' && inst[5] == '1')
		return true;
	return false;
}

void InstParser::ReadInstsFromFile()
{
	std::string buf;
	bool bIsBreak = false;
	int  InstAddr = BeginAddr;
	while (getline(_infile,buf))
	{
		if (!bIsBreak)
		{
			bIsBreak = bBreakInst(buf);
			_addr2OpInst.insert(std::make_pair(InstAddr,buf));
		}
		else
		{
			_addr2DataInst.insert(std::make_pair(InstAddr,buf));
		}
		InstAddr += 4;
	}
	_infile.close();
}
class MIPSsim
{
	public:
		explicit MIPSsim(const char* filename);
		~MIPSsim();
		int Parse();
	private:
		std::shared_ptr<InstParser>    _parserPtr;
};
MIPSsim::MIPSsim(const char* filename)
	:_parserPtr(new InstParser(filename))
{
}
MIPSsim::~MIPSsim()
{
}
int MIPSsim::Parse()
{
	_parserPtr->ReadInstsFromFile();
	_parserPtr->instParse();
	return 0;
}

int main(int argc, char*argv[])
{
	if (argc < 2){
		cout<<"filename lost"<<endl;
		return -1;
	}
	MIPSsim mipssim(argv[1]);
	mipssim.Parse();
	return 0;
}
