#include<iostream>
#include<vector>
#include<string>
#include<sstream>
#include<fstream>
#include<ctime>

using namespace std;

int get_int(string hexstring)
{
	unsigned int x;   
	std::stringstream ss;
	ss << std::hex << hexstring;
	ss >> x;
	return x;
}
int main()
{
	ofstream data_file;

	srand (time(NULL));

	data_file.open("ML_data.txt");
	vector<string> words;
	words.push_back("00400A10");
	words.push_back("00400A1C");

	words.push_back("00400A2C");

	words.push_back("00400A2F");

	words.push_back("00400A34");

	words.push_back("00400A4D");


	// convert in int
	
	vector<int> target;

	for(int i=0;i<words.size();i++)
	{
		target.push_back(get_int(words[i]));
	}	
	vector<pair<vector<int>,string> > ml_data;


	int i=150;
	while(i>0)	
	{
		int val = rand() % 10 + 1;
		string label = "LABEL_"+to_string(val);
		random_shuffle(target.begin(),target.end());

		for(int j=0;j<target.size();j++)
		{
			data_file<<target[j]<<",";
		}
		data_file<<label<<endl;
		i--;
		
	}	
	return 0;
}
