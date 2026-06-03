#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <iomanip> 
using namespace std;

class Transaction {
public:
    string type;       // deposit, withdraw, transfer
    double amount;
    string timestamp;
    string remark;

    Transaction(string t, double amt, string rem = "") {
        type = t;
        amount = amt;
        remark = rem;
        time_t now = time(0);
        timestamp = ctime(&now);
        timestamp.pop_back(); // remove newline
    }

    void display() const {
        cout << left << setw(10) << type 
             << setw(10) << amount 
             << setw(25) << timestamp 
             << remark << endl;
    }
};

class Account {
private:
    int accountNumber;
    string name;
    string type;  // Savings / Current
    double balance;
    vector<Transaction> history;

public:
    Account() {}
    Account(int accNo, string accName, string accType, double initialBalance) {
        accountNumber = accNo;
        name = accName;
        type = accType;
        balance = initialBalance;
        history.push_back(Transaction("create", initialBalance, "Initial Deposit"));
    }

    int getAccNo() const { return accountNumber; }
    string getName() const { return name; }
    double getBalance() const { return balance; }

    void deposit(double amount) {
        balance += amount;
        history.push_back(Transaction("deposit", amount));
        cout << "✅ Deposited Rs. " << amount << " successfully.\n";
    }

    bool withdraw(double amount) {
        if (amount > balance) {
            cout << "❌ Insufficient balance!\n";
            return false;
        }
        balance -= amount;
        history.push_back(Transaction("withdraw", amount));
        cout << "✅ Withdrawn Rs. " << amount << " successfully.\n";
        return true;
    }

    bool transfer(Account &receiver, double amount) {
        if (amount > balance) {
            cout << "❌ Insufficient balance for transfer!\n";
            return false;
        }
        balance -= amount;
        receiver.balance += amount;
        history.push_back(Transaction("transfer-out", amount, "To: " + receiver.name));
        receiver.history.push_back(Transaction("transfer-in", amount, "From: " + name));
        cout << "✅ Transferred Rs. " << amount << " to " << receiver.name << ".\n";
        return true;
    }

    void showDetails() const {
        cout << "\n--- Account Details ---\n";
        cout << "Account No: " << accountNumber << endl;
        cout << "Name: " << name << endl;
        cout << "Type: " << type << endl;
        cout << fixed << setprecision(2);
        cout << "Balance: Rs. " << balance << endl;
    }

    void showTransactions() const {
        cout << "\n--- Transaction History for Account " << accountNumber << " ---\n";
        cout << left << setw(10) << "Type" << setw(10) << "Amount" 
             << setw(25) << "Timestamp" << "Remarks" << endl;
        cout << "-------------------------------------------------------------\n";
        for (auto &t : history) t.display();
    }

    // --- File handling (save/load) ---
    void saveToFile(ofstream &out) const {
        out << accountNumber << "|" << name << "|" << type << "|" << balance << endl;
    }
    static Account loadFromFile(const string &line) {
        Account acc;
        size_t p1 = line.find('|');
        size_t p2 = line.find('|', p1 + 1);
        size_t p3 = line.find('|', p2 + 1);
        acc.accountNumber = stoi(line.substr(0, p1));
        acc.name = line.substr(p1 + 1, p2 - p1 - 1);
        acc.type = line.substr(p2 + 1, p3 - p2 - 1);
        acc.balance = stod(line.substr(p3 + 1));
        return acc;
    }
};

class Bank {
private:
    vector<Account> accounts;
    int nextAccountNo;

public:
    Bank() {
        nextAccountNo = 1001;
        loadAccounts();
    }

    void createAccount() {
        string name, type;
        double initial;
        cout << "Enter Name: ";
        cin.ignore();
        getline(cin, name);
        cout << "Enter Account Type (Savings/Current): ";
        getline(cin, type);
        cout << "Enter Initial Deposit: ";
        cin >> initial;

        Account acc(nextAccountNo++, name, type, initial);
        accounts.push_back(acc);
        cout << "✅ Account created successfully! Account No: " << acc.getAccNo() << endl;
        saveAccounts();
    }

    Account* findAccount(int accNo) {
        for (auto &a : accounts)
            if (a.getAccNo() == accNo) return &a;
        return nullptr;
    }

    void depositMoney() {
        int accNo;
        double amt;
        cout << "Enter Account No: ";
        cin >> accNo;
        cout << "Enter Amount to Deposit: ";
        cin >> amt;
        Account *acc = findAccount(accNo);
        if (acc) {
            acc->deposit(amt);
            saveAccounts();
        } else cout << "❌ Account not found!\n";
    }

    void withdrawMoney() {
        int accNo;
        double amt;
        cout << "Enter Account No: ";
        cin >> accNo;
        cout << "Enter Amount to Withdraw: ";
        cin >> amt;
        Account *acc = findAccount(accNo);
        if (acc) {
            acc->withdraw(amt);
            saveAccounts();
        } else cout << "❌ Account not found!\n";
    }

    void transferMoney() {
        int fromAcc, toAcc;
        double amt;
        cout << "Enter Sender Account No: ";
        cin >> fromAcc;
        cout << "Enter Receiver Account No: ";
        cin >> toAcc;
        cout << "Enter Amount to Transfer: ";
        cin >> amt;

        Account *sender = findAccount(fromAcc);
        Account *receiver = findAccount(toAcc);

        if (sender && receiver) {
            sender->transfer(*receiver, amt);
            saveAccounts();
        } else cout << "❌ Invalid Account(s)!\n";
    }

    void showAccountDetails() {
        int accNo;
        cout << "Enter Account No: ";
        cin >> accNo;
        Account *acc = findAccount(accNo);
        if (acc) acc->showDetails();
        else cout << "❌ Account not found!\n";
    }

    void showTransactionHistory() {
        int accNo;
        cout << "Enter Account No: ";
        cin >> accNo;
        Account *acc = findAccount(accNo);
        if (acc) acc->showTransactions();
        else cout << "❌ Account not found!\n";
    }

    // File persistence
    void saveAccounts() {
        ofstream fout("accounts.dat");
        for (auto &a : accounts)
            a.saveToFile(fout);
        fout.close();
    }

    void loadAccounts() {
        ifstream fin("accounts.dat");
        if (!fin) return; // file not found yet
        string line;
        while (getline(fin, line)) {
            Account acc = Account::loadFromFile(line);
            accounts.push_back(acc);
            nextAccountNo = max(nextAccountNo, acc.getAccNo() + 1);
        }
        fin.close();
    }
};

// ------------------- MAIN -------------------

int main() {
    Bank bank;
    int choice;
    do {
        cout << "\n========= BANK ACCOUNT MANAGEMENT SYSTEM =========\n";
        cout << "1. Create Account\n";
        cout << "2. Deposit Money\n";
        cout << "3. Withdraw Money\n";
        cout << "4. Transfer Money\n";
        cout << "5. Show Account Details\n";
        cout << "6. Transaction History\n";
        cout << "7. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
            case 1: bank.createAccount(); break;
            case 2: bank.depositMoney(); break;
            case 3: bank.withdrawMoney(); break;
            case 4: bank.transferMoney(); break;
            case 5: bank.showAccountDetails(); break;
            case 6: bank.showTransactionHistory(); break;
            case 7: cout << "💾 Exiting... Data saved successfully.\n"; break;
            default: cout << "❌ Invalid option! Try again.\n";
        }
    } while (choice != 7);
    return 0;
}