#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

class Transaction {
public:
  enum Type { DEPOSIT, WITHDRAWAL, LOAN, LOAN_REPAYMENT };

  Transaction(Type type, float amount) : type(type), amount(amount) {
    time(&timestamp); // Get current time
  }

  void saveToFile(ofstream &outFile) const {
    outFile << type << " " << amount << " " << timestamp << "\n";
  }

  void displayTransaction() const {
    cout << "Type: "
         << (type == DEPOSIT      ? "Deposit"
             : type == WITHDRAWAL ? "Withdrawal"
             : type == LOAN       ? "Loan"
                                  : "Loan Repayment")
         << ", Amount: " << amount << ", Date: " << ctime(&timestamp);
  }

private:
  Type type;
  float amount;
  time_t timestamp;
};

class Loan {
public:
  enum Type { HOME, CAR, GOLD };

  Loan(Type type, float amount, float interestRate, bool repaid = false)
      : type(type), amount(amount), interestRate(interestRate), repaid(repaid) {
  }

  float calculateTotalRepayment() const {
    return amount + (amount * interestRate / 100);
  }

  Type getLoanType() const { return type; }

  void repay() { repaid = true; }

  bool isRepaid() const { return repaid; }

  void displayLoan() const {
    cout << "Loan Type: "
         << (type == HOME  ? "Home"
             : type == CAR ? "Car"
                           : "Gold")
         << ", Amount: " << amount
         << ", Total Repayment: " << calculateTotalRepayment()
         << (repaid ? ", Status: Repaid" : ", Status: Pending") << endl;
  }

private:
  Type type;
  float amount;
  float interestRate;
  bool repaid;
};

class Account {
  int accountNumber;
  string accountHolderName;
  float balance;
  string password;  // Store the user's password
  string accountType;
  vector<Loan> loans;

public:
  void createAccount() {
    cout << "\nEnter Account Number: ";
    cin >> accountNumber;
    cout << "Enter Account Holder Name: ";
    cin.ignore();
    getline(cin, accountHolderName);
    cout << "Enter Initial Balance: ";
    cin >> balance;
    cout << "Enter Account Type (Savings/Current): ";
    cin >> accountType;
    cout << "Set Account Password: ";  // Collect user password
    cin >> password;
    cout << "\nAccount Created Successfully!\n";
  }
  
  bool verifyPassword(const string &enteredPassword) const {
    return password == enteredPassword;
  }

void displayAccount() const {
    cout << left << setw(15) << "Account No:" << accountNumber << endl;
    cout << left << setw(15) << "Holder Name:" << accountHolderName << endl;
    cout << left << setw(15) << "Balance:" << balance << endl;
    cout << left << setw(15) << "Account Type:" << accountType << endl;

    if (loans.empty()) {
        cout << "\nNo loans associated with this account.\n";
    } else {
        cout << "\n=== Loans ===\n";
        for (const auto &loan : loans) {
            loan.displayLoan();
        }
    }
}


  int getAccountNumber() const { return accountNumber; }

  float getBalance() const { return balance; }

  void updateBalance(float amount) { balance += amount; }

  void saveToFile(ofstream &outFile) const {
    outFile << accountNumber << "\n"
            << accountHolderName << "\n"
            << balance << "\n"
            << accountType << "\n"
            << password << "\n";  // Save password to file

    outFile << loans.size() << "\n";
    for (const auto &loan : loans) {
      outFile << static_cast<int>(loan.getLoanType()) << "\n";
      outFile << loan.calculateTotalRepayment() << "\n";
      outFile << loan.isRepaid() << "\n"; // Save the repaid status
    }
  }

  void loadFromFile(ifstream &inFile) {
    string line;

    getline(inFile, line);
    accountNumber = atoi(line.c_str());
    getline(inFile, accountHolderName);
    getline(inFile, line);
    balance = atof(line.c_str());
    getline(inFile, accountType);
    getline(inFile, password);  // Load password

    size_t loanCount;
    getline(inFile, line);
    loanCount = atoi(line.c_str());
    loans.clear();
    for (size_t i = 0; i < loanCount; ++i) {
      int loanType;
      getline(inFile, line);
      loanType = atoi(line.c_str());

      float amount;
      getline(inFile, line);
      amount = atof(line.c_str());

      bool repaid;
      getline(inFile, line);
      repaid = atoi(line.c_str()) != 0; // Load repaid status

      loans.emplace_back(static_cast<Loan::Type>(loanType), amount, 5.0,
                         repaid);
    }
  }

  void applyForLoan(Loan::Type type, float amount) {
    float interestRate = 5.0;
    Loan newLoan(type, amount, interestRate);
    loans.push_back(newLoan);
    updateBalance(amount);
    Transaction(Transaction::LOAN, amount);
  }

  void repayLoan(int loanIndex) {
    if (loanIndex >= 0 && loanIndex < loans.size()) {
      float totalRepayment = loans[loanIndex].calculateTotalRepayment();
      if (balance >= totalRepayment) {
        balance -= totalRepayment;
        loans[loanIndex].repay();
        Transaction(Transaction::LOAN_REPAYMENT, totalRepayment);
        cout << "\nLoan Repayment Successful!\n";
      } else {
        cout << "\nInsufficient Balance for Repayment!\n";
      }
    } else {
      cout << "\nInvalid Loan Index!\n";
    }
  }
};

// Function to log transactions
void logTransaction(Transaction::Type type, float amount) {
  ofstream transFile("transactions.txt", ios::app);
  Transaction trans(type, amount);
  trans.saveToFile(transFile);
  transFile.close();
}

// Function to add a new account to the database
void addAccount() {
  Account acc;
  acc.createAccount();
  ofstream outFile("accounts.txt", ios::app);
  acc.saveToFile(outFile);
  outFile.close();
}

// Function to display all accounts
void displayAllAccounts() {
  string adminPassword;
  cout << "\nEnter Admin Password: ";
  cin >> adminPassword;

  if (adminPassword != "1212") {
    cout << "\nIncorrect Admin Password!\n";
    return;
  }

  ifstream inFile("accounts.txt");
  if (!inFile) {
    cout << "\nNo accounts found!\n";
    return;
  }
  Account acc;
  cout << "\n=== Account Details ===\n";
  while (inFile.peek() != EOF) {
    acc.loadFromFile(inFile);
    acc.displayAccount();
    cout << "----------------------\n";
  }
  inFile.close();
}

// Function to search for an account by account number
void searchAccount(int accNo) {
  ifstream inFile("accounts.txt");
  Account acc;
  bool found = false;

  while (inFile.peek() != EOF) {
    acc.loadFromFile(inFile);
    if (acc.getAccountNumber() == accNo) {
      string enteredPassword;
      cout << "Enter Account Password: ";
      cin >> enteredPassword;

      if (acc.verifyPassword(enteredPassword)) {
        cout << "\n=== Account Found ===\n";
        acc.displayAccount();
      } else {
        cout << "\nIncorrect Password!\n";
      }
      found = true;
      break;
    }
  }
  inFile.close();
  if (!found) {
    cout << "\nAccount not found!\n";
  }
}


// Function to deposit money into an account
void depositMoney(int accNo, float amount) {
  ifstream inFile("accounts.txt");
  ofstream tempFile("temp.txt");
  Account acc;
  bool found = false;
  while (inFile.peek() != EOF) {
    acc.loadFromFile(inFile);
    if (acc.getAccountNumber() == accNo) {
      acc.updateBalance(amount);
      logTransaction(Transaction::DEPOSIT, amount);
      cout << "\nAmount Deposited Successfully!\n";
      found = true;
    }
    acc.saveToFile(tempFile);
  }
  inFile.close();
  tempFile.close();
  remove("accounts.txt");
  rename("temp.txt", "accounts.txt");
  if (!found) {
    cout << "\nAccount not found!\n";
  }
}

// Function to withdraw money from an account
void withdrawMoney(int accNo, float amount) {
  ifstream inFile("accounts.txt");
  ofstream tempFile("temp.txt");
  Account acc;
  bool found = false;
  while (inFile.peek() != EOF) {
    acc.loadFromFile(inFile);
    if (acc.getAccountNumber() == accNo) {
      if (amount <= acc.getBalance()) {
        acc.updateBalance(-amount);
        logTransaction(Transaction::WITHDRAWAL, amount);
        cout << "\nAmount Withdrawn Successfully!\n";
      } else {
        cout << "\nInsufficient Balance!\n";
      }
      found = true;
    }
    acc.saveToFile(tempFile);
  }
  inFile.close();
  tempFile.close();
  remove("accounts.txt");
  rename("temp.txt", "accounts.txt");
  if (!found) {
    cout << "\nAccount not found!\n";
  }
}

// Function to delete an account
void deleteAccount(int accNo) {
  ifstream inFile("accounts.txt");
  ofstream tempFile("temp.txt");
  Account acc;
  bool found = false;
  while (inFile.peek() != EOF) {
    acc.loadFromFile(inFile);
    if (acc.getAccountNumber() == accNo) {
      found = true;
      cout << "\nAccount Deleted Successfully!\n";
      continue;
    }
    acc.saveToFile(tempFile);
  }
  inFile.close();
  tempFile.close();
  remove("accounts.txt");
  rename("temp.txt", "accounts.txt");
  if (!found) {
    cout << "\nAccount not found!\n";
  }
}

// Function to display transaction history
void displayTransactionHistory() {
  ifstream transFile("transactions.txt");
  if (!transFile) {
    cout << "\nNo transaction history found!\n";
    return;
  }
  cout << "\n=== Transaction History ===\n";
  string line;
  while (getline(transFile, line)) {
    cout << line << endl;
  }
  transFile.close();
}

// Function to apply for a loan
void applyForLoan(int accNo) {
  ifstream inFile("accounts.txt");
  ofstream tempFile("temp.txt");
  Account acc;
  bool found = false;
  while (inFile.peek() != EOF) {
    acc.loadFromFile(inFile);
    if (acc.getAccountNumber() == accNo) {
      found = true;
      int loanType;
      float loanAmount;
      cout << "Select Loan Type (0: Home, 1: Car, 2: Gold): ";
      cin >> loanType;
      cout << "Enter Loan Amount: ";
      cin >> loanAmount;

      acc.applyForLoan(static_cast<Loan::Type>(loanType), loanAmount);
      cout << "\nLoan Applied Successfully!\n";
    }
    acc.saveToFile(tempFile);
  }
  inFile.close();
  tempFile.close();
  remove("accounts.txt");
  rename("temp.txt", "accounts.txt");
  if (!found) {
    cout << "\nAccount not found!\n";
  }
}

// Function to repay a loan
void repayLoan(int accNo) {
  ifstream inFile("accounts.txt");
  ofstream tempFile("temp.txt");
  Account acc;
  bool found = false;
  while (inFile.peek() != EOF) {
    acc.loadFromFile(inFile);
    if (acc.getAccountNumber() == accNo) {
      found = true;
      int loanIndex;
      cout << "Enter Loan Index to Repay (0 for first loan, etc.): ";
      cin >> loanIndex;

      acc.repayLoan(loanIndex);
      cout << "\nRepayment Processed!\n";
    }
    acc.saveToFile(tempFile);
  }
  inFile.close();
  tempFile.close();
  remove("accounts.txt");
  rename("temp.txt", "accounts.txt");
  if (!found) {
    cout << "\nAccount not found!\n";
  }
}

// Function to transfer funds between two accounts
void transferFunds(int fromAccNo, int toAccNo, float amount) {
    ifstream inFile("accounts.txt");
    ofstream tempFile("temp.txt");
    Account fromAcc, toAcc;
    bool fromFound = false, toFound = false;

    while (inFile.peek() != EOF) {
        fromAcc.loadFromFile(inFile);
        if (fromAcc.getAccountNumber() == fromAccNo) {
            fromFound = true;
            if (amount <= fromAcc.getBalance()) {
                fromAcc.updateBalance(-amount);  // Deduct amount from source account
            } else {
                cout << "\nInsufficient Balance in source account!\n";
                inFile.close();
                tempFile.close();
                return; // Exit the function if insufficient balance
            }
        }

        // Store the account in temp file if it's not the source account
        if (fromAcc.getAccountNumber() != fromAccNo) {
            toAcc = fromAcc; // Store other accounts
            tempFile.write(reinterpret_cast<const char*>(&toAcc), sizeof(toAcc));
        }
    }
    
    inFile.close();
    tempFile.close();
    
    // Now check if the destination account exists and update balance
    inFile.open("accounts.txt");
    tempFile.open("temp.txt", ios::app);  // Open in append mode for destination account
    while (inFile.peek() != EOF) {
        toAcc.loadFromFile(inFile);
        if (toAcc.getAccountNumber() == toAccNo) {
            toFound = true;
            toAcc.updateBalance(amount);  // Add amount to destination account
        }
        toAcc.saveToFile(tempFile); // Save back to temp file
    }
    
    inFile.close();
    tempFile.close();
    
    remove("accounts.txt");
    rename("temp.txt", "accounts.txt");
    
    if (fromFound && toFound) {
        logTransaction(Transaction::WITHDRAWAL, amount);  // Log withdrawal from source account
        logTransaction(Transaction::DEPOSIT, amount);     // Log deposit to destination account
        cout << "\nTransfer Completed Successfully!\n";
    } else {
        cout << "\nOne of the accounts was not found!\n";
    }
} 

// Main menu function
int menu() {
  int choice, accNo, toAccNo;
  float amount;
  do {
    cout << "\n==== Bank Management System ====\n";
    cout << "1. Add New Account\n";
    cout << "2. Display All Accounts\n";
    cout << "3. Search for an Account\n";
    cout << "4. Deposit Money\n";
    cout << "5. Withdraw Money\n";
    cout << "6. Delete Account\n";
    cout << "7. Display Transaction History\n";
    cout << "8. Apply for a Loan\n";
    cout << "9. Repay a Loan\n";
    cout << "10. Transfer Funds\n"; 
    cout << "11. Exit\n";
    cout << "Enter your choice: ";
    cin >> choice;

    switch (choice) {
    case 1:
      addAccount();
      break;
    case 2:
      displayAllAccounts();
      break;
    case 3:
      cout << "\nEnter Account Number: ";
      cin >> accNo;
      searchAccount(accNo);
      break;
    case 4:
      cout << "\nEnter Account Number: ";
      cin >> accNo;
      cout << "Enter Amount to Deposit: ";
      cin >> amount;
      depositMoney(accNo, amount);
      break;
    case 5:
      cout << "\nEnter Account Number: ";
      cin >> accNo;
      cout << "Enter Amount to Withdraw: ";
      cin >> amount;
      withdrawMoney(accNo, amount);
      break;
    case 6:
      cout << "\nEnter Account Number to Delete: ";
      cin >> accNo;
      deleteAccount(accNo);
      break;
    case 7:
      displayTransactionHistory();
      break;
    case 8:
      cout << "\nEnter Account Number to Apply for Loan: ";
      cin >> accNo;
      applyForLoan(accNo);
      break;
    case 9:
      cout << "\nEnter Account Number to Repay Loan: ";
      cin >> accNo;
      repayLoan(accNo);
      break;
    case 10:  // Transfer funds case
        cout << "\nEnter Source Account Number: ";
        cin >> accNo;
        cout << "Enter Destination Account Number: ";
        cin >> toAccNo;
        cout << "Enter Amount to Transfer: ";
        cin >> amount;
        transferFunds(accNo, toAccNo, amount);
        break;
    case 11:
      cout << "\nThank you for using the Bank Management System!\n";
      return 0;
      break;
    default:
      cout << "\nInvalid choice! Please try again.\n";
    }
  } while (choice != 10);
}

int main() {
  menu();
  return 0;
}
