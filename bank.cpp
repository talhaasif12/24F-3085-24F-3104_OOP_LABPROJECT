#include "stronghold.h"

Bank::Bank(double initialReserves, double initInterestRate)
    : reserves(initialReserves), interestRate(initInterestRate), fraudLevel(0.0) {
    loans = new Loan * [10];
    numLoans = 0;
    maxLoans = 10;
}

Bank::~Bank() {
    for (int i = 0; i < numLoans; i++) {
        delete loans[i];
    }
    delete[] loans;
}

double Bank::getReserves() const {
    return reserves;
}

void Bank::setReserves(double amount) {
    if (amount < 0) {
        throw std::invalid_argument("Bank reserves cannot be negative");
    }
    reserves = amount;
}

double Bank::getInterestRate() const {
    return interestRate;
}

void Bank::setInterestRate(double rate) {
    if (rate < 0) {
        throw std::invalid_argument("Interest rate cannot be negative");
    }
    interestRate = rate;
}

double Bank::getFraudLevel() const {
    return fraudLevel;
}

void Bank::setFraudLevel(double level) {
    if (level < 0 || level > 1.0) {
        throw std::invalid_argument("Fraud level must be between 0 and 1");
    }
    fraudLevel = level;
}

bool Bank::issueLoan(const std::string& borrower, double amount, int termMonths) {
    if (amount <= 0 || termMonths <= 0) {
        throw std::invalid_argument("Loan amount and term must be positive");
    }
    if (amount > reserves * 0.7) {
        return false;
    }
    Loan* newLoan = new Loan;
    newLoan->borrower = borrower;
    newLoan->amount = amount;
    newLoan->interestRate = interestRate + (fraudLevel * 0.05);
    newLoan->termMonths = termMonths;
    newLoan->monthsPaid = 0;
    newLoan->defaulted = false;
    if (numLoans >= maxLoans) {
        int newMaxLoans = maxLoans * 2;
        Loan** newLoans = new Loan * [newMaxLoans];
        for (int i = 0; i < numLoans; i++) {
            newLoans[i] = loans[i];
        }
        delete[] loans;
        loans = newLoans;
        maxLoans = newMaxLoans;
    }
    loans[numLoans] = newLoan;
    numLoans++;
    reserves -= amount;
    return true;
}

double Bank::calculateMonthlyPayment(const Loan& loan) const {
    double totalAmount = loan.amount * (1 + (loan.interestRate * loan.termMonths / 12));
    return totalAmount / loan.termMonths;
}

void Bank::processLoanPayments(Economy& economy) {
    for (int i = 0; i < numLoans; i++) {
        if (!loans[i]->defaulted && loans[i]->monthsPaid < loans[i]->termMonths) {
            double payment = calculateMonthlyPayment(*loans[i]);
            bool paymentMade = economy.payExpense(payment, "Loan payment");
            if (paymentMade) {
                loans[i]->monthsPaid++;
                reserves += payment;
                if (loans[i]->monthsPaid >= loans[i]->termMonths) {
                }
            }
            else {
                loans[i]->defaulted = true;
            }
        }
    }
}

void Bank::auditBank(double thoroughness) {
    if (thoroughness < 0 || thoroughness > 1.0) {
        throw std::invalid_argument("Audit thoroughness must be between 0 and 1");
    }
    double detectionChance = thoroughness * (1 - fraudLevel * 0.5);
    if (((double)rand() / RAND_MAX) < detectionChance) {
        double reductionAmount = fraudLevel * thoroughness;
        fraudLevel -= reductionAmount;
        if (fraudLevel < 0) fraudLevel = 0;
    }
}

void Bank::updateInterestRate(double economicStability) {
    if (economicStability < 0 || economicStability > 1.0) {
        throw std::invalid_argument("Economic stability must be between 0 and 1");
    }
    double baseRate = 0.05 + ((1 - economicStability) * 0.1);
    double fraudAdjustment = fraudLevel * 0.05;
    interestRate = baseRate + fraudAdjustment + (((double)rand() / RAND_MAX) * 0.02 - 0.01);
    if (interestRate < 0.01) interestRate = 0.01;
    if (interestRate > 0.25) interestRate = 0.25;
}

double Bank::getTotalLoans() const {
    double total = 0;
    for (int i = 0; i < numLoans; i++) {
        if (!loans[i]->defaulted) {
            double monthlyPayment = calculateMonthlyPayment(*loans[i]);
            double remainingPayments = loans[i]->termMonths - loans[i]->monthsPaid;
            total += monthlyPayment * remainingPayments;
        }
    }
    return total;
}

void Bank::handleFraud(double leaderCorruptionInfluence) {
    double fraudChange = (leaderCorruptionInfluence - 0.5) * 0.05;
    fraudChange += ((double)rand() / RAND_MAX) * 0.03 - 0.015;
    fraudLevel += fraudChange;
    if (fraudLevel < 0) fraudLevel = 0;
    if (fraudLevel > 1.0) fraudLevel = 1.0;
    if (fraudLevel > 0.7) {
        double embezzledAmount = reserves * fraudLevel * 0.05;
        reserves -= embezzledAmount;
        if (reserves < 0) reserves = 0;
    }
}

void Bank::saveToFile(std::ofstream& outFile) const {
    if (!outFile.is_open()) {
        throw std::runtime_error("Failed to save bank data: File not open");
    }
    outFile << reserves << std::endl;
    outFile << interestRate << std::endl;
    outFile << fraudLevel << std::endl;
    outFile << numLoans << std::endl;
    for (int i = 0; i < numLoans; i++) {
        outFile << loans[i]->borrower << std::endl;
        outFile << loans[i]->amount << std::endl;
        outFile << loans[i]->interestRate << std::endl;
        outFile << loans[i]->termMonths << std::endl;
        outFile << loans[i]->monthsPaid << std::endl;
        outFile << loans[i]->defaulted << std::endl;
    }
}

void Bank::loadFromFile(std::ifstream& inFile) {
    if (!inFile.is_open()) {
        throw std::runtime_error("Failed to load bank data: File not open");
    }
    inFile >> reserves;
    inFile >> interestRate;
    inFile >> fraudLevel;
    for (int i = 0; i < numLoans; i++) {
        delete loans[i];
    }
    delete[] loans;
    inFile >> numLoans;
    maxLoans = numLoans > 10 ? numLoans : 10;
    loans = new Loan * [maxLoans];
    std::string dummy;
    std::getline(inFile, dummy);
    for (int i = 0; i < numLoans; i++) {
        loans[i] = new Loan;
        std::getline(inFile, loans[i]->borrower);
        inFile >> loans[i]->amount;
        inFile >> loans[i]->interestRate;
        inFile >> loans[i]->termMonths;
        inFile >> loans[i]->monthsPaid;
        inFile >> loans[i]->defaulted;
        std::getline(inFile, dummy);
    }
}
