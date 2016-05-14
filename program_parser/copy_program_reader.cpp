// FILE TO READ OTHER PROGRAM DATA 
#include "program_reader.h"


#include<sstream>
#include<fstream>
#include<unordered_map>
#include<algorithm>
#include<cstdlib>
#include<ctime>
#include <iomanip>

using namespace std;

// function to spit data into tokens
vector<string> spplit(string s,char del)
{
	stringstream ss(s);
	string item;
	vector<string> res;
	while(getline(ss,item,del))
	{
		res.push_back(item);
	}

	return res;
}

// create new instruction lookup
inst* create_new_inst_ref(string iname,string pc,int val)
{
	inst *new_inst = new inst(iname,pc,val);
	return new_inst;
}

// function to read program file
string int_to_hex( int i )
{
  stringstream stream;
  stream << "0x" 
         << setfill ('0') << setw(sizeof(int)*2) 
         << hex << i;
  return stream.str();
}
void read_program_file(string file_name,tracker& tracker_obj)
{
	ifstream ifile;

	ifile.open(file_name.c_str());	
	string input;
	bool is_loop = false;
	int loop_times=0;
	string loop_value;
	srand(time(NULL));

	int base_pc = rand()%1120 + 3178;

	while(ifile >> input)
	{
		
		vector<string> split_line = spplit(input,',');

		if(split_line[0].compare("#")==0 && split_line[1].compare("LOOP_END")==0)
		{
			is_loop=false;
			continue;
		}

		if(is_loop)
		{
			// instruction to be executed under loop for loop_times
			int cur_exec_loop=loop_times;
			string pc_hex_string = int_to_hex(base_pc);
			while(cur_exec_loop>0)
			{
				int inst_val = rand()%1000+800+rand()%200;
				tracker_obj.inst_tracker.push_back(create_new_inst_ref(split_line[0]+loop_value,pc_hex_string,inst_val));
				cur_exec_loop--;
			}
			base_pc+=4;
			continue;
			
		}
		if(split_line[0].compare("#")==0 && split_line[1].compare("LOOP_START")==0)
		{
			// Starting point of a loop instruction
			loop_times = stoi(split_line[LOOP_VAL_LOC]);
			loop_value.assign(split_line[LOOP_NAME]);
			is_loop=true; 
				
		}
		else if(split_line[0].compare("#")!=0)
		{
			// A Valid instruction
			string pc_hex_string = int_to_hex(base_pc);
			int inst_val= rand()%1233445;
			tracker_obj.inst_tracker.push_back(create_new_inst_ref(split_line[0],pc_hex_string,inst_val));
			base_pc+=4;
		}	
		
	}
}
