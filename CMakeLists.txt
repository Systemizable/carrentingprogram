cmake_minimum_required(VERSION 3.27)
project(CarRentalProgram)

set(CMAKE_CXX_STANDARD 20)

# Set the path to the PDFlib library
set(PDFLIB_DIR "C:/Users/Joseph/Documents/University/CarRentalProgram/lib/pdflibrary/pdflib")

# Include the PDFlib headers
include_directories(${PDFLIB_DIR})

# Add the source files for your project
add_executable(CarRentalProgram
        Authentication.cpp
        sha256.cpp
        sha256.h
        Client.cpp
        Administration.cpp
)

target_link_libraries(CarRentalProgram "C:/Users/Joseph/Documents/University/CarRentalProgram/lib/pdflibrary/pdflib/pdflib.dll")