#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "sha256.h"
#include "Client.h"

using namespace std;

bool isValidNumber(string& phoneNumber){
    while(phoneNumber.length()!=8)
        cout<<"Invalid phone number. Please enter a valid phone number containing 8 digits.";
    for(int i=0; i<phoneNumber.length(); i++){
        if(!isdigit(phoneNumber[i])){
            return false;
        }
                            }
    return true;

}

bool isValidEmail(string& email){
    int pos1= email.find('@');
    int pos2= email.find('.');
    return(pos1!=string::npos && pos2!=string::npos && pos1<pos2 && pos2<email.length()-1);
}
bool isValidPassword(string& password){
    if(password.length()<8){
        cout<<"Password must be at least 8 characters long.";
        return false;
    }
    bool hasDigit=false, hasUpper=false, hasLower=false;
    for(int i=0; i<password.length(); i++){
        if(isdigit(password[i])){
            hasDigit=true;
        }
        if(isupper(password[i])){
            hasUpper=true;
        }
        if(islower(password[i])){
            hasLower=true;
        }
    }
    if(!hasDigit){
        cout<<"Password must contain at least one digit.";
        return false;
    }
    if(!hasUpper){
        cout<<"Password must contain at least one uppercase letter.";
        return false;
    }
    if(!hasLower){
        cout<<"Password must contain at least one lowercase letter.";
        return false;
    }
    return true;

}

User createAccount(){
    User newUser;
    char isAdminChar;
    cout << "Are you creating an Administrator account? (Y/N): ";
    cin >> isAdminChar;
    cin.ignore();

    if (isAdminChar == 'Y' || isAdminChar == 'y') {
        newUser.isAdmin = true;
    } else {
        newUser.isAdmin = false;
    }
    cout<<"Enter your first name:";
    getline(cin, newUser.firstName);

    cout<<"Enter your last name:";
    getline(cin, newUser.lastName);

    cout<<"Enter your email address:";
    getline(cin, newUser.email);
    while(!isValidEmail(newUser.email)){
        cout<<"Invalid email address. Please enter a valid email address.";
        getline(cin, newUser.email);
    }

    cout<<"Enter your password:";
    getline(cin, newUser.password);
    while(!isValidPassword(newUser.password)){
        cout<<"Invalid password. Please enter a valid password containing at least 8 characters, one uppercase letter, one lowercase letter, and one digit.";
        getline(cin, newUser.password);
    }

    cout<<"Enter your phone number:";
    getline(cin, newUser.phoneNumber);while(!isValidNumber(newUser.phoneNumber)){
        cout<<"Invalid phone number. Please enter a valid phone number containing 8 digits.";
        getline(cin, newUser.phoneNumber);
    }
    return newUser;

}
string getNextUserID() {
    ifstream file("C:\\Games\\Car Rental Program\\user.csv");

    string line;
    int count = 0;
    bool isFirstLine = true;
    while (getline(file, line)) {
        if (isFirstLine) {
            isFirstLine = false;
            continue;
        }
        ++count;
    }
    file.close();

    if (count == 0) {
        return "1";
    }

    return to_string(count + 1);
}

void writeUserData(const User& newUser) {
    ofstream File("C:\\Games\\Car Rental Program\\user.csv", ios::app);

    if (File.is_open()) {
        string newPhoneNumber = newUser.phoneNumber.substr(0, 2) + "-" + newUser.phoneNumber.substr(2);
        string hashedPassword = sha256(newUser.password);

        string role = newUser.isAdmin ? "admin" : "user";
        string Data =newUser.userID + "," + newUser.firstName + "," +
                     newUser.lastName + "," + newUser.email + "," +
                     hashedPassword + "," + newPhoneNumber + "," + role +"\n";

        File << Data;
        File.close();
        cout << "User data saved to user.csv" << endl;
    } else {
        cout << "Error: Unable to open CSV file for writing." << endl;
    }
}
bool authenticateExistingUser(const string& userID, const string& password, bool& isAdmin) {
    ifstream file("C:\\Games\\Car Rental Program\\user.csv");
    if (!file.is_open()) {
        cout << "Error: Unable to open CSV file for reading." << endl;
        return false;
    }

    string line;
    bool isFirstLine = true;
    while (getline(file, line)) {
        if (isFirstLine) {
            isFirstLine = false;
            continue; // Skip the first line
        }

        stringstream ss(line);
        string currentUserId, firstName, lastName, email, hashedPassword, phoneNumber, role;
        getline(ss, currentUserId, ',');
        getline(ss, firstName, ',');
        getline(ss, lastName, ',');
        getline(ss, email, ',');
        getline(ss, hashedPassword, ',');
        getline(ss, phoneNumber, ',');
        getline(ss, role);

        if (currentUserId == userID) {
            string hashedInputPassword = sha256(password);
            if (hashedPassword == hashedInputPassword) {
                isAdmin = (role == "admin");
                file.close();
                return true;
            }
        }
    }

    file.close();
    return false;
}




int main() {
    int choice;
    cout << "\t***********************************************************" << endl;
    cout << "\t        Welcome to Breaking Drive - Main Menu" << endl;
    cout << "\t***********************************************************" << endl;
    cout << "1 - Create account" << endl;
    cout << "2 - Login" << endl;
    cout << "3 - Exit" << endl;
    cout << "Please input your choice: ";
    cin >> choice;
    cin.ignore();

    while (true) {
        switch(choice) {
            case 1: {
                User newUser = createAccount();
                newUser.userID = getNextUserID();
                writeUserData(newUser);
                cout << "Congratulations! Your account has been created successfully. Your user ID is: " << newUser.userID << endl;


                cout << "Please login with your newly created account." << endl;

                bool isAdmin = false;
                string userID, password;
                do {
                    cout << "Enter your user ID: ";
                    getline(cin, userID);
                    cout << "Enter your password: ";
                    getline(cin, password);
                    if (!authenticateExistingUser(userID, password, isAdmin)) {
                        cout << "Login failed. Incorrect user ID or password." << endl;
                    }
                } while (!authenticateExistingUser(userID, password, isAdmin));

                if (!isAdmin) {
                    cout << "Login successful! Welcome, " << newUser.firstName << "!" << endl;
                     executeClientFunctionalities();
                } else {
                    cout << "Login successful! Welcome back, Admin " << newUser.userID << "!" << endl;
                    executeAdminFunctionalities();
                }
                return 0;
            }

            case 2: {
                bool isAdmin = false;
                string userID, password;
                do {
                    cout << "Enter your user ID: ";
                    getline(cin, userID);
                    cout << "Enter your password: ";
                    getline(cin, password);
                    if (!authenticateExistingUser(userID, password, isAdmin)) {
                        cout << "Login failed. Incorrect user ID or password." << endl;
                    }
                } while (!authenticateExistingUser(userID, password, isAdmin));

                if (!isAdmin) {
                    cout << "Login successful! Welcome back, User " << userID << "!" << endl;
                    executeClientFunctionalities();
                } else {
                    cout << "Login successful! Welcome back, Admin " << userID << "!" << endl;
                    executeAdminFunctionalities();
                }
                return 0;
            }

            case 3:
                cout << "Exiting the application."
                        "Thanks for visiting us!" << endl;
                return 0;

            default:
                cout << "Invalid input. Please enter a valid option (1-3): ";
                cin >> choice;
                cin.ignore();
        }
    }

    return 0;
}