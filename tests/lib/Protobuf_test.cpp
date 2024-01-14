#include <fstream>
#include <iostream>
#include <string>

#include "../../src/proto/addressbook.pb.h"

// This function fills in a Person message based on user input.
void PromptForAddress(tutorial::Person* person) {
  std::cout << "Enter person ID number: ";
  int id;
  std::cin >> id;
  person->set_id(id);
  std::cin.ignore(256, '\n');

  std::cout << "Enter name: ";
  getline(std::cin, *person->mutable_name());

  std::cout << "Enter email address (blank for none): ";
  std::string email;
  getline(std::cin, email);
  if (!email.empty()) {
    person->set_email(email);
  }

  while (true) {
    std::cout << "Enter a phone number (or leave blank to finish): ";
    std::string number;
    getline(std::cin, number);
    if (number.empty()) {
      break;
    }

    tutorial::Person::PhoneNumber* phone_number = person->add_phones();
    phone_number->set_number(number);

    std::cout << "Is this a mobile, home, or work phone? ";
    std::string type;
    getline(std::cin, type);
    if (type == "mobile") {
      phone_number->set_type(tutorial::Person::PHONE_TYPE_MOBILE);
    } else if (type == "home") {
      phone_number->set_type(tutorial::Person::PHONE_TYPE_HOME);
    } else if (type == "work") {
      phone_number->set_type(tutorial::Person::PHONE_TYPE_WORK);
    } else {
      std::cout << "Unknown phone type.  Using default." << std::endl;
    }
  }
}

// Main function:  Reads the entire address book from a file,
//   adds one person based on user input, then writes it back out to the same
//   file.
int main() {
  // Verify that the version of the library that we linked against is
  // compatible with the version of the headers we compiled against.
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  try {
    const char* infile = "../assets/proto/addressbook.bin";
    const char* outfile = "../assets/proto/addressbook.bin";

    tutorial::AddressBook address_book;
    {
      //   // Read the existing address book.
      std::fstream input(infile, std::ios::in | std::ios::binary);

      // input address_book.ParseFromString();

      if (!input) {
        std::cout << infile << ": File not found.  Creating a new file." << std::endl;
      } else if (!address_book.ParseFromIstream(&input)) {
        std::cerr << "Failed to parse address book." << std::endl;
        return -1;
      }
    }

    // std::cout << address_book.SerializeAsString() << std::endl;

    // Add an address.
    PromptForAddress(address_book.add_people());

    // std::cout << address_book.SerializeAsString() << std::endl;

    {
      // Write the new address book back to disk.
      std::fstream output(outfile, std::ios::out | std::ios::trunc | std::ios::binary);
      // output << address_book.SerializeAsString();
      // output.close();

      // output.exceptions(std::ifstream::failbit | std::ifstream::badbit);
      if (!address_book.SerializeToOstream(&output)) {
        std::cerr << "Failed to write address book." << std::endl;
        return -1;
      }
    }

    // Optional:  Delete all global objects allocated by libprotobuf.
    google::protobuf::ShutdownProtobufLibrary();
  } catch (std::system_error& e) {
    std::cerr << e.code().message() << std::endl;
    return -1;
  } catch (std::exception& e) {
    std::cerr << "Unhandled exception: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}