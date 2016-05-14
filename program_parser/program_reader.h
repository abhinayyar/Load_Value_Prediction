#ifndef __PP__
#define __PP__
#include<iostream>
#include<vector>
#include<cstring>

#include "../instructions/instructions.h"

enum
{
	NAME=0,
	PC,
	VALUE,
};

using namespace std;

void read_program_file(string file_name,tracker& tracker_obj);
#endif
