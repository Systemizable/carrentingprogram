#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "Client.h"
#include <cctype>
using namespace std;

bool isAllDigits(const string& str) {
    for (char c : str) {
        if (!isdigit(c)) {
            return false;
        }
    }
    return true;
}

void writeCarsToFile(Car* cars, int count) {
    ofstream file("C:\\Games\\Car Rental Program\\cars.csv");
    if (!file.is_open()) {
        cout << "Error: Could not open file for writing!" << endl;
        return;
    }
    for (int i = 0; i < count; ++i) {
        file << cars[i].plateNumber << "," << cars[i].brand << "," << to_string(cars[i].year) << ","
             << cars[i].model << "," << cars[i].pricePerDay << "," << cars[i].color << "\n";
    }
    file.close();
}
int readCarsFromFile(Car** cars, int* count, int* capacity) {
    ifstream file("C:\\Games\\Car Rental Program\\cars.csv");
    if (!file.is_open()) {
        cout << "Error: Could not open file for reading!" << endl;
        return 0;
    }

    string line;
    while (getline(file, line)) {
        if (*count == *capacity) {
            *capacity *= 2;
            Car* temp = new Car[*capacity];
            for (int i = 0; i < *count; ++i) {
                temp[i] = (*cars)[i];
            }
            delete[] *cars;
            *cars = temp;
        }

        istringstream iss(line);
        getline(iss, (*cars)[*count].plateNumber, ',');
        getline(iss, (*cars)[*count].brand, ',');
        string yearStr;
        getline(iss, yearStr, ',');
        if (isAllDigits(yearStr) && !yearStr.empty()) {
            (*cars)[*count].year = stoi(yearStr);
        } else {
            cerr << "Invalid year format: " << yearStr << endl;
            continue; // Skip this car or handle error
        }
        getline(iss, (*cars)[*count].model, ',');
        iss >> (*cars)[*count].pricePerDay;
        if (iss.peek() == ',') iss.ignore();
        getline(iss, (*cars)[*count].color);
        (*count)++;
    }

    file.close();
    return *count;
}
void resizeCarsArray(Car** cars, int oldSize, int newSize) {
    Car* temp = new Car[newSize];
    for (int i = 0; i < oldSize; ++i) {
        temp[i] = (*cars)[i];
    }
    delete[] *cars;
    *cars = temp;
}
void addCar(Car** cars, int* count, int* capacity) {
    if (*count >= *capacity) {
        int newCapacity = *capacity * 2;
        resizeCarsArray(cars, *count, newCapacity);
        *capacity = newCapacity;
    }

    if (*count < *capacity) {
        cout << "Enter details for the new car:" << endl;
        cout << "Plate Number: ";
        cin >> (*cars)[*count].plateNumber;
        cout << "Brand: ";
        cin >> (*cars)[*count].brand;
        cout << "Year: ";
        cin >> (*cars)[*count].year;
        cout << "Model: ";
        cin >> (*cars)[*count].model;
        cout << "Price per Day: ";
        cin >> (*cars)[*count].pricePerDay;
        cout << "Color: ";
        cin >> (*cars)[*count].color;

        (*count)++;
    } else {
        cout << "No space to add a new car. Please resize the array." << endl;
    }
}

void removeCar(Car** cars, int* count) {
	if (*count > 0) {
		int index;
		cout << "Enter index of the car to remove (0 to " << *count - 1 << "): ";
		cin >> index;
		if (index >= 0 && index < *count) {
			for (int i = index; i < *count - 1; ++i) {
				(*cars)[i] = (*cars)[i + 1];
			}
			(*count)--;
		}
		else {
			cout << "Invalid index." << endl;
		}
	}
	else {
		cout << "No cars to remove." << endl;
	}
}
void modifyCar(Car* cars, int count) {
    if (count > 0) {
        int index;
        cout << "Enter index of the car to modify (0 to " << count - 1 << "): ";
        cin >> index;
        if (index >= 0 && index < count) {
            string plateNumber, brand, yearStr, model, color;
            double pricePerDay;

            cout << "Enter new plate number: ";
            cin >> plateNumber;
            cout << "Enter new brand: ";
            cin >> brand;
            cout << "Enter new year: ";
            cin >> yearStr;
            if (isAllDigits(yearStr) && !yearStr.empty()) {
                cars[index].year = stoi(yearStr);
            } else {
                cerr << "Invalid year format: " << yearStr << endl;
                return; // Exit or handle error
            }
            cout << "Enter new model: ";
            cin >> model;
            cout << "Enter new price per day: ";
            cin >> pricePerDay;
            cout << "Enter new color: ";
            cin >> color;

            cars[index].plateNumber = plateNumber;
            cars[index].brand = brand;
            cars[index].year = stoi(yearStr);
            cars[index].model = model;
            cars[index].pricePerDay = pricePerDay;
            cars[index].color = color;
        }
        else {
            cout << "Invalid index." << endl;
        }
    }
    else {
        cout << "No cars to modify." << endl;
    }
}
void executeAdminFunctionalities() {
    cout << "Admin functionalities started." << endl;
    cout << "Welcome Master"<<endl;
    int capacity = 2;
    int count = 0;
    Car* cars = new Car[capacity];

    readCarsFromFile(&cars, &count, &capacity);

    int choice;
    do {
        cout << "\nCar Rental Program Menu" << endl;
        cout << "1. Add Car" << endl;
        cout << "2. Remove Car" << endl;
        cout << "3. Modify Car" << endl;
        cout << "4. Exit" << endl;
        cout << "Enter your choice: ";
        cin >> choice;

        cout << "Choice entered: " << choice << endl; // Debug statement

        switch (choice) {
            case 1:
                cout << "Adding car..." << endl; // Debug statement
                addCar(&cars, &count, &capacity);
                writeCarsToFile(cars, count); // Write data to file after adding
                break;
            case 2:
                cout << "Removing car..." << endl; // Debug statement
                removeCar(&cars, &count);
                writeCarsToFile(cars, count); // Write data to file after removal
                break;
            case 3:
                cout << "Modifying car..." << endl; // Debug statement
                modifyCar(cars, count);
                writeCarsToFile(cars, count); // Write data to file after modification
                break;
            case 4:
                cout << "Exiting program..." << endl;
                break;
            default:
                cout << "Invalid choice." << endl;
        }
        cout << "After switch statement." << endl; // Debug statement
    } while (choice != 4);

    delete[] cars;

    cout << "Admin functionalities completed." << endl; // Debug statement
}