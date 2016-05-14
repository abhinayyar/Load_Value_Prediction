// ############## Approx table class ####################################

#include<iostream>
#include<vector>
#include<unordered_map>
#include<utility>

using namespace std;

#define LVA_TABLE_SIZE 512

class approx_table_entry
{
	public:
		int conf_estimate;
		int approx_degree;
		// created the table only for integer type, later :FIXME handle it for float value also
		vector<pair<int,bool> > lhb;
		approx_table_entry(int lhb_sz,int app)
		{
			conf_estimate=1;
			approx_degree=app;
			pair<int,bool> a;
			a.first=0;
			a.second=true;
			lhb.resize(lhb_sz,a);
		}
};
class lva
{
	public:
		vector<pair<int,bool> > ghb;
		int ghb_size;
		int lhb_size;
		float confidence_window;
		int lva_size;
		int app_d;
		//approx table entry 
		unordered_map<string,approx_table_entry*> lva_table;
		lva(int ghb_n,int lhb_n,float cw,int app)
		{
			ghb_size=ghb_n;
			lhb_size=lhb_n;
			confidence_window=cw;
			lva_size=LVA_TABLE_SIZE;
			pair<int,bool> a;
			ghb.resize(ghb_size,a);
			app_d=app;
			
		}
		string get_tag_for_ap(int current_pc);
		bool approx_value(string tag,int& ad,int& av);
		void update_ce(int actual_val,int approx_val,string tag);
		void update_lhb(string tag,int data);
		void update_ghb(int data);
		
};
// update confidence estimate
void lva::update_ce(int actual_val,int approx_val,string tag)
{
	approx_table_entry *ptr = lva_table[tag];
	
	if(abs(actual_val-approx_val)<=(confidence_window/100)*actual_val)
	{
		ptr->conf_estimate++;
	}
	else
	{
		ptr->conf_estimate--;
	}
}
// returns the tag value for approx table
string lva::get_tag_for_ap(int current_pc)
{
	// XOR PC AND GHB VALUE
	int res=current_pc; 	
	for(int i=0;i<ghb.size();i++)
	{
		res^=ghb[i].first;
	}
	
	return to_string(res);
}
void lva::update_lhb(string tag,int data)
{
	approx_table_entry *ptr = lva_table[tag];
	// set approx degree to max
	ptr->approx_degree=app_d;
	for(int i=0;i<ptr->lhb.size();i++)
	{
		if(ptr->lhb[i].second)
		{
			ptr->lhb[i].first=data;
			ptr->lhb[i].second=false;
			return;
		}
	}
	
	// lhb full , need to FIFO rep
	
	if(ptr->lhb.size()>0)
	{
		ptr->lhb.erase(ptr->lhb.begin());
		pair<int,bool> a;
		a.first=data;
		a.second=false;
		ptr->lhb.push_back(a);
	}
}
void lva::update_ghb(int data)
{
	for(int i=0;i<ghb.size();i++)
	{
		if(ghb[i].second)
		{
			ghb[i].first=data;
			ghb[i].second=false;
			return;
		}
	}
	// ghb full , FIFO Rep
	if(ghb.size()>0)
	{
		ghb.erase(ghb.begin());
		pair<int,bool> a;
		a.first=data;
		a.second=false;
		ghb.push_back(a);
	}
}
// approx actual value
bool lva::approx_value(string tag,int& ad,int& av)
{
	if(lva_table.find(tag)!=lva_table.end())
	{
		// tag found , use average to get predicted value
		approx_table_entry *ptr = lva_table[tag];
		
		int val=0;
		int valid_col=0;
		for(int i=0;i<ptr->lhb.size();i++)
		{
			if(ptr->lhb[i].second==false) valid_col++;
			val+=ptr->lhb[i].first;
		}
		if(ptr->conf_estimate>=0)
		{
			// approx value
			if(valid_col!=0)
			av=val/valid_col;
			ptr->approx_degree--;
			ad=ptr->approx_degree;
			return true;		
		}	
		
	}
	else
	{
		approx_table_entry *ptr = new approx_table_entry(lhb_size,app_d);
		lva_table.insert(make_pair(tag,ptr));
		ptr->approx_degree--;
		ad=ptr->approx_degree;
	}
	av=0;
	return false;
}

