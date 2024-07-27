#ifndef CLIENT_H
#define CLIENT_H
#include "pdflib.h"
#include <string>
using namespace std;

struct Date {
    std::string startDate;
    std::string endDate;
};

struct Receipt {
    string userID;
    string plateNumber;
    string startDate;
    string endDate;
    double pricePerDay;
};
struct RentalInfo {
    string userID;
    string plateNumber;
    string startDate;
    string endDate;
    double pricePerDay;
};
struct Car {
    string plateNumber;
    string brand;
    int year;
    string model;
    double pricePerDay;
    string color;
    bool rented = false; // Default value for new members to maintain compatibility
    string startDate;
    string endDate;
};
struct Rental {
    string idNumber;
    string plateNumber;
    string userID;
    double pricePerDay;
    Date d;
};
struct User {
    std::string userID;
    std::string firstName;
    std::string lastName;
    std::string email;
    std::string password;
    std::string phoneNumber;
    bool isAdmin;
};
struct Client {
    int userID;
    string firstName;
    string lastName;
    string password;
    string phone;
    string email;
    int nbReservation;
    Car* c; // Pointer to a car structure
};

Car* readCarsFromCSV(const std::string& filename, int& numCars);
Rental* readRentalsFromCSV(const std::string& filename, int& numRentals);
void displayAvailableCars(const Car* cars, int numCars);
bool checkAvailability(const Rental* rentals, int numRentals, const std::string& plateNumber, const std::string& rentedDate);
void executeClientFunctionalities();
void executeAdminFunctionalities();
bool isAllDigits(const string& s);

#endif