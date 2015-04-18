#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<iostream>
#include<algorithm>
#include<vector>

using namespace std;

FILE *input;
FILE *output;

int min_sup; // min_sup
int min_sup_num; // (min_sup / 100) * size of transacrions

vector<vector<int>> transaction;

typedef struct _tid_List{
	int data;
	vector<int> list;
}tid_list; // list of tid which include specific data

vector<tid_list> vertical_type;

int mapDataToVID[100000]; // mapping data to vertical type idx

vector<vector<vector<int>>> L;
vector<vector<vector<int>>> C;

vector<vector<int>> freq_Itemsets;
vector<pair<vector<int>, vector<int>>> rules;

void init(){
	int i, j;

	for (i = 0; i < 100000; i++){
		mapDataToVID[i] = -1;
	}
}

void printTransaction(){
	int i, j;
	for (i = 0; i < transaction.size(); i++){
		for (j = 0; j < transaction[i].size(); j++){
			printf("%d ", transaction[i][j]);
		}
		printf("\n");
	}
	printf("transaction count is %d\n", transaction.size());
}

void printVertical(){
	int i, j;

	printf("print Vertical type\n");

	for (i = 0; i < vertical_type.size(); i++){
		printf("%d : {", vertical_type[i].data);
		for (j = 0; j < vertical_type[i].list.size(); j++){
			printf("%d ", vertical_type[i].list[j]);
		}
		printf("}\n");
	}
	printf("vertical type count is %d\n", vertical_type.size());
}

// generate C1 and L1
void generateL1(){
	int i;
	printf("generateL1\n");
	vector<vector<int>> L1;
	vector<vector<int>> C1;

	for (i = 0; i < vertical_type.size(); i++){
		vector<int> temp;
		temp.push_back(vertical_type[i].data);
		C1.push_back(temp);
		if (vertical_type[i].list.size() >= min_sup_num)
			L1.push_back(temp);
	}
	sort(C1.begin(), C1.end());
	C.push_back(C1);

	sort(L1.begin(), L1.end());
	L.push_back(L1);

}

void printL(int k){
	int i, j;
	printf("printL%d\n", k);
	for (i = 0; i < L[k - 1].size(); i++){
		for (j = 0; j < L[k - 1][i].size(); j++){
			printf("%d ", L[k - 1][i][j]);
		}
		printf("\n");
	}
}

void printC(int k){
	int i, j;
	printf("printC%d\n", k);
	for (i = 0; i < C[k - 1].size(); i++){
		for (j = 0; j < C[k - 1][i].size(); j++){
			printf("%d ", C[k - 1][i][j]);
		}
		printf("\n");
	}
}

bool isExist(vector<vector<int>> list, vector<int> target){
	int i, j;
	for (i = 0; i < list.size(); i++){
		int flag = 1;
		for (j = 0; j < target.size(); j++){
			if (list[i][j] != target[j]){
				flag = 0;
				break;
			}
		}
		if (flag == 1)
			return true;
	}
	return false;
}

void printVector(vector<int> temp){
	int i;
	printf("%d : ", temp.size());
	for (i = 0; i<temp.size(); i++){
		printf("%d ", temp[i]);
	}
}

// self-joining Lk and make Ck+1 candidates
vector<vector<int>> join(int k){
	printf("self-joining L%d\n", k);
	int i, j;
	int ida = 0, idb = 0;
	vector<vector<int>> temp_c_list;
	for (i = 0; i < L[k - 1].size(); i++){
		for (j = i + 1; j < L[k - 1].size(); j++){
			if (k>1 && L[k - 1][i][k - 2] != L[k - 1][j][k - 2])
				break;
			vector<int> temp_c;
			int idx = 0;

			while (idx<k - 1){
				temp_c.push_back(L[k - 1][i][idx]);
				idx++;
			}
			temp_c.push_back(L[k - 1][i][idx]);
			temp_c.push_back(L[k - 1][j][idx]);

			temp_c_list.push_back(temp_c);
		}
	}
	return temp_c_list;
}

// pruning Ck candidates
bool pruning(int k, vector<vector<int>> temp_c_list){
	printf("pruning C%d\n", k);

	if (temp_c_list.size() == 0)
		return false;

	int C_id, i, L_id;
	vector<vector<int>> pruned_c_list;

	for (C_id = 0; C_id < temp_c_list.size(); C_id++){
		int flag = 1;
		for (i = 0; i < k - 1; i++){
			vector<int> temp = temp_c_list[C_id];
			temp.erase(temp.begin() + i);
			for (L_id = 0; L_id < L[k - 2].size(); L_id++){
				if (L[k - 2][L_id][0] != temp[0])
					continue;

				else{
					if (k>2 && L[k - 2][L_id][1] != temp[1])
						continue;
				}

				if (equal(temp.begin(), temp.end(), L[k - 2][L_id].begin())){
					break;
				}
			}
			if (L_id == L[k - 2].size()){
				flag = 0;
				break;
			}
		}
		if (flag == 1){
			pruned_c_list.push_back(temp_c_list[C_id]);
		}
	}

	if (pruned_c_list.size() > 0){
		C.push_back(pruned_c_list);
		return true;
	}
	else
		return false;
}

// generate Ck by joining and pruning
bool apriori_gen(int k){
	return pruning(k, join(k - 1));
}

// calculate itemset's support
int findSupport(vector<int> data_set){
	int i;

	vector<int> result = vertical_type[mapDataToVID[data_set[0]]].list;

	for (i = 1; i < data_set.size(); i++){
		vector<int> temp(transaction.size());
		vector<int>::iterator it;
		vector<int> target = vertical_type[mapDataToVID[data_set[i]]].list;

		it = set_intersection(result.begin(), result.end(), target.begin(),
			target.end(), temp.begin());
		temp.resize(it - temp.begin());

		result.clear();

		for (it = temp.begin(); it != temp.end(); ++it){
			result.push_back(*it);
		}
	}

	return result.size();
}

void print_subset(vector<int> subset){
	int i = 0;
	fprintf(output, "{%d", subset[i]);
	for (i = 1; i < subset.size(); i++)
		fprintf(output, ",%d", subset[i]);
	fprintf(output, "}");
}

// find all subset by recursion
void find_subset(int k, int l, vector<int> pre, vector<int> total, int sup_total){
	int ti;
	if (pre.size() < k){
		for (ti = l + 1; ti < total.size(); ti++){
			pre.push_back(total[ti]);
			find_subset(k, ti, pre, total, sup_total);
			pre.pop_back();
		}
	}
	else{
		int sup_pre = findSupport(pre);

		print_subset(pre);
		fprintf(output, "\t");

		vector<int> aft = total;
		int pi;
		for (pi = 0; pi < pre.size(); pi++){
			vector<int>::iterator it = find(aft.begin(), aft.end(), pre[pi]);
			int pos = distance(aft.begin(), it);
			aft.erase(aft.begin() + pos);
		}
		print_subset(aft);
		fprintf(output, "\t");
		fprintf(output, "%.2lf\t%.2lf\n",
			(double)(sup_total * 100) / (double)transaction.size(),
			(double)sup_total * 100 / (double)sup_pre);
		return;
	}
}


// calculate Ck's support and filtering Ck which doesn't satisfy min_sup
void scanning(int k){
	printf("scanning C%d\n", k);
	int i;

	vector<vector<int>> temp_L_list;

	for (i = 0; i < C[k - 1].size(); i++){
		int cnt = findSupport(C[k - 1][i]);

		if (cnt >= min_sup_num){
			temp_L_list.push_back(C[k - 1][i]);
			int j;
			for (j = 1; j < C[k - 1][i].size(); j++){
				vector<int> pre;
				find_subset(j, -1, pre, C[k - 1][i], cnt);
			}
		}
	}
	L.push_back(temp_L_list);
}

int main(int argc, char* argv[]){
	int num;
	char c;
	int temp;
	char str[100];
	char *token;

	if (argc < 4){
		printf("check input!\n");
		return 0;
	}

	input = fopen(argv[2], "r");
	output = fopen(argv[3], "w");

	init();

	// parsing to transaction array
	while (!feof(input)){
		if (fgets(str, sizeof(str), input)){
			token = strtok(str, "\t");
			int i;
			vector<int> transaction_temp;
			for (i = 0; token != NULL; i++){
				temp = atoi(token);
				transaction_temp.push_back(temp);

				if (mapDataToVID[temp] == -1){
					tid_list temp_list;
					temp_list.data = temp;

					vertical_type.push_back(temp_list);
					mapDataToVID[temp] = vertical_type.size() - 1;
				}

				vertical_type[mapDataToVID[temp]].list.push_back(transaction.size());

				token = strtok(NULL, "\t");
			}
			sort(transaction_temp.begin(), transaction_temp.end());
			transaction.push_back(transaction_temp);
		}
	}

	min_sup = atoi(argv[1]);
	min_sup_num = min_sup * transaction.size() / 100;

	// convert min_sup ratio to # of transaction 
	printf("min_sup_num is %d\n", min_sup_num);

	// generate C1 and L1
	generateL1();

	int k;
	// generate frequent itemsets
	for (k = 2; k < vertical_type.size(); k++){
		if (apriori_gen(k)){
			scanning(k);
		}
		else
			break;
	}

	fclose(output);
	return 0;
}