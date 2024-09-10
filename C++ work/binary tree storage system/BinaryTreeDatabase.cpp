#include <iostream>
#include <fstream>
#include <string>

using namespace std;

ifstream fin;
struct person {
	int SSN, BD, ZIP; string FN, LN;
	person() {
		SSN = BD = ZIP = 0;
		FN = LN = "";
	}
};
class BT {//Binary tree that stores a person struct sorted by lastname, firstname
	// how do you search through an entire unorded binary tree and return the 'smallest' or 'oldest' value?
private:
	struct node {
		person data;
		node* right, * left;
		node(person p) {
			right = NULL;
			left = NULL;
			data = p;
		}
	};
	node* root;
	int BDtoken = 2147483647; //max a 32-bit int
	person OPtoken; //Oldest Person Token
	node* ParentHelp = NULL; //Helper value for the findParentNode() function
public:
	BT() {
		root = NULL;
	}
	~BT() {
	}
	node* Root() {
		return root;
	}
	void addPerson(person p) {// add a person to the Binary Tree, sorted by last name, first name
		if (root == NULL) {
			root = new node(p);
		}
		else {
			node* index = root;
			while (true) {
				if (p.LN < index->data.LN) {
					if (index->left == NULL) {
						index->left = new node(p);
						return;
					}
					index = index->left;
				}
				else if (p.LN > index->data.LN) {
					if (index->right == NULL) {
						index->right = new node(p);
						return;
					}
					index = index->right;
				}
				else {
					if (p.FN <= index->data.FN) {
						if (index->left == NULL) {
							index->left = new node(p);
							return;
						}
						index = index->left;
					}
					else {
						if (index->right == NULL) {
							index->right = new node(p);
							return;
						}
						index = index->right;
					}
				}
			}
		}

	}
	void deletePerson(string FN, string LN) {
		node* ParentNode = findParentNode(FN, LN);
		node* targetNode;
		bool isRight = false; //isRight and isLeft indicate that the target node is to the right or left of its parent node
		bool isLeft = false;
		if (ParentNode == NULL) {
			cout << "Person could not be found" << endl;
			return;
		}
		else if ((ParentNode->left != NULL) && (ParentNode->left->data.FN == FN) && (ParentNode->left->data.LN == LN)){
			isLeft = true; 
			targetNode = ParentNode->left;
		}
		else if ((ParentNode->right != NULL) && (ParentNode->right->data.FN == FN) && (ParentNode->right->data.LN == LN)) {
			isRight = true;
			targetNode = ParentNode->right;
		}
		else {
			cout << "Person could not be found." << endl;
			return;
		}

		if (noChildren(targetNode)) {
			cout << targetNode->data.FN << " " << targetNode->data.LN << " has been removed from the database" << endl;
			delete targetNode;
			if (isLeft) { //must set pointers to NULL after deleting node
				ParentNode->left = NULL;
			}
			if (isRight) {
				ParentNode->right = NULL;
			}
		}
		else if (oneChild(targetNode)) {
			if (isLeft) {
				if (targetNode->left != NULL) {//No need to set anything to NULL as there was a value to the left of target node
					ParentNode->left = targetNode->left;
					cout << targetNode->data.FN << " " << targetNode->data.LN << " has been removed from the database" << endl;
					delete targetNode;
				}
				else {//same but to the right 
					ParentNode->left = targetNode->right;
					cout << targetNode->data.FN << " " << targetNode->data.LN << " has been removed from the database" << endl;
					delete targetNode;
				}
			}
			else if (isRight) {
				if (targetNode->left != NULL) {//same left
					ParentNode->right = targetNode->left;
					cout << targetNode->data.FN << " " << targetNode->data.LN << " has been removed from the database" << endl;
					delete targetNode;
				}
				else {//same right
					ParentNode->right = targetNode->right;
					cout << targetNode->data.FN << " " << targetNode->data.LN << " has been removed from the database" << endl;
					delete targetNode;
				}
			}
		}
		else if (twoChildren(targetNode)) {//take biggest to the left or smallest to the right, swap biggest/smallest with targetNode, delete targetNode at new position
			node* index = targetNode->left;
			node* ParentOfIndex = targetNode;
			while (index->left != NULL) {//finds biggest to the left
				ParentOfIndex = index;
				index = index->left;
			}
			if (index->right == NULL) {
				person a; //swap
				a = index->data;
				index->data = targetNode->data;
				targetNode->data = a; //delete index and what it points to
				
				cout << index->data.FN << " " << index->data.LN << " has been removed from the database" << endl;
				delete index; 
				ParentOfIndex->left = NULL; //Delete deallocates memory, it does not set pointer value to NULL. Must do that manually
			}
			else {
				person b; //swap
				b = index->data;
				index->data = targetNode->data;
				targetNode->data = b;
				//Connect the parent pointer to what the target node was pointing to. No need to set anything to NULL as there was a value to the right of the 'biggest value to the left'
				ParentOfIndex->left = index->right;
				cout << index->data.FN << " " << index->data.LN << " has been removed from the database" << endl;
				delete index;

			}
		}
		delete ParentHelp;
		ParentHelp = NULL; //Reset ParentHelp for reuse at end of function

	}
	bool noChildren(node* n) {//only if node has no children
		if ((n->left == NULL) && (n->right == NULL)) {
			return true;
		}
		return false;
	}
	bool oneChild(node* n) {//only if node has one child
		if ((n->left != NULL) && (n->right == NULL)) {
			return true;
		}
		if ((n->right != NULL) && (n->left == NULL)) {
			return true;
		}
		return false;
	}
	bool twoChildren(node* n) {//only if node has two children
		if ((n->left != NULL) && (n->right != NULL)) {
			return true;
		}
		return false;
	}
	void addfile() {// reads and adds a text file to the binary tree
		fin.open("/home/ece218/database.txt");
		if (fin.fail()) {
			cout << "Unable to open file" << endl;
			exit(1);
		}
		while (!fin.fail()) {
			person a;
			fin >> a.SSN >> a.BD >> a.FN >> a.LN >> a.ZIP;
			if (a.BD == 0) { //The very last line of the database.txt file has a bunch of zeros that this picked up. (Most likely due to the person constructor)
							 // It was messing up the findOldest() function	
							 //can make sure data is a 'valid input' before adding to database, would be more secure than current code
				break;
			}
			addPerson(a);
			//for findOldest() function
			if (BDtoken > a.BD) {
				BDtoken = a.BD;
				OPtoken = a;
			}
		}
		fin.close();
	}
	node* findParentNode(string FN, string LN) {//returns the parent node of person you're looking for
		node* index = root;
		if (index == NULL) {
			return NULL;
		}
		if ((index->data.FN == FN) && (index->data.LN == LN)) {//creates a new parent node for the root node, used for deleting the root
			person a;
			ParentHelp = new node(a); 
			ParentHelp->left = root;
			return ParentHelp;
		}
		while (true) {
			if (LN < index->data.LN) {
				if (index->left == NULL) {
					return NULL;
				}
				if ((index->left->data.LN == LN) && index->left->data.FN == FN) {
					return index;
				}
				index = index->left;
			}
			if (LN > index->data.LN) {
				if (index->right == NULL) {
					return NULL;
				}
				if ((index->right->data.LN == LN) && index->right->data.FN == FN) {
					return index;
				}
				index = index->right;
			}
			if (LN == index->data.LN) {
				if (FN < index->data.FN) {
					if (index->left == NULL) {
						return NULL;
					}
					if ((index->left->data.LN == LN) && index->left->data.FN == FN) {
						return index;
					}
					index = index->left;
				}
				if (FN > index->data.FN) {
					if (index->right == NULL) {
						return NULL;
					}
					if ((index->right->data.LN == LN) && index->right->data.FN == FN) {
						return index;
					}
					index = index->right;
				}
			}
		}
	}
	person* findPerson(string FN, string LN) { //finds person based on First and Last name
		node* index = root;
		while (true) {
			if (LN < index->data.LN) {
				if (index->left == NULL) {
					return NULL;
				}
				index = index->left;
			}
			if (LN > index->data.LN) {
				if (index->right == NULL) {
					return NULL;
				}
				index = index->right;
			}
			if (LN == index->data.LN) {
				if (FN == index->data.FN) {
					return &index->data;
				}
				if (FN < index->data.FN) {
					if (index->left == NULL) {
						return NULL;
					}
					index = index->left;
				}
				if (FN > index->data.FN) {
					if (index->right == NULL) {
						return NULL;
					}
					index = index->right;
				}
			}
		}
		return NULL; //just in case
	}
	person* findOldest() { //returns single person as oldest, wont work if multiple people share same 'oldest' BD.
		return &OPtoken; 
	}
	void displayMatchingZip(int z, node* n) { //Displays any person with matching zip code
		if (z == n->data.ZIP) {
			displayName(&n->data);
		}
		if (n->left != NULL) {
			displayMatchingZip(z, n->left);
		}
		if (n->right != NULL) {
			displayMatchingZip(z, n->right);
		}
	}
	//Maybe combine all display functions into one, like display(person p, identifier i); i can be fn/ln/zip/bd/ssn
	void displayAllInfo(person* p) { // returns all info from person
		if (p == NULL) {
			cout << "Person could not be found" << endl;
		}
		else {
			cout << p->SSN << " " << p->BD << " " << p->FN << " " << p->LN << " " << p->ZIP << endl;
		}
	}
	void displayName(person* p) { //only displays the persons name
		if (p == NULL) {
			cout << "Person could not be found" << endl;
		}
		else {
			cout << p->FN << " " << p->LN << endl;
		}
	}
	void displayNameZip(person* p) {//display name and zipcode
		if (p == NULL) {
			cout << "Person could not be found" << endl;
		}
		else {
			cout << p->FN << " " << p->LN << " " << p->ZIP << endl;
		}
	}
	void displayAllZip(int z){ //Displays the names of all people living in the given zip code
		cout << "The following people have the provided zip code: " << endl;
		displayMatchingZip(z, root);
	}
	void displayAllOrdered(node* i) { //Prints all names in alphabetical order, with last name then first name as the priority order
		if (i->left != NULL) {
			displayAllOrdered(i->left);
		}
		cout << i->data.SSN << " " << i->data.BD << " " << i->data.FN << " " << i->data.LN << " " << i->data.ZIP << endl;
		if (i->right != NULL) {
			displayAllOrdered(i->right);
		}
	}
	person* nodeToPerson(node* n) {
		if (n == NULL) {
			return NULL;
		}
		return &n->data;
	}
	
};

void LOC() {//list of commands
	cout << "List of Commands: EXIT, FIND, PRINT, ZIP, OLDEST, DELETE, HELP" << endl;
}
void LOCI() {//list of commands and their info
	cout << "**************************************************************************************************" << endl;
	cout << "|                                                                                                |" << endl;
	cout << "| List of Commands: EXIT, FIND, PRINT, ZIP, OLDEST, DELETE, HELP                                 |" << endl;
	cout << "| Exit - Exit from the program                                                                   |" << endl;
	cout << "| FIND - Display all information about the named person                                          |" << endl;
	cout << "| PRINT - (Enter firstname then lastname) Display all information about everyone in the database |" << endl;
	cout << "|         in alphabetical order based on their names                                             |" << endl;
	cout << "| ZIP - (Enter zipcode) Display the names of all people living in the given zip code             |" << endl;
	cout << "| OLDEST - Print the name and zipcode of the oldest person in the database                       |" << endl;
	cout << "| DELETE - (Enter firstname then lastname) Remove the indicated entry from the tree              |" << endl;
	cout << "| HELP - Display list of commands and their info                                                 |" << endl;
	cout << "|                                                                                                |" << endl;
	cout << "**************************************************************************************************" << endl;
	cout << endl;
}
bool check_number(string str) { //true if int, false if string
	for (int i = 0; i < str.length(); i++) {
		if (isdigit(str[i]) == false) {
			return false;
		}
	}
	return true;
}
int main() {
	BT A;
	A.addfile();
	string input;
	LOCI();
	while (true) {
		cout << endl;
		cout << "Enter Command: ";
		cin >> input;
		if (input == "EXIT") {
			cout << "Exiting Program" << endl;
			exit(1);
		}
		else if (input == "HELP") {
			LOCI();
		}
		else if (input == "FIND") {
			string FN, LN;
			cout << "Enter first name: ";
			cin >> FN;
			cout << "Enter last name: ";
			cin >> LN;
			A.displayAllInfo(A.findPerson(FN, LN));
		}
		else if (input == "PRINT") {
			cout << "This will display all information about everyone in the database, do you wish to proceed?" << endl;
			string answer;
			while (true) {
				cout << "Please type \"YES\" or \"NO\": ";
				cin >> answer;
				if (answer == "YES") {
					A.displayAllOrdered(A.Root());
					break;
				}
				if (answer == "NO") {
					break;
				}
			}
		}
		else if (input == "ZIP") {
			string INPUT;
			int zip = 0;
			cout << "Enter zip code: ";
			cin >> INPUT;
			if (check_number(INPUT)) {// checks if input is a string or int
				zip = stoi(INPUT);
				if ((zip >= 11111) && (zip <= 99999)) {
					A.displayAllZip(zip);
				}
				else {
					cout << "Invalid zip code, please enter a five digit zip code" << endl;
				}
			}
			else {
				cout << "Invalid zip code, please enter a five digit zip code" << endl;
			}
		}
		else if (input == "OLDEST") { //oldest function will only display oldest from original database from data file. Will not give accurate result when DELETE function is used
			A.displayNameZip(A.findOldest());
		}
		else if (input == "DELETE") {
			string FN, LN;
			cout << "Enter first name: ";
			cin >> FN;
			cout << "Enter last name: ";
			cin >> LN;
			A.deletePerson(FN, LN);
		}
		else {
			cout << "Please enter a valid commands" << endl;
			LOC();
		}
	}
	return 0;
}

