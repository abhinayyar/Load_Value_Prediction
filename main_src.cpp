// main file for program execution

#include "program_parser/program_reader.h"
#include "instructions/instructions.h"
#include "approx_table.h"
#include "cache.h"
#include<sstream>
#include<fstream>
#include<cmath>
#include<unordered_set>

using namespace std;
//#define DEBUG 0
//#define VAL_DEBUG 0
/*  ----- L1 CACHE CONFIGRATIONS ---- */
#define BLOCK_SIZE 64
#define L1_SIZE 16
#define WAYS 8
// Cache replacement policy : LRU
/* ------------------ */

#define MEM_LATENCY 4
#define APPROX_LATENCY 1

unsigned int hex_to_int(string pc)
{
	stringstream converter(pc);
	unsigned int value;
	converter >> std::hex >> value;
	return value;
}
inline int get_sets_bits()
{
	return log2((L1_SIZE*1024)/(BLOCK_SIZE*WAYS));
}
inline int get_sets_no()
{
	return (L1_SIZE*1024)/(BLOCK_SIZE*WAYS);
}
inline int get_offset_bits()
{
	return log2(BLOCK_SIZE);
}
void decode_pc(string pc,int& tag,int& offset,int& set_id)
{
	// in 8 way associative
	unsigned int bit_off = 1;
	unsigned int new_offset =0;
	bit_off = bit_off | 1;
	
	int offset_bit = get_offset_bits();
	int set_index_bit = get_sets_bits();
	int i=offset_bit;
	while(i>1)
	{
		bit_off = bit_off << 1;
		bit_off = bit_off | 1;		
		i--;
	}
	new_offset=hex_to_int(pc);
	offset=new_offset & bit_off;
	bit_off=1;
	i=set_index_bit;
	while(i>1)
	{
		bit_off = bit_off << 1;
		bit_off = bit_off | 1;		
		i--;	
	}
	i=offset_bit;
	while(i>1)
	{
		bit_off = bit_off << 1;
		i--;
	}
	set_id = new_offset & bit_off;
	i=offset_bit;
	while(i>1)
	{	
		set_id = set_id >> 1;
		i--;
	}
	tag=new_offset;
	i=set_index_bit+offset_bit;
	while(i>1)
	{
		tag = tag >> 1;	
		i--;
	}
}
template<typename T>
vector<bool> get_bits_mem(T data,int size)
{
	vector<bool> res;
	int count=0;
	while(count<size)
	{
		(data & 1) == 1 ? res.insert(res.begin(),true) : res.insert(res.begin(),false);
		data = data >> 1;
		count++;
	}
	return res;
}
unsigned get_decimal(vector<bool> data)
{
	unsigned num=0;
	int count = data.size()-1;
	for(int i=0;i<data.size();i++)
	{
		if(data[i])
			num+=(pow(2,count));
		count--;
	}
	return num;
}
template<typename T>
void set_mem_block(int offset,T data,vector<unsigned>& block)
{
	assert(offset+sizeof(T)<block.size());
	vector<bool> bit_array;
	bit_array=get_bits_mem(data,sizeof(T)*8);
	vector<unsigned> tmp;
	

	for(int i=0;i<bit_array.size();i+=8)
	{
		vector<bool> char_data; 
		for(int j=i;j<i+8;j++)
		char_data.push_back(bit_array[j]);
		unsigned num=get_decimal(char_data);
		tmp.push_back(num);
	}
	
	assert(tmp.size()==sizeof(T));
	int tmp_tack=0;
	for(int i=offset;i<offset+sizeof(T);i++)
	{
		block[i]=tmp[tmp_tack];
		tmp_tack++;
	}
}
template<typename T>
void get_value_in_block(int offset,T& data,vector<unsigned> block)
{
	vector<bool> bit_array;
	for(int i=offset;i<offset+sizeof(T);i++)
	{
		vector<bool> tmp=get_bits_mem(block[i],8);

		bit_array.insert(bit_array.end(),tmp.begin(),tmp.end());
	}
	
	T num=0;
	int count =0;
	for(int i=bit_array.size()-1;i>=0;i--)
	{
		num += bit_array[i]==true ? 1*pow(2,count) : 0 * pow(2,count);
		count++;
	}
	data=num;
}
unordered_map<string,vector<unsigned> > init_mem_blocks(tracker tracker_obj)
{
	unordered_map<string,vector<unsigned> > mem_repl;
	unordered_set<string> over_w;
	
	int tag=0,offset=0,set_id=0;
	for(int i=0;i<tracker_obj.inst_tracker.size();i++)
	{
		string current_pc = tracker_obj.inst_tracker[i]->pc;
		decode_pc(current_pc,tag,offset,set_id);
		string key = to_string(tag)+ to_string(set_id);
		string tag_token = key + to_string(offset);
		if(over_w.find(tag_token)==over_w.end())
		{
			if(mem_repl.find(key)==mem_repl.end())
			{
				vector<unsigned> block(BLOCK_SIZE,0);
				set_mem_block(offset,tracker_obj.inst_tracker[i]->val,block);
				mem_repl.insert(make_pair(key,block));
			}
			else
			{
				set_mem_block(offset,tracker_obj.inst_tracker[i]->val,mem_repl[key]);
			}
			over_w.insert(tag_token);
		}
		
	}
	return mem_repl;
}
void print_block_data(vector<unsigned> block)
{
	cout<<"Block Size : "<<block.size()<<"\n";
	for(int i=0;i<block.size();i++)
	{
		cout<<block[i]<<"\t";
	}
	cout<<"\n";
}
int main(int argv,char *argc[])
{
	if(argv<6)
	{
		cout<<"Please include program name,ghb_size,lhb_size,cw ,ad to test ..\n";
		return 0;
	}
		
	/* BYTE ADDRESSABLE */
	int block_bytes = BLOCK_SIZE;
	int nsets = get_sets_no();
	unordered_map<string,vector<unsigned> > mem_repl;
	
	
	tracker tracker_obj;
	lva lva_obj(stoi(argc[2]),stoi(argc[3]),stof(argc[4]),stoi(argc[5]));
	cache cache_obj(block_bytes,nsets,WAYS);
	read_program_file(argc[1],tracker_obj);
	mem_repl=init_mem_blocks(tracker_obj);

	// NOW START APPROX VALUE
	cout<<"Begin Instruction fetch ... >> "<<__TIME__<<"\n";
	
	int tag=0,offset=0,set_id=0,data_ret=0;
	float cache_hit=0;
	int approx_degree=INT_MAX;
	bool approx_miss = false;
	vector<float> data_err;
	for(int i=0;i<tracker_obj.inst_tracker.size();i++)
	{
	
		string current_pc = tracker_obj.inst_tracker[i]->pc;
		#ifdef DEBUG
		cout<<"[CURRENT PC ] "<<current_pc<<endl;
		#endif
		
		decode_pc(current_pc,tag,offset,set_id);
		#ifdef DEBUG
		cout<<"Fetched [TAG] [OFFSET] [SET] "<<"\t"<<tag<<"\t"<<offset<<"\t"<<set_id<<endl;
		#endif
		if(set_id>=0 && set_id<nsets)
		{
			cache_obj.cycles_approx++;
			cache_obj.cycles_predict++;
			
			if(cache_obj.search_data(set_id,tag,offset,data_ret,cache_hit)!=HIT)
			{
				
				#ifdef DEBUG
				cout<<"MISS [TAG] [OFFSET] [SET] "<<"\t"<<tag<<"\t"<<offset<<"\t"<<set_id<<"\n";
				#endif
				// l1 cache miss
				// get approx data
				cache_obj.cache_miss++;
				tracker_obj.mem_fetch_predict++;
				// FIXME : can add for predict cycles also for comparison
				string ap_table_tag = lva_obj.get_tag_for_ap(hex_to_int(current_pc));
				if(lva_obj.approx_value(ap_table_tag,approx_degree,tracker_obj.inst_tracker[i]->approx_val)==false)
				{
					cache_obj.approx_miss++;
					approx_miss=true;
				}
				else
				{
					approx_miss=false;
					
					cache_obj.cycles_approx+=APPROX_LATENCY;
				
				}
				
				// fetch data from main mem or LLC 
				if(approx_miss==true || approx_degree<=0)
				{
					// fetch data
					tracker_obj.mem_fetch_approx++;
					cache_obj.cycles_approx+=MEM_LATENCY;
					
					// actual fetched value block
					string key = to_string(tag)+ to_string(set_id);
					vector<unsigned> block_data = mem_repl[key];
					#ifdef DEBUG
					print_block_data(block_data);
					#endif
					// retrieve actual value
					int actual_val=0;
					get_value_in_block(offset,actual_val,block_data);
					#ifdef DEBUG
					cout<<"Value Retrieved .. \t"<<actual_val<<endl;
					#endif
					int approx_val = tracker_obj.inst_tracker[i]->approx_val;
					lva_obj.update_ce(actual_val,approx_val,ap_table_tag);
					// add fetched block to cache
					cache_obj.update_cache(set_id,tag,offset,block_data);
					
					// update lhb and ghb
					lva_obj.update_lhb(ap_table_tag,actual_val);
					lva_obj.update_ghb(actual_val);
					
					float err = abs(approx_val-actual_val)/actual_val;
					data_err.push_back(err);
					#ifdef VAL_DEBUG
					cout<<"Approx Value : "<<approx_val<<"\t"<<"Actual Value : "<<actual_val<<"\t"<<"APP TAG "<<ap_table_tag<<endl;
					#endif
				}
						
				
				
			}
			else
			{
				// l1 cache hit
				cache_obj.cache_hit = cache_hit;
				string key = to_string(tag)+ to_string(set_id);
				vector<unsigned> block_data = mem_repl[key];
				int actual_val=0;
				get_value_in_block(offset,actual_val,block_data);
				#ifdef DEBUG
				cout<<"HIT VALUE : "<<actual_val<<endl;
				#endif
			} 
			
			// Update cache with next value for proper fetch
			if(i!=tracker_obj.inst_tracker.size()-1)
			{
				int next_tag=0,next_offset=0,next_set_id=0;
				// not last instruction to be executed
				string next_pc = tracker_obj.inst_tracker[i+1]->pc;
				decode_pc(next_pc,next_tag,next_offset,next_set_id);
				#ifdef DEBUG
				cout<<"Pre - Fetched [TAG] [OFFSET] [SET] "<<"\t"<<tag<<"\t"<<offset<<"\t"<<set_id<<endl;
				#endif
				
				if(tag==next_tag && offset==next_offset && set_id == next_set_id)
				{
					// basic replica of write back in cache, when value updates it should be invalidated 
					// in cache
					string key = to_string(next_tag)+ to_string(next_set_id);
					set_mem_block(next_offset,tracker_obj.inst_tracker[i+1]->val,mem_repl[key]);
					
					//invalidate cache
					cache_obj.set_block_invalid(next_set_id,next_tag);
				}
				
			}
				
		}
		else
		{
			cout<<"ERROR : Invalid set index \n";
		}			
	} 
	// mean error
	float err=0.0;
	for(int i=0;i<data_err.size();i++) err+=data_err[i];
	
	if(data_err.size()>0)
	err=err/data_err.size();
	
	/* ============== STATS =================== */
	cout<<" Cache Hit  : "<<cache_obj.cache_hit<<endl;;
	cout<<" Cache Miss  : "<<cache_obj.cache_miss<<endl;
	cout<<" Cache Approx Miss  : "<<cache_obj.approx_miss<<endl;
	cout<<" Cycles APPROX: "<<cache_obj.cycles_approx<<endl;
	cout<<" Memory Fetch Predict : "<<tracker_obj.mem_fetch_predict<<endl;
	cout<<" Memory Fetch Approx : "<<tracker_obj.mem_fetch_approx<<endl;
	cout<<"Geo Mean Error : "<<err<<endl;
	
	return 0;
}
