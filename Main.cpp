#include <iostream> //allows input cin and output cout
#include <string>   //uses string dta type
#include <fstream> //file handling
#include <ctime>   //sir hum system ka date and time acess kr skty
#include <iomanip> //format output
#include <sstream> //string ko split krta hai..like mera jo file mai data store hai wo aik line mai as string hai..ye uss ko split krta hai
#include <limits> // ap ki data type kitni value store kr skti hai ..max or min kya hai
#include <vector>   // we can use vectors
#include <map>      // For std::map
#include <algorithm> //in this code we could do sort, max, min
#include <cctype>   // deals with charachter data type for toupper/tolower
#include <functional> // For std::function

using namespace std;

// Ckasses Defined
class Patient {
public:
    int patientID;
    string name;
    int age;
    char gender;
    string address;
    string contactNumber;
    string disease;
    string doctorAssigned; // Consider changing to doctorID for robustness
    string admissionDate;
    bool isAdmitted;
    Patient* next; 
};

class Doctor {
public:
    int doctorID;    
    string name;
    string specialization;
    string contactNumber;
    string email;
    int experienceYears;
    bool isAvailable; 
    map<string, vector<string>> bookedSlots; // Date (YYYY-MM-DD) Time-> (HH:MM)
    int maxAppointmentsPerDay; // Max appointments allowed per day for this doctor
    Doctor* next;   // For linked list traversal
    Doctor* left;   // for bst
    Doctor* right;
};

class Resource {
public:
    int resourceID;
    string name;
    string type;
    bool isAvailable;
    Resource* next;
};

class Appointment {
public:
    int appointmentID;
    int patientID;
    string patientName;
    int doctorID;
    string doctorName;
    string date; //YYYY-MM-DD
    string time; // HH:MM
    string reason;
    string status; // e.g., "Scheduled", "Completed", "Cancelled"
    Appointment* next;
};

class Emergency {
public:
    int caseID;
    string patientName;
    string condition;
    string timeLogged; // time when emergency was logged
    string status;     //pending dispatched resolved
    Emergency* next;
};

string getCurrentDate();
string mapToString(const map<string, vector<string>>& m); //function that take map jahan key is string and value is list of string
map<string, vector<string>> stringToMap(const string& s);
int getValidatedInt(const string& prompt);
string getValidatedText(const string& prompt);
string toLower(const string& str);
string getCurrentTime(); // Forward declaration for getCurrentTime
Doctor* searchDoctorByID(Doctor* node, int id); // Forward declaration for searchDoctorByID
Appointment* searchAppointmentByID(int id); // Forward declaration for searchAppointmentByID
// Forward declaration for updateAppointmentStatus (used in cascading deletes)
void updateAppointmentStatus(int appointmentID, const string& newStatus);
void deleteAppointmentNode(int appointmentID); // Forward declaration for helper delete function


//.................................GLOBAL POINTERS FOR LINKED LISTS/BST..............................
Patient* patientHead = nullptr; // Head of the Patient linked list
Doctor* docRoot = nullptr;     // Root of the Doctor BST
Resource* resourceHead = nullptr; // Head of the Resource linked list
Appointment* appointmentHead = nullptr; // Head of the Appointment linked list
Emergency* emergencyHead = nullptr; // Head of the Emergency linked list

//...................................HELPER FUNCTIONS................................................
string toLower(const string& str) {
    string lowerStr = str;
    transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

string getCurrentDate() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    stringstream ss;
    ss << (1900 + ltm->tm_year) << '-'
       << setfill('0') << setw(2) << (1 + ltm->tm_mon) << '-'
       << setfill('0') << setw(2) << ltm->tm_mday;
    return ss.str();
}

string getCurrentTime() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    stringstream ss;
    ss << setfill('0') << setw(2) << ltm->tm_hour << ':'
       << setfill('0') << setw(2) << ltm->tm_min;
    return ss.str();
}

// Helper to convert map to string for saving
string mapToString(const map<string, vector<string>>& m) {
    stringstream ss;
    for (const auto& pair : m) {
        ss << pair.first << ":";
        for (const string& s : pair.second) {
            ss << s << ",";
        }
        ss << ";";
    }
    return ss.str();
}

// Helper to convert string back to map for loading
map<string, vector<string>> stringToMap(const string& s) {
    map<string, vector<string>> m;
    stringstream ss(s);
    string segment;
    while (getline(ss, segment, ';')) {
        if (segment.empty()) continue;
        size_t colonPos = segment.find(':');
        if (colonPos == string::npos) continue;

        string date = segment.substr(0, colonPos);
        string timesStr = segment.substr(colonPos + 1);
        stringstream ts(timesStr);
        string time;
        while (getline(ts, time, ',')) {
            if (!time.empty()) {
                m[date].push_back(time);
            }
        }
    }
    return m;
}

// Validated input for integers
int getValidatedInt(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        cin >> value;
        if (cin.fail()) {
            cout << "Invalid input. Please enter a number.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear buffer
            return value;
        }
    }
}

// Validated input for text (allows spaces)
string getValidatedText(const string& prompt) {
    string value;
    cout << prompt;
    getline(cin >> ws, value); // Reads the whole line, including spaces
    return value;
}

// Converts "HH:MM" string to total minutes from midnight
int timeToMinutes(const string& timeStr) {
    // Basic validation for HH:MM format
    if (timeStr.length() != 5 || timeStr[2] != ':') {
        return -1; // Indicate invalid format
    }
    try {
        int hour = stoi(timeStr.substr(0, 2));
        int minute = stoi(timeStr.substr(3, 2));
        if (hour >= 0 && hour < 24 && minute >= 0 && minute < 60) {
            return hour * 60 + minute;
        }
    } catch (const std::invalid_argument& e) {
        // stoi failed
    } catch (const std::out_of_range& e) {
        // stoi out of range
    }
    return -1; // Indicate invalid values
}

// Converts total minutes from midnight to "HH:MM" string
string minutesToTime(int totalMinutes) {
    if (totalMinutes < 0 || totalMinutes >= 24 * 60) {
        return "Invalid Time"; // Should not happen with valid input
    }
    int hour = totalMinutes / 60;
    int minute = totalMinutes % 60;
    stringstream ss;
    ss << setfill('0') << setw(2) << hour << ":" << setfill('0') << setw(2) << minute;
    return ss.str();
}

//..................................PATIENT MANAGEMENT...............................................
Patient* createPatient(int id, string name, int age, char gender, string address, string contact, string disease, string doctor, string admission, bool admitted) {
    Patient* newPatient = new Patient;
    newPatient->patientID = id;
    newPatient->name = name;
    newPatient->age = age;
    newPatient->gender = gender;
    newPatient->address = address;
    newPatient->contactNumber = contact;
    newPatient->disease = disease;
    newPatient->doctorAssigned = doctor;
    newPatient->admissionDate = admission;
    newPatient->isAdmitted = admitted;
    newPatient->next = nullptr;
    return newPatient;
}

void addPatient() {
    int id = getValidatedInt("Enter Patient ID: ");
    Patient* current = patientHead;
    while (current) {
        if (current->patientID == id) {
            cout << "Error: Patient with ID " << id << " already exists.\n";
            return;
        }
        current = current->next;
    }
    string name = getValidatedText("Enter Name: ");
    int age = getValidatedInt("Enter Age: ");
    char gender;
    // Input validation for gender
    while (true) {
        cout << "Enter Gender (M/F): ";
        cin >> gender;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        gender = toupper(gender);
        if (gender == 'M' || gender == 'F') {
            break;
        } else {
            cout << "Invalid gender. Please enter 'M' or 'F'.\n";
        }
    }
    string address = getValidatedText("Enter Address: ");
    string contact = getValidatedText("Enter Contact Number: "); // Add more specific validation if needed
    string disease = getValidatedText("Enter Disease: ");
    string doctor = getValidatedText("Enter Doctor Assigned (if any, else N/A): ");
    string admission = getCurrentDate();
    char admittedChoice;
    // Input validation for admitted status
    while (true) {
        cout << "Is patient admitted? (Y/N): ";
        cin >> admittedChoice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        admittedChoice = toupper(admittedChoice);
        if (admittedChoice == 'Y' || admittedChoice == 'N') {
            break;
        } else {
            cout << "Invalid input. Please enter 'Y' or 'N'.\n";
        }
    }
    bool admitted = (admittedChoice == 'Y');

    Patient* newPatient = createPatient(id, name, age, gender, address, contact, disease, doctor, admission, admitted);

    // Insert into sorted linked list by ID
    if (!patientHead || newPatient->patientID < patientHead->patientID) {
        newPatient->next = patientHead;
        patientHead = newPatient;
    } else {
        current = patientHead;
        while (current->next && current->next->patientID < newPatient->patientID) {
            current = current->next;
        }
        newPatient->next = current->next;
        current->next = newPatient;
    }
    cout << "Patient added successfully.\n";
}

void displayPatients() {
    if (!patientHead) {
        cout << "No patients to display.\n";
        return;
    }
    Patient* current = patientHead;
    cout << "\n--- Patient List ---\n";
    cout << left << setw(5) << "ID" << setw(20) << "Name" << setw(5) << "Age" << setw(8) << "Gender"
         << setw(15) << "Disease" << setw(10) << "Admitted" << endl;
    cout << string(63, '-') << endl;
    while (current) {
        cout << left << setw(5) << current->patientID << setw(20) << current->name
             << setw(5) << current->age << setw(8) << current->gender
             << setw(15) << current->disease << setw(10) << (current->isAdmitted ? "Yes" : "No") << endl;
        current = current->next;
    }
    cout << string(63, '-') << endl;
}

Patient* searchPatientByID(int id) {
    Patient* current = patientHead;
    while (current) {
        if (current->patientID == id) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}

void updatePatient() {
    int id = getValidatedInt("Enter Patient ID to update: ");
    Patient* patient = searchPatientByID(id);
    if (patient) {
        cout << "Patient Found. Enter new details (press Enter to keep current value):\n";
        string newName = getValidatedText("Enter New Name (" + patient->name + "): ");
        if (!newName.empty()) patient->name = newName;

        string newAgeStr;
        cout << "Enter New Age (" << patient->age << "): ";
        getline(cin, newAgeStr);
        if (!newAgeStr.empty()) {
            try {
                patient->age = stoi(newAgeStr);
            } catch (const std::invalid_argument& e) {
                cout << "Invalid age format. Age not updated.\n";
            }
        }
        
        char newGender;
        cout << "Enter New Gender (M/F) (" << patient->gender << ") (leave empty to keep): ";
        string genderInput;
        getline(cin, genderInput);
        if (!genderInput.empty()) {
            newGender = toupper(genderInput[0]);
            if (newGender == 'M' || newGender == 'F') {
                patient->gender = newGender;
            } else {
                cout << "Invalid gender input. Gender not updated.\n";
            }
        }

        string newAddress = getValidatedText("Enter New Address (" + patient->address + "): ");
        if (!newAddress.empty()) patient->address = newAddress;

        string newContact = getValidatedText("Enter New Contact Number (" + patient->contactNumber + "): ");
        if (!newContact.empty()) patient->contactNumber = newContact;

        string newDisease = getValidatedText("Enter New Disease (" + patient->disease + "): ");
        if (!newDisease.empty()) patient->disease = newDisease;

        string newDoctor = getValidatedText("Enter New Doctor Assigned (" + patient->doctorAssigned + "): ");
        if (!newDoctor.empty()) patient->doctorAssigned = newDoctor;

        char admittedChoice;
        cout << "Is patient Admitted? (Y/N) (" << (patient->isAdmitted ? "Y" : "N") << ") (leave empty to keep): ";
        string admittedInput;
        getline(cin, admittedInput);
        if (!admittedInput.empty()) {
            admittedChoice = toupper(admittedInput[0]);
            if (admittedChoice == 'Y' || admittedChoice == 'N') {
                patient->isAdmitted = (admittedChoice == 'Y');
            } else {
                cout << "Invalid admitted status input. Status not updated.\n";
            }
        }
        cout << "Patient details updated successfully.\n";
    } else {
        cout << "Patient with ID " << id << " not found.\n";
    }
}

void deletePatient() {
    int id = getValidatedInt("Enter Patient ID to delete: ");
    Patient* current = patientHead;
    Patient* prev = nullptr;

    while (current && current->patientID != id) {
        prev = current;
        current = current->next;
    }

    if (current) {
        // --- REAL-LIFE APPLICATION IMPROVEMENT: Cascading Deletion for Appointments ---
        // Iterate through all appointments and cancel those belonging to this patient
        Appointment* appCurrent = appointmentHead;
        while(appCurrent) {
            if (appCurrent->patientID == id && appCurrent->status == "Scheduled") {
                // Change status to Cancelled and remove from doctor's booked slots
                updateAppointmentStatus(appCurrent->appointmentID, "Cancelled");
                cout << "  -> Appointment " << appCurrent->appointmentID << " for patient " << current->name << " cancelled.\n";
            }
            appCurrent = appCurrent->next;
        }
        // --- End Cascading Deletion ---

        if (prev) {
            prev->next = current->next;
        } else {
            patientHead = current->next;
        }
        delete current;
        cout << "Patient with ID " << id << " deleted successfully.\n";
    } else {
        cout << "Patient with ID " << id << " not found.\n";
    }
}

void savePatients() {
    ofstream file("patients.txt");
    if (!file.is_open()) {
        cout << "Error: Could not open patients.txt for saving.\n";
        return;
    }
    Patient* current = patientHead;
    while (current) {
        file << current->patientID << '|'
             << current->name << '|'
             << current->age << '|'
             << current->gender << '|'
             << current->address << '|'
             << current->contactNumber << '|'
             << current->disease << '|'
             << current->doctorAssigned << '|'
             << current->admissionDate << '|'
             << current->isAdmitted << '\n';
        current = current->next;
    }
    file.close();
    cout << "Patients data saved.\n";
}

void loadPatients() {
    ifstream file("patients.txt");
    if (!file.is_open()) {
        cout << "No existing patients data found. Starting fresh.\n";
        return;
    }
    string line;
    while (getline(file, line)) {
        Patient* newPatient = new Patient;
        stringstream ss(line);
        string segment;

        try {
            getline(ss, segment, '|'); newPatient->patientID = stoi(segment);
            getline(ss, newPatient->name, '|');
            getline(ss, segment, '|'); newPatient->age = stoi(segment);
            getline(ss, segment, '|'); newPatient->gender = segment[0];
            getline(ss, newPatient->address, '|');
            getline(ss, newPatient->contactNumber, '|');
            getline(ss, newPatient->disease, '|');
            getline(ss, newPatient->doctorAssigned, '|');
            getline(ss, newPatient->admissionDate, '|');
            getline(ss, segment); newPatient->isAdmitted = (segment == "1");
        } catch (const std::exception& e) {
            cerr << "Error loading patient data: " << line << " - " << e.what() << endl;
            delete newPatient;
            continue; // Skip corrupted line
        }

        newPatient->next = nullptr;
        // Insert into sorted linked list by ID
        if (!patientHead || newPatient->patientID < patientHead->patientID) {
            newPatient->next = patientHead;
            patientHead = newPatient;
        } else {
            Patient* current = patientHead;
            while (current->next && current->next->patientID < newPatient->patientID) {
                current = current->next;
            }
            newPatient->next = current->next;
            current->next = newPatient;
        }
    }
    file.close();
    cout << "Patients data loaded successfully.\n";
}

//..................................DOCTOR MANAGEMENT (BST)........................................

// FIX: Modified createDoctor to accept maxAppointmentsPerDay
Doctor* createDoctor(int id, string name, string specialization, string contactNumber, string email, int experienceYears, int maxAppointmentsPerDay) {
    Doctor* newDoc = new Doctor;
    newDoc->doctorID = id;
    newDoc->name = name;
    newDoc->specialization = specialization;
    newDoc->contactNumber = contactNumber;
    newDoc->email = email;
    newDoc->experienceYears = experienceYears;
    newDoc->isAvailable = true; // Default to available when created
    newDoc->maxAppointmentsPerDay = maxAppointmentsPerDay; // Set max appointments
    newDoc->left = nullptr;
    newDoc->right = nullptr;
    newDoc->next = nullptr; // Not strictly used for BST, but good practice
    return newDoc;
}

Doctor* insertDoctor(Doctor* node, Doctor* newDoc) {
    if (!node) {
        return newDoc;
    }
    if (newDoc->doctorID < node->doctorID) {
        node->left = insertDoctor(node->left, newDoc);
    } else if (newDoc->doctorID > node->doctorID) {
        node->right = insertDoctor(node->right, newDoc);
    } else { // Doctor with this ID already exists
        cout << "Warning: Doctor with ID " << newDoc->doctorID << " already exists during load. Skipping insertion.\n";
        // In a real scenario, you might want to delete newDoc here or merge
        delete newDoc; // Prevent memory leak for duplicate
        return node;
    }
    return node;
}

void addDoctor() {
    int id = getValidatedInt("Enter Doctor ID: ");
    if (searchDoctorByID(docRoot, id)) {
        cout << "Error: Doctor with ID " << id << " already exists.\n";
        return;
    }
    string name = getValidatedText("Enter Name: ");
    string spec = getValidatedText("Enter Specialization: ");
    string contact = getValidatedText("Enter Contact Number: ");
    string email = getValidatedText("Enter Email: "); // Add email format validation if needed
    int exp = getValidatedInt("Enter Years of Experience: ");
    if (exp < 0) {
        cout << "Experience cannot be negative. Setting to 0.\n";
        exp = 0;
    }
    int maxAppts = getValidatedInt("Enter Maximum Appointments per Day for this Doctor: "); // NEW INPUT
    if (maxAppts <= 0) {
        cout << "Maximum appointments must be positive. Setting to 1.\n";
        maxAppts = 1;
    }

    Doctor* newDoc = createDoctor(id, name, spec, contact, email, exp, maxAppts); // Pass maxAppts
    docRoot = insertDoctor(docRoot, newDoc);
    cout << "Doctor added successfully.\n";
}

void inOrderTraversal(Doctor* node) {
    if (node) {
        inOrderTraversal(node->left);
        cout << left << setw(5) << node->doctorID << setw(20) << node->name
             << setw(20) << node->specialization << setw(15) << node->experienceYears
             << setw(10) << node->maxAppointmentsPerDay << endl;
        inOrderTraversal(node->right);
    }
}

void displayDoctors() {
    if (!docRoot) {
        cout << "No doctors to display.\n";
        return;
    }
    cout << "\n--- Doctor List (Sorted by ID) ---\n";
    cout << left << setw(5) << "ID" << setw(20) << "Name" << setw(20) << "Specialization"
         << setw(15) << "Experience" << setw(10) << "Max Appts" << endl;
    cout << string(70, '-') << endl;
    inOrderTraversal(docRoot);
    cout << string(70, '-') << endl;
}

Doctor* searchDoctorByID(Doctor* node, int id) {
    if (!node || node->doctorID == id) {
        return node;
    }
    if (id < node->doctorID) {
        return searchDoctorByID(node->left, id);
    } else {
        return searchDoctorByID(node->right, id);
    }
}

Doctor* findMin(Doctor* node) {
    Doctor* current = node;
    while (current && current->left != nullptr) {
        current = current->left;
    }
    return current;
}

Doctor* deleteDoctorNode(Doctor* root, int id) {
    if (!root) {
        return root;
    }
    if (id < root->doctorID) {
        root->left = deleteDoctorNode(root->left, id);
    } else if (id > root->doctorID) {
        root->right = deleteDoctorNode(root->right, id);
    } else {
        // --- REAL-LIFE APPLICATION IMPROVEMENT: Cascading Deletion for Appointments ---
        // Iterate through all appointments and cancel those belonging to this doctor
        Appointment* appCurrent = appointmentHead;
        while(appCurrent) {
            if (appCurrent->doctorID == id && appCurrent->status == "Scheduled") {
                updateAppointmentStatus(appCurrent->appointmentID, "Cancelled (Doctor removed)");
                cout << "  -> Appointment " << appCurrent->appointmentID << " for Doctor " << root->name << " cancelled.\n";
            }
            appCurrent = appCurrent->next;
        }
        // --- End Cascading Deletion ---

        // Node with only one child or no child
        if (root->left == nullptr) {
            Doctor* temp = root->right;
            delete root;
            return temp;
        } else if (root->right == nullptr) {
            Doctor* temp = root->left;
            delete root;
            return temp;
        }

        // Node with two children: Get the inorder successor (smallest in the right subtree)
        Doctor* temp = findMin(root->right);

        // Copy the inorder successor's content to this node
        root->doctorID = temp->doctorID;
        root->name = temp->name;
        root->specialization = temp->specialization;
        root->contactNumber = temp->contactNumber;
        root->email = temp->email;
        root->experienceYears = temp->experienceYears;
        root->isAvailable = temp->isAvailable;
        root->bookedSlots = temp->bookedSlots; // Copy booked slots
        root->maxAppointmentsPerDay = temp->maxAppointmentsPerDay; // Copy max appointments

        // Delete the inorder successor
        root->right = deleteDoctorNode(root->right, temp->doctorID);
    }
    return root;
}

void deleteDoctor() {
    int id = getValidatedInt("Enter Doctor ID to delete: ");
    if (searchDoctorByID(docRoot, id)) {
        docRoot = deleteDoctorNode(docRoot, id);
        cout << "Doctor with ID " << id << " deleted successfully.\n";
    } else {
        cout << "Doctor with ID " << id << " not found.\n";
    }
}

void updateDoctor() {
    int id = getValidatedInt("Enter Doctor ID to update: ");
    Doctor* doctor = searchDoctorByID(docRoot, id);
    if (doctor) {
        cout << "Doctor Found. Enter new details (press Enter to keep current value):\n";
        string newName = getValidatedText("Enter New Name (" + doctor->name + "): ");
        if (!newName.empty()) doctor->name = newName;

        string newSpecialization = getValidatedText("Enter New Specialization (" + doctor->specialization + "): ");
        if (!newSpecialization.empty()) doctor->specialization = newSpecialization;

        string newContact = getValidatedText("Enter New Contact Number (" + doctor->contactNumber + "): ");
        if (!newContact.empty()) doctor->contactNumber = newContact;

        string newEmail = getValidatedText("Enter New Email (" + doctor->email + "): ");
        if (!newEmail.empty()) doctor->email = newEmail;
        
        string newExpStr;
        cout << "Enter New Years of Experience (" << doctor->experienceYears << "): ";
        getline(cin, newExpStr);
        if (!newExpStr.empty()) {
            try {
                int exp = stoi(newExpStr);
                if (exp >= 0) doctor->experienceYears = exp;
                else cout << "Experience cannot be negative. Value not updated.\n";
            } catch (const std::invalid_argument& e) {
                cout << "Invalid experience format. Experience not updated.\n";
            }
        }

        string newMaxApptsStr;
        cout << "Enter New Maximum Appointments per Day (" << doctor->maxAppointmentsPerDay << "): ";
        getline(cin, newMaxApptsStr);
        if (!newMaxApptsStr.empty()) {
            try {
                int maxAppts = stoi(newMaxApptsStr);
                if (maxAppts > 0) doctor->maxAppointmentsPerDay = maxAppts;
                else cout << "Maximum appointments must be positive. Value not updated.\n";
            } catch (const std::invalid_argument& e) {
                cout << "Invalid max appointments format. Value not updated.\n";
            }
        }

        char availChoice;
        cout << "Is doctor available? (Y/N) (" << (doctor->isAvailable ? "Y" : "N") << ") (leave empty to keep): ";
        string availInput;
        getline(cin, availInput);
        if (!availInput.empty()) {
            availChoice = toupper(availInput[0]);
            if (availChoice == 'Y' || availChoice == 'N') {
                doctor->isAvailable = (availChoice == 'Y');
            } else {
                cout << "Invalid availability input. Status not updated.\n";
            }
        }
        cout << "Doctor details updated successfully.\n";
    } else {
        cout << "Doctor with ID " << id << " not found.\n";
    }
}

void saveDoctorsRecursive(Doctor* node, ofstream& file) {
    if (!node) return;
    file << node->doctorID << '|'
         << node->name << '|'
         << node->specialization << '|'
         << node->contactNumber << '|'
         << node->email << '|'
         << node->experienceYears << '|'
         << node->maxAppointmentsPerDay << '|' // NEW: Save maxAppointmentsPerDay
         << mapToString(node->bookedSlots) << '\n';
    saveDoctorsRecursive(node->left, file);
    saveDoctorsRecursive(node->right, file);
}

void saveDoctors() {
    ofstream file("doctors.txt");
    if (file.is_open()) {
        saveDoctorsRecursive(docRoot, file);
        file.close();
        cout << "Doctors data saved.\n";
    } else {
        cout << "Unable to open doctors.txt for saving.\n";
    }
}

void loadDoctors() {
    ifstream file("doctors.txt");
    if (!file.is_open()) {
        cout << "No existing doctors data found. Starting fresh.\n";
        return;
    }
    string line;
    while (getline(file, line)) {
        Doctor* newDoc = new Doctor;
        stringstream ss(line);
        string segment;

        try {
            getline(ss, segment, '|'); newDoc->doctorID = stoi(segment);
            getline(ss, newDoc->name, '|');
            getline(ss, newDoc->specialization, '|');
            getline(ss, newDoc->contactNumber, '|');
            getline(ss, newDoc->email, '|');
            getline(ss, segment, '|'); newDoc->experienceYears = stoi(segment);
            getline(ss, segment, '|'); newDoc->maxAppointmentsPerDay = stoi(segment); // NEW: Load maxAppointmentsPerDay
            getline(ss, segment); // Read the rest of the line (bookedSlots string)
            newDoc->bookedSlots = stringToMap(segment);
        } catch (const std::exception& e) {
            cerr << "Error loading doctor data: " << line << " - " << e.what() << endl;
            delete newDoc;
            continue; // Skip corrupted line
        }

        newDoc->isAvailable = true; // Assume available on load, status derived from bookedSlots
        newDoc->left = nullptr;
        newDoc->right = nullptr;
        newDoc->next = nullptr;
        docRoot = insertDoctor(docRoot, newDoc); // Use insertDoctor to maintain BST property
    }
    file.close();
    cout << "Doctors data loaded successfully.\n";
}

//..................................RESOURCE MANAGEMENT..............................................
Resource* createResource(int id, string name, string type, bool available) {
    Resource* newResource = new Resource;
    newResource->resourceID = id;
    newResource->name = name;
    newResource->type = type;
    newResource->isAvailable = available;
    newResource->next = nullptr;
    return newResource;
}

void addResource() {
    int id = getValidatedInt("Enter Resource ID: ");
    Resource* current = resourceHead;
    while (current) {
        if (current->resourceID == id) {
            cout << "Error: Resource with ID " << id << " already exists.\n";
            return;
        }
        current = current->next;
    }
    string name = getValidatedText("Enter Name: ");
    string type = getValidatedText("Enter Type: ");
    char availChoice;
    while (true) {
        cout << "Is resource available? (Y/N): ";
        cin >> availChoice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        availChoice = toupper(availChoice);
        if (availChoice == 'Y' || availChoice == 'N') {
            break;
        } else {
            cout << "Invalid input. Please enter 'Y' or 'N'.\n";
        }
    }
    bool available = (availChoice == 'Y');

    Resource* newResource = createResource(id, name, type, available);

    // Insert into sorted linked list by ID
    if (!resourceHead || newResource->resourceID < resourceHead->resourceID) {
        newResource->next = resourceHead;
        resourceHead = newResource;
    } else {
        current = resourceHead;
        while (current->next && current->next->resourceID < newResource->resourceID) {
            current = current->next;
        }
        newResource->next = current->next;
        current->next = newResource;
    }
    cout << "Resource added successfully.\n";
}

void displayResources() {
    if (!resourceHead) {
        cout << "No resources to display.\n";
        return;
    }
    Resource* current = resourceHead;
    cout << "\n--- Resource List ---\n";
    cout << left << setw(5) << "ID" << setw(20) << "Name" << setw(20) << "Type" << setw(10) << "Available" << endl;
    cout << string(55, '-') << endl;
    while (current) {
        cout << left << setw(5) << current->resourceID << setw(20) << current->name
             << setw(20) << current->type << setw(10) << (current->isAvailable ? "Yes" : "No") << endl;
        current = current->next;
    }
    cout << string(55, '-') << endl;
}

Resource* searchResourceByID(int id) {
    Resource* current = resourceHead;
    while (current) {
        if (current->resourceID == id) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}

void updateResource() {
    int id = getValidatedInt("Enter Resource ID to update: ");
    Resource* resource = searchResourceByID(id);
    if (resource) {
        cout << "Resource Found. Enter new details (press Enter to keep current value):\n";
        string newName = getValidatedText("Enter New Name (" + resource->name + "): ");
        if (!newName.empty()) resource->name = newName;

        string newType = getValidatedText("Enter New Type (" + resource->type + "): ");
        if (!newType.empty()) resource->type = newType;

        char availChoice;
        cout << "Is resource available? (Y/N) (" << (resource->isAvailable ? "Y" : "N") << ") (leave empty to keep): ";
        string availInput;
        getline(cin, availInput);
        if (!availInput.empty()) {
            availChoice = toupper(availInput[0]);
            if (availChoice == 'Y' || availChoice == 'N') {
                resource->isAvailable = (availChoice == 'Y');
            } else {
                cout << "Invalid availability input. Status not updated.\n";
            }
        }
        cout << "Resource details updated successfully.\n";
    } else {
        cout << "Resource with ID " << id << " not found.\n";
    }
}

void deleteResource() {
    int id = getValidatedInt("Enter Resource ID to delete: ");
    Resource* current = resourceHead;
    Resource* prev = nullptr;

    while (current && current->resourceID != id) {
        prev = current;
        current = current->next;
    }

    if (current) {
        if (prev) {
            prev->next = current->next;
        } else {
            resourceHead = current->next;
        }
        delete current;
        cout << "Resource with ID " << id << " deleted successfully.\n";
    } else {
        cout << "Resource with ID " << id << " not found.\n";
    }
}

void saveResources() {
    ofstream file("resources.txt");
    if (!file.is_open()) {
        cout << "Error: Could not open resources.txt for saving.\n";
        return;
    }
    Resource* current = resourceHead;
    while (current) {
        file << current->resourceID << '|'
             << current->name << '|'
             << current->type << '|'
             << current->isAvailable << '\n';
        current = current->next;
    }
    file.close();
    cout << "Resources data saved.\n";
}

void loadResources() {
    ifstream file("resources.txt");
    if (!file.is_open()) {
        cout << "No existing resources data found. Starting fresh.\n";
        return;
    }
    string line;
    while (getline(file, line)) {
        Resource* newResource = new Resource;
        stringstream ss(line);
        string segment;

        try {
            getline(ss, segment, '|'); newResource->resourceID = stoi(segment);
            getline(ss, newResource->name, '|');
            getline(ss, newResource->type, '|');
            getline(ss, segment); newResource->isAvailable = (segment == "1");
        } catch (const std::exception& e) {
            cerr << "Error loading resource data: " << line << " - " << e.what() << endl;
            delete newResource;
            continue; // Skip corrupted line
        }

        newResource->next = nullptr;
        // Insert into sorted linked list by ID
        if (!resourceHead || newResource->resourceID < resourceHead->resourceID) {
            newResource->next = resourceHead;
            resourceHead = newResource;
        } else {
            Resource* current = resourceHead;
            while (current->next && current->next->resourceID < newResource->resourceID) {
                current = current->next;
            }
            newResource->next = current->next;
            current->next = newResource;
        }
    }
    file.close();
    cout << "Resources data loaded successfully.\n";
}

//..................................APPOINTMENT MANAGEMENT...........................................
bool isDoctorAvailableAtTime(int doctorID, const string& date, const string& time, int durationMinutes) {
    Doctor* doc = searchDoctorByID(docRoot, doctorID);
    if (doc == nullptr) {
        return false; // Doctor not found
    }
    int requestedStartMinutes = timeToMinutes(time);
    if (requestedStartMinutes == -1) { // Invalid time format
        return false;
    }
    int requestedEndMinutes = requestedStartMinutes + durationMinutes;

    // Define doctor's working hours (e.g., 9 AM to 5 PM)
    int dayStartMinutes = timeToMinutes("09:00"); // 9 AM
    int dayEndMinutes = timeToMinutes("17:00");   // 5 PM (5 PM is 17:00, meaning until 17:00, so last appt ends at 17:00)

    // 1. Ensure the requested appointment falls within working hours
    if (requestedStartMinutes < dayStartMinutes || requestedEndMinutes > dayEndMinutes) {
        return false; // Outside working hours
    }

    if (doc->bookedSlots.count(date) && doc->bookedSlots[date].size() >= doc->maxAppointmentsPerDay) {
        return false; // Daily limit reached
    }

    // 3. Check for time overlaps with existing appointments
    if (doc->bookedSlots.count(date)) {
        for (const string& bookedTime : doc->bookedSlots[date]) {
            int bookedStartMinutes = timeToMinutes(bookedTime);
            if (bookedStartMinutes == -1) continue; // Skip invalid booked times

            int bookedDuration = 30; // Assuming 30 minutes for existing appointments
            int bookedEndMinutes = bookedStartMinutes + bookedDuration;

            // Check for overlap: [A_start, A_end) and [B_start, B_end) overlap if (A_start < B_end && B_start < A_end)
            if (requestedStartMinutes < bookedEndMinutes && bookedStartMinutes < requestedEndMinutes) {
                return false; // Conflict found
            }
        }
    }
    
    return true; // No conflict and not over daily limit
}

string getSpecializationForDisease(const string& disease) {
    string lowerDisease = toLower(disease);

    if (lowerDisease.find("fever") != string::npos || lowerDisease.find("cold") != string::npos || lowerDisease.find("flu") != string::npos) {
        return "General Physician";
    } else if (lowerDisease.find("heart") != string::npos || lowerDisease.find("cardiac") != string::npos) {
        return "Cardiology";
    } else if (lowerDisease.find("bone") != string::npos || lowerDisease.find("fracture") != string::npos || lowerDisease.find("joint") != string::npos) {
        return "Orthopedics";
    } else if (lowerDisease.find("skin") != string::npos || lowerDisease.find("dermatitis") != string::npos) {
        return "Dermatology";
    } else if (lowerDisease.find("diabetes") != string::npos || lowerDisease.find("thyroid") != string::npos) {
        return "Endocrinology";
    } else if (lowerDisease.find("cancer") != string::npos) {
        return "Oncology";
    }
    return "General Physician"; // Default specialization if no specific match
}

void collectDoctorsBySpecialization(Doctor* node, const string& targetSpec, vector<Doctor*>& foundDoctors) {
    if (!node) return;
    collectDoctorsBySpecialization(node->left, targetSpec, foundDoctors);
    if (toLower(node->specialization) == toLower(targetSpec)) {
        foundDoctors.push_back(node);
    }
    collectDoctorsBySpecialization(node->right, targetSpec, foundDoctors);
}

Doctor* findBestDoctor(const string& patientDisease, int appointmentDurationMinutes) {
    string requiredSpecialization = getSpecializationForDisease(patientDisease);
    vector<Doctor*> specialistDoctors;
    collectDoctorsBySpecialization(docRoot, requiredSpecialization, specialistDoctors);

    if (specialistDoctors.empty()) {
        cout << "No doctors found for '" << requiredSpecialization << "' specialization.\n";
        return nullptr;
    }

    // Sort by experience (descending) to find the "best"
    sort(specialistDoctors.begin(), specialistDoctors.end(), [](Doctor* a, Doctor* b) {
        return a->experienceYears > b->experienceYears;
    });

    cout << "\n--- Recommended Doctors for " << patientDisease << " (" << requiredSpecialization << ") ---\n";
    int counter = 1;
    bool foundAnyAvailable = false;

    // To check immediate availability for display, we need a current date.
    string today = getCurrentDate();
    string startCheckTime = "09:00"; // Start checking from 9 AM

    for (Doctor* doc : specialistDoctors) {
        cout << counter++ << ". Dr. " << doc->name
             << " (ID: " << doc->doctorID << ")"
             << ", Specialization: " << doc->specialization
             << ", Experience: " << doc->experienceYears << " years";

        if (doc->bookedSlots.count(today) && doc->bookedSlots[today].size() >= doc->maxAppointmentsPerDay) {
            cout << " (Daily limit reached today)";
        } else {
            
            int currentMin = max(timeToMinutes(startCheckTime), timeToMinutes(getCurrentTime())); // Start from 9 AM or current time
            bool availableToday = false;
            while(currentMin + appointmentDurationMinutes <= timeToMinutes("17:00")) { // Within working hours
                if(isDoctorAvailableAtTime(doc->doctorID, today, minutesToTime(currentMin), appointmentDurationMinutes)) {
                    cout << " (Available today from " << minutesToTime(currentMin) << ")";
                    availableToday = true;
                    foundAnyAvailable = true;
                    break;
                }
                currentMin += 30; // Check every 30 minutes
            }
            if (!availableToday) {
                 cout << " (No immediate slots today)";
            }
        }
        cout << "\n";
    }

    if (!foundAnyAvailable) {
        cout << "\nNo immediate availability found for any recommended doctor today.\n";
        // Optionally suggest trying another date if no immediate availability
    }

    int choice;
    while (true) {
        choice = getValidatedInt("Enter the number of the doctor you want to choose (0 to cancel): ");
        if (choice == 0) {
            cout << "Doctor selection cancelled.\n";
            return nullptr;
        }
        if (choice > 0 && choice <= specialistDoctors.size()) {
            return specialistDoctors[choice - 1];
        } else {
            cout << "Invalid choice. Please enter a valid number.\n";
        }
    }
}

string getNextDay(const string& current_date) {
   
    int year = stoi(current_date.substr(0, 4));
    int month = stoi(current_date.substr(5, 2));
    int day = stoi(current_date.substr(8, 2));

    day++;

    stringstream ss;
    ss << year << '-' << setfill('0') << setw(2) << month << '-' << setfill('0') << setw(2) << day;
    return ss.str();
}

bool displayDoctorAvailabilityChart(Doctor* doc, const string& date, int appointmentDurationMinutes) {
    if (!doc) return false;

    cout << "\n--- Availability Chart for Dr. " << doc->name << " on " << date << " ---\n";
    cout << "Working Hours: 09:00 - 17:00 (Appointments are " << appointmentDurationMinutes << " minutes)\n";
    
    // Check daily limit upfront and return if exceeded
    if (doc->bookedSlots.count(date) && doc->bookedSlots[date].size() >= doc->maxAppointmentsPerDay) {
        cout << "STATUS: Doctor has reached daily appointment limit (" << doc->maxAppointmentsPerDay << ") for this date.\n";
        return false;
    }

    bool anyAvailable = false;
    int dayStartMinutes = timeToMinutes("09:00");
    int dayEndMinutes = timeToMinutes("17:00"); // 5 PM

    vector<pair<string, string>> slots; // Stores {time, status}
    for (int currentMin = dayStartMinutes; currentMin + appointmentDurationMinutes <= dayEndMinutes; currentMin += appointmentDurationMinutes) {
        string slotTime = minutesToTime(currentMin);
        // Pass doc->doctorID instead of doc->name
        if (isDoctorAvailableAtTime(doc->doctorID, date, slotTime, appointmentDurationMinutes)) {
            slots.push_back({slotTime, "Available"});
            anyAvailable = true;
        } else {
            slots.push_back({slotTime, "Booked"});
        }
    }

    if (!anyAvailable) {
        cout << "No available slots found for Dr. " << doc->name << " on " << date << ".\n";
        return false;
    }

    // Always print individual slots
    for (const auto& slot : slots) {
        cout << slot.first << " - " << slot.second << "\n";
    }
    
    return anyAvailable;
}

void addAppointment() {
    Appointment* newApp = new Appointment();
    newApp->appointmentID = getValidatedInt("Enter Appointment ID: ");

    // Check if appointment ID already exists
    if (searchAppointmentByID(newApp->appointmentID)) {
        cout << "Error: Appointment with ID " << newApp->appointmentID << " already exists. Please choose a different ID.\n";
        delete newApp;
        return;
    }

    newApp->patientID = getValidatedInt("Enter Patient ID: ");
    Patient* patient = searchPatientByID(newApp->patientID);
    if (!patient) {
        cout << "Error: Patient not found. Please add the patient first or enter an existing patient ID.\n";
        delete newApp;
        return;
    }
    newApp->patientName = patient->name;

    Doctor* chosenDoc = nullptr;
    char specificDocChoice;
    cout << "Do you want to book with a specific doctor? (Y/N): ";
    cin >> specificDocChoice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    specificDocChoice = toupper(specificDocChoice);

    if (specificDocChoice == 'Y') {
        newApp->doctorID = getValidatedInt("Enter Doctor ID: ");
        chosenDoc = searchDoctorByID(docRoot, newApp->doctorID);
        if (!chosenDoc) {
            cout << "Error: Doctor with ID " << newApp->doctorID << " does not exist. Appointment not booked.\n";
            delete newApp;
            return;
        }
    } else { // Patient doesn't want a specific doctor, suggest best
        string patientDisease = patient->disease; 
        if (patientDisease.empty() || toLower(patientDisease) == "n/a") {
            cout << "Patient's disease not specified in record. This affects doctor recommendation.\n";
            patientDisease = getValidatedText("Please enter the patient's disease for a more accurate doctor recommendation: ");
            if (patientDisease.empty()) {
                cout << "No disease entered. Defaulting to 'General Physician' for recommendation.\n";
                patientDisease = "General Checkup";
            }
        }
        
        int appointmentDuration = 30; // Assume 30 min default duration for best doctor search
        chosenDoc = findBestDoctor(patientDisease, appointmentDuration); 
        
        if (!chosenDoc) {
            cout << "No suitable doctor found or selected. Appointment not booked.\n";
            delete newApp;
            return;
        }
        newApp->doctorID = chosenDoc->doctorID; // Assign the ID of the chosen best doctor
    }
    newApp->doctorName = chosenDoc->name; // Set doctor name based on chosenDoc

    string preferredDate;
    string chosenTime;
    int appointmentDuration = 30; // Standard appointment duration in minutes

    bool slotBooked = false;
    char tryAnotherDate;
    do {
        preferredDate = getValidatedText("Enter preferred Date for appointment (YYYY-MM-DD) (e.g., " + getCurrentDate() + "): ");
        
        if (displayDoctorAvailabilityChart(chosenDoc, preferredDate, appointmentDuration)) {
            // --- REAL-LIFE APPLICATION IMPROVEMENT: Guided Time Selection ---
            bool validTimeChosen = false;
            do {
                chosenTime = getValidatedText("Enter your feasible time (HH:MM) from the 'Available' slots shown above, or type 'back' to choose another date: ");
                if (toLower(chosenTime) == "back") {
                    validTimeChosen = true; // Exit inner loop to re-enter date
                    slotBooked = false; // Indicate no slot booked yet
                    break;
                }

                if (timeToMinutes(chosenTime) == -1) {
                    cout << "Invalid time format. Please use HH:MM (e.g., 09:30).\n";
                    continue;
                }

                if (isDoctorAvailableAtTime(chosenDoc->doctorID, preferredDate, chosenTime, appointmentDuration)) {
                    newApp->date = preferredDate;
                    newApp->time = chosenTime;
                    slotBooked = true;
                    validTimeChosen = true; // Valid time chosen, exit inner loop
                } else {
                    cout << "The chosen time slot '" << chosenTime << "' is not available. Please pick an 'Available' slot from the chart.\n";
                }
            } while (!validTimeChosen);
            // --- End Guided Time Selection ---
        } else {
            // No slots available for the date, or daily limit reached
            cout << "No available slots on " << preferredDate << ".";
            cout << " Would you like to try another date? (Y/N): ";
            cin >> tryAnotherDate;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (toupper(tryAnotherDate) != 'Y') {
                cout << "Appointment booking cancelled.\n";
                delete newApp;
                return;
            }
            slotBooked = false; // Stay in the outer loop
        }
    } while (!slotBooked && (toupper(tryAnotherDate) == 'Y' || toLower(chosenTime) == "back")); // Continue if user wants to try another date

    if (!slotBooked) {
        cout << "Appointment booking cancelled.\n";
        delete newApp;
        return;
    }

    // Update doctor's booked slots
    chosenDoc->bookedSlots[newApp->date].push_back(newApp->time);
    sort(chosenDoc->bookedSlots[newApp->date].begin(), chosenDoc->bookedSlots[newApp->date].end());

    newApp->reason = getValidatedText("Enter Reason for appointment: ");
    newApp->status = "Scheduled";
    newApp->next = nullptr;

    // Insert appointment into sorted linked list (by date and then time)
    if (!appointmentHead || newApp->date < appointmentHead->date ||
        (newApp->date == appointmentHead->date && newApp->time < appointmentHead->time)) {
        newApp->next = appointmentHead;
        appointmentHead = newApp;
    } else {
        Appointment* current = appointmentHead;
        while (current->next &&
               (current->next->date < newApp->date ||
               (current->next->date == newApp->date && current->next->time < newApp->time))) {
            current = current->next;
        }
        newApp->next = current->next;
        current->next = newApp;
    }

    cout << "Appointment added successfully for Dr. " << chosenDoc->name << " on " << newApp->date << " at " << newApp->time << ".\n";
}

void displayAppointments() {
    if (!appointmentHead) {
        cout << "No appointments to display.\n";
        return;
    }
    Appointment* current = appointmentHead;
    cout << "\n--- Appointment List (Sorted by Date and Time) ---\n";
    cout << left << setw(5) << "ID" << setw(10) << "Pat ID" << setw(18) << "Patient Name"
         << setw(10) << "Doc ID" << setw(18) << "Doctor Name" << setw(12) << "Date"
         << setw(8) << "Time" << setw(15) << "Reason" << setw(12) << "Status" << endl;
    cout << string(128, '-') << endl;
    while (current) {
        cout << left << setw(5) << current->appointmentID << setw(10) << current->patientID
             << setw(18) << current->patientName << setw(10) << current->doctorID
             << setw(18) << current->doctorName << setw(12) << current->date
             << setw(8) << current->time << setw(15) << current->reason
             << setw(12) << current->status << endl;
        current = current->next;
    }
    cout << string(128, '-') << endl;
}

Appointment* searchAppointmentByID(int id) {
    Appointment* current = appointmentHead;
    while (current) {
        if (current->appointmentID == id) {
            return current;
        }
        current = current->next;
    }
    return nullptr;
}

// Helper function to update appointment status and clean doctor's booked slots
void updateAppointmentStatus(int appointmentID, const string& newStatus) {
    Appointment* app = searchAppointmentByID(appointmentID);
    if (app) {
        // Only modify if status is "Scheduled" or similar for active appointments
        if (app->status == "Scheduled" || app->status == "Rescheduled") {
            // Remove from doctor's booked slots if changing from an active status
            Doctor* doctor = searchDoctorByID(docRoot, app->doctorID);
            if (doctor && doctor->bookedSlots.count(app->date)) {
                auto& bookedTimes = doctor->bookedSlots[app->date];
                bookedTimes.erase(remove(bookedTimes.begin(), bookedTimes.end(), app->time), bookedTimes.end());
                if (bookedTimes.empty()) {
                    doctor->bookedSlots.erase(app->date); // Remove date entry if no more appointments
                }
            }
        }
        app->status = newStatus;
        cout << "Appointment " << appointmentID << " status updated to '" << newStatus << "'.\n";
    } else {
        cout << "Appointment " << appointmentID << " not found to update status.\n";
    }
}

void updateAppointment() {
    int id = getValidatedInt("Enter Appointment ID to update: ");
    Appointment* app = searchAppointmentByID(id);
    if (app) {
        cout << "Appointment Found. Current status: " << app->status << ". Enter new details (press Enter to keep current value):\n";
        
        string newDate = app->date; // Default to current date
        string newTime = app->time; // Default to current time

        cout << "Enter New Date (YYYY-MM-DD) (" + app->date + ") (leave empty to keep): ";
        string dateInput;
        getline(cin, dateInput);
        if (!dateInput.empty()) {
            // Basic date format check (YYYY-MM-DD)
            if (dateInput.length() == 10 && dateInput[4] == '-' && dateInput[7] == '-') {
                newDate = dateInput;
            } else {
                cout << "Invalid date format. Date not updated. Please use YYYY-MM-DD.\n";
            }
        }

        cout << "Enter New Time (HH:MM) (" + app->time + ") (leave empty to keep): ";
        string timeInput;
        getline(cin, timeInput);
        if (!timeInput.empty()) {
            if (timeToMinutes(timeInput) != -1) {
                newTime = timeInput;
            } else {
                cout << "Invalid time format. Time not updated. Please use HH:MM.\n";
            }
        }
        
        // If doctor, date, or time changes, re-validate availability
        Doctor* doctor = searchDoctorByID(docRoot, app->doctorID);
        if (doctor && (newDate != app->date || newTime != app->time)) {
            // Temporarily remove old booking from doctor's map for availability check
            if (doctor->bookedSlots.count(app->date)) {
                auto& bookedTimes = doctor->bookedSlots[app->date];
                bookedTimes.erase(remove(bookedTimes.begin(), bookedTimes.end(), app->time), bookedTimes.end());
                if (bookedTimes.empty()) {
                    doctor->bookedSlots.erase(app->date); 
                }
            }
            
            // Check if new slot is available
            if (!isDoctorAvailableAtTime(app->doctorID, newDate, newTime, 30)) { // Assuming 30 min duration
                cout << "Doctor not available at the new date/time: " << newDate << " " << newTime << ". Reverting changes.\n";
                // Re-add old booking
                doctor->bookedSlots[app->date].push_back(app->time);
                sort(doctor->bookedSlots[app->date].begin(), doctor->bookedSlots[app->date].end());
                return;
            } else {
                // Update appointment object and doctor's booked slots with new time/date
                app->date = newDate;
                app->time = newTime;
                doctor->bookedSlots[app->date].push_back(app->time);
                sort(doctor->bookedSlots[app->date].begin(), doctor->bookedSlots[app->date].end());
                app->status = "Rescheduled"; // Update status to reflect change
                cout << "Appointment date/time updated and status set to 'Rescheduled'.\n";
            }
        }

        string newReason = getValidatedText("Enter New Reason (" + app->reason + "): ");
        if (!newReason.empty()) app->reason = newReason;

        string newStatus = getValidatedText("Enter New Status (Scheduled/Completed/Cancelled) (" + app->status + "): ");
        if (!newStatus.empty()) {
            string lowerNewStatus = toLower(newStatus);
            if (lowerNewStatus == "scheduled" || lowerNewStatus == "completed" || lowerNewStatus == "cancelled" || lowerNewStatus == "rescheduled") {
                // If status is changed to cancelled/completed, remove from doctor's active bookings
                if ((lowerNewStatus == "cancelled" || lowerNewStatus == "completed") && (app->status == "Scheduled" || app->status == "Rescheduled")) {
                     Doctor* doctor = searchDoctorByID(docRoot, app->doctorID);
                     if (doctor && doctor->bookedSlots.count(app->date)) {
                        auto& bookedTimes = doctor->bookedSlots[app->date];
                        bookedTimes.erase(remove(bookedTimes.begin(), bookedTimes.end(), app->time), bookedTimes.end());
                        if (bookedTimes.empty()) {
                            doctor->bookedSlots.erase(app->date); 
                        }
                     }
                }
                app->status = newStatus;
            } else {
                cout << "Invalid status entered. Status not updated.\n";
            }
        }
        cout << "Appointment details updated successfully.\n";
    } else {
        cout << "Appointment with ID " << id << " not found.\n";
    }
}

// Helper to actually delete the appointment node from the list
void deleteAppointmentNode(int appointmentID) {
    Appointment* current = appointmentHead;
    Appointment* prev = nullptr;

    while (current && current->appointmentID != appointmentID) {
        prev = current;
        current = current->next;
    }

    if (current) {
        // Remove from doctor's booked slots (if it was an active appointment)
        if (current->status == "Scheduled" || current->status == "Rescheduled") {
            Doctor* doc = searchDoctorByID(docRoot, current->doctorID);
            if (doc && doc->bookedSlots.count(current->date)) {
                auto& bookedTimes = doc->bookedSlots[current->date];
                bookedTimes.erase(remove(bookedTimes.begin(), bookedTimes.end(), current->time), bookedTimes.end());
                if (bookedTimes.empty()) {
                    doc->bookedSlots.erase(current->date); // Remove date entry if no more appointments
                }
            }
        }

        if (prev) {
            prev->next = current->next;
        } else {
            appointmentHead = current->next;
        }
        delete current;
        // Message handled by calling function
    } else {
        cout << "Error: Appointment with ID " << appointmentID << " not found for internal deletion.\n";
    }
}

void deleteAppointment() {
    int id = getValidatedInt("Enter Appointment ID to delete: ");
    Appointment* appToDelete = searchAppointmentByID(id);
    if (appToDelete) {
        deleteAppointmentNode(id); // Use the helper function
        cout << "Appointment with ID " << id << " deleted successfully.\n";
    } else {
        cout << "Appointment with ID " << id << " not found.\n";
    }
}

void saveAppointments() {
    ofstream file("appointments.txt");
    if (!file.is_open()) {
        cout << "Error: Could not open appointments.txt for saving.\n";
        return;
    }
    Appointment* current = appointmentHead;
    while (current) {
        file << current->appointmentID << '|'
             << current->patientID << '|'
             << current->patientName << '|'
             << current->doctorID << '|'
             << current->doctorName << '|'
             << current->date << '|'
             << current->time << '|'
             << current->reason << '|'
             << current->status << '\n';
        current = current->next;
    }
    file.close();
    cout << "Appointments data saved.\n";
}

void loadAppointments() {
    ifstream file("appointments.txt");
    if (!file.is_open()) {
        cout << "No existing appointments data found. Starting fresh.\n";
        return;
    }
    string line;
    while (getline(file, line)) {
        Appointment* newApp = new Appointment();
        stringstream ss(line);
        string segment;

        try {
            getline(ss, segment, '|'); newApp->appointmentID = stoi(segment);
            getline(ss, segment, '|'); newApp->patientID = stoi(segment);
            getline(ss, newApp->patientName, '|');
            getline(ss, segment, '|'); newApp->doctorID = stoi(segment);
            getline(ss, newApp->doctorName, '|');
            getline(ss, newApp->date, '|');
            getline(ss, newApp->time, '|');
            getline(ss, newApp->reason, '|');
            getline(ss, newApp->status);
        } catch (const std::exception& e) {
            cerr << "Error loading appointment data: " << line << " - " << e.what() << endl;
            delete newApp;
            continue; // Skip corrupted line
        }

        newApp->next = nullptr;
        // Insert in sorted order (by date, then time)
        if (!appointmentHead || newApp->date < appointmentHead->date ||
            (newApp->date == appointmentHead->date && newApp->time < appointmentHead->time)) {
            newApp->next = appointmentHead;
            appointmentHead = newApp;
        } else {
            Appointment* current = appointmentHead;
            while (current->next &&
                   (current->next->date < newApp->date ||
                   (current->next->date == newApp->date && current->next->time < newApp->time))) {
                current = current->next;
            }
            newApp->next = current->next;
            current->next = newApp;
        }

        // Re-populate doctor's bookedSlots map from loaded appointments, only for active ones
        if (newApp->status == "Scheduled" || newApp->status == "Rescheduled") {
            Doctor* doc = searchDoctorByID(docRoot, newApp->doctorID);
            if (doc) {
                // Ensure the slot isn't already there (e.g., if re-loading or duplicates)
                bool alreadyBooked = false;
                for (const string& bookedTime : doc->bookedSlots[newApp->date]) {
                    if (bookedTime == newApp->time) {
                        alreadyBooked = true;
                        break;
                    }
                }
                if (!alreadyBooked) {
                    doc->bookedSlots[newApp->date].push_back(newApp->time);
                    sort(doc->bookedSlots[newApp->date].begin(), doc->bookedSlots[newApp->date].end());
                }
            } else {
                cerr << "Warning: Doctor ID " << newApp->doctorID << " for appointment " << newApp->appointmentID
                     << " not found during load. Booked slot not added to doctor's schedule.\n";
            }
        }
    }
    file.close();
    cout << "Appointments data loaded successfully.\n";
}

//..................................EMERGENCY MANAGEMENT.............................................
Emergency* createEmergency(int id, string name, string condition, string timeLogged, string status) {
    Emergency* newEmergency = new Emergency;
    newEmergency->caseID = id;
    newEmergency->patientName = name;
    newEmergency->condition = condition;
    newEmergency->timeLogged = timeLogged;
    newEmergency->status = status;
    newEmergency->next = nullptr;
    return newEmergency;
}

void logEmergency() {
    int id = getValidatedInt("Enter Case ID: ");
    Emergency* current = emergencyHead;
    while (current) {
        if (current->caseID == id) {
            cout << "Error: Emergency with Case ID " << id << " already exists.\n";
            return;
        }
        current = current->next;
    }
    string name = getValidatedText("Enter Patient Name: ");
    string condition = getValidatedText("Enter Condition: ");
    string time = getCurrentDate() + " " + getCurrentTime(); // Log current date and time
    string status = "Pending";

    Emergency* newEmergency = createEmergency(id, name, condition, time, status);

    // Insert into sorted linked list by ID
    if (!emergencyHead || newEmergency->caseID < emergencyHead->caseID) {
        newEmergency->next = emergencyHead;
        emergencyHead = newEmergency;
    } else {
        current = emergencyHead;
        while (current->next && current->next->caseID < newEmergency->caseID) {
            current = current->next;
        }
        newEmergency->next = current->next;
        current->next = newEmergency;
    }
    cout << "Emergency logged successfully. Status: Pending.\n";
}

void dispatchEmergency() {
    // A simplified dispatch logic: just changes status to "Dispatched" for the first pending emergency
    if (!emergencyHead) {
        cout << "No emergencies to dispatch.\n";
        return;
    }
    Emergency* current = emergencyHead;
    bool dispatched = false;
    while (current) {
        if (current->status == "Pending") {
            current->status = "Dispatched";
            cout << "Emergency Case ID " << current->caseID << " for " << current->patientName << " has been dispatched.\n";
            dispatched = true;
            break; // Dispatch one at a time
        }
        current = current->next;
    }
    if (!dispatched) {
        cout << "No pending emergencies to dispatch.\n";
    }
}

void viewAllEmergencies() {
    if (!emergencyHead) {
        cout << "No emergencies to display.\n";
        return;
    }
    Emergency* current = emergencyHead;
    cout << "\n--- All Emergencies ---\n";
    cout << left << setw(10) << "Case ID" << setw(20) << "Patient Name"
         << setw(30) << "Condition" << setw(20) << "Time Logged" << setw(15) << "Status" << endl;
    cout << string(95, '-') << endl;
    while (current) {
        cout << left << setw(10) << current->caseID << setw(20) << current->patientName
             << setw(30) << current->condition << setw(20) << current->timeLogged
             << setw(15) << current->status << endl;
        current = current->next;
    }
    cout << string(95, '-') << endl;
}

void resolveEmergencyByID(int id) {
    Emergency* current = emergencyHead;
    while (current) {
        if (current->caseID == id) {
            if (current->status == "Resolved") {
                cout << "Emergency Case ID " << id << " is already resolved.\n";
            } else {
                current->status = "Resolved";
                cout << "Emergency Case ID " << id << " resolved successfully.\n";
            }
            return;
        }
        current = current->next;
    }
    cout << "Emergency with Case ID " << id << " not found.\n";
}

void saveEmergencies() {
    ofstream file("emergencies.txt");
    if (!file.is_open()) {
        cout << "Error: Could not open emergencies.txt for saving.\n";
        return;
    }
    Emergency* current = emergencyHead;
    while (current) {
        file << current->caseID << '|'
             << current->patientName << '|'
             << current->condition << '|'
             << current->timeLogged << '|'
             << current->status << '\n';
        current = current->next;
    }
    file.close();
    cout << "Emergencies data saved.\n";
}

void loadEmergencies() {
    ifstream file("emergencies.txt");
    if (!file.is_open()) {
        cout << "No existing emergencies data found. Starting fresh.\n";
        return;
    }
    string line;
    while (getline(file, line)) {
        Emergency* newEmergency = new Emergency;
        stringstream ss(line);
        string segment;

        try {
            getline(ss, segment, '|'); newEmergency->caseID = stoi(segment);
            getline(ss, newEmergency->patientName, '|');
            getline(ss, newEmergency->condition, '|');
            getline(ss, newEmergency->timeLogged, '|');
            getline(ss, newEmergency->status);
        } catch (const std::exception& e) {
            cerr << "Error loading emergency data: " << line << " - " << e.what() << endl;
            delete newEmergency;
            continue; // Skip corrupted line
        }

        newEmergency->next = nullptr;
        // Insert into sorted linked list by ID
        if (!emergencyHead || newEmergency->caseID < emergencyHead->caseID) {
            newEmergency->next = emergencyHead;
            emergencyHead = newEmergency;
        } else {
            Emergency* current = emergencyHead;
            while (current->next && current->next->caseID < newEmergency->caseID) {
                current = current->next;
            }
            newEmergency->next = current->next;
            current->next = newEmergency;
        }
    }
    file.close();
    cout << "Emergencies data loaded successfully.\n";
}

//..................................REPORTING AND STATISTICS.......................................
void generatePatientStatistics() {
    int totalPatients = 0;
    int admittedPatients = 0;
    map<string, int> diseaseCounts;
    map<char, int> genderCounts;

    Patient* current = patientHead;
    while (current) {
        totalPatients++;
        if (current->isAdmitted) {
            admittedPatients++;
        }
        diseaseCounts[current->disease]++;
        genderCounts[current->gender]++;
        current = current->next;
    }

    cout << "\n--- Patient Statistics ---\n";
    cout << "Total Patients: " << totalPatients << endl;
    cout << "Patients Currently Admitted: " << admittedPatients << endl;
    
    cout << "Patients by Gender:\n";
    for (const auto& pair : genderCounts) {
        cout << "  " << pair.first << ": " << pair.second << endl;
    }
    
    cout << "Patients by Disease:\n";
    if (diseaseCounts.empty()) {
        cout << "  No disease data.\n";
    } else {
        for (const auto& pair : diseaseCounts) {
            cout << "  " << pair.first << ": " << pair.second << endl;
        }
    }
}

void generateDoctorStatistics() {
    int totalDoctors = 0;
    map<string, int> specializationCounts;
    map<string, int> doctorAppointmentCounts; // Doctor Name (ID) -> Count of appointments

    // Collect doctor specializations and initialize appointment counts
    std::function<void(Doctor*)> traverseDocs =
        [&](Doctor* node) {
        if (!node) return;
        totalDoctors++;
        specializationCounts[node->specialization]++;
        doctorAppointmentCounts[node->name + " (ID:" + to_string(node->doctorID) + ")"] = 0;
        traverseDocs(node->left);
        traverseDocs(node->right);
    };
    traverseDocs(docRoot);

    // Count appointments per doctor (only scheduled/rescheduled ones)
    Appointment* currentApp = appointmentHead;
    while (currentApp) {
        if (currentApp->status == "Scheduled" || currentApp->status == "Rescheduled") {
            string doctorKey = currentApp->doctorName + " (ID:" + to_string(currentApp->doctorID) + ")";
            // Ensure the doctor exists in the map (might not if doctor was deleted but app record remains)
            if (doctorAppointmentCounts.count(doctorKey)) {
                doctorAppointmentCounts[doctorKey]++;
            } else {
                // If doctor not in map (e.g., deleted doctor with old appointments), add them
                doctorAppointmentCounts[doctorKey] = 1;
            }
        }
        currentApp = currentApp->next;
    }

    cout << "\n--- Doctor Statistics ---\n";
    cout << "Total Doctors: " << totalDoctors << endl;
    cout << "Doctors by Specialization:\n";
    if (specializationCounts.empty()) {
        cout << "  No specialization data.\n";
    } else {
        for (const auto& pair : specializationCounts) {
            cout << "  " << pair.first << ": " << pair.second << endl;
        }
    }
    cout << "Appointments per Doctor (Scheduled/Rescheduled):\n";
    if (doctorAppointmentCounts.empty()) {
        cout << "  No appointments recorded.\n";
    } else {
        for (const auto& pair : doctorAppointmentCounts) {
            cout << "  " << pair.first << ": " << pair.second << endl;
        }
    }
}

void generateAppointmentStatistics() {
    int totalAppointments = 0;
    int scheduledAppointments = 0;
    int completedAppointments = 0;
    int cancelledAppointments = 0;
    int rescheduledAppointments = 0;
    map<string, int> dailyAppointments; // Date -> Count

    Appointment* current = appointmentHead;
    while (current) {
        totalAppointments++;
        if (current->status == "Scheduled") {
            scheduledAppointments++;
        } else if (current->status == "Completed") {
            completedAppointments++;
        } else if (current->status == "Cancelled") {
            cancelledAppointments++;
        } else if (current->status == "Rescheduled") {
            rescheduledAppointments++;
        }
        dailyAppointments[current->date]++;
        current = current->next;
    }

    cout << "\n--- Appointment Statistics ---\n";
    cout << "Total Appointments: " << totalAppointments << endl;
    cout << "Scheduled Appointments: " << scheduledAppointments << endl;
    cout << "Completed Appointments: " << completedAppointments << endl;
    cout << "Cancelled Appointments: " << cancelledAppointments << endl;
    cout << "Rescheduled Appointments: " << rescheduledAppointments << endl;
    cout << "Appointments by Date:\n";
    if (dailyAppointments.empty()) {
        cout << "  No appointment dates recorded.\n";
    } else {
        for (const auto& pair : dailyAppointments) {
            cout << "  " << pair.first << ": " << pair.second << endl;
        }
    }
}

void generateResourceUsageReport() {
    int totalResources = 0;
    int availableResources = 0;
    map<string, int> resourceTypeCounts;
    map<string, int> availableResourceTypeCounts;

    Resource* current = resourceHead;
    while (current) {
        totalResources++;
        if (current->isAvailable) {
            availableResources++;
            availableResourceTypeCounts[current->type]++;
        }
        resourceTypeCounts[current->type]++;
        current = current->next;
    }

    cout << "\n--- Resource Usage Report ---\n";
    cout << "Total Resources: " << totalResources << endl;
    cout << "Currently Available Resources: " << availableResources << endl;
    cout << "Resources by Type (Total / Available):\n";
    if (resourceTypeCounts.empty()) {
        cout << "  No resource types recorded.\n";
    } else {
        for (const auto& pair : resourceTypeCounts) {
            cout << "  " << pair.first << ": " << pair.second << " / "
                 << (availableResourceTypeCounts.count(pair.first) ? availableResourceTypeCounts[pair.first] : 0) << endl;
        }
    }
}

//..................................ADVANCED TOOLS (MENU) ........................................
void advancedToolsMenu() {
    int choice;
    do {
        cout << "\n--- Advanced Tools ---\n";
        cout << "1. Search for a Doctor by Specialization\n";
        cout << "2. Search for Patients by Disease\n";
        cout << "3. Back to Main Menu\n";
        cout << "Enter your choice: ";
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear buffer

        switch (choice) {
            case 1: {
                string spec = getValidatedText("Enter Specialization to search: ");
                vector<Doctor*> foundDocs;
                collectDoctorsBySpecialization(docRoot, spec, foundDocs);
                if (foundDocs.empty()) {
                    cout << "No doctors found with specialization: " << spec << endl;
                } else {
                    cout << "\nDoctors with specialization '" << spec << "':\n";
                    cout << left << setw(5) << "ID" << setw(20) << "Name"
                         << setw(15) << "Experience" << setw(10) << "Max Appts" << endl;
                    cout << string(50, '-') << endl;
                    for (Doctor* doc : foundDocs) {
                        cout << left << setw(5) << doc->doctorID << setw(20) << doc->name
                             << setw(15) << doc->experienceYears << setw(10) << doc->maxAppointmentsPerDay << endl;
                    }
                    cout << string(50, '-') << endl;
                }
                break;
            }
            case 2: {
                string disease = getValidatedText("Enter Disease to search patients for: ");
                bool found = false;
                Patient* current = patientHead;
                cout << "\nPatients with disease '" << disease << "':\n";
                cout << left << setw(5) << "ID" << setw(20) << "Name" << setw(20) << "Doctor Assigned" << endl;
                cout << string(45, '-') << endl;
                while (current) {
                    if (toLower(current->disease) == toLower(disease)) {
                        cout << left << setw(5) << current->patientID << setw(20) << current->name
                             << setw(20) << current->doctorAssigned << endl;
                        found = true;
                    }
                    current = current->next;
                }
                if (!found) {
                    cout << "No patients found with disease: " << disease << endl;
                }
                cout << string(45, '-') << endl;
                break;
            }
            case 3: break;
            default: cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 3);
}

//..................................SAVE/LOAD ALL DATA...........................................
void saveAllData() {
    savePatients();
    saveDoctors();
    saveResources();
    saveAppointments();
    saveEmergencies();
    cout << "\nAll data saved successfully.\n";
}

void loadAllData() {
    loadPatients();
    loadDoctors();
    loadResources();
    loadAppointments();
    loadEmergencies();
    cout << "\nAll data loaded successfully.\n";
}


//..................................MAIN MENU....................................................
int main() {
    loadAllData(); // Load all data at program start

    int choice;
    do {
        cout << "\n--- Hospital Management System ---\n";
        cout << "1. Patient Management\n";
        cout << "2. Doctor Management\n";
        cout << "3. Resource Management\n";
        cout << "4. Appointment Management\n";
        cout << "5. Emergency Management\n";
        cout << "6. Reports and Statistics\n";
        cout << "7. Advanced Tools\n";
        cout << "8. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear buffer

        switch (choice) {
            case 1: {
                int pChoice;
                do {
                    cout << "\n--- Patient Management ---\n";
                    cout << "1. Add New Patient\n";
                    cout << "2. Display All Patients\n";
                    cout << "3. Search Patient by ID\n";
                    cout << "4. Update Patient Info\n";
                    cout << "5. Delete Patient\n";
                    cout << "6. Back to Main Menu\n";
                    cout << "Enter your choice: ";
                    cin >> pChoice;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    switch (pChoice) {
                        case 1: addPatient(); break;
                        case 2: displayPatients(); break;
                        case 3: {
                            int id = getValidatedInt("Enter Patient ID to search: ");
                            Patient* p = searchPatientByID(id);
                            if (p) {
                                cout << "\n--- Patient Found ---\n";
                                cout << "ID: " << p->patientID << "\nName: " << p->name
                                     << "\nAge: " << p->age << "\nGender: " << p->gender
                                     << "\nAddress: " << p->address << "\nContact: " << p->contactNumber
                                     << "\nDisease: " << p->disease << "\nDoctor Assigned: " << p->doctorAssigned
                                     << "\nAdmission Date: " << p->admissionDate << "\nAdmitted: " << (p->isAdmitted ? "Yes" : "No") << endl;
                            } else {
                                cout << "Patient not found.\n";
                            }
                            break;
                        }
                        case 4: updatePatient(); break;
                        case 5: deletePatient(); break;
                        case 6: break;
                        default: cout << "Invalid choice. Try again.\n";
                    }
                } while (pChoice != 6);
                break;
            }

            case 2: {
                int dChoice;
                do {
                    cout << "\n--- Doctor Management ---\n";
                    cout << "1. Add New Doctor\n";
                    cout << "2. Display All Doctors\n";
                    cout << "3. Search Doctor by ID\n";
                    cout << "4. Update Doctor Info\n";
                    cout << "5. Delete Doctor\n";
                    cout << "6. Back to Main Menu\n";
                    cout << "Enter your choice: ";
                    cin >> dChoice;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    switch (dChoice) {
                        case 1: addDoctor(); break;
                        case 2: displayDoctors(); break;
                        case 3: {
                            int id = getValidatedInt("Enter Doctor ID to search: ");
                            Doctor* d = searchDoctorByID(docRoot, id);
                            if (d) {
                                cout << "\n--- Doctor Found ---\n";
                                cout << "ID: " << d->doctorID << "\nName: " << d->name
                                     << "\nSpecialization: " << d->specialization
                                     << "\nContact: " << d->contactNumber << "\nEmail: " << d->email
                                     << "\nExperience: " << d->experienceYears << " years"
                                     << "\nMax Appointments/Day: " << d->maxAppointmentsPerDay << endl;
                            } else {
                                cout << "Doctor not found.\n";
                            }
                            break;
                        }
                        case 4: updateDoctor(); break;
                        case 5: deleteDoctor(); break;
                        case 6: break;
                        default: cout << "Invalid choice. Try again.\n";
                    }
                } while (dChoice != 6);
                break;
            }

            case 3: {
                int rChoice;
                do {
                    cout << "\n--- Resource Management ---\n";
                    cout << "1. Add New Resource\n";
                    cout << "2. Display All Resources\n";
                    cout << "3. Search Resource by ID\n";
                    cout << "4. Update Resource Info\n";
                    cout << "5. Delete Resource\n";
                    cout << "6. Back to Main Menu\n";
                    cout << "Enter your choice: ";
                    cin >> rChoice;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    switch (rChoice) {
                        case 1: addResource(); break;
                        case 2: displayResources(); break;
                        case 3: {
                            int id = getValidatedInt("Enter Resource ID to search: ");
                            Resource* r = searchResourceByID(id);
                            if (r) {
                                cout << "\n--- Resource Found ---\n";
                                cout << "ID: " << r->resourceID << "\nName: " << r->name
                                     << "\nType: " << r->type << "\nAvailable: " << (r->isAvailable ? "Yes" : "No") << endl;
                            } else {
                                cout << "Resource not found.\n";
                            }
                            break;
                        }
                        case 4: updateResource(); break;
                        case 5: deleteResource(); break;
                        case 6: break;
                        default: cout << "Invalid choice. Try again.\n";
                    }
                } while (rChoice != 6);
                break;
            }

            case 4: {
                int aChoice;
                do {
                    cout << "\n--- Appointment Management ---\n";
                    cout << "1. Add New Appointment\n";
                    cout << "2. Display All Appointments\n";
                    cout << "3. Search Appointment by ID\n";
                    cout << "4. Update Appointment Info\n";
                    cout << "5. Delete Appointment\n";
                    cout << "6. Back to Main Menu\n";
                    cout << "Enter your choice: ";
                    cin >> aChoice;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    switch (aChoice) {
                        case 1: addAppointment(); break;
                        case 2: displayAppointments(); break;
                        case 3: {
                            int id = getValidatedInt("Enter Appointment ID to search: ");
                            Appointment* app = searchAppointmentByID(id);
                            if (app) {
                                cout << "\n--- Appointment Found ---\n";
                                cout << "ID: " << app->appointmentID << "\nPatient: " << app->patientName << " (ID: " << app->patientID << ")"
                                     << "\nDoctor: " << app->doctorName << " (ID: " << app->doctorID << ")"
                                     << "\nDate: " << app->date << "\nTime: " << app->time
                                     << "\nReason: " << app->reason << "\nStatus: " << app->status << endl;
                            } else {
                                cout << "Appointment not found.\n";
                            }
                            break;
                        }
                        case 4: updateAppointment(); break;
                        case 5: deleteAppointment(); break;
                        case 6: break;
                        default: cout << "Invalid choice. Try again.\n";
                    }
                } while (aChoice != 6);
                break;
            }

            case 5: {
                int eChoice;
                do {
                    cout << "\n--- Emergency Management ---\n";
                    cout << "1. Log New Emergency\n";
                    cout << "2. Dispatch Emergency\n";
                    cout << "3. View All Emergencies\n";
                    cout << "4. Resolve by Case ID\n";
                    cout << "5. Back to Main Menu\n";
                    cout << "Enter your choice: ";
                    cin >> eChoice;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    switch (eChoice) {
                        case 1: logEmergency(); break;
                        case 2: dispatchEmergency(); break;
                        case 3: viewAllEmergencies(); break;
                        case 4: {
                            int id = getValidatedInt("Enter Case ID: ");
                            resolveEmergencyByID(id);
                            break;
                        }
                        case 5: break;
                        default: cout << "Invalid choice. Try again.\n";
                    }
                } while (eChoice != 5);
                break;
            }

            case 6: {
                cout << "\n--- Reports and Statistics ---\n";
                generatePatientStatistics();
                generateDoctorStatistics();
                generateAppointmentStatistics();
                generateResourceUsageReport();
                break;
            }
            case 7: advancedToolsMenu(); break; 

            case 8: cout << "Exiting the program. All data will be saved. Thank you!\n"; break;

            default: cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 8); 
    saveAllData(); // Save all data before exiting
    return 0;
}