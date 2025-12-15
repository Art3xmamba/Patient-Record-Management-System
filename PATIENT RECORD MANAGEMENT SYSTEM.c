#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_USERS 100
#define MAX_PATIENTS 1000
#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 50
#define MAX_NAME_LEN 128
#define MAX_GUARDIAN_LEN 128
#define MAX_PHONE_LEN 16
#define MAX_ADDRESS_LEN 256
#define MAX_DISEASE_LEN 200
#define MAX_DOCTOR_LEN 100
#define MAX_BLOOD_GROUP_LEN 8
#define SCREEN_WIDTH 80
#define HEADER_WIDTH 40

#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_MAROON  "\033[38;5;88m"
#define COLOR_RESET   "\033[0m"

typedef enum {
    ROLE_ADMIN,
    ROLE_MODERATOR
} UserRole;

typedef struct {
    int user_id;
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    UserRole role;
    int is_active;
} User;

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    char guardian[MAX_GUARDIAN_LEN];
    char gender[10];
    int age;
    char blood_group[MAX_BLOOD_GROUP_LEN];
    char phone[MAX_PHONE_LEN];
    char address[MAX_ADDRESS_LEN];
    char disease[MAX_DISEASE_LEN];
    char referred_doctor[MAX_DOCTOR_LEN];
    char registration_date[20];
    int is_active;
} Patient;

User users[MAX_USERS];
Patient patients[MAX_PATIENTS];
int user_count = 0;
int patient_count = 0;
int next_user_id = 1;
int next_patient_id = 1;
User *current_user = NULL;

/* ===================== HELPER FUNCTIONS ===================== */

int read_line(char *buffer, int max_len) {
    if (fgets(buffer, max_len, stdin) == NULL) {
        return 0;
    }
    buffer[strcspn(buffer, "\n")] = '\0';
    return 1;
}

void trim(char *str) {
    int i = 0, j = 0;
    int len = (int)strlen(str);

    while (isspace((unsigned char)str[i])) i++;

    if (i > 0) {
        for (j = 0; i <= len; i++, j++) {
            str[j] = str[i];
        }
    }

    len = (int)strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[len - 1] = '\0';
        len--;
    }
}

int is_digits_only(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit((unsigned char)str[i])) {
            return 0;
        }
    }
    return 1;
}

int contains_pipe(const char *str) {
    return strchr(str, '|') != NULL;
}

void get_current_date(char *buffer) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buffer, 20, "%Y-%m-%d", tm_info);
}

void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void print_centered(const char *text) {
    int width = 80;
    int text_len = (int)strlen(text);
    int padding = (width - text_len) / 2;
    if (padding < 0) padding = 0;

    printf("%*s%s\n", padding, "", text);
}

void print_centered_title(const char *title) {
    int width = 80;
    int header_width = 40;
    int left_margin = (width - header_width) / 2;
    if (left_margin < 0) left_margin = 0;

    int title_len = (int)strlen(title);
    int inner_pad = (header_width - title_len) / 2;
    if (inner_pad < 0) inner_pad = 0;

    printf("\n%s%*s", COLOR_MAROON, left_margin, "");
    for (int i = 0; i < header_width; i++) printf("=");
    printf("\n");

    printf("%*s%*s%s\n", left_margin, "", inner_pad, "", title);

    printf("%*s", left_margin, "");
    for (int i = 0; i < header_width; i++) printf("=");
    printf("%s\n\n", COLOR_RESET);
}

int is_duplicate_patient(const char *name, const char *guardian, const char *phone) {
    for (int i = 0; i < patient_count; i++) {
        if (!patients[i].is_active) continue;

        char name1[MAX_NAME_LEN], name2[MAX_NAME_LEN];
        strcpy(name1, name);
        strcpy(name2, patients[i].name);
        for (char *p = name1; *p; p++) *p = (char)tolower((unsigned char)*p);
        for (char *p = name2; *p; p++) *p = (char)tolower((unsigned char)*p);

        char guard1[MAX_GUARDIAN_LEN], guard2[MAX_GUARDIAN_LEN];
        strcpy(guard1, guardian);
        strcpy(guard2, patients[i].guardian);
        for (char *p = guard1; *p; p++) *p = (char)tolower((unsigned char)*p);
        for (char *p = guard2; *p; p++) *p = (char)tolower((unsigned char)*p);

        if (strcmp(name1, name2) == 0 &&
            strcmp(guard1, guard2) == 0 &&
            strcmp(phone, patients[i].phone) == 0) {
            return patients[i].id;
        }
    }
    return 0;
}

/* ===================== FILE OPERATIONS ===================== */

int load_users() {
    FILE *file = fopen("users.txt", "r");
    if (!file) {
        return 0;
    }

    user_count = 0;
    next_user_id = 1;

    char line[512];
    while (fgets(line, sizeof(line), file) && user_count < MAX_USERS) {
        User *user = &users[user_count];
        int role_int;

        if (sscanf(line, "%d|%[^|]|%[^|]|%d|%d",
                   &user->user_id,
                   user->username,
                   user->password,
                   &role_int,
                   &user->is_active) == 5) {

            user->role = (role_int == 0) ? ROLE_ADMIN : ROLE_MODERATOR;

            if (user->user_id >= next_user_id) {
                next_user_id = user->user_id + 1;
            }

            user_count++;
        }
    }

    fclose(file);
    return 1;
}

int save_users() {
    FILE *file = fopen("users.txt", "w");
    if (!file) {
        perror("Error opening users.txt");
        return 0;
    }

    for (int i = 0; i < user_count; i++) {
        User *user = &users[i];
        fprintf(file, "%d|%s|%s|%d|%d\n",
                user->user_id,
                user->username,
                user->password,
                user->role,
                user->is_active);
    }

    fclose(file);
    return 1;
}

int load_patients() {
    FILE *file = fopen("patients.txt", "r");
    if (!file) {
        return 0;
    }

    patient_count = 0;
    next_patient_id = 1;

    char line[1024];
    while (fgets(line, sizeof(line), file) && patient_count < MAX_PATIENTS) {
        Patient *patient = &patients[patient_count];
        int temp_active;

        if (sscanf(line, "%d|%[^|]|%[^|]|%[^|]|%d|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%d",
                   &patient->id,
                   patient->name,
                   patient->guardian,
                   patient->gender,
                   &patient->age,
                   patient->blood_group,
                   patient->phone,
                   patient->address,
                   patient->disease,
                   patient->referred_doctor,
                   patient->registration_date,
                   &temp_active) == 12) {

            patient->is_active = temp_active;

            if (patient->id >= next_patient_id) {
                next_patient_id = patient->id + 1;
            }

            patient_count++;
        }
    }

    fclose(file);
    return 1;
}

int save_patients() {
    FILE *test_file = fopen("patients.txt", "a");
    if (!test_file) {
        perror("Error creating/opening patients.txt");
        return 0;
    }
    fclose(test_file);

    FILE *file = fopen("patients.txt", "w");
    if (!file) {
        perror("Error opening patients.txt for writing");
        return 0;
    }

    for (int i = 0; i < patient_count; i++) {
        Patient *patient = &patients[i];
        fprintf(file, "%d|%s|%s|%s|%d|%s|%s|%s|%s|%s|%s|%d\n",
                patient->id,
                patient->name,
                patient->guardian,
                patient->gender,
                patient->age,
                patient->blood_group,
                patient->phone,
                patient->address,
                patient->disease,
                patient->referred_doctor,
                patient->registration_date,
                patient->is_active);
    }

    fclose(file);
    return 1;
}

/* ===================== USER OPERATIONS ===================== */

User* find_user_by_username(const char *username) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0 && users[i].is_active) {
            return &users[i];
        }
    }
    return NULL;
}

int authenticate_user(const char *username, const char *password) {
    User *user = find_user_by_username(username);
    if (!user) {
        return 0;
    }
    return strcmp(user->password, password) == 0;
}

int count_active_admins() {
    int count = 0;
    for (int i = 0; i < user_count; i++) {
        if (users[i].is_active && users[i].role == ROLE_ADMIN) {
            count++;
        }
    }
    return count;
}

int register_user(const char *username, const char *password, UserRole role) {
    if (user_count >= MAX_USERS) {
        return 0;
    }

    if (find_user_by_username(username)) {
        return -1;
    }

    User *new_user = &users[user_count];
    new_user->user_id = next_user_id++;
    strncpy(new_user->username, username, MAX_USERNAME_LEN);
    new_user->username[MAX_USERNAME_LEN - 1] = '\0';
    strncpy(new_user->password, password, MAX_PASSWORD_LEN);
    new_user->password[MAX_PASSWORD_LEN - 1] = '\0';
    new_user->role = role;
    new_user->is_active = 1;

    user_count++;
    save_users();
    return 1;
}

/* ===================== PATIENT OPERATIONS ===================== */

int add_patient(Patient *patient) {
    if (patient_count >= MAX_PATIENTS) {
        return 0;
    }

    patient->id = next_patient_id++;
    patient->is_active = 1;

    patients[patient_count] = *patient;
    int old_count = patient_count;
    int old_next_id = next_patient_id - 1;
    patient_count++;

    if (!save_patients()) {
        patient_count = old_count;
        next_patient_id = old_next_id;
        return 0;
    }

    return 1;
}

int modify_patient(int patient_id, Patient *updated_patient) {
    for (int i = 0; i < patient_count; i++) {
        if (patients[i].id == patient_id && patients[i].is_active) {
            updated_patient->id = patient_id;
            updated_patient->is_active = 1;
            strcpy(updated_patient->registration_date, patients[i].registration_date);
            patients[i] = *updated_patient;
            return save_patients();
        }
    }
    return 0;
}

int delete_patient(int patient_id) {
    for (int i = 0; i < patient_count; i++) {
        if (patients[i].id == patient_id && patients[i].is_active) {
            patients[i].is_active = 0;
            return save_patients();
        }
    }
    return 0;
}

Patient* find_patient_by_id(int patient_id) {
    for (int i = 0; i < patient_count; i++) {
        if (patients[i].id == patient_id && patients[i].is_active) {
            return &patients[i];
        }
    }
    return NULL;
}

int find_patients_by_name(const char *search_name, int *result_indices, int max_results) {
    int found_count = 0;
    char search_lower[MAX_NAME_LEN];

    strcpy(search_lower, search_name);
    for (char *p = search_lower; *p; p++) *p = (char)tolower((unsigned char)*p);

    for (int i = 0; i < patient_count && found_count < max_results; i++) {
        if (!patients[i].is_active) continue;

        char patient_name_lower[MAX_NAME_LEN];
        strcpy(patient_name_lower, patients[i].name);
        for (char *p = patient_name_lower; *p; p++) *p = (char)tolower((unsigned char)*p);

        if (strstr(patient_name_lower, search_lower) != NULL) {
            result_indices[found_count++] = i;
        }
    }

    return found_count;
}

/* ===================== UI FUNCTIONS ===================== */

void show_startup_menu() {
    clear_screen();
    print_centered_title("PATIENT RECORD MANAGEMENT SYSTEM");

    printf("1) Login\n");
    printf("2) Exit\n\n");
    printf("Enter your choice: ");
}

void show_admin_menu() {
    clear_screen();
    print_centered_title("ADMIN DASHBOARD");

    printf("1. Add New Patient\n");
    printf("2. View All Patients\n");
    printf("3. Search Patient\n");
    printf("4. Modify Patient\n");
    printf("5. Delete Patient\n");
    printf("6. Register New User\n");
    printf("7. Logout\n\n");
    printf("Enter your choice: ");
}

void show_moderator_menu() {
    clear_screen();
    print_centered_title("MODERATOR DASHBOARD");

    printf("1. Add New Patient\n");
    printf("2. View All Patients\n");
    printf("3. Search Patient\n");
    printf("4. Logout\n\n");
    printf("Enter your choice: ");
}

/* ===================== PATIENT FORMS ===================== */

void add_patient_form() {
    Patient patient = {0};
    char input[256];

    clear_screen();
    print_centered_title("ADD NEW PATIENT");

    printf("Enter '0' For Go Back.\n\n");

    while (1) {
        printf("Patient Name: ");
        if (!read_line(input, sizeof(input))) continue;
        trim(input);

        if (strcmp(input, "0") == 0) {
            printf("Operation cancelled.\n");
            return;
        }

        if (strlen(input) == 0) {
            printf(COLOR_RED "Please enter a name.\n" COLOR_RESET);
            continue;
        }

        if (contains_pipe(input)) {
            printf(COLOR_RED "Name cannot contain '|' character.\n" COLOR_RESET);
            continue;
        }

        strcpy(patient.name, input);
        break;
    }

    while (1) {
        printf("Guardian Name: ");
        if (!read_line(input, sizeof(input))) continue;
        trim(input);

        if (strcmp(input, "0") == 0) {
            printf("Operation cancelled.\n");
            return;
        }

        if (strlen(input) == 0) {
            printf(COLOR_RED "Please enter guardian name.\n" COLOR_RESET);
            continue;
        }

        if (contains_pipe(input)) {
            printf(COLOR_RED "Guardian name cannot contain '|' character.\n" COLOR_RESET);
            continue;
        }

        strcpy(patient.guardian, input);
        break;
    }

    while (1) {
        printf("Enter gender (M=Male, F=Female): ");
        if (!read_line(input, sizeof(input))) continue;
        trim(input);

        if (strcmp(input, "0") == 0) {
            printf("Operation cancelled.\n");
            return;
        }

        if (strlen(input) == 0) {
            printf(COLOR_RED "Please select a gender.\n" COLOR_RESET);
            continue;
        }

        char gender = (char)toupper((unsigned char)input[0]);
        if (gender == 'M' || gender == 'F') {
            strcpy(patient.gender, (gender == 'M') ? "Male" : "Female");
            break;
        } else {
            printf(COLOR_RED "That doesn't look right. Please enter M or F.\n" COLOR_RESET);
        }
    }

    while (1) {
        printf("Age: ");
        if (!read_line(input, sizeof(input))) continue;
        trim(input);

        if (strcmp(input, "0") == 0) {
            printf("Operation cancelled.\n");
            return;
        }

        if (strlen(input) == 0) {
            printf(COLOR_RED "Please enter age.\n" COLOR_RESET);
            continue;
        }

        patient.age = atoi(input);
        if (patient.age <= 0 || patient.age > 150) {
            printf(COLOR_RED "Age must be between 1 and 150.\n" COLOR_RESET);
            continue;
        }
        break;
    }

    printf("\nSelect Blood Group:\n");
    printf("1) A+\n");
    printf("2) A-\n");
    printf("3) B+\n");
    printf("4) B-\n");
    printf("5) AB+\n");
    printf("6) AB-\n");
    printf("7) O+\n");
    printf("8) O-\n");

    while (1) {
        printf("Select Blood Group [1-8]: ");
        if (!read_line(input, sizeof(input))) continue;
        trim(input);

        if (strcmp(input, "0") == 0) {
            printf("Operation cancelled.\n");
            return;
        }

        if (strlen(input) == 0) {
            printf(COLOR_RED "Please select blood group.\n" COLOR_RESET);
            continue;
        }

        int choice = atoi(input);
        if (choice < 1 || choice > 8) {
            printf(COLOR_RED "Please select 1-8.\n" COLOR_RESET);
            continue;
        }

        switch (choice) {
            case 1: strcpy(patient.blood_group, "A+"); break;
            case 2: strcpy(patient.blood_group, "A-"); break;
            case 3: strcpy(patient.blood_group, "B+"); break;
            case 4: strcpy(patient.blood_group, "B-"); break;
            case 5: strcpy(patient.blood_group, "AB+"); break;
            case 6: strcpy(patient.blood_group, "AB-"); break;
            case 7: strcpy(patient.blood_group, "O+"); break;
            case 8: strcpy(patient.blood_group, "O-"); break;
        }
        break;
    }

    while (1) {
        printf("Phone (11 digits): ");
        if (!read_line(input, sizeof(input))) continue;
        trim(input);

        if (strcmp(input, "0") == 0) {
            printf("Operation cancelled.\n");
            return;
        }

        if (strlen(input) == 0) {
            printf(COLOR_RED "Phone number is required.\n" COLOR_RESET);
            continue;
        }

        if (strlen(input) != 11 || !is_digits_only(input)) {
            printf(COLOR_RED "Phone must be exactly 11 digits.\n" COLOR_RESET);
            continue;
        }

        strcpy(patient.phone, input);
        break;
    }

    int duplicate_id = is_duplicate_patient(patient.name, patient.guardian, patient.phone);
    if (duplicate_id) {
        printf(COLOR_RED "\n⚠️  Similar patient found!\n" COLOR_RESET);
        printf("Existing Patient ID: %d\n", duplicate_id);
        printf("Same name, guardian, and phone already exists.\n\n");

        while (1) {
            printf("1) Cancel (keep existing)\n");
            printf("2) Add anyway (new record)\n");
            printf("Choice [1]: ");

            if (!read_line(input, sizeof(input))) continue;
            trim(input);

            if (strlen(input) == 0 || strcmp(input, "1") == 0) {
                printf("Cancelled. Patient not added.\n");
                printf("\nPress Enter to continue...");
                getchar();
                return;
            }

            if (strcmp(input, "2") == 0) {
                printf("Proceeding with new patient...\n");
                break;
            }

            printf(COLOR_RED "Please enter 1 or 2.\n" COLOR_RESET);
        }
    }

    while (1) {
        printf("Address: ");
        if (!read_line(input, sizeof(input))) continue;
        trim(input);

        if (strcmp(input, "0") == 0) {
            printf("Operation cancelled.\n");
            return;
        }

        if (strlen(input) == 0) {
            printf(COLOR_RED "Address is required.\n" COLOR_RESET);
            continue;
        }

        if (contains_pipe(input)) {
            printf(COLOR_RED "Address cannot contain '|' character.\n" COLOR_RESET);
            continue;
        }

        strcpy(patient.address, input);
        break;
    }

    while (1) {
        printf("Disease: ");
        if (!read_line(input, sizeof(input))) continue;
        trim(input);

        if (strcmp(input, "0") == 0) {
            printf("Operation cancelled.\n");
            return;
        }

        if (strlen(input) == 0) {
            printf(COLOR_RED "Please enter disease.\n" COLOR_RESET);
            continue;
        }

        if (contains_pipe(input)) {
            printf(COLOR_RED "Disease cannot contain '|' character.\n" COLOR_RESET);
            continue;
        }

        strcpy(patient.disease, input);
        break;
    }

    while (1) {
        printf("Referred Doctor: ");
        if (!read_line(input, sizeof(input))) continue;
        trim(input);

        if (strcmp(input, "0") == 0) {
            printf("Operation cancelled.\n");
            return;
        }

        if (strlen(input) == 0) {
            printf(COLOR_RED "Please enter doctor's name.\n" COLOR_RESET);
            continue;
        }

        if (contains_pipe(input)) {
            printf(COLOR_RED "Doctor name cannot contain '|' character.\n" COLOR_RESET);
            continue;
        }

        strcpy(patient.referred_doctor, input);
        break;
    }

    get_current_date(patient.registration_date);

    if (add_patient(&patient)) {
        printf(COLOR_GREEN "\nPatient added successfully!\n" COLOR_RESET);
        printf("Patient ID: %d\n", patient.id);
        printf("Registration Date: %s \n", patient.registration_date);
    } else {
        printf(COLOR_RED "\nFailed to add patient.\n" COLOR_RESET);
    }

    printf("\nPress Enter to continue...");
    getchar();
}

void view_all_patients() {
    clear_screen();
    print_centered_title("ALL PATIENT RECORDS");

    int active_count = 0;
    for (int i = 0; i < patient_count; i++) {
        if (patients[i].is_active) active_count++;
    }

    if (active_count == 0) {
        printf("No patient records found.\n");
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }

    printf("S.No  ID    Name                Gender  Age  Phone       Disease\n");
    printf("----------------------------------------------------------------\n");

    int serial = 1;
    for (int i = 0; i < patient_count; i++) {
        if (!patients[i].is_active) continue;

        printf("%-5d %-5d %-20s %-7s %-4d %-11s %s\n",
               serial++, patients[i].id, patients[i].name, patients[i].gender,
               patients[i].age, patients[i].phone, patients[i].disease);

        if ((serial - 1) % 20 == 0 && (serial - 1) < active_count) {
            printf("\nPress Enter to continue...");
            getchar();
            clear_screen();
            print_centered_title("ALL PATIENT RECORDS (CONTINUED)");
            printf("S.No  ID    Name                Gender  Age  Phone       Disease\n");
            printf("----------------------------------------------------------------\n");
        }
    }

    printf("\nTotal patients: %d\n", serial - 1);
    printf("\nPress Enter to continue...");
    getchar();
}

void search_patient_menu() {
    char search[256];
    int patient_id;

    clear_screen();
    print_centered_title("SEARCH PATIENT");

    printf("Enter patient ID or name to search: ");
    if (!read_line(search, sizeof(search))) {
        return;
    }

    trim(search);

    if (strlen(search) == 0) {
        return;
    }

    if (is_digits_only(search)) {
        patient_id = atoi(search);
        Patient *patient = find_patient_by_id(patient_id);

        if (patient) {
            printf("\nPatient Found:\n");
            printf("ID: %d\n", patient->id);
            printf("Name: %s\n", patient->name);
            printf("Guardian: %s\n", patient->guardian);
            printf("Gender: %s\n", patient->gender);
            printf("Age: %d\n", patient->age);
            printf("Blood Group: %s\n", patient->blood_group);
            printf("Phone: %s\n", patient->phone);
            printf("Address: %s\n", patient->address);
            printf("Disease: %s\n", patient->disease);
            printf("Referred Doctor: %s\n", patient->referred_doctor);
            printf("Registration Date: %s\n", patient->registration_date);
        } else {
            printf("\nNo patient found with ID: %d\n", patient_id);
        }
    } else {
        int result_indices[100];
        int found_count = find_patients_by_name(search, result_indices, 100);

        if (found_count == 0) {
            printf("\nNo patients found with name containing: %s\n", search);
        } else if (found_count == 1) {
            Patient *patient = &patients[result_indices[0]];
            printf("\nPatient Found:\n");
            printf("ID: %d\n", patient->id);
            printf("Name: %s\n", patient->name);
            printf("Guardian: %s\n", patient->guardian);
            printf("Gender: %s\n", patient->gender);
            printf("Age: %d\n", patient->age);
            printf("Blood Group: %s\n", patient->blood_group);
            printf("Phone: %s\n", patient->phone);
            printf("Address: %s\n", patient->address);
            printf("Disease: %s\n", patient->disease);
            printf("Referred Doctor: %s\n", patient->referred_doctor);
            printf("Registration Date: %s\n", patient->registration_date);
        } else {
            printf("\nMultiple patients found:\n");
            printf("S.No  ID    Name\n");
            printf("----------------------------\n");

            for (int i = 0; i < found_count; i++) {
                Patient *patient = &patients[result_indices[i]];
                printf("%-5d %-5d %s\n", i + 1, patient->id, patient->name);
            }

            printf("\nEnter patient ID to view details: ");
            if (!read_line(search, sizeof(search))) {
                return;
            }

            trim(search);

            if (is_digits_only(search)) {
                patient_id = atoi(search);
                if (patient_id == 0) {
                    return;
                }

                Patient *patient = find_patient_by_id(patient_id);
                if (patient) {
                    printf("\nPatient Details:\n");
                    printf("ID: %d\n", patient->id);
                    printf("Name: %s\n", patient->name);
                    printf("Guardian: %s\n", patient->guardian);
                    printf("Gender: %s\n", patient->gender);
                    printf("Age: %d\n", patient->age);
                    printf("Blood Group: %s\n", patient->blood_group);
                    printf("Phone: %s\n", patient->phone);
                    printf("Address: %s\n", patient->address);
                    printf("Disease: %s\n", patient->disease);
                    printf("Referred Doctor: %s\n", patient->referred_doctor);
                    printf("Registration Date: %s\n", patient->registration_date);
                } else {
                    printf(COLOR_RED "\nPatient not found.\n" COLOR_RESET);
                }
            }
        }
    }

    printf("\nPress Enter to continue...");
    getchar();
}

void modify_patient_form() {
    int patient_id;
    Patient *patient;
    Patient updated_patient = {0};
    char input[256];

    clear_screen();
    print_centered_title("MODIFY PATIENT");

    printf("Enter Patient ID to modify: ");
    if (!read_line(input, sizeof(input))) return;

    patient_id = atoi(input);
    if (patient_id == 0) {
        return;
    }

    patient = find_patient_by_id(patient_id);
    if (!patient) {
        printf(COLOR_RED "Patient not found.\n" COLOR_RESET);
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }

    printf("\nCurrent Details:\n");
    printf("Name: %s\n", patient->name);
    printf("Leave field blank to keep current value.\n\n");

    while (1) {
        printf("Patient Name [%s]: ", patient->name);
        if (!read_line(input, sizeof(input))) break;
        trim(input);

        if (strcmp(input, "0") == 0) {
            return;
        }

        if (strlen(input) == 0) {
            strcpy(updated_patient.name, patient->name);
            break;
        }

        if (contains_pipe(input)) {
            printf(COLOR_RED "Name cannot contain '|' character.\n" COLOR_RESET);
            continue;
        }

        strcpy(updated_patient.name, input);
        break;
    }

    while (1) {
        printf("Guardian Name [%s]: ", patient->guardian);
        if (!read_line(input, sizeof(input))) break;
        trim(input);

        if (strcmp(input, "0") == 0) return;
        if (strlen(input) == 0) {
            strcpy(updated_patient.guardian, patient->guardian);
            break;
        }

        if (contains_pipe(input)) {
            printf(COLOR_RED "Guardian name cannot contain '|' character.\n" COLOR_RESET);
            continue;
        }

        strcpy(updated_patient.guardian, input);
        break;
    }

    while (1) {
        printf("Gender (M/F) [%s]: ", patient->gender);
        if (!read_line(input, sizeof(input))) break;
        trim(input);

        if (strcmp(input, "0") == 0) return;
        if (strlen(input) == 0) {
            strcpy(updated_patient.gender, patient->gender);
            break;
        }

        char gender = (char)toupper((unsigned char)input[0]);
        if (gender == 'M' || gender == 'F') {
            strcpy(updated_patient.gender, (gender == 'M') ? "Male" : "Female");
            break;
        } else {
            printf(COLOR_RED "Please enter M or F.\n" COLOR_RESET);
        }
    }

    while (1) {
        printf("Age [%d]: ", patient->age);
        if (!read_line(input, sizeof(input))) break;
        trim(input);

        if (strcmp(input, "0") == 0) return;
        if (strlen(input) == 0) {
            updated_patient.age = patient->age;
            break;
        }

        int age_input = atoi(input);
        if (age_input > 0 && age_input <= 150) {
            updated_patient.age = age_input;
            break;
        } else {
            printf(COLOR_RED "Age must be 1-150.\n" COLOR_RESET);
        }
    }

    while (1) {
        printf("Phone [%s]: ", patient->phone);
        if (!read_line(input, sizeof(input))) break;
        trim(input);

        if (strcmp(input, "0") == 0) return;
        if (strlen(input) == 0) {
            strcpy(updated_patient.phone, patient->phone);
            break;
        }

        if (strlen(input) == 11 && is_digits_only(input)) {
            strcpy(updated_patient.phone, input);
            break;
        } else {
            printf(COLOR_RED "Phone must be 11 digits.\n" COLOR_RESET);
        }
    }

    while (1) {
        printf("Address [%s]: ", patient->address);
        if (!read_line(input, sizeof(input))) break;
        trim(input);

        if (strcmp(input, "0") == 0) return;
        if (strlen(input) == 0) {
            strcpy(updated_patient.address, patient->address);
            break;
        }

        if (contains_pipe(input)) {
            printf(COLOR_RED "Address cannot contain '|' character.\n" COLOR_RESET);
            continue;
        }

        strcpy(updated_patient.address, input);
        break;
    }

    while (1) {
        printf("Disease [%s]: ", patient->disease);
        if (!read_line(input, sizeof(input))) break;
        trim(input);

        if (strcmp(input, "0") == 0) return;
        if (strlen(input) == 0) {
            strcpy(updated_patient.disease, patient->disease);
            break;
        }

        if (contains_pipe(input)) {
            printf(COLOR_RED "Disease cannot contain '|' character.\n" COLOR_RESET);
            continue;
        }

        strcpy(updated_patient.disease, input);
        break;
    }

    while (1) {
        printf("Referred Doctor [%s]: ", patient->referred_doctor);
        if (!read_line(input, sizeof(input))) break;
        trim(input);

        if (strcmp(input, "0") == 0) return;
        if (strlen(input) == 0) {
            strcpy(updated_patient.referred_doctor, patient->referred_doctor);
            break;
        }

        if (contains_pipe(input)) {
            printf(COLOR_RED "Doctor name cannot contain '|' character.\n" COLOR_RESET);
            continue;
        }

        strcpy(updated_patient.referred_doctor, input);
        break;
    }

    strcpy(updated_patient.blood_group, patient->blood_group);
    strcpy(updated_patient.registration_date, patient->registration_date);

    if (modify_patient(patient_id, &updated_patient)) {
        printf(COLOR_GREEN "\nPatient updated.\n" COLOR_RESET);
    } else {
        printf(COLOR_RED "\nUpdate failed.\n" COLOR_RESET);
    }

    printf("\nPress Enter to continue...");
    getchar();
}

void delete_patient_form() {
    int patient_id;
    char confirm[10];

    clear_screen();
    print_centered_title("DELETE PATIENT");

    printf("Enter Patient ID to delete: ");
    if (!read_line(confirm, sizeof(confirm))) return;

    patient_id = atoi(confirm);
    if (patient_id == 0) {
        return;
    }

    Patient *patient = find_patient_by_id(patient_id);
    if (!patient) {
        printf(COLOR_RED "Patient not found.\n" COLOR_RESET);
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }

    printf("\nPatient Details:\n");
    printf("ID: %d\n", patient->id);
    printf("Name: %s\n", patient->name);
    printf("Age: %d\n", patient->age);
    printf("Disease: %s\n", patient->disease);
    printf("Phone: %s\n", patient->phone);

    printf("\nAre you sure? (y/N): ");
    read_line(confirm, sizeof(confirm));
    trim(confirm);

    if (confirm[0] == 'y' || confirm[0] == 'Y') {
        if (delete_patient(patient_id)) {
            printf(COLOR_GREEN "\nPatient deleted.\n" COLOR_RESET);
        } else {
            printf(COLOR_RED "\nFailed to delete.\n" COLOR_RESET);
        }
    } else {
        printf("\nCancelled.\n");
    }

    printf("\nPress Enter to continue...");
    getchar();
}

/* ===================== USER MANAGEMENT ===================== */

void registration_flow(int is_first_user) {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char confirm_password[MAX_PASSWORD_LEN];
    UserRole role = ROLE_MODERATOR;

    clear_screen();
    print_centered_title("USER REGISTRATION");

    if (is_first_user) {
        printf("ADMIN REGISTRATION.\n\n");
        role = ROLE_ADMIN;
    } else if (current_user && current_user->role == ROLE_ADMIN) {
        printf("New User Registration\n\n");
    } else {
        printf("New users will be MODERATORs.\n\n");
    }

    while (1) {
        printf("Username: ");
        if (!read_line(username, MAX_USERNAME_LEN)) continue;
        trim(username);

        if (strlen(username) == 0) {
            printf(COLOR_RED "Username required.\n" COLOR_RESET);
            continue;
        }

        if (contains_pipe(username)) {
            printf(COLOR_RED "Username cannot contain '|'.\n" COLOR_RESET);
            continue;
        }

        if (find_user_by_username(username)) {
            printf(COLOR_RED "Username taken.\n" COLOR_RESET);
            continue;
        }

        break;
    }

    while (1) {
        printf("Password: ");
        if (!read_line(password, MAX_PASSWORD_LEN)) continue;
        trim(password);

        if (strlen(password) == 0) {
            printf(COLOR_RED "Password required.\n" COLOR_RESET);
            continue;
        }

        if (contains_pipe(password)) {
            printf(COLOR_RED "Password cannot contain '|'.\n" COLOR_RESET);
            continue;
        }

        break;
    }

    while (1) {
        printf("Confirm Password: ");
        if (!read_line(confirm_password, MAX_PASSWORD_LEN)) continue;
        trim(confirm_password);

        if (strcmp(password, confirm_password) != 0) {
            printf(COLOR_RED "Passwords don't match.\n" COLOR_RESET);

            printf("Password: ");
            if (!read_line(password, MAX_PASSWORD_LEN)) continue;
            trim(password);

            if (strlen(password) == 0) {
                printf(COLOR_RED "Password required.\n" COLOR_RESET);
                continue;
            }

            continue;
        }
        break;
    }

    if (!is_first_user && current_user && current_user->role == ROLE_ADMIN) {
        printf("\nRole (1=ADMIN, 2=MODERATOR): ");
        char role_input[10];
        read_line(role_input, sizeof(role_input));
        trim(role_input);

        if (atoi(role_input) == 1) {
            role = ROLE_ADMIN;
        }
    }

    int result = register_user(username, password, role);
    if (result == 1) {
        printf(COLOR_GREEN "\nRegistration successful.\n" COLOR_RESET);

        if (is_first_user) {
            current_user = find_user_by_username(username);
            printf("Automatically logged in as first user.\n");
        }
    } else if (result == -1) {
        printf(COLOR_RED "\nUsername already exists.\n" COLOR_RESET);
    } else {
        printf(COLOR_RED "\nRegistration failed.\n" COLOR_RESET);
    }

    printf("\nPress Enter to continue...");
    getchar();
}

void login_flow() {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];

    clear_screen();
    print_centered_title("LOGIN");

    printf("Username: ");
    read_line(username, MAX_USERNAME_LEN);
    trim(username);

    if (strlen(username) == 0) {
        printf(COLOR_RED "\nUsername required.\n" COLOR_RESET);
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }

    printf("Password: ");
    read_line(password, MAX_PASSWORD_LEN);
    trim(password);

    if (strlen(password) == 0) {
        printf(COLOR_RED "\nPassword required.\n" COLOR_RESET);
        printf("\nPress Enter to continue...");
        getchar();
        return;
    }

    if (authenticate_user(username, password)) {
        current_user = find_user_by_username(username);
        printf(COLOR_GREEN "\nWelcome, %s.\n" COLOR_RESET, current_user->username);
    } else {
        printf(COLOR_RED "\nInvalid Password or Username.\n" COLOR_RESET);
    }

    printf("\nPress Enter to continue...");
    getchar();
}

/* ===================== MAIN APPLICATION FLOW ===================== */

void admin_flow() {
    while (current_user && current_user->role == ROLE_ADMIN) {
        show_admin_menu();

        char choice_str[10];
        read_line(choice_str, sizeof(choice_str));
        int choice = atoi(choice_str);

        switch (choice) {
            case 1:
                add_patient_form();
                break;
            case 2:
                view_all_patients();
                break;
            case 3:
                search_patient_menu();
                break;
            case 4:
                modify_patient_form();
                break;
            case 5:
                delete_patient_form();
                break;
            case 6:
                registration_flow(0);
                break;
            case 7:
                current_user = NULL;
                return;
            default:
                printf(COLOR_RED "Invalid choice.\n" COLOR_RESET);
                printf("\nPress Enter to continue...");
                getchar();
                break;
        }
    }
}

void moderator_flow() {
    while (current_user && current_user->role == ROLE_MODERATOR) {
        show_moderator_menu();

        char choice_str[10];
        read_line(choice_str, sizeof(choice_str));
        int choice = atoi(choice_str);

        switch (choice) {
            case 1:
                add_patient_form();
                break;
            case 2:
                view_all_patients();
                break;
            case 3:
                search_patient_menu();
                break;
            case 4:
                current_user = NULL;
                return;
            default:
                printf(COLOR_RED "Invalid choice.\n" COLOR_RESET);
                printf("\nPress Enter to continue...");
                getchar();
                break;
        }
    }
}

int main() {
    load_users();
    load_patients();

    while (1) {
        if (current_user) {
            if (current_user->role == ROLE_ADMIN) {
                admin_flow();
            } else {
                moderator_flow();
            }
        } else {
            if (user_count == 0) {
                clear_screen();
                print_centered_title("PATIENT RECORD MANAGEMENT SYSTEM");
                printf("No users found. Please Register First User.\n\n");
                registration_flow(1);
            } else {
                show_startup_menu();

                char choice_str[10];
                read_line(choice_str, sizeof(choice_str));
                int choice = atoi(choice_str);

                switch (choice) {
                    case 1:
                        login_flow();
                        break;
                    case 2:
                        clear_screen();
                        printf("Thank you for using Patient Record Management System!\n");
                        return 0;
                    default:
                        printf(COLOR_RED "Invalid choice.\n" COLOR_RESET);
                        printf("\nPress Enter to continue...");
                        getchar();
                        break;
                }
            }
        }
    }

    return 0;
}
