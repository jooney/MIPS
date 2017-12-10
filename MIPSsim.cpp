/*On my honor, I have neither given nor received unauthorized aid on this assignment*/
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <memory>
#include <bitset>
#include <set>
#include <queue>
#include <vector>
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
			int         branchAddr;
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
			bool  _bIssued;
			int  _tobewriteowner; 
			int  _tobereadowner; 
			RegInfo()
				:_bWrite(false),_bRead(false),_bTobeWrite(false),_bTobeRead(false),_bIssued(false),_val(0),
		    _tobereadowner(0),_tobewriteowner(0){}
		};
		struct RegIdx
		{
			int dstNum;
			int dstIdx;
			int srcNum;
			std::vector<int> srcIdx;
			RegIdx():srcNum(0),dstNum(0){}
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
		void DoPipeLine(std::queue<std::pair<int,std::string>>&);
		void DoJumpInst();
		bool bBreakInst(const std::string& inst);
		void trace2File();
		void IssueInstruction();
		void WriteResultBack();
		void RestoreRegisterTag();
		void SetInstIssued(std::pair<int,std::string>&);
		bool IsBranchInst(std::string&);
		bool IsSWInst(std::string&);
		bool IsLWInst(std::string&);
		inline bool Finished()const {return _bFinished;} 
		inline bool Jump() const {return _bJump;}
		inline bool CanFetchNextInst(const size_t sz)const {return !IsPreIssueQuFull(sz) && !_IfUnit.IsStalled();}
		inline bool Branch1(const std::string& buf) const {return ('0' == buf[0] && '1' == buf[1])? true:false;}
		inline bool Branch2(const std::string& buf) const {return ('1' == buf[0] && '1' == buf[1])? true:false;};
		inline bool IsPreIssueQuFull(const size_t sz) const {return sz > PreQuSize;};
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
		std::queue<int>           _dstAddrqu;
		std::queue<std::pair<int,std::string>>   _preIssuequ;
		std::queue<std::pair<int,std::string>>   _preAlu1qu;
		std::queue<std::pair<int,std::string>>   _preMemqu;
		std::queue<std::pair<int,std::string>>   _postMemqu;
		std::queue<std::pair<int,std::string>>   _preAlu2qu;
		std::queue<std::pair<int,std::string>>   _postAlu2qu;
		std::queue<std::pair<int,std::string>>   _writeBackqu;
		int                       _InstructionAddr;
		size_t                    _startPos;  
		bool                      _bFinished;
		bool                      _bJump;
		int                       _dstInstAddr;
		IFUnit                    _IfUnit;
		typedef std::set<std::string> BRANCHSET;
		const BRANCHSET   _branchSet = {"J","JR","BEQ","BLTZ","BGTZ","BREAK","NOP"};
		typedef std::map<int,RegIdx> ADDR2REGIDX;
		ADDR2REGIDX _addr2RegIdx;
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

bool InstParser::IsBranchInst(std::string& inst)
{
	size_t pos = inst.find(" ");
	pos = (pos == std::string::npos) ? inst.length()-1 : pos;
	std::string sub = inst.substr(1,pos-1);
	auto it = _branchSet.find(sub);
	return it != _branchSet.end() ? true:false;
}
bool InstParser::IsSWInst(std::string& inst)
{
	size_t pos = inst.find(" ");
	std::string sub = inst.substr(1,pos-1);
	return sub.compare("SW") == 0 ? true : false;
}
bool InstParser::IsLWInst(std::string& inst)
{
	size_t pos = inst.find(" ");
	std::string sub = inst.substr(1,pos-1);
	return sub.compare("LW") == 0 ? true : false;
}
void InstParser::ParseInst()
{
	bool bFirstIsBranch = false;
	std::string ParsedInst("");
	std::queue<std::pair<int,std::string>> tmpqu;
	int nFetchTimes = 0; //each time IFunit has most 2 times to fetch inst
	auto it = _addr2OpInst.begin();
	while (it != _addr2OpInst.end() && !Finished()) //simulationfile
	{
		size_t qusize = _preIssuequ.size();
		DoJumpInst();
		while (CanFetchNextInst(nFetchTimes + 1 + qusize) && nFetchTimes < 2 )
		{
			if (Jump())
			{
				int dstAddr = GetDstInstAddr();
				it = _addr2OpInst.find(dstAddr);
				_bJump = false;
			}
			_InstructionAddr = it->first;
			_InstStr = it->second;
			assert(InstLen == _InstStr.length());
			ParseEachInst(false,ParsedInst);
			if (IsBranchInst(ParsedInst))
			{
				if (nFetchTimes == 0)
				{
					bFirstIsBranch = true;
					it --; //next instruction needs to be re-fetched
				}
				_IfUnit._waitingInst = ParsedInst;
				_IfUnit.branchAddr = _InstructionAddr;
			}
			else if (!bFirstIsBranch){
				tmpqu.push(std::make_pair(_InstructionAddr,ParsedInst));
			}
			nFetchTimes ++;
			it ++;
		}
		DoPipeLine(tmpqu);
		nFetchTimes = 0;
		bFirstIsBranch = false;
	}
}
void InstParser::DoPipeLine(std::queue<std::pair<int,std::string>>& tmpqu)
{
	//from bottom to top to process pipeline 
	RestoreRegisterTag();
	if (!_postAlu2qu.empty()) //Post-ALU2 Queue -> WBqueue
	{
		auto it = _postAlu2qu.front();
		_writeBackqu.push(it);
		_postAlu2qu.pop();
		WriteResultBack();
	}
	if (!_postMemqu.empty()) //Post-MEM queue -> WBqueue 
	{
		auto it = _postMemqu.front();
		_writeBackqu.push(it);
		_postMemqu.pop();
		WriteResultBack();
	}
	if (!_preAlu2qu.empty()) // Pre-ALU2 Queue -> Post-ALU2 Queue
	{
		auto it = _preAlu2qu.front();
		SetInstIssued(it);
		_postAlu2qu.push(it);
		_preAlu2qu.pop();
	}
	if (!_preMemqu.empty())  //Pre-MEM Queue -> Post-MEM Queue or finish
	{
		std::string inst = _preMemqu.front().second;
		if (IsSWInst(inst)) //write back to memory
		{
			int dstAddr = _preMemqu.front().first;
			auto it = _addr2OpInst.find(dstAddr);
			_InstStr = it->second;
			ParseEachInst(true,inst);
		}
		else if (IsLWInst(inst)) // Pre-MEM Queue -> Post-MEM Queue
		{
			auto it = _preMemqu.front();
			_postMemqu.push(it);
		}
		_preMemqu.pop();
	}
	if (!_preAlu1qu.empty()) //Pre-ALU1 Queue -> Pre-MEM Queue
	{
		auto it = _preAlu1qu.front();
		SetInstIssued(it);
		_preMemqu.push(it);
		_preAlu1qu.pop();
	}
	//from Pre-Issue Queue -> Pre-ALU2 Queue & Pre-ALU1 Queue
	IssueInstruction();
	while (!tmpqu.empty())
	{
		auto it = tmpqu.front();
		_preIssuequ.push(it);
		tmpqu.pop();
	}
	trace2File();
}
void InstParser::SetInstIssued(std::pair<int,std::string>& it)
{
	int instAddr = it.first;
	auto regidx = _addr2RegIdx.find(instAddr);
	for (int i = 0; i< regidx->second.srcNum;i++)
	{
		_regVec[regidx->second.srcIdx[i]]._bIssued = true;
	}
}
void InstParser::DoJumpInst()
{
	if (!_IfUnit._executedInst.empty())
	{
		std::string ParsedInst,instStr;
		_InstStr = _addr2OpInst.find(_IfUnit.branchAddr)->second; //danger
		_startPos  = 0;
		_startPos += BranchLen;
		std::string Opcode(_InstStr.substr(_startPos,OpLen));
		if (Branch1(_InstStr)){
			OPBRANCH1::iterator it = _opBranch1.find(Opcode);
			if (it != _opBranch1.end()){
				OP2FUNC::iterator iter = _Op2Func.find(it->second);
				if (iter != _Op2Func.end()){
					_startPos += OpLen;
					(iter->second)(true,ParsedInst);
				}
			}
		}
		_IfUnit._executedInst = "";
	}
}
void InstParser::IssueInstruction()
{
	static int cycle = 1;
	int  branch1, branch2, instAddr,i;
	std::string ParsedInst;
	bool bCondDst, bCondSrc;
	std::vector<std::pair<int,std::string>> tmpvec;
	while (!_preIssuequ.empty())//because we can't use queue to bianli, so use vector instead
	{
		auto it = _preIssuequ.front();
		tmpvec.push_back(it);
		_preIssuequ.pop();
	}
	branch1 = branch2 = 0;
	for (auto it = tmpvec.begin();it != tmpvec.end();++it)
	{
		bCondDst= bCondSrc = false;
		instAddr = it->first;
		ParsedInst = it->second;
		auto regidx = _addr2RegIdx.find(instAddr);
		// it can issue at most 2 instructions each cycle, but 1 instruction each type,for 2 type
		if ( (IsLWInst(ParsedInst) || IsSWInst(ParsedInst)) && (branch1 < 1) )
		{
			branch1 ++;
			if ( regidx->second.dstNum > 0 &&
				(!_regVec[regidx->second.dstIdx]._bWrite &&  //WAW
				!_regVec[regidx->second.dstIdx]._bTobeWrite &&  //WAW 
				!_regVec[regidx->second.dstIdx]._bTobeRead) ) //WAR
			{
				_regVec[regidx->second.dstIdx]._bTobeWrite = true;
				_regVec[regidx->second.dstIdx]._tobewriteowner = instAddr;
				bCondDst = true;
			}
			else if (!bCondDst && regidx->second.dstNum > 0 && _regVec[regidx->second.dstIdx]._tobewriteowner == instAddr)
			{//the guy who set tag tobewrite is himself, so condition is satisfied
				bCondDst = true;
			}
			else if (regidx->second.dstNum == 0) bCondDst = true;
			i = 0;
			bCondSrc = true;
			while (i < regidx->second.srcNum)
			{
				if (_regVec[regidx->second.srcIdx[i]]._bWrite ||   //RAW
					_regVec[regidx->second.srcIdx[i]]._bTobeWrite) 
				{
					bCondSrc = false;
					break;
				}
				else{
					_regVec[regidx->second.srcIdx[i]]._bTobeRead = true;
					_regVec[regidx->second.srcIdx[i]]._tobereadowner = instAddr;
				}
				i++;
			}
			if (bCondDst && bCondSrc){ //Pre-Issue Queue -> Pre-ALU1 Queue
				_preAlu1qu.push(*it);
				tmpvec.erase(it);
				if (regidx->second.dstNum > 0)
				{
					_regVec[regidx->second.dstIdx]._bTobeWrite = false;
					_regVec[regidx->second.dstIdx]._bWrite = true;
					_regVec[regidx->second.dstIdx]._tobewriteowner = 0;
				}
				for (i = 0;i<regidx->second.srcNum;i++)
				{
					_regVec[regidx->second.srcIdx[i]]._bTobeRead = false;
					_regVec[regidx->second.srcIdx[i]]._bRead = true;
					_regVec[regidx->second.srcIdx[i]]._tobereadowner = 0;
				}
				it --;
				branch1++;
			}
		}
		else if ((!IsLWInst(ParsedInst) && !IsSWInst(ParsedInst) && (branch2 < 1)))
		{
			if (!_regVec[regidx->second.dstIdx]._bWrite &&  //WAW
				!_regVec[regidx->second.dstIdx]._bTobeWrite && //WAW
				!_regVec[regidx->second.dstIdx]._bTobeRead)	// WAR
			{
				_regVec[regidx->second.dstIdx]._bTobeWrite = true;
				_regVec[regidx->second.dstIdx]._tobewriteowner = instAddr;
				bCondDst = true;
			}
			else if (!bCondDst && _regVec[regidx->second.dstIdx]._tobewriteowner == instAddr)
			{// the guy who set tag tobewrite is himself, so condition is satisfied
				bCondDst = true;
			}
			if (!_regVec[regidx->second.dstIdx]._bIssued && _regVec[regidx->second.dstIdx]._bRead)
			{// prevent WAR
				bCondDst = false; // not issued and to be read ,we call it WAR
			}
			else if(_regVec[regidx->second.dstIdx]._bIssued && _regVec[regidx->second.dstIdx]._bRead)
			{
				bCondDst = true;
			}
			i = 0;
			bCondSrc = true;
			while (i < regidx->second.srcNum)  //RAW
			{
				if (_regVec[regidx->second.srcIdx[i]]._bWrite  ||  //src register is occupied and can't read
					_regVec[regidx->second.srcIdx[i]]._bTobeWrite)
				{
					if (regidx->second.dstIdx != regidx->second.srcIdx[i])
					{
						bCondSrc = false;
						break;
					}
				}
				else{
					_regVec[regidx->second.srcIdx[i]]._bTobeRead = true;
					_regVec[regidx->second.srcIdx[i]]._tobereadowner = instAddr;
				}
				i++;
			}
			if (bCondDst && bCondSrc){ //Pre-Issue Queue -> Pre-ALU2 Queue
				_preAlu2qu.push(*it);
				tmpvec.erase(it);
				_regVec[regidx->second.dstIdx]._bTobeWrite = false;
				_regVec[regidx->second.dstIdx]._tobewriteowner = 0;
				_regVec[regidx->second.dstIdx]._bWrite = true;
				for (i = 0;i<regidx->second.srcNum;i++)
				{
					_regVec[regidx->second.srcIdx[i]]._bTobeRead = false;
					_regVec[regidx->second.srcIdx[i]]._bRead = true;
					_regVec[regidx->second.srcIdx[i]]._tobereadowner = 0;
				}
				it --;
				branch2 ++;
			}
		}
	}
	//write back to _preIssuequ 
	for (auto& it : tmpvec)
	{
		_preIssuequ.push(it);
	}
	if (!_IfUnit._waitingInst.empty())
	{
		bCondSrc = true; // only consider regSrc
		i = 0;
		_IfUnit._bStalled = true;
		auto regidx = _addr2RegIdx.find(_IfUnit.branchAddr);
		while (i < regidx->second.srcNum)	
		{
			if (_regVec[regidx->second.srcIdx[i]]._bWrite || 
					_regVec[regidx->second.srcIdx[i]]._bTobeWrite)
			{
				bCondSrc = false;
				break;
			}
			i++;
		}
		if (bCondSrc) //jump condition satisfied
		{
			_IfUnit._executedInst = _IfUnit._waitingInst;
			_IfUnit._waitingInst = "";
			_IfUnit._bStalled = false;
		}
	}
	cycle ++;
}
void InstParser::WriteResultBack()
{
	int dstAddr;
	std::string ParsedInst;
	while (!_writeBackqu.empty())
	{
		dstAddr = _writeBackqu.front().first;
		auto it = _addr2OpInst.find(dstAddr);
		_dstAddrqu.push(dstAddr);
		_InstStr = it->second;// current instruction which to be executed and write result back 
		ParseEachInst(true,ParsedInst);
		_writeBackqu.pop();
	}
}
void InstParser::RestoreRegisterTag()
{
	while (!_dstAddrqu.empty())
	{
		auto regidx = _addr2RegIdx.find(_dstAddrqu.front());
		_regVec[regidx->second.dstIdx]._bWrite = false;
		_regVec[regidx->second.dstIdx]._bIssued = false;
		for (int i = 0;i < regidx->second.srcNum;i++)
		{
			_regVec[regidx->second.srcIdx[i]]._bRead = false;
			_regVec[regidx->second.srcIdx[i]]._bIssued = false;
		}
		_dstAddrqu.pop();	
	}
}
void InstParser::ParseEachInst(bool bWB, std::string& ParsedInst)
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
void InstParser::trace2File()
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
	oss<<"Cycle:"<<cycle<<endl<<endl;
	_traceFile<<oss.str();
	//IF Unit
	_traceFile<<"IF Unit:"<<endl<<"\t"<<"Waiting Instruction: "<<_IfUnit._waitingInst<<endl<<"\t"<<"Executed Instruction: "<<_IfUnit._executedInst<<endl;
	//Pre-Issus Queue
	tmpqu = _preIssuequ;
	_traceFile<<"Pre-Issue Queue:"<<endl<<"\tEntry 0: "<<lambda()<<endl;
	_traceFile<<"\tEntry 1: "<<lambda()<<endl;
	_traceFile<<"\tEntry 2: "<<lambda()<<endl;
	_traceFile<<"\tEntry 3: "<<lambda()<<endl;
	//Pre-Alu1 Queue
	tmpqu = _preAlu1qu;
	_traceFile<<"Pre-ALU1 Queue:"<<endl<<"\tEntry 0: "<<lambda()<<endl;
	_traceFile<<"\tEntry 1: "<<lambda()<<endl;
	//Pre-MEM Queue
	tmpqu = _preMemqu;
	_traceFile<<"Pre-MEM Queue: "<<lambda()<<endl;
	//Post-MEM Queue
	tmpqu = _postMemqu;
	_traceFile<<"Post-MEM Queue: "<<lambda()<<endl;
	//Pre-ALU2 Queue
	tmpqu = _preAlu2qu;
	_traceFile<<"Pre-ALU2 Queue: "<<endl<<"\tEntry 0:"<<lambda()<<endl;
	_traceFile<<"\tEntry 1: "<<lambda()<<endl;
	//Post-ALU2 Queue
	tmpqu = _postAlu2qu;
	_traceFile<<"Post-ALU2 Queue: "<<lambda()<<endl<<endl;
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
	if (bWB){
		_regVec[idxD]._val = _regVec[idxS]._val + _regVec[idxT]._val;
	}
	else{
		RegIdx regidx;
		regidx.dstIdx = idxD; regidx.srcIdx.push_back(idxS); regidx.srcIdx.push_back(idxT);
		regidx.srcNum = 2; regidx.dstNum = 1;
		_addr2RegIdx.insert(std::make_pair(_InstructionAddr,regidx));
	}
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
	if (bWB)
		_regVec[idxD]._val = ~(_regVec[idxS]._val | _regVec[idxT]._val);
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
	if (bWB)
		_regVec[idxD]._val = _regVec[idxS]._val - _regVec[idxT]._val;
	else{
		RegIdx regidx;
		regidx.dstIdx = idxD; regidx.srcIdx.push_back(idxS); regidx.srcIdx.push_back(idxT);
		regidx.srcNum = 2; regidx.dstNum = 1;
		_addr2RegIdx.insert(std::make_pair(_InstructionAddr,regidx));
	}
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
	int immed = std::strtoul(_InstStr.substr(_startPos+2*RegLen,16).c_str(),NULL,2);
	if (bWB)
		_regVec[idxT]._val = _regVec[idxS]._val + immed;
	else{
		RegIdx regidx;
		regidx.dstIdx = idxT; regidx.srcIdx.push_back(idxS); regidx.srcNum = 1;regidx.dstNum = 1;
		_addr2RegIdx.insert(std::make_pair(_InstructionAddr,regidx));
	}
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
	if (bWB){
		if (_regVec[idxS]._val == _regVec[idxT]._val)
		{
			SetDstInstAddr(_InstructionAddr + offset + 4);
			_bJump = true; // modify pc
		}else{
			_bJump = false;
		}
	}
	else{
		RegIdx regidx ;
		regidx.srcIdx.push_back(idxS); regidx.srcIdx.push_back(idxT);regidx.srcNum = 2;
		_addr2RegIdx.insert(std::make_pair(_InstructionAddr,regidx));
	}
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
	else{
		RegIdx regidx ;
		regidx.srcIdx.push_back(idxS);regidx.srcNum = 1;
		_addr2RegIdx.insert(std::make_pair(_InstructionAddr,regidx));
	}
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
	SetDstInstAddr(InstrSet.to_ulong()); // modify pc
	RegIdx regidx; regidx.srcNum = 0;
	_addr2RegIdx.insert(std::make_pair(_InstructionAddr,regidx));
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
	if (bWB){
		if (_regVec[idxS]._val > 0)
		{
			SetDstInstAddr(_InstructionAddr + offset + 4); //modify pc
			_bJump = true;
		}else{
			_bJump = false;
		}
	}
	else{
		RegIdx regidx ;
		regidx.srcIdx.push_back(idxS);regidx.srcNum = 1;
		_addr2RegIdx.insert(std::make_pair(_InstructionAddr,regidx));
	}
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
	if (bWB){
		_regVec[idxD]._val = (_regVec[idxT]._val << sa);
	}
	else 
	{
		RegIdx regidx;
		regidx.dstIdx = idxD;regidx.srcIdx.push_back(idxT);regidx.srcNum = 1;regidx.dstNum = 1;
		_addr2RegIdx.insert(std::make_pair(_InstructionAddr,regidx));
	}
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
	if (bWB)
	{
		unsigned int tmp = _regVec[idxT]._val;
		_regVec[idxD]._val = (tmp >> sa);
	}
	else 
	{
		RegIdx regidx;
		regidx.dstIdx = idxD;regidx.srcIdx.push_back(idxT);regidx.srcNum = 1;regidx.dstNum = 1;
		_addr2RegIdx.insert(std::make_pair(_InstructionAddr,regidx));
	}
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
	if (bWB)
		_regVec[idxD]._val = ( _regVec[idxT]._val >> sa);
	else 
	{
		RegIdx regidx;
		regidx.dstIdx = idxD;regidx.srcIdx.push_back(idxT);regidx.srcNum = 1;regidx.dstNum = 1;
		_addr2RegIdx.insert(std::make_pair(_InstructionAddr,regidx));
	}
}

void InstParser::NOP(bool bWB,std::string& ParsedInst)//no op
{
	std::stringstream oss, format;
	format<<"[NOP]";
	ParsedInst = format.str();
	RegIdx regidx; regidx.srcNum = 0;
	_addr2RegIdx.insert(std::make_pair(_InstructionAddr,regidx));
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
	SetDstInstAddr(_regVec[idxS]._val + 4);
	RegIdx regidx; regidx.srcNum = 0;
	_addr2RegIdx.insert(std::make_pair(_InstructionAddr,regidx));
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
	if (bWB)
		_regVec[idxD]._val = _regVec[idxS]._val & _regVec[idxT]._val;
	else{
		RegIdx regidx;
		regidx.dstIdx = idxD; regidx.srcIdx.push_back(idxS); regidx.srcIdx.push_back(idxT);
		regidx.srcNum = 2; regidx.dstNum = 1;
		_addr2RegIdx.insert(std::make_pair(_InstructionAddr,regidx));
	}
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
	if (bWB)
		_regVec[idxT]._val = _regVec[idxS]._val & imme;
	else{
		RegIdx regidx;
		regidx.dstIdx = idxT; regidx.srcIdx.push_back(idxT); regidx.srcIdx.push_back(idxS);
		regidx.srcNum = 1; regidx.dstNum = 1;
		_addr2RegIdx.insert(std::make_pair(_InstructionAddr,regidx));
	}
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
	if (bWB)
		_regVec[idxD]._val = _regVec[idxS]._val | _regVec[idxT]._val;
	else{
		RegIdx regidx;
		regidx.dstIdx = idxD; regidx.srcIdx.push_back(idxT); regidx.srcIdx.push_back(idxS);
		regidx.srcNum = 2; regidx.dstNum = 1;
		_addr2RegIdx.insert(std::make_pair(_InstructionAddr,regidx));
	}
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
	if (bWB)
		_regVec[idxT]._val = _regVec[idxS]._val | imme;
	else{
		RegIdx regidx;
		regidx.dstIdx = idxT; regidx.srcIdx.push_back(idxS);
		regidx.srcNum = 1; regidx.dstNum = 1;
		_addr2RegIdx.insert(std::make_pair(_InstructionAddr,regidx));
	}
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
	if (bWB)
		_regVec[idxD]._val = _regVec[idxS]._val ^ _regVec[idxT]._val;
	else{
		RegIdx regidx;
		regidx.dstIdx = idxD; regidx.srcIdx.push_back(idxS);regidx.srcIdx.push_back(idxT);
		regidx.srcNum = 2; regidx.dstNum = 1;
		_addr2RegIdx.insert(std::make_pair(_InstructionAddr,regidx));
	}
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
	if (bWB)
		_regVec[idxT]._val = _regVec[idxS]._val ^ imme;
	else{
		RegIdx regidx;
		regidx.dstIdx = idxT; regidx.srcIdx.push_back(idxS);
		regidx.srcNum = 1; regidx.dstNum = 1;
		_addr2RegIdx.insert(std::make_pair(_InstructionAddr,regidx));
	}
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
	if (bWB)
		_regVec[idxD]._val = (_regVec[idxS]._val < _regVec[idxT]._val)? 1:0;
	else{
		RegIdx regidx;
		regidx.dstIdx = idxD; regidx.srcIdx.push_back(idxS);regidx.srcIdx.push_back(idxT);
		regidx.srcNum = 2; regidx.dstNum = 1;
		_addr2RegIdx.insert(std::make_pair(_InstructionAddr,regidx));
	}
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
	if (bWB){
		auto it = _addr2MemData.find(_regVec[base]._val+offset);
		if (it != _addr2MemData.end())
		{
			_regVec[idxT]._val = it->second;
		}
	}
	else{
		RegIdx regidx;
		regidx.dstIdx = idxT;regidx.srcIdx.push_back(base);regidx.srcNum = 1;regidx.dstNum = 1;
		_addr2RegIdx.insert(std::make_pair(_InstructionAddr,regidx));
	}
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
	if (bWB)
		_regVec[idxD]._val = _regVec[idxS]._val * _regVec[idxT]._val;
	else{
		RegIdx regidx;
		regidx.dstIdx = idxD;regidx.srcIdx.push_back(idxS);regidx.srcIdx.push_back(idxT);
		regidx.srcNum = 2;regidx.dstNum = 1;
		_addr2RegIdx.insert(std::make_pair(_InstructionAddr,regidx));
	}
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
	if (bWB){
		auto it = _addr2MemData.find(_regVec[idxBase]._val + idxOffset);
		if (it != _addr2MemData.end())
		{
			it->second = _regVec[idxT]._val;
		}
	}
	else{
		RegIdx regidx;
		regidx.srcIdx.push_back(idxBase);regidx.srcIdx.push_back(idxT);regidx.srcNum = 2;
		_addr2RegIdx.insert(std::make_pair(_InstructionAddr,regidx));
	}
}

void InstParser::BREAK(bool bWB,std::string& ParsedInst)
{
	std::stringstream oss,format;
	format<<"[BREAK]";
	ParsedInst = format.str();
	RegIdx regidx;
	regidx.srcNum = 0;
	_addr2RegIdx.insert(std::make_pair(_InstructionAddr,regidx));
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
