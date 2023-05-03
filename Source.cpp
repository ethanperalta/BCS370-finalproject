// Ethan Peralta
// R01912247
// Professor Zhao
// BCS370
// April 30 2023
// College Student Homework Management System

// imports & namespace
#include <algorithm>
#include <chrono>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
using namespace std;

// Assignment class (singular object)
class Assignment {
private:
	string name, course; // assignment name, course code
	double length, weight; // estimated number of hours to spend on assignment, percentage of grade (decimal between 0 and 1)
	chrono::system_clock::time_point due; // date and time assignment is due
	bool complete; // completion status

public:
	//constructor w/ default assignments [should work parameterless? need to test]
	Assignment(string n = "unnamed", string c = "n/a", chrono::system_clock::time_point dd = chrono::system_clock::now(), double l = -1, double w = -1,  bool comp = false) {
		this->name = n;
		this->course = c;
		this->due = dd;
		this->length = l;
		this->weight = w;
		this->complete = comp;
	}

	// setters [function-chained]
	Assignment& setName(string n) {
		this->name = n;
		cout << "Assignment name set to \"" << n << "\"" << endl << endl;
		return *this;
	}

	Assignment& setCourse(string c) {
		this->course = c;
		cout << "Course set to \"" << c << "\"" << endl << endl;
		return *this;
	}

	Assignment& setDueDate(chrono::system_clock::time_point dd) {
		due = dd;
		
		/*
		the following block of code (lines labeled REF CODE) you'll see a lot throughout my program
		i think i found this solution on stackoverflow while i was troubleshooting how to store a date and time in c++
		im so accustomed to SQL that i actually expected there to be a datetime data type, sadly there was not

		i can't find the link for where i found this because i was coding for like a full day straight
		and i was constantly googling the whole day about exceptions and how to make linkedlist do the things i wanted
		so its buried in a list of hundreds of searches
		*/

		tm dtm; // REF CODE
		time_t dt = chrono::system_clock::to_time_t(due); // REF CODE
		if (gmtime_s(&dtm, &dt) != 0) { return *this; } // this is the only way i could get gmtime_s to work, it wouldn't work unless i caught errors like this (which makes no sense to me, but whatever) i had to do this to convert my janky datetime format to string
		char duedate[20]; // create char array because strftime only takes char pointers
		strftime(duedate /* char ptr */, sizeof(duedate) /* size_t maxsize of char ptr */, "%m/%d/%Y %H:%M:%S" /* format specifier */, &dtm /* tm* timeptr */);
		cout << "Due date set to: " << duedate << endl << endl; // echo the update with new due date
		return *this;
	}

	Assignment& setLength(double l) {
		this->length = l;
		cout << "Assignment length [hrs] set to: " << l << endl << endl; // echo the update with new assignment length
		return *this;
	}

	Assignment& setWeight(double w) {
		this->weight = w;
		cout << "Assignment weight [0 - 1] set to: " << w << endl << endl; // echo the update with new assignment weight
		return *this;
	}

	Assignment& setCompletionStatus(bool comp) {
		this->complete = comp;
		cout << "Assignment completion status set to: " << (comp ? "COMPLETE" : "INCOMPLETE") << endl << endl; // echo the update with new completion status
		return *this;
	}

	// getters
	string getName() { return this->name;	}
	string getCourse() { return this->course; }
	chrono::system_clock::time_point getDueDate() { return due; }
	double getLength() { return this->length; }
	double getWeight() { return this->weight; }
	bool getCompletionStatus() { return this->complete; }

	// other
	// basically a toString() method (in Java terms)
	void display() {
		tm dtm;
		time_t dt = chrono::system_clock::to_time_t(due);
		if (gmtime_s(&dtm, &dt) != 0) { return; }
		char duedate[20];
		strftime(duedate, sizeof(duedate), "%m/%d/%Y %H:%M:%S", &dtm);
		printf("%-18s%-30s%-27s%-19.2lf%-21.2lf%-20s\n", course.c_str(), name.c_str(), duedate, weight, length, complete ? "Complete" : "Incomplete");
	}

	// converts each assignment to a binary string ready for file-writing
	string binary_convert() {
		stringstream bstr; // create stringstream to push binary assignment data onto

		bstr.write( (char*)&this->length, sizeof(this->length) ); // took forever, once i got this line to work the rest was easy
		bstr.write( (char*)&this->weight, sizeof(this->weight) ); // ^^
		bstr.write( (char*)&this->due, sizeof(this->due) ); // ^^
		bstr.write( (char*)&this->complete, sizeof(this->complete) ); // ^^

		size_t name_size = this->name.size(); // strings needed this, was buggy if i tried to do it the normal way ;; [ serial.write((char*)&this->name, sizeof(this->name)) ] would not work for me
		bstr.write( (char*)&name_size, sizeof(name_size)); // ^^
		bstr.write(this->name.c_str(), name_size); // ^^

		size_t course_size = this->course.size(); // same 3 lines as above, but for the course member variable instead of the name member variable
		bstr.write( (char*)&course_size, sizeof(course_size) );
		bstr.write(this->course.c_str(), course_size);

		return bstr.str(); // return fully prep'd binary string
	}
}; 

// AssignmentList class (STL linked list of Assignment objects)
class AssignmentList {
private:
	list<Assignment> al; //sneaky way of using STL linked list to create my AssignmentList class

public:
	// function chained add for pre-built assignment object
	AssignmentList& add(Assignment a) {
		this->al.push_back(a); // push paramaterized assignment onto back of member list
		return *this; // return calling instance
	}

	// function chained add that accepts non-instanced assignment (given normal parameters for a new assignment object)
	AssignmentList& add(string n = "unnamed", string c = "n/a", chrono::system_clock::time_point dd = chrono::system_clock::now(), double l = -1, double w = -1, bool comp = false) {
		Assignment a(n, c, dd, l, w, comp); // create assignment from given parameters
		this->al.push_back(a); // push assignment onto back of member list
		return *this; // return calling instance
	}

	// function chained method to update assignment due date (specify assignment using course + assignment name [can use incompletes, searches by substring] think of these 2 member variables as a compound primary key to uniquely identify assignment objects)
	AssignmentList& updateDueDate(string c, string n, chrono::system_clock::time_point dd = chrono::system_clock::now()) {
		int ct = 0; // for iterator
		for (auto& a : this->al) { // iterate through member list (non iterator loop because the iterator loop kept bugging out)
			if (a.getCourse().find(c) != -1 && a.getName().find(n) != -1) { // check if specified course and name strings exist within course and name member strings for current assignment
				Assignment b(a.getName(), a.getCourse(), dd, a.getLength(), a.getWeight(), a.getCompletionStatus()); // create new assignment instance with updated due date
				list<Assignment>::iterator i = this->al.begin(); // create iterator at head of list
				advance(i, ct); // relocate iterator to current node
				this->al.erase(i); // erase current node
				return this->add(b); // add new assignment with updated due date that we previously created
			} ct++; // if no match, increment this variable, we use this to reposition the iterator
		}
		cout << "No results found!\n"; // if no match
		return *this; // catch case, return calling instanec
	}

	// function chained method to mark assignment as complete (specify assignment the same as previous method, by using course + assignment name)
	AssignmentList& markAsComplete(string c, string n) {
		int ct = 0; // for iterator
		for (auto& a : this->al) {
			if (a.getCourse().find(c) != -1 && a.getName().find(n) != -1) {
				Assignment b(a.getName(), a.getCourse(), a.getDueDate(), a.getLength(), a.getWeight(), true); // create new assignment instance with updated completion status
				list<Assignment>::iterator i = this->al.begin();
				advance(i, ct);
				this->al.erase(i);
				return this->add(b); // add new assignment with updated completion status that we previously created
			} ct++;
		}
		cout << "No results found!\n";
		return *this;
	}

	// specify course name to show list of assignments that match that course name (can use incompletes, searches by substring)
	// originally i had done this with a return type (returning an AssignmentList) but it was much simpler this way
	void searchByCourse(string c) {
		list<Assignment> results; // create display list
		for (auto& a : this->al) {
			if (a.getCourse().find(c) != -1) { results.push_back(a); } // if specified course name exists within course member string for current assignment, push current assignment onto back of display list
		}
		if (results.empty()) { cout << "No results found!\n"; } // if display list is empty after we have iterated through member list, display this instead
		else { // if display list has some nodes, display the following
			cout << "List of assignments with '" << c << "' in course title: " << endl << endl;
			printf("%-18s%-30s%-27s%-19s%-21s%-20s\n", "Course title", "Name of assignment", "Due date & time", "Weight (0 - 1)", "Est. length (hrs)", "Status");
			cout << "------------------------------------------------------------------------------------------------------------------------------" << endl;
			for (auto& a : results) { a.display(); } // loop through display list and call Assignment::display() for each
			cout << endl << endl;
		}
	}

	// specify number of days (from today) to display assignments due within specified period
	// essentially works the same as searchByCourse() just with modified logic
	// does the same trick with building a new list and displaying it to output stream
	void searchByDaysFromDueDate(int days) {
		list<Assignment> results;
		for (auto& a : this->al) {
			tm dtm; // same REF CODE as before
			time_t dt = chrono::system_clock::to_time_t(a.getDueDate()); // same REF CODE as before
			if (gmtime_s(&dtm, &dt) != 0) { return; } // same catch statement i used before in Assignment::setDueDate()
			time_t cur = time(NULL); // convert current time to time_t
			time_t conv = mktime(&dtm); // convert assignment due date to time_t
			if ( difftime(conv, cur) / 86400 <= days ) { results.push_back(a); } // calculate difference between current time and assignment due datetime, then convert value to days by dividing by 86400 (sec/day) if value is bounded by specified number of days, push onto display list
		}
		if (results.empty()) { cout << "No results found!\n"; }
		else {
			cout << "List of assignments due in the next " << days << " days: " << endl << endl;
			printf("%-18s%-30s%-27s%-19s%-21s%-20s\n", "Course title", "Name of assignment", "Due date & time", "Weight (0 - 1)", "Est. length (hrs)", "Status");
			cout << "------------------------------------------------------------------------------------------------------------------------------" << endl;
			for (auto& a : results) { a.display(); }
			cout << endl << endl;
		}
	}

	// display current in progress assignments
	void displayInProgress() {
		list<Assignment> results;
		for (auto& a : this->al) {
			tm dtm; // same REF CODE as before
			time_t dt = chrono::system_clock::to_time_t(a.getDueDate()); // same REF CODE as before
			if (gmtime_s(&dtm, &dt) != 0) { return; }
			time_t cur = time(NULL);
			time_t conv = mktime(&dtm);
			if (!a.getCompletionStatus() && conv > cur) { results.push_back(a); } // if assignment is incomplete AND assignment isn't overdue, push onto display list
		}
		if (results.empty()) { cout << "No results found!\n"; }
		else {
			cout << "List of in-progress assignments: " << endl << endl;
			printf("%-18s%-30s%-27s%-19s%-21s%-20s\n", "Course title", "Name of assignment", "Due date & time", "Weight (0 - 1)", "Est. length (hrs)", "Status");
			cout << "------------------------------------------------------------------------------------------------------------------------------" << endl;
			for (auto& a : results) { a.display(); }
			cout << endl << endl;
		}
	}

	// display all assignments held in calling instance of AssignmentList
	void displayall() {
		printf("%-18s%-30s%-27s%-19s%-21s%-20s\n", "Course title", "Name of assignment", "Due date & time", "Weight (0 - 1)", "Est. length (hrs)", "Status");
		cout << "------------------------------------------------------------------------------------------------------------------------------" << endl;
		for (auto& a : this->al) {
			a.display();
		}
		cout << endl << endl;
	}

	// aggregate binary strings into a "binary array" 
	// as of now ( May 3 2023 1:14 AM ) i have not written a file-read / binary-deconvert implementation yet
	// so without that implementation, this data is currently useless (but should still meet the basic rubric requirements)
	string binary_aggregate() {
		string agg_str;
		for (auto &a : this->al) { // loop through member list and call binary_convert() for each, add comma unless end of list has been reached
			agg_str += a.binary_convert();
			if (!this->al.empty()) { agg_str += ","; }
		}
		return ("[" + agg_str + "]") ;
	}

	// simple file write method (binary conversion is done elsewhere, so this method is extremely lightweight)
	void writefile(string filename) {
		ofstream fs(filename, ios::out | ios::binary);
		if (fs.is_open()) {
			fs.write(this->binary_aggregate().c_str(), this->binary_aggregate().size());
			fs.close();
			cout << "Successfully wrote program state to " << filename << endl << endl;
		}
		else { cout << "There was an error writing the file." << endl << endl; }
	}
};


int main() {
	Assignment a1("Programming Homework #1", "BCS370", chrono::system_clock::from_time_t(1683503999), 3, 0.05, false);
	Assignment a2("Programming Homework #2", "BCS370", chrono::system_clock::from_time_t(1684108799), 3, 0.05, false);
	Assignment a3("Programming Homework #3", "BCS370", chrono::system_clock::from_time_t(1684713599), 3, 0.05, false);
	Assignment a4("Programming Homework #4", "BCS370", chrono::system_clock::from_time_t(1685318399), 3, 0.05, false);
	Assignment a5("Discussion Board", "BCS301W", chrono::system_clock::from_time_t(1682985599), 0.5, 0.15, false);
	AssignmentList assignments;
	assignments.add(a1).add(a2).add(a3).add(a4);

	//assignments.displayall();
	//assignments.searchByCourse("BCS");
	
	//assignments.searchByDaysFromDueDate(15);
	//assignments.updateDueDate("370", "#3", chrono::system_clock::from_time_t(1684108799)).searchByDaysFromDueDate(15);
	//assignments.markAsComplete("370", "#3").searchByDaysFromDueDate(15);
	//assignments.displayInProgress();
	assignments.displayall();
	assignments.writefile("assignment_data.DAT");
}