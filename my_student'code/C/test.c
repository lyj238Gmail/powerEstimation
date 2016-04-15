extern "C"

#include <iostream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <time.h>
#include "util.h"
#include "cudd.h"
#include "cuddInt.h"
#include "cuddObj.hh"

using namespace std;

void CreatNeighbors(vector<string>& neighbors, string obj)
{
	int len = obj.length();
	for(int i = 0; i < len; i++)
	{
		string temp;
		if(obj.at(i) == '0') temp = obj.substr(0,i)+"1"+obj.substr(i+1);
		else temp = obj.substr(0,i)+"0"+obj.substr(i+1);
		neighbors.push_back(temp);
	}
}

DdNode* CreatRestrict(DdManager *manager, string obj)
{
	DdNode * res;
	if(obj.at(0) == '1') res = Cudd_bddIthVar(manager, 0);
	else res = Cudd_Not(Cudd_bddIthVar(manager, 0));
	Cudd_Ref(res);
	int len = obj.length();
	for(int i = 1; i < len; i++)
	{
		if(obj.at(i) == '0') res = Cudd_bddAnd(manager, res, Cudd_Not(Cudd_bddIthVar(manager, i)));
		else res = Cudd_bddAnd(manager, res, Cudd_bddIthVar(manager, i));
		//Cudd_Ref(res);引用数没有增加
	}
	return res;
}

int subsitute(DdManager *manager, DdNode **node, DdNode * restricts, vector<int> weightedlist)
{
	//DdNode * innerRestrict;
	int sum = 0;
	int loop = weightedlist.size();
	DdNode *res;
	for(int i = 0; i < loop; i++)
	{
		res = Cudd_bddRestrict(manager, node[i], restricts);
		Cudd_Ref(res);
		if((res->type).value == 1) sum += weightedlist.at(i);
	}
	return sum;
}

string init(int len)
{
	string res;
	srand((unsigned)time(NULL));
	for(int i = 0; i < len; i++)
	{
		if(rand()%2 == 0) res += "0";
		else res += "1";
	}
	return res;
}

void HL(DdManager *manager, DdNode **node, vector<string> bvars, vector<int> weightedlist)
{
	bool stop = false;
	string bestassign = init(bvars.size());
	vector<string> neighbors;
	int bestweight = subsitute(manager, node, CreatRestrict(manager, bestassign), weightedlist);
	while(!stop)
	{
		stop = true;
		int innerbest;
		CreatNeighbors(neighbors, bestassign);
		vector<string>::iterator ite = neighbors.begin();
		for(; ite != neighbors.end(); ite++)
		{
			innerbest = subsitute(manager, node, CreatRestrict(manager, (*ite)), weightedlist);
			if(innerbest > bestweight)
			{
				bestweight = innerbest;
				bestassign = (*ite);
				stop = false;
			}
		}
		neighbors.clear();
	}
	cout<<"the best assignment is: \n"<<endl;
	int j = 0;
	for(std::vector<std::string>::iterator i = bvars.begin(); i != bvars.end(); i++)
	{
		j++;
		cout<<(*i)<<" assigns: "<<bestassign.at(j)<<endl;
	}
}

int main(int argc, char** argv)
{
	DdManager *manager;
	manager = Cudd_Init(0,0,CUDD_UNIQUE_SLOTS,CUDD_CACHE_SLOTS,0);

	//如下产生c17.bench的例子
	//输入分别为以下
	vector<string> bvars;
	bvars.push_back("x0");
	bvars.push_back("x1");
	bvars.push_back("x2");
	bvars.push_back("y0");
	bvars.push_back("y1");
	bvars.push_back("y2");
	//权中
	vector<int> weightedlist;
	weightedlist.push_back(1);
	weightedlist.push_back(2);
	weightedlist.push_back(1);
	weightedlist.push_back(1);
	//编号
	DdNode *x0 = Cudd_bddIthVar(manager, 0);
	DdNode *x1 = Cudd_bddIthVar(manager, 1);
	DdNode *x2 = Cudd_bddIthVar(manager, 2);
	DdNode *y0 = Cudd_bddIthVar(manager, 3);
	DdNode *y1 = Cudd_bddIthVar(manager, 4);
	DdNode *y2 = Cudd_bddIthVar(manager, 5);
	//构造inner gate
	DdNode *f1 = Cudd_Not(x0);
 	Cudd_Ref(f1);

	DdNode *f2 = Cudd_bddNand(manager, x0, x1);
	Cudd_Ref(f2);

	DdNode *f3 = Cudd_bddNand(manager, f1, f2);
	Cudd_Ref(f3);

	DdNode *f4 = Cudd_bddXor(manager, x2, f2);
	Cudd_Ref(f4);

	//构造inner gate
	DdNode *f11 = Cudd_Not( y0);
 	Cudd_Ref(f11);

	DdNode *f22 = Cudd_bddNand(manager, y0, y1);
	Cudd_Ref(f22);

	DdNode *f33 = Cudd_bddNand(manager, f11, f22);
	Cudd_Ref(f33);

	DdNode *f44 = Cudd_bddXor(manager, y2, f22);
	Cudd_Ref(f44);


	DdNode** nodes =  new DdNode *[4];
	//构造xor
	DdNode* node0 = Cudd_bddXor(manager, f1, f11);
	Cudd_Ref(node0);
	DdNode* node1 = Cudd_bddXor(manager, f2, f22);
	Cudd_Ref(node1);
	DdNode* node2 = Cudd_bddXor(manager, f3, f33);
	Cudd_Ref(node2);
	DdNode* node3 = Cudd_bddXor(manager, f4, f44);
	Cudd_Ref(node3);


	nodes[0] = node0;
	nodes[1] = node1;
	nodes[2] = node2;
	nodes[3] = node3;

	Cudd_RecursiveDeref(manager, f1);
 	Cudd_RecursiveDeref(manager, f2);
 	Cudd_RecursiveDeref(manager, f3);
	Cudd_RecursiveDeref(manager, f4);
 	Cudd_RecursiveDeref(manager, f11);
 	Cudd_RecursiveDeref(manager, f22);
	Cudd_RecursiveDeref(manager, f33);
 	Cudd_RecursiveDeref(manager, f44);
	//求解
	HL(manager, nodes, bvars, weightedlist);

	printf("\ntest over\n");
	Cudd_Quit(manager);
	return 0;
}
