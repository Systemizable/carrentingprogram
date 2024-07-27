
#include "Client.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "pdflib.h"


using namespace std;
void sortByPricePerDay(Rental* rentals, int numRentals) {
    for (int i = 0; i < numRentals - 1; ++i) {
        for (int j = 0; j < numRentals - i - 1; ++j) {
            if (rentals[j].pricePerDay > rentals[j + 1].pricePerDay) {
                swap(rentals[j], rentals[j + 1]);
            }
        }
    }
}
const int MAX_RENTALS = 100;
RentalInfo rentalInfos[MAX_RENTALS];
int numRentals = 0;

void generateRentalReceiptPDF(const Rental* rentals, int numRentals) {
    // Create a new PDF document
    PDF *pdf = PDF_new();

    // Specify the file path for the PDF document
    const char* filePath = "C:\\Games\\Car Rental Program\\rental_receipt.pdf";

    // Begin the PDF document with the specified file path
    if (PDF_begin_document(pdf, filePath, 0, "") == -1) {
        cerr << "Error beginning document: " << PDF_get_errmsg(pdf) << endl;
        PDF_delete(pdf);
        return;
    }

    // Begin a new page in the PDF document
    PDF_begin_page_ext(pdf, 595, 842, "");  // A4 size

    // Load the font for text rendering
    int font = PDF_load_font(pdf, "Arial", 0, "host", "");
    if (font == -1) {
        cerr << "Error loading font: " << PDF_get_errmsg(pdf) << endl;
        PDF_delete(pdf);
        return;
    }

    // Set the font and font size for the text
    PDF_setfont(pdf, font, 12.0);
    PDF_set_text_pos(pdf, 50, 800);

    // Add the rental information to the PDF
    for (int i = 0; i < numRentals; ++i) {
        PDF_show(pdf, "Rental Receipt");
        PDF_continue_text(pdf, "\n");
        PDF_continue_text(pdf, ("User ID: " + rentals[i].userID).c_str());
        PDF_continue_text(pdf, "\n");
        PDF_continue_text(pdf, ("Plate Number: " + rentals[i].plateNumber).c_str());
        PDF_continue_text(pdf, "\n");
        PDF_continue_text(pdf, ("Start Date: " + rentals[i].d.startDate).c_str());
        PDF_continue_text(pdf, "\n");
        PDF_continue_text(pdf, ("End Date: " + rentals[i].d.endDate).c_str());
        PDF_continue_text(pdf, "\n");
        PDF_continue_text(pdf, ("Price Per Day: $" + to_string(rentals[i].pricePerDay)).c_str());
        PDF_continue_text(pdf, "\n\n");
    }

    // End the current page in the PDF document
    PDF_end_page_ext(pdf, "");

    // End the PDF document
    PDF_end_document(pdf, "");

    // Delete the PDF object to free resources
    PDF_delete(pdf);

    // Print a success message
    cout << "Rental receipt PDF generated successfully." << endl;
}
string getUserID(const string& email) {
    ifstream file("C:\\Games\\Car Rental Program\\user.csv");
    string line;
    string id, userEmail;

    if (!file.is_open()) {
        cerr << "Error: Unable to open user.csv for reading." << endl;
        return "";
    }

    while (getline(file, line)) {
        stringstream ss(line);
        // Assuming ID is the first column, and email is the fourth column
        getline(ss, id, ','); // Assuming ID is the first column
        // Skip three columns to reach the email
        for (int i = 0; i < 3; ++i) {
            getline(ss, userEmail, ','); // Skipping columns until reaching email
        }

        if (userEmail == email) {
            file.close();
            return id;
        }
    }

    file.close();
    cerr << "User not found." << endl;
    return "";
}

double getPricePerDay(const string& plateNumber) {
    ifstream file("C:\\Games\\Car Rental Program\\cars.csv");
    string line, carPlate, priceStr;

    if (!file.is_open()) {
        cerr << "Error: Unable to open cars.csv for reading." << endl;
        return 0.0;
    }

    while (getline(file, line)) {
        stringstream ss(line);
        // Assuming plate number is the first column
        getline(ss, carPlate, ','); // Assuming plate number is the first column
        // Skip three columns to reach the price per day
        for (int i = 0; i < 3; ++i) {
            string temp;
            getline(ss, temp, ','); // Skipping columns until reaching the price
        }
        getline(ss, priceStr, ','); // Assuming price is the last column

        if (carPlate == plateNumber) {
            file.close();
            try {
                return stod(priceStr);
            } catch (const invalid_argument& e) {
                cerr << "Invalid price format in CSV for plate " << carPlate << ": " << priceStr << endl;
                return 0.0;
            }
        }
    }

    file.close();
    cerr << "Car not found." << endl;
    return 0.0;
}
Car* readCarsFromCSV(const string& filename, int& numCars) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Unable to open " << filename << " for reading." << endl;
        numCars = 0;
        return nullptr;
    }

    Car* cars = new Car[10];
    string line;
    int index = 0;
    int capacity = 10;

    while (getline(file, line)) {
        if (line.empty()) continue;

        if (index >= capacity) {

            int newCapacity = capacity * 2;
            Car* temp = new Car[newCapacity];
            for (int i = 0; i < capacity; ++i) {
                temp[i] = cars[i];
            }
            delete[] cars;
            cars = temp;
            capacity = newCapacity;
        }

        stringstream ss(line);
        string yearStr, priceStr;

        getline(ss, cars[index].plateNumber, ',');
        getline(ss, cars[index].brand, ',');
        getline(ss, yearStr, ',');
        getline(ss, cars[index].model, ',');
        getline(ss, priceStr, ',');
        getline(ss, cars[index].color);

        if (isAllDigits(yearStr)) {
            cars[index].year = stoi(yearStr);
        } else {
            cout << "Invalid year format in CSV for plate " << cars[index].plateNumber << ": " << yearStr << endl;
            cars[index].year = 0;
        }

        try {
            cars[index].pricePerDay = stod(priceStr);
        } catch (const invalid_argument& e) {
            cout << "Invalid price format in CSV for plate " << cars[index].plateNumber << ": " << priceStr << endl;
            cars[index].pricePerDay = 0.0;
        }

        index++;
    }

    file.close();
    numCars = index;
    return cars;
}

Rental* readRentalsFromCSV(const string& filename, int& numRentals) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Unable to open " << filename << " for reading." << endl;
        numRentals = 0;
        return nullptr;
    }

    string line;
    getline(file, line);

    Rental* rentals = new Rental[10];
    int capacity = 10;
    numRentals = 0;

    while (getline(file, line)) {
        if (line.empty()) continue;


        if (numRentals == capacity) {
            capacity *= 2;
            Rental* newRentals = new Rental[capacity];
            for (int i = 0; i < numRentals; ++i) {
                newRentals[i] = rentals[i];
            }
            delete[] rentals;
            rentals = newRentals;
        }

        stringstream ss(line);
        getline(ss, rentals[numRentals].idNumber, ',');
        getline(ss, rentals[numRentals].plateNumber, ',');
        getline(ss, rentals[numRentals].d.startDate, ',');
        getline(ss, rentals[numRentals].d.endDate);

        numRentals++;
    }

    file.close();
    return rentals;
}
void writeRentalCarsToCSV(Rental* rentals, int numRentals) {
    ofstream file("C:\\Games\\Car Rental Program\\rentalcars.csv");
    if (!file.is_open()) {
        cout << "Error: Unable to open rentalcars.csv for writing." << endl;
        return;
    }

    file << "User ID,Plate Number,Start Date,End Date" << endl;

    for (int i = 0; i < numRentals; ++i) {
        file << rentals[i].idNumber << "," << rentals[i].plateNumber << "," << rentals[i].d.startDate << "," << rentals[i].d.endDate << endl;
    }

    file.close();
    cout << "Rented cars data written to rentalcars.csv successfully." << endl;
}
void displayAvailableCars(const Car* cars, int numCars) {
    cout << "Available cars:" << endl;
    for (int i = 0; i < numCars; ++i) {
        cout << "Plate Number: " << cars[i].plateNumber << endl;
        cout << "Brand: " << cars[i].brand << endl;
        cout << "Year: " << cars[i].year << endl;
        cout << "Model: " << cars[i].model << endl;
        cout << "Price per Day: " << cars[i].pricePerDay << endl;
        cout << "Color: " << cars[i].color << endl;
        cout << endl;
    }
}


bool checkAvailability(const Rental* rentals, int numRentals, const string& plateNumber,     const string& rentedDate) {
    for (int i = 0; i < numRentals; ++i) {
        if (rentals[i].plateNumber == plateNumber && rentals[i].d.startDate <= rentedDate &&  rentals[i].d.endDate >= rentedDate) {
            return false;
        }
    }
    return true;

}

void rentCar(Rental*& rentals, int& numRentals, const string& userID, const string& plateNumber, const string& startDate, const string& endDate, double pricePerDay) {
    // Check if numRentals is initialized properly
    if (numRentals < 0) {
        cout << "Error: numRentals is not initialized properly." << endl;
        return;
    }

    // Create a temporary array to hold the rentals with increased size
    Rental* temp = new Rental[numRentals + 1];

    // Copy existing rentals to the temporary array
    for (int i = 0; i < numRentals; ++i) {
        temp[i] = rentals[i];
    }

    // Add the new rental to the end of the temporary array
    temp[numRentals].idNumber = to_string(numRentals + 1);
    temp[numRentals].plateNumber = plateNumber;
    temp[numRentals].d.startDate = startDate;
    temp[numRentals].d.endDate = endDate;
    temp[numRentals].userID = userID;
    temp[numRentals].pricePerDay = pricePerDay;

    // Increment the number of rentals
    ++numRentals;

    // Delete the old rentals array
    delete[] rentals;

    // Assign the temporary array to the rentals pointer
    rentals = temp;

    // Sort the rentals by price per day in ascending order
    sortByPricePerDay(rentals, numRentals);

    // Write the updated rentals to the CSV file
    writeRentalCarsToCSV(rentals, numRentals);

    // Generate the rental receipt PDF
    generateRentalReceiptPDF(rentals, numRentals);

    // Print a success message
    cout << "Car rented successfully!" << endl;
}

void cancelRental(Rental*& rentals, int& numRentals, Car* cars, int numCars) {
    cout << "Rented cars:" << endl;
    for (int i = 0; i < numRentals; ++i) {
        cout << "Reservation ID: " << rentals[i].idNumber << endl;
        cout << "Plate Number: " << rentals[i].plateNumber << endl;
        cout << "Start Date: " << rentals[i].d.startDate << endl;
        cout << "End Date: " << rentals[i].d.endDate << endl;
        cout << endl;
    }

    string plateNumber, startDate, endDate;
    cout << "Enter the plate number of the car reservation to cancel: ";
    getline(cin, plateNumber);
    cout << "Enter the start date (YYYY-MM-DD): ";
    getline(cin, startDate);
    cout << "Enter the end date (YYYY-MM-DD): ";
    getline(cin, endDate);

    bool found = false;
    for (int i = 0; i < numRentals; ++i) {
        if (rentals[i].plateNumber == plateNumber && rentals[i].d.startDate == startDate && rentals[i].d.endDate == endDate) {
            for (int j = i; j < numRentals - 1; ++j) {
                rentals[j] = rentals[j + 1];
            }
            --numRentals;
            found = true;
            break;
        }
    }

    if (found) {
        writeRentalCarsToCSV(rentals, numRentals);
        generateRentalReceiptPDF(rentals, numRentals);

        cout << "Reservation canceled successfully." << endl;
    } else {
        cout << "No reservation found for the entered details." << endl;
    }
}
void modifyRentalDates(Rental* rentals, int numRentals, Car* cars, int numCars) {
    cout << "Currently rented cars:" << endl;
    for (int i = 0; i < numRentals; ++i) {
        cout << "Rental ID: " << rentals[i].idNumber << " - Plate Number: " << rentals[i].plateNumber
             << " (From " << rentals[i].d.startDate << " to " << rentals[i].d.endDate << ")" << endl;
    }

    string rentalId;
    cout << "Enter the Rental ID of the car whose dates you want to modify: ";
    getline(cin, rentalId);

    bool found = false;
    for (int i = 0; i < numRentals; ++i) {
        if (rentals[i].idNumber == rentalId) {
            found = true;
            string newStartDate, newEndDate;
            cout << "Enter the new start date (YYYY-MM-DD): ";
            getline(cin, newStartDate);
            cout << "Enter the new end date (YYYY-MM-DD): ";
            getline(cin, newEndDate);

            rentals[i].d.startDate = newStartDate;
            rentals[i].d.endDate = newEndDate;

            for (int j = 0; j < numCars; ++j) {
                if (cars[j].plateNumber == rentals[i].plateNumber) {
                    cars[j].startDate = newStartDate;
                    cars[j].endDate = newEndDate;
                    break;
                }
            }

            cout << "Rental dates modified successfully for Rental ID: " << rentalId << endl;
            writeRentalCarsToCSV(rentals, numRentals);
            generateRentalReceiptPDF(rentals, numRentals);

            break;
        }
    }

    if (!found) {
        cout << "No rental found for the entered Rental ID." << endl;
    }
}
void executeClientFunctionalities() {
    int numCars = 0;
    Car* cars = readCarsFromCSV("C:\\Games\\Car Rental Program\\cars.csv", numCars);
    if (!cars) {
        cout << "Error: Unable to read cars data." << endl;
        return;
    }

    int numRentals = 0;
    Rental* rentals = readRentalsFromCSV("C:\\Games\\Car Rental Program\\rentalcars.csv", numRentals);
    if (!rentals) {
        cout << "No rentals data available or failed to read." << endl;
        rentals = new Rental[10];
        numRentals = 0;
    }

    string choice, userID, plateNumber, startDate, endDate;
    double pricePerDay;
    do {
        cout << "\nMenu:" << endl;
        cout << "1. View available cars" << endl;
        cout << "2. Check car availability" << endl;
        cout << "3. Rent a car" << endl;
        cout << "4. Cancel rental reservation" << endl;
        cout << "5. Modify rented car dates" << endl;
        cout << "6. Exit" << endl;
        cout << "Enter your choice: ";
        getline(cin, choice);

        if (choice == "1") {
            displayAvailableCars(cars, numCars);
        } else if (choice == "2") {
            cout << "Enter the plate number of the car: ";
            getline(cin, plateNumber);
            cout << "Enter current date (YYYY-MM-DD): ";
            getline(cin, startDate);
            if (checkAvailability(rentals, numRentals, plateNumber, startDate)) {
                cout << "The car with plate number " << plateNumber << " is available." << endl;
            } else {
                cout << "The car with plate number " << plateNumber << " is not available." << endl;
            }
        } else if (choice == "3") {
            cout << "Enter user email (to retrieve user ID): ";
            string email;
            getline(cin, email);
            userID = getUserID(email); // Assuming getUserID is defined
            cout << "Enter the plate number of the car: ";
            getline(cin, plateNumber);
            cout << "Enter the start date (YYYY-MM-DD): ";
            getline(cin, startDate);
            cout << "Enter the end date (YYYY-MM-DD): ";
            getline(cin, endDate);
            pricePerDay = getPricePerDay(plateNumber); // Assuming getPricePerDay is defined
            if (!userID.empty() && pricePerDay > 0) {
                rentCar(rentals, numRentals, userID, plateNumber, startDate, endDate, pricePerDay);
            } else {
                cout << "Failed to retrieve user ID or price per day." << endl;
            }
        } else if (choice == "4") {
            cancelRental(rentals, numRentals, cars, numCars);
        } else if (choice == "5") {
            modifyRentalDates(rentals, numRentals, cars, numCars);
        } else if (choice == "6") {
            cout << "Exiting the application. Thanks for visiting us!" << endl;
            break;
        } else {
            cout << "Invalid choice. Please enter a number between 1 and 6." << endl;
        }
        sortByPricePerDay(rentals, numRentals);
    } while (true);

    delete[] cars;
    delete[] rentals;
}