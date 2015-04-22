#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<iostream>
#include<algorithm>
#include<vector>
#include<string>
#include<math.h>

using namespace std;

FILE *train;
FILE *test;
FILE *updated;

/*
	attribute & labels
*/

typedef struct _label{
	string label;
	int lid;
}tlabel;

typedef struct _attribute{
	string attr;
	vector<tlabel> labels; // attribute's label list
}tattribute;

vector<tattribute> attributes; // attributes list

/*
	Node
*/

typedef struct _tNode{
	string label; // selected label;
	string attr; // attribute or class label
	int flag; // class label/attribute flag. 1 : class label, 0 : attribute
	vector<struct _tNode> list; // edge list
}tNode; 

tNode root;

vector<vector<int>> tid_lists; // list of tid_list
string Class;
tattribute attr_Class;

vector<string> results; // result string list

int findLabel(tattribute attr, string label){
	int i;
	for (i = 0; i < attr.labels.size(); i++){
		if (label == attr.labels[i].label)
			return attr.labels[i].lid;
	}
	return -1;
}

int findAttr(string attr){
	int i;
	for (i = 0; i < attributes.size(); i++){
		if (attr == attributes[i].attr)
			return i;
	}
	return -1;
}

void printAttributes(){
	int i;
	for (i = 0; i < attributes.size(); i++){
		cout << attributes[i].attr;
		cout << " ";
	}
	cout << endl;
}

void printTidLists(){
	int i,j,k;
	for (i = 0; i < attributes.size(); i++){
		cout << attributes[i].attr << endl;
		for (j = 0; j < attributes[i].labels.size(); j++){
			int tlid = attributes[i].labels[j].lid;
			cout << "(" << attributes[i].labels[j].label<<") : ";
			for (k = 0; k < tid_lists[tlid].size();k++)
				printf("%d ", tid_lists[tlid][k]);
			cout << endl;
		}
		cout << endl;
	}
}

double log2(double x){
	return log(x) / log(2.);
}
// calculate information gain
double calculated(vector<int> range, tattribute target){
	int i,j;
	double infoAD = 0;
	for (j = 0; j < target.labels.size(); j++){
		double weight;
		vector<int>::iterator it;
		vector<int> target_tid_list = tid_lists[target.labels[j].lid];
		vector<int> temp(range.size());

		it = set_intersection(range.begin(), range.end(), 
			target_tid_list.begin(), target_tid_list.end(),temp.begin());
		temp.resize(it - temp.begin());

		weight = (double)temp.size() / range.size();
		double infoDj = 0;
		if (weight){

			for (i = 0; i < attr_Class.labels.size(); i++){
				double weight2;
				vector<int>::iterator it2;
				vector<int> class_tid_list = tid_lists[attr_Class.labels[i].lid];
				vector<int> temp2(temp.size());

				it2 = set_intersection(temp.begin(), temp.end(), 
					class_tid_list.begin(), class_tid_list.end(), temp2.begin());
				temp2.resize(it2 - temp2.begin());
				weight2 = (double)temp2.size() / temp.size();
				if (weight2)
					infoDj += (-1)*weight2*log2(weight2);
			}
		}
		infoAD += weight*infoDj;
	}
	cout << target.attr << "'s infoAD : " << infoAD << endl;
	
	return infoAD;
}
// make decision tree about parent node with remained attributes in the range tuples
void makeDecisionTree(vector<int> range, vector<string> remained, tNode *parent){
	int i;
	double min = 100;
	string min_attr;

	// find max classified class label in the range.
	int max = 0;
	int max_lid;
	for (i = 0; i < attr_Class.labels.size(); i++){
		vector<int>::iterator it;
		vector<int> class_tid_list = tid_lists[attr_Class.labels[i].lid];
		vector<int> temp(range.size());

		it = set_intersection(range.begin(), range.end(), class_tid_list.begin(), 
			class_tid_list.end(), temp.begin());
		temp.resize(it - temp.begin());
		if (max < temp.size()){
			max = temp.size();
			max_lid = i;
		}
	}

	// when all range tuples are classified as one label or no attributes are remain, 
	// label to max class label and return
	if (max == range.size()||remained.size() == 0){
		parent->flag = 1;
		parent->attr = attr_Class.labels[max_lid].label;
		cout << "**** labeled to " << parent->attr << endl;
		return;
	}

	parent->flag = 0;

	// calculated gain and find max
	for (i = 0; i < remained.size();i++){
		tattribute target = attributes[findAttr(remained[i])];
		double temp_gain = calculated(range, target);
		if (min > temp_gain){
			min = temp_gain;
			min_attr = target.attr;
		}
	}

	parent->attr = min_attr;

	remained.erase(remove(remained.begin(), remained.end(), min_attr), remained.end());
	int aid = findAttr(min_attr);
	
	// partitioning by the attribute's labels
	for (i = 0; i < attributes[aid].labels.size();i++){
		
		// intersection range & select the label;
		vector<int>::iterator it;
		vector<int> selected_tid_list = tid_lists[attributes[aid].labels[i].lid];
		vector<int> temp_range(range.size());
		it = set_intersection(range.begin(), range.end(), selected_tid_list.begin(), selected_tid_list.end(), temp_range.begin());
		temp_range.resize(it - temp_range.begin());
		// when there are remain tuples, partitioning
		if (temp_range.size()>0){
			tNode temp_child;
			temp_child.label = attributes[aid].labels[i].label;
			parent->list.push_back(temp_child);

			cout << endl << attributes[aid].attr << "(" << temp_child.label << ") pruning..." << endl;
			makeDecisionTree(temp_range, remained, &parent->list[parent->list.size()-1]);
		}
		// when there is no remain tuple, just labeled to max classified class label
		else{
			tNode temp_child;
			temp_child.label = attributes[aid].labels[i].label;
			temp_child.flag = 1;
			temp_child.attr = attr_Class.labels[max_lid].label;
			parent->list.push_back(temp_child);

			cout << endl << attributes[aid].attr << "(" << temp_child.label << ") doesn't have range..." << endl;
			cout << "**** labeled to " << temp_child.attr << endl;
		}
	}
}
// find proper class label by decision tree
string Labeling(vector<string> tuple){
	tNode *point = &root;
	while(!point->flag){
		int i;
		for(i=0;i<point->list.size();i++){
			int aid = findAttr(point->attr);
			// cout<<point->attr << " : " << point->list[i].label << " " << tuple[aid] << endl;
			if(point->list[i].label == tuple[aid]){
				point = &point->list[i];
				break;
			}
		}
	}
	string label = point->attr;
	cout<< "**** labeled to " << label << endl; 
	return label;
}

int main(int argc, char* argv[]){
	char str[100];
	char *token;

	if (argc < 3){
		printf("check input!\n");
		return 0;
	}

	train = fopen(argv[1], "r");
	test = fopen(argv[2], "r");

	// tuple
	int sflag = 0; // start flag
	int tid = -1;

	vector<string> remained; // doesn't selected attributes
	vector<int> range; // splited range by selected attributes 

	// parsing tuples
	while (!feof(train)){
		if (fgets(str, sizeof(str), train)){

			if (sflag == 0){
				// parsing attributes
				sflag = 1;

				token = strtok(str, "\t");
				
				int i;

				for (i = 0; token != NULL; i++){
					
					char * ptoken = token;
					string temp = ptoken;

					token = strtok(NULL, "\t");
					if (token == NULL){
						ptoken = strtok(ptoken, "\n");
						temp = ptoken;
						Class = temp;
					}
					else{
						remained.push_back(temp);
					}

					tattribute tattr;
					tattr.attr = temp;
					attributes.push_back(tattr);
						
				}
//				printAttributes();
			}
			else{
				// parsing tuples to tid_lists
				token = strtok(str, "\t");
				int i;

				for (i = 0; token != NULL; i++){
					if (i == attributes.size()-1)
						token = strtok(token, "\n");
					string label = token;
					int temp_lid = findLabel(attributes[i], label);
					if (temp_lid<0){
						// new Label
						tlabel temp_label;
						temp_label.label = label;
						temp_label.lid = tid_lists.size();
						temp_lid = temp_label.lid;

						attributes[i].labels.push_back(temp_label);
						
						vector<int> temp_tid_list;
						tid_lists.push_back(temp_tid_list);
					}
					tid_lists[temp_lid].push_back(tid);
					token = strtok(NULL, "\t");
				}
			}
			range.push_back(tid);
			tid++;
		}
	}

	attr_Class = attributes[attributes.size() - 1];

//	printTidLists();

	// make Decision Tree by information gain
	makeDecisionTree(range, remained,&root);
	
	// labeling
	sflag = 0;
	while (!feof(test)){
		if (fgets(str, sizeof(str), test)){
			string temp_str = str;
			
			if (sflag == 0){
				// parsing attributes
				sflag = 1;
				temp_str.insert(temp_str.size()-1,"\t" + Class);
				cout<<temp_str<<endl;
				results.push_back(temp_str);
			}
			else{
				// parsing tuple
				token = strtok(str, "\t");
				int i;
				vector<string> tuple;

				for (i = 0; token != NULL; i++){
					if(i==attributes.size()-2)
						token = strtok(token, "\n");
					string label = token;
	
					tuple.push_back(label);
					token = strtok(NULL, "\t");
				}

				string labeled = Labeling(tuple);
				temp_str.insert(temp_str.size()-1,"\t" + labeled);
				cout<<temp_str<<endl;
				results.push_back(temp_str);

			}
		}
	}
	int rid;
	updated = fopen(argv[2], "w");

	for(rid=0;rid<results.size();rid++)
		fprintf(updated,results[rid].c_str());
	
	return 0;
}