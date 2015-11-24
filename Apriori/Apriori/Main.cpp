// by Shi Jianming
/*
数据挖掘：关联规则求解算法Apriori的实现
*/

#define _CRT_SECURE_NO_WARNINGS
#define HOME

#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <cmath>
#include <map>
#include <locale>
using namespace std;
const double eps = 1e-8;
const int MaxColNum = 100;

int rowNum, columnNum; // 行数，列数
double supportMin, confidenceMin; // 最小支持度， 最小置信度
int supporNum; // 最小支持数
int total;
int Case;

vector<vector<int> > dataBase; // 保存原始数据集
vector<string> columnName; // 保存每一列的栏目名

// 数据集
struct itemset
{
	vector<int> item; // 事务（包含0个或多个项）
	int cnt; // 事务出现次数
	int id; // 事务唯一标识
	itemset()
	{
		cnt = 0;
		id = -1;
	}
};

vector<itemset> preL; // 频繁(k-1)-项集
vector<itemset> C; // 候选(k)-项集
vector<itemset> L; // 频繁(k)-项集

map<int, itemset> forL; // 为构造频繁(k)-项集

int C1[MaxColNum]; // 记录C1


/****************************************************/
/*
Hash树：
Hash函数： h(p) = p mod k
时间复杂度：O(k)
*/

struct hashTrie
{
	hashTrie *next[MaxColNum]; // Hash树后继节点
	vector<itemset> C; // 候选(k)-项集
	hashTrie()
	{
		fill(next, next + MaxColNum, nullptr);
	}
};
// 创建Hash树
void CrehashTrie(hashTrie *root, vector<int> branch)
{
	hashTrie *p = root;
	for (auto i = 0; i < branch.size(); ++i)
	{
		int pos = branch[i] % branch.size();
		if (nullptr == p->next[pos])
		{
			p->next[pos] = new hashTrie;
		}
		p = p->next[pos];
	}
	itemset itsetTmp;
	itsetTmp.item = branch;
	itsetTmp.id = (total++);
	p->C.push_back(itsetTmp);
}
// 查找branch的值，判断是否可以在Hash树中匹配成功，并记录在Hash树中匹配成功的次数，保存频繁集
bool FindhashTrie(hashTrie *root, vector<int> branch)
{
	hashTrie *p = root;
	for (auto i = 0; i < branch.size(); ++i)
	{
		int pos = branch[i] % branch.size();
		if (nullptr == p->next[pos])
		{
			return false;
		}
		p = p->next[pos];
	}
	for (auto &tmp : p->C)
	{
		auto i = 0;
		for (; i != tmp.item.size(); ++i)
		{
			if (tmp.item[i] != branch[i])
			{
				break;
			}
		}
		if (i == tmp.item.size())
		{

			++(tmp.cnt);
			if (tmp.cnt >= (supporNum))
			{
				if (forL.find(tmp.id) != forL.end())
				{
					++(forL[tmp.id].cnt);
				}else
				{
					forL.insert({tmp.id, tmp});
				}
			}
			return true;
		}
	}
	return false;
}
// 销毁Hash树
void DelhashTrie(hashTrie *T, int len)
{
	for (int i = 0; i < len; ++i)
	{
		if (T->next[i] != nullptr)
		{
			DelhashTrie(T->next[i], len);
		}
	}
	if (!T->C.empty())
	{
		T->C.clear();
	}
	delete[] T->next;
	total = 0;
}

/****************************************************/



/****************************************************/
/*
从集合{0,1,2,3..,(N-1)} 中找出所有大小为k的子集, 并按照字典序排序
*/
vector<vector<int>> combine;
int arr[MaxColNum];
int visit[MaxColNum];
int combineN, combineK;
// 起始：dfs(0, 0)
void dfs(int d, int pos)
{
	if (d == combineK)
	{
		vector<int> tmp;
		for (int i = 0; i < combineK; ++i)
		{
			tmp.push_back(arr[i]);
		}
		combine.push_back(tmp);
		return;
	}
	for (int i = pos; i < combineN; ++i)
	{
		if (!visit[i])
		{
			visit[i] = true;
			arr[d] = i;
			dfs(d + 1, i + 1);
			visit[i] = false;
		}
	}
}
/****************************************************/

// 读取原始数据集
void Input()
{
	cin >> rowNum >> columnNum;
	supporNum = ceil(supportMin*(rowNum - 1));
	string rowFirst;
	for (auto i = 0; i < rowNum; ++i)
	{
		cin >> rowFirst;
		vector<int> dataRow;
		int valueTmp;
		// 去掉输入数据的第一列
		for (auto j = 0; j < (columnNum - 1); ++j)
		{
			if (i != 0)
			{
				cin >> valueTmp;
				if (valueTmp) {
					++C1[j];
					dataRow.push_back(j);
				}
			}
			else
			{
				string colNameTmp;
				cin >> colNameTmp; 
				columnName.push_back(colNameTmp);
			}
		}
		if (i != 0) dataBase.push_back(dataRow);
	}
}

// 获取频繁1-项集
void Ini()
{
	for (auto i = 0; i < (columnNum - 1); ++i)
	{
		if (C1[i] >= supporNum)
		{
			itemset itemsetTmp;
			itemsetTmp.item.push_back(i);
			itemsetTmp.cnt = C1[i];
			preL.push_back(itemsetTmp);
		}
	}
}


// 枚举所有事务（即原始数据）包含的k-项集，计算支持度
void getItemsK(hashTrie *root, int k)
{
	vector<int> branch;
//	int bbb = 0;
	for (auto tmp : dataBase)
	{
//		cout << bbb++ << " : " << endl;
		if (tmp.size() < k) continue;

		combineN = tmp.size();
		combineK = k;
		dfs(0, 0);

		for (int i = 0; i < combine.size(); ++i)
		{
			for (int j = 0; j < combine[i].size(); ++j)
			{
				branch.push_back(tmp[combine[i][j]]);
			}
			/***********************/
			/*
			匹配候选k-项集，计算支持数
			*/
			FindhashTrie(root, branch);
//			if (FindhashTrie(root, branch))
//			{
//				for (auto aaa = 0; aaa < branch.size(); ++aaa)
//				{
//					cout << branch[aaa] << " ";
//				}
//				cout << endl;
//			}
//			/***********************/
			branch.clear();
		}
		combine.clear();
//		cout << endl;
	}
	
}

// 判断生成的候选(k)-项集的某个(k-1)-项子集是否为频繁项集
bool isInfrequentSubset(itemset c)
{
	hashTrie *root = new hashTrie;
	int k = c.item.size() - 1;
	for (auto tmp : preL)
	{
		CrehashTrie(root, tmp.item);
	}
	vector<int> branch;

	combineN = c.item.size();
	combineK = k;
	dfs(0, 0);

	for (int i = 0; i < combine.size(); ++i)
	{
		for (int j = 0; j < combine[i].size(); ++j)
		{
			branch.push_back(c.item[combine[i][j]]);
		}

		/***********************/
		/*
		判断生成的((k-1)-项子集是否为频繁的。
		*/
		if (!FindhashTrie(root, branch))
		{
			combine.clear();
			DelhashTrie(root, k);
			return true;
		}
		/***********************/
		branch.clear();
	}
	combine.clear();
	DelhashTrie(root, k);
	return false;
}

// 产生候选(k)-项集
void apriori_gen(int k)
{
	for (auto L1 = 0; L1 < preL.size(); ++L1)
	{
		for (auto L2 = L1 + 1; L2 < preL.size(); ++L2)
		{
		    auto judge = true;
			for (auto i = 0; i < (k - 1); ++i)
			{
				if (preL[L1].item[i] != preL[L2].item[i])
				{
					judge = false;
				}
			}
			if (!judge) continue;
			itemset itemsetTmp;
			for (auto i = 0; i < (k - 1); ++i)
			{
				itemsetTmp.item.push_back(preL[L1].item[i]);
			}
			itemsetTmp.item.push_back(preL[L1].item[k - 1]);
			itemsetTmp.item.push_back(preL[L2].item[k - 1]);
			if (isInfrequentSubset(itemsetTmp)) {
				continue;
			}
			C.push_back(itemsetTmp);
		}
	}
}
// Apriori算法实现，并输出关联规则集
void Apriori()
{
	for (auto k = 2; !preL.empty(); ++k)
	{
		hashTrie *root = new hashTrie;
		apriori_gen(k - 1); // 求出候选(k)-项集;
		for (auto i = 0; i < C.size(); ++i)
		{
			CrehashTrie(root, C[i].item);
		}
		C.clear();
		getItemsK(root, k);
		DelhashTrie(root, k);
		for (auto tmp : forL)
		{
			L.push_back(tmp.second);
		}
		forL.clear();
		if (L.empty())
		{
			break;
		}
		for (auto fromTmp : L)
		{
			for (auto toTmp : preL)
			{
				auto i = 0;
				for (; i < toTmp.item.size(); ++i)
				{
					if (toTmp.item[i] != fromTmp.item[i])
					{
						break;
					}
				}
				if (i == toTmp.item.size())
				{
//					double aaa = (1.0*fromTmp.cnt) / (1.0*toTmp.cnt);
//					double bbb = (1.0*fromTmp.cnt) / (1.0*toTmp.cnt) - confidenceMin;
					if ((1.0*fromTmp.cnt)/(1.0*toTmp.cnt) - confidenceMin >= 0.0)
					{
						cout << "Case " << Case++ << " : " << endl;
						for (auto j = 0; j < toTmp.item.size(); ++j)
						{
							cout << columnName[toTmp.item[j]];
							if (j != toTmp.item.size() - 1)
							{
								cout << ",";
							}
						}
						cout << " => " << columnName[fromTmp.item[toTmp.item.size()]] << endl;
					}
				}
			}
		}
		preL.clear();
		preL = L;
		L.clear();
	}
}

int main()
{
#ifdef HOME
	freopen("in", "r", stdin);
	freopen("out", "w", stdout);
#endif
	cin >> supportMin >> confidenceMin;
	Case = 0;
	total = 0;
	Input();
	Ini();
	Apriori();

#ifdef HOME
	cerr << "Time elapsed: " << clock() / CLOCKS_PER_SEC << " ms" << endl;
#endif
	return 0;
}




