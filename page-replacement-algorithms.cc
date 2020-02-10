#include <bits/stdc++.h>

using namespace std;

enum class ReplacementMethods {Optimal, FIFO, LRU, Clock};

class Method{
	string name;
public:
	void setName(const string &name){
		this->name = name;
	}
	string const getName(){
		return name;
	}
	virtual void update(int updateArg) {};
	virtual void replace(vector<int> &frames, int pageRef) {};
	virtual void replace(vector<int> &frames, int pageRef, vector<int> &futureReferences) {};
};
class Clock: public Method{
	int pointer;
	vector<bool> bits;
public:
	Clock(int framesNumber){
		setName("CLOCK");
		pointer = 0;
		bits.resize(framesNumber, true);
	}
	void update(int index){
		bits[index] = true;
	}
	void replace(vector<int> &frames, int pageRef){
		int i = pointer;
		int cycle = 0;
		while(bits[i] && cycle < bits.size()){
			bits[i] = false;
			i ++;
			i %= bits.size();
			cycle ++;
		}		
		bits[i] = true;
		frames[i] = pageRef;
		pointer = (i+1) % bits.size();
		return;
	}
};
class FIFO: public Method{
	int index;
public:
	FIFO(){
		setName("FIFO");
		index = 0;
	}
	void replace(vector<int> &frames, int pageRef){
		frames[index] = pageRef;
		index ++;
		index %= frames.size();
		return;
	}
};
class LRU: public Method{
	vector<int> lru;
public:
	LRU(){
		setName("LRU");
	}
	void update(int pageRef){
		auto index = find(lru.begin(), lru.end(), pageRef);
		if(index != lru.end()){
			lru.erase(index);	
		}
		lru.push_back(pageRef);
	}
	void replace(vector<int> &frames, int pageRef){
		auto index = find(frames.begin(), frames.end(), lru[0]);
		*index = pageRef;
		update(pageRef);
		lru.erase(lru.begin());
		return;
	}
};
class Optimal: public Method{
public:
	Optimal(){
		setName("OPTIMAL");
	}
	void replace(vector<int> &frames, int pageRef, vector<int> &futureReferences){
		int replacingPageIndex;
		int farthestReference = INT_MIN;
		for(int i=0; i<frames.size(); i++){
			int page = frames[i];
			int index = getNextReference(page, futureReferences);
			if(index == -1){
				replacingPageIndex = i;
				break;
			}
			else if(index > farthestReference){
				farthestReference = index;
				replacingPageIndex = i;
			}
		}
		frames[replacingPageIndex] = pageRef;
		return;
	}
private:
	int getNextReference(int page, vector<int> &futureReferences){
		for(int i=0; i<futureReferences.size(); i++){
			if(futureReferences[i] == page){
				return i;
			}
		}
		return -1;
	}
};
class Process{
	vector<int> frames;
	ReplacementMethods methodName;
	int longestPageRef;
	int segFaults;
	Method* method;
public:
	Process(int framesNumber, ReplacementMethods methodName){
		this->frames.resize(framesNumber, -1);
		this->methodName = methodName;
		this->longestPageRef = 2;	
		segFaults = 0;
		buildMethod();
		printBegHeader();
	}
	void accessPages(vector<int> &pageReferences){
		if(methodName == ReplacementMethods::Optimal){
			for(int i=0; i<pageReferences.size(); i++){
				vector<int> futureRefernces(pageReferences.begin()+i, pageReferences.end());
				accessPageOptimal(pageReferences[i], futureRefernces);
			}
			printEndHeader();
			return;
		}
		for(int i=0; i<pageReferences.size(); i++){
			accessPage(pageReferences[i]);
		}
		printEndHeader();
	}
	void accessPage(int pageRef){
		for(int i=0; i<frames.size(); i++){
			int updateArg = getUpdateArg(i, pageRef);
			if(frames[i] == pageRef){ // no seg. fault
				method->update(updateArg);
				print(pageRef, false);
				return;
			}
			if(frames[i] == -1){ // no seg. fault
				frames[i] = pageRef;
				method->update(updateArg);
				print(pageRef, false);
				return;
			}
		}
		// seg. fault
		method->replace(frames, pageRef);
		print(pageRef, true);
		segFaults++;
	}
	void printEndHeader(){
		cout << string(37, '-') << endl;
		cout << "Number of page faults = " << segFaults << endl;
	}
private:
	void buildMethod(){
		if(methodName == ReplacementMethods::Optimal){
			method = new Optimal();
			return;
		}
		if(methodName == ReplacementMethods::LRU){
			method = new LRU();
			return;
		}
		if(methodName == ReplacementMethods::FIFO){
			method = new FIFO();
			return;
		}
		if(methodName == ReplacementMethods::Clock){
			method = new Clock(frames.size());
			return;
		}
	}
	void accessPageOptimal(int pageRef, vector<int> &futureReferences){
		for(int i=0; i<frames.size(); i++){
			if(frames[i] == pageRef){ // no seg. fault
				print(pageRef, false);
				return;
			}
			if(frames[i] == -1){ // no seg. fault
				frames[i] = pageRef;
				print(pageRef, false);
				return;
			}
		}
		// seg. fault
		method->replace(frames, pageRef, futureReferences);
		print(pageRef, true);
		segFaults++;
		return;
	}
	int getUpdateArg(int currFrame, int pageRef){
		if(methodName == ReplacementMethods::LRU){
			return pageRef;
		}
		return currFrame;
	}
	void printBegHeader(){
		cout << "Replacement Policy = ";
		cout << method->getName() << endl;
		cout << string(37, '-') << endl;
		cout << "Page   Content of Frames\n";
		cout << string(4, '-') << "   " << string(17, '-') << endl;
	}
	void print(int pageRef, bool segFault){
		int diff = longestPageRef - (int)(log10(pageRef)+1);
		while(diff--) cout << '0';
		cout << pageRef << ' ';
		if(segFault) cout << "F   ";
		else cout << "    ";
		for(int page: frames){
			if(page == -1){
				continue;
			}
			diff = longestPageRef - (int)(log10(page)+1);
			while(diff--) cout << '0';
			cout << page << ' ';
		}
		cout << endl;
	}
};
ReplacementMethods getMethod(string method){
	if(method == "OPTIMAL")
		return ReplacementMethods::Optimal;
	if(method == "LRU")
		return ReplacementMethods::LRU;
	if(method == "FIFO")
		return ReplacementMethods::FIFO;
	return ReplacementMethods::Clock;
}
int main(){
	int framesNumber;
	cin >> framesNumber;
	string method;
	cin >> method;
	vector<int> pageRefs;
	int n;
	while(1){
		cin >> n;
		if(n == -1) break;
		pageRefs.push_back(n);
	}
	Process* process = new Process(framesNumber, getMethod(method));
	process->accessPages(pageRefs);
	return 0;
}
