// FILE for implementating cache scenario

#include<iostream>
#include<vector>
#include<unordered_map>
#include<list>
#include<cassert>

using namespace std;

// 8 way, 32 entry set assoc cache
enum
{
	COMPULSORY_MISS=0,
	CAPACITY_MISS,
	CONFLICT_MISS,
	HIT,
	MISS,
};

class blocks
{
	public:
	int tag;
	bool is_valid;
	bool is_filled;
	int block_no;
	vector<unsigned> data;
	blocks(int block_bytes)
	{
		tag=0;
		is_valid=true;
		// BYTE ADDRESSABLE
		data.resize(block_bytes,0);
		is_filled=false;
		
	}		
};
class cache_entry
{
	public:
	list<blocks*> no_blocks;
	int block_bytes;
	int ways;
	cache_entry(int bb,int w)
	{
		block_bytes=bb;
		ways = w;
		for(int i=0;i<ways;i++)
		{
			blocks *bck = new blocks(block_bytes);
			bck->block_no=(i+1);
			no_blocks.push_back(bck);
		}
	}
	int search_blocks(int ttag,int offset,int& data_ret,float& cache_hit)
	{
		list<blocks*>::iterator it = no_blocks.begin();
			
		for(;it!=no_blocks.end();it++)
		{
			blocks *ptr = *it;
			if(ptr->is_valid==true && ptr->tag==ttag)
			{
				// hit
				if(offset>=0 && offset<block_bytes)
				{
					get_cache_data(offset,ptr,data_ret);
					cache_hit++;
					/* update LRU block */
					no_blocks.splice(no_blocks.begin(),no_blocks,it);
					return HIT;
				}
				else
				{
					cout<<"ERROR : wrong offset \n";
				}	
			}		
		}
		// miss
		return MISS;	
	}
	void update_block(int tag,int offset,vector<unsigned> block_data)
	{
		list<blocks*>::iterator it = no_blocks.begin();
		for(;it!=no_blocks.end();it++)
		{
			blocks *ptr = *it;
			if(ptr->is_valid==true && ptr->is_filled==false)
			{
				ptr->tag=tag;
				ptr->data=block_data;
				ptr->is_filled=true;
			
				// move it in front
				no_blocks.splice(no_blocks.begin(),no_blocks,it);
				return;
				
			}	
		}
		
		// all block full, need to replace LRU block
		blocks *ptr = no_blocks.back();
		no_blocks.pop_back();
		blocks *bck = new blocks(block_bytes);
		bck->tag=tag;
		bck->block_no=1;
		ptr->data=block_data;
		bck->is_filled=true;
		
		// push it in front
		no_blocks.emplace_front(bck);
	}
	template<typename T>
	void get_cache_data(int offset,blocks *ptr,T& gdata);	
	void invalidate_block(int ttag)
	{
		list<blocks*>::iterator it = no_blocks.begin();
		for(;it!=no_blocks.end();it++)
		{
			blocks *ptr = *it;
			if(ptr->is_valid==true && ptr->tag==ttag)
			{
				ptr->is_valid=false;
			}
		}	
	}
};
template<typename T>
vector<bool> get_bits(T data,int size)
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
template<typename T>
void cache_entry::get_cache_data(int offset,blocks *ptr,T& gdata)
{
	vector<bool> bit_array;
	assert(offset+sizeof(T)<ptr->data.size());
	for(int i=offset;i<offset+sizeof(T);i++)
	{
		vector<bool> bits = get_bits(ptr->data[i],sizeof(unsigned char)*8);
		bit_array.insert(bit_array.end(),bits.begin(),bits.end());
	}
	
	T res=0;
	int count =0;
	for(int i=bit_array.size()-1;i>=0;i--)
	{
		res+= bit_array[i]==true ? 1*pow(2,count) : 0 * pow(2,count);
		count++; 
	}
	gdata=res;
	
}
class cache
{
	public:
	vector<cache_entry*> cache_lines;
	
	/* ==== Cache config ==== */
	int block_bytes;
	int cache_sets;
	int ways;
	
	/* ==== FOR STATS ===== */
	float cache_hit;
	float cache_miss;
	float cycles_approx;
	float cycles_predict;
	float compulsory_miss;
	float capacity_miss;
	float conflict_miss;
	float approx_miss;
	cache(int bb,int nsets,int w)
	{
		cache_sets = nsets;
		block_bytes = bb;
		ways = w;
		for(int i=0;i<cache_sets;i++)
		{
			cache_entry *et = new cache_entry(block_bytes,ways);
			cache_lines.push_back(et);
		}
		cache_hit=0;
		cache_miss=0;
		cycles_approx =0;
		cycles_predict=0;
		approx_miss=0;
		
		
	}
	
	int search_data(int id,int ttag,int offset,int& data_ret,float& cache_hit)
	{
		cache_entry *rev_cache_line=cache_lines[id];
		// search blocks
		return rev_cache_line->search_blocks(ttag,offset,data_ret,cache_hit);	
	}
	void update_cache(int id,int ttag,int offset,vector<unsigned> block_data)
	{
		cache_entry *rev_cache_line=cache_lines[id];
		rev_cache_line->update_block(ttag,offset,block_data);
	}
	void set_block_invalid(int id,int ttag)
	{
		cache_entry *rev_cache_line=cache_lines[id];
		rev_cache_line->invalidate_block(ttag);
	}
	
	
};

