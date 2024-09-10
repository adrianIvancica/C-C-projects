#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;


int LOCATION_TOTAL = 0; 
ifstream conIN, interIN;
struct connection {
	string name, code;
	int isecA, isecB; //intersection A and B
	double length; //in miles
	connection(string n, string c, int a, int b, double l) {
		name = n;
		code = c;
		isecA = a;
		isecB = b;
		length = l;
	}
};
struct intersection {
	string state, name; //nearest named place name and state
	double lon, lat, dist; // distance from nearest named place
	intersection(string st, string na, double lo, double la, double dis) {
		state = st;
		name = na;
		lon = lo;
		lat = la;
		dist = dis;
	}
};
struct node {
	vector<connection*> v_conn_data;// where to location
	vector<node*> connects_to;
	intersection* inter_data;
	int number;
	node(intersection* inter, int num) {
		inter_data = inter;
		number = num;
	}
};
struct graph {
	vector<node*> v_nodes;
};
void readInterToGraph(graph& g) {
	interIN.open("intersections.txt");
	if (interIN.fail()) {
		cout << "unable to open intersections.txt" << endl;
		exit(1);
	}

	string s, n; double lo, la, d;
	string name; node* nptr; intersection* iptr;

	while (!interIN.fail()) {
		interIN >> lo >> la >> d >> s;
		getline(interIN, name);
		name.erase(0, 1); //removes space from beginning of name

		iptr = new intersection(s, name, lo, la, d);

		nptr = new node(iptr, LOCATION_TOTAL);//0 is no place exists
		LOCATION_TOTAL++;

		g.v_nodes.push_back(nptr);
	}

	interIN.close();
}
void connectGraph(graph &g) {

	conIN.open("connections.txt");
	if (conIN.fail()) {
		cout << "unable to open connections.txt" << endl;
		exit(1);
	}
	

	string N, C; int A, B; double L; connection* cptr;

	while (!conIN.fail()) {
		conIN >> N >> C >> A >> B >> L;
		cptr = new connection(N, C, A, B, L);
		g.v_nodes.at(A)->v_conn_data.push_back(cptr);
		g.v_nodes.at(A)->connects_to.push_back(g.v_nodes.at(B));

		g.v_nodes.at(B)->v_conn_data.push_back(cptr);
		g.v_nodes.at(B)->connects_to.push_back(g.v_nodes.at(A));
	}

	conIN.close();
	
}
node* goToLocation(graph g, int number) {
	if (number > 0 && number < g.v_nodes.size()) {
		return g.v_nodes.at(number);
	}
	return NULL;
}
bool check_number(string str) {
	for (int i = 0; i < str.length(); i++) {
		if (isdigit(str[i]) == false) {
			return false;
		}
	}
	return true;
}


int main() {
	graph G;
	readInterToGraph(G);
	connectGraph(G);
	bool nextState = false;
	string input;
	cout << "Please enter a location number between 1 and " << G.v_nodes.size() << endl << endl;
	while (!nextState) {
		cout << "Location to start: ";
		cin >> input;
		if (input == "exit") {
			exit(0);
		}
		else if (!check_number(input)) {// if not a number
			cout << "Please enter a location number between 1 and " << G.v_nodes.size() << endl << endl;
		}
		else if (!(stoi(input) > 0 && stoi(input) < G.v_nodes.size())) {// if not in range
			cout << "Please enter a location number between 1 and " << G.v_nodes.size() << endl;
		}
		else {
			nextState = true;
		}
	}
	cout << endl;
	node* nptr = NULL;
	string input2;
	nptr = goToLocation(G, stoi(input));

	bool next = false;
	while (1) {
		next = false;
		if (nptr->connects_to.size() <= 0) {
			cout << "connects_to.size() error" << endl;
			exit(1);
		}
		if (nptr == NULL) {
			cout << "Error, could not find location" << endl;
			exit(1);
		}
		else {
			cout << "Location " << nptr->number << ", " << nptr->inter_data->dist << " miles from " << nptr->inter_data->name << ", " << nptr->inter_data->state << endl;
			cout << "Roads leading away:" << endl;
			int i = 0;
			for (i; i < nptr->connects_to.size(); i++) {
				cout << i + 1 << ": " << nptr->v_conn_data.at(i)->name << ", " << nptr->v_conn_data.at(i)->length << " miles to location " << nptr->connects_to.at(i)->number << endl;
			}
		}

		cout << "which road? ";
		while (!next) {
			cin >> input2;
			if (input2 == "exit") {
				exit(0);
			}
			else if (!check_number(input2)) {// if not a number
				cout << " input an indexed number given from the list or type \"exit\" to exit the program" << endl;
			}
			else if (stoi(input2) > 0 && stoi(input2) <= nptr->connects_to.size()) {// if in range
				nptr = nptr->connects_to.at(stoi(input2) - 1);
				next = true;
			}
			else {
				cout << " input an indexed number given from the list or type \"exit\" to exit the program" << endl;
			}
		}
		cout << endl;
	}
	
	return 0;
}