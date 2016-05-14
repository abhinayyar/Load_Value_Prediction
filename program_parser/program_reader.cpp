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

void read_program_file(string file_name,tracker& tracker_obj)
{
	ifstream ifile;

	ifile.open(file_name.c_str());	
	string input;

	while(ifile >> input)
	{
		
		vector<string> split_line = spplit(input,',');
		tracker_obj.inst_tracker.push_back(create_new_inst_ref(split_line[NAME],split_line[PC],stoi(split_line[VALUE])));

	}
}
