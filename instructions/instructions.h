// This file tracks all instructions and it's corresponding PC value

#ifndef __INST__
#define __INST__
 
#include<vector>
#include<unordered_map>
using namespace std;

class inst
{
	public:
		string name;
		string pc;
		int opcode;
		int val;
		int approx_val;
		inst(string iname,string ipc,int ival)
		{
			//cout<<"Instruction fetched ..  "<<iname<<"\t"<<ipc<<"\t"<<ival<<"\n";
			name.assign(iname);
			pc.assign(ipc);
			val=ival;
			approx_val=0;
		}
	};
class tracker
{
	public:
	vector<inst*> inst_tracker;
	float mem_fetch_approx;
	float mem_fetch_predict;
	tracker()
	{
		mem_fetch_approx=0;
		mem_fetch_predict=0;
	}
};
#endif
