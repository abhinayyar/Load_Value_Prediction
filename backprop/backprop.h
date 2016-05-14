#ifndef __BACKPROP_H__
#define __BACKPROP_H__

#include<iostream>
#include<vector>
#include<utility>
#include<string>
#include<cassert>
#include<cmath>
#include<algorithm>

using namespace std;

class input_node
{
	public:
	float input;
	float output;
	vector<float> weights;
	input_node(int next_layer_nodes)
	{
		weights.resize(next_layer_nodes);
		input=1.0;
		output=0.0;
	}
	~input_node()
	{
	}
};
class output_node
{
	public:
	float input;
	float output;
	float lambda;
	float target;
	output_node() 
	{
		lambda=0.0;
		target=0.0;
	}
	~output_node() {}
};
class hidden_node
{
	public:
	float input;
	float output;
	float lambda;
	vector<float> weights;
	hidden_node(int next_layer_nodes)
	{
		weights.resize(next_layer_nodes);
		input=1.0;
		output=0.0;
		lambda=0.0;
	}
	~hidden_node()
	{
	}
};
class nn
{
	public:
	float lr;
	int ni;
	int no;
	int nh; // referes to nodes in hidden layer, so far we assume only single hidden layer packprop	
	vector<input_node*> i_nodes;
	vector<output_node*> o_nodes;
	// assume only single hidden layer , can be extended to multiple hidden layers
	vector<hidden_node*> h_nodes;
	nn(int _ni,int _no,int _nh,float _lr);
	void randomize_wts(vector<float>& weights);
	void feedforward(vector<float> attr);
	float backpass();
	void update_weights();
	void backprop(vector<pair<vector<float> , int> > train_set);
	float get_mse();
	void testprop(vector<pair<vector<float> , int> > test_set);
	void refresh_nodes();
	
};
nn::nn(int _ni,int _no,int _nh,float _lr)
{
	ni=_ni+1; // for bias
	no=_no;
	nh=_nh;
	lr=_lr;
	
	// init input nodes;
	for(int i=0;i<ni;i++)
	{
		input_node *node = new input_node(nh);
		randomize_wts(node->weights);
		i_nodes.push_back(node);
			
	}
	
	// init hidden nodes
	
	for(int i=0;i<nh;i++)
	{
		hidden_node *node = new hidden_node(no);
		randomize_wts(node->weights);
		h_nodes.push_back(node);
	}
	
	// init output node
	
	for(int i=0;i<no;i++)
	{
		output_node *node = new output_node();
		o_nodes.push_back(node);
	}
	
}
void nn::refresh_nodes()
{
	// input_node
	for(int i=0;i<ni;i++)
	{
		i_nodes[i]->input =0.0;
		i_nodes[i]->output=0.0;
	}
	
	// hidden node
	
	for(int i=0;i<nh;i++)
	{
		h_nodes[i]->input=0.0;
		h_nodes[i]->output=0.0;	
		h_nodes[i]->lambda=0.0;	
	}
	
	// output node
	
	for(int i=0;i<no;i++)
	{
		o_nodes[i]->input=0.0;
		o_nodes[i]->output=0.0;
		o_nodes[i]->lambda=0.0;	
	}
}
double GetFloatPrecision(double value, double precision)
{
    return (floor((value * pow(10, precision) + 0.5)) / pow(10, precision)); 
}
void nn::testprop(vector<pair<vector<float> , int> > test_set)
{
	float pass=0;
	float max_v=0,max_i=-1;
	for(pair<vector<float>,int> input : test_set)
	{
		vector<float> attr = input.first;
		int target = input.second;
		
		for(int i=0;i<o_nodes.size();i++)
			o_nodes[i]->target=0.0;
		o_nodes[target]->target = 1.0;
		
		feedforward(attr);
		vector<float> out_set;
		for(int i=0;i<no;i++)
			out_set.push_back(o_nodes[i]->output);
		
		int index = distance(out_set.begin(),max_element(out_set.begin(),out_set.end()));
		
		if(index==target) pass++;	
	}
	cout<<pass<<"\t"<<test_set.size()<<endl;
	float pass_per = pass/(float)test_set.size();
	cout<<"Pass % "<<pass_per*100<<endl;
}
void nn::backprop(vector<pair<vector<float> , int> > train_set)
{
	int iterations =1;
	
		float error_epoch =0.0;
	while(1)
	{
		for(pair<vector<float>,int> input : train_set)
		{
			vector<float> attr = input.first;
			int target = input.second;
			// set target nodes for output >> get in form of index
			for(int i=0;i<o_nodes.size();i++)
			o_nodes[i]->target=0.0;
			o_nodes[target]->target = 1.0;
			
    		// set input unit to value of attributes
			assert(attr.size()+1==i_nodes.size());
			feedforward(attr);
			error_epoch=backpass();
			/*
			while(1)
			{
				feedforward(attr);
				error_epoch=backpass();
				if(error_epoch<0.0001) break;
			}
			*/
			//cout<<"Error : "<<error_epoch<<endl;
			//break;
		}
		
		error_epoch = error_epoch/train_set.size();
		//cout<<error_epoch<<"\t"<<iterations<<endl;
		// commented only for lva case , in gen not if(error_epoch<0.000006 /*|| iterations > 10000*/)
		if(error_epoch<0.000006 || iterations > 10000)
		{
			//cout<<error_epoch<<"\t"<<iterations<<endl;
		 	break;
		 }
		iterations++;
		//refresh_nodes();
	}
}
float nn::get_mse()
{
	float err =0.0;
	for(int i=0;i<no;i++)
	{
		//cout<<o_nodes[i]->target<<"\t"<<o_nodes[i]->output<<endl;
		
		float val = (o_nodes[i]->target-o_nodes[i]->output);
		err+=(pow(val,2)*0.5);
	}
	//cout<<"========\n";
	
	return err;
}
float sigmoid(float sum)
{
	float val = 1/(1+exp(-sum));
	//cout<<val<<endl;
	return val;
}
void nn::feedforward(vector<float> attr)
{
	// fill inputs for input node
	// feedforward is repeated the time MSE is  minimum
		
	for(int i=0;i<ni-1;i++)
	{
		i_nodes[i]->input=attr[i];
	}
	// set input for hidden
	for(int j=0;j<nh;j++)
	{
		float sum=0.0;
		for(int i=0;i<ni;i++)
		{
			sum+=(i_nodes[i]->weights[j]*i_nodes[i]->input);
			
		}
		// squash it >> sigmoid 
		h_nodes[j]->input = sigmoid(sum);
			
	}
		
		// set output for target
	for(int j=0;j<no;j++)
	{
		float sum = 0.0;
		for(int i=0;i<nh;i++)
		{
			sum+=(h_nodes[i]->weights[j]*h_nodes[i]->input);
		}
		// squash it >> sigmoid
		o_nodes[j]->output = sigmoid(sum);
	}
		
}
float nn::backpass()
{
	// lambda output
	for(int i=0;i<no;i++)
	{
		o_nodes[i]->lambda=o_nodes[i]->output*(1-o_nodes[i]->output)*(o_nodes[i]->target-o_nodes[i]->output);
	}
	
	// lambda hidden node
	
	for(int i=0;i<nh;i++)
	{
		float error = 0.0;
		for(int j=0;j<no;j++)
		{
			error += (o_nodes[j]->lambda*h_nodes[i]->weights[j]);
		}
		h_nodes[i]->lambda = error * h_nodes[i]->input * (1-h_nodes[i]->input);
	}
	
	// weight update
	update_weights();
	return get_mse();
}
void nn::update_weights()
{
	// input - hidden
	for(int i=0;i<ni;i++)
	{
		for(int j=0;j<nh;j++)
		{
			i_nodes[i]->weights[j]+=(lr*h_nodes[j]->lambda*i_nodes[i]->input);
		}
	}
	// hidden - output
	
	for(int i=0;i<nh;i++)
	{
		for(int j=0;j<no;j++)
		{
			h_nodes[i]->weights[j]+=(lr*o_nodes[j]->lambda*h_nodes[i]->input);
		}
	}
}
float RandomFloat(float a, float b) {
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}
void nn::randomize_wts(vector<float>& weights)
{
	for(int i=0;i<weights.size();i++)
	{
		weights[i]=RandomFloat(-0.05,0.05);
	}
}
#endif
