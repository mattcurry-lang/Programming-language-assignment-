#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// NAMED CONSTANTS & BINDING TIME DEMONSTRATION
// Compile-time Bindings: Types, sizes, and constant values are bound at compile time.

#define REQUIRED_FEE 100000.0
#define REGISTRATION_CHARGE 5000.0
#define MAX_STUDENTS 10
#define MAX_TRANSACTIONS 30
#define ID_LEN 20
#define NAME_LEN 50
#define CHANNEL_LEN 30

// Global variable for scope/shadowing experiment
const char* institutionName = "Jomo Kenyatta University of Agriculture and Technology";

// Transaction Types
typedef enum {
    TX_PAYMENT,
    TX_WAIVER,
    TX_REVERSAL,
    TX_DUPLICATE
} TransactionType;

const char* typeToString(TransactionType t) {
    switch (t) {
        case TX_PAYMENT:   return "PAYMENT";
        case TX_WAIVER:    return "WAIVER";
        case TX_REVERSAL:  return "REVERSAL";
        case TX_DUPLICATE: return "DUPLICATE";
        default:           return "UNKNOWN";
    }
}

// DATA MODELS
typedef struct {
    int id;
    char channel[CHANNEL_LEN];
    double amount;
    TransactionType type;
} Transaction;

typedef struct {
    char studentId[ID_LEN];
    char studentName[NAME_LEN];
    double totalBilled;
    double grossPayments;
    double approvedWaivers;
    double reversals;
    double currentBalance;
} StudentAccount;

// Constructor-like initializer for StudentAccount
void initStudentAccount(StudentAccount *acc, const char *id, const char *name) {
    strncpy(acc->studentId, id, ID_LEN - 1);
    acc->studentId[ID_LEN - 1] = '\0';

    strncpy(acc->studentName, name, NAME_LEN - 1);
    acc->studentName[NAME_LEN - 1] = '\0';

    acc->totalBilled = REQUIRED_FEE + REGISTRATION_CHARGE;
    acc->grossPayments = 0.0;
    acc->approvedWaivers = 0.0;
    acc->reversals = 0.0;
    acc->currentBalance = REQUIRED_FEE + REGISTRATION_CHARGE;
}

bool isCleared(const StudentAccount *acc) {
    return acc->currentBalance <= 0.0;
}

// ============================================================================
// RECONCILIATION ENGINE & ALIASING DEMONSTRATION
// Task C: Pointer parameter creates an explicit alias inside the function.
// ============================================================================
bool applyTransaction(StudentAccount *account, const Transaction *t, int *seenTxIds, int *seenCount, int *rejectedDuplicates) {
    // 1. Input Validation: Check for duplicate Transaction IDs
    for (int i = 0; i < *seenCount; i++) {
        if (seenTxIds[i] == t->id) {
            (*rejectedDuplicates)++;
            printf("  [REJECTED] Duplicate Transaction ID detected: #%d\n", t->id);
            return false;
        }
    }

    // 2. Input Validation: Reject negative payment amounts
    if (t->amount < 0.0) {
        printf("  [REJECTED] Invalid negative transaction amount: KES %.2f (Tx #%d)\n", t->amount, t->id);
        return false;
    }

    // Register processed Transaction ID
    seenTxIds[*seenCount] = t->id;
    (*seenCount)++;

    // 3. Apply state mutation through pointer alias
    switch (t->type) {
        case TX_PAYMENT:
            account->grossPayments += t->amount;
            account->currentBalance -= t->amount;
            break;

        case TX_WAIVER:
            account->approvedWaivers += t->amount;
            account->currentBalance -= t->amount;
            break;

        case TX_REVERSAL: {
            // Block-local variable experiment
            double blockLocalAdjustment = t->amount; // Scope: inner block; Lifetime: block execution
            account->reversals += blockLocalAdjustment;
            account->currentBalance += blockLocalAdjustment; // Restores balance owed
            break;
        }

        case TX_DUPLICATE:
            (*rejectedDuplicates)++;
            printf("  [REJECTED] Transaction explicitly marked DUPLICATE: #%d\n", t->id);
            return false;
    }
    return true;
}

// ============================================================================
// SCOPE & SHADOWING EXPERIMENT
// Task D: Demonstrate global vs. local scope shadowing in C
// ============================================================================
void printReconciliationReport(const StudentAccount *acc, const Transaction *txHistory, int historyCount) {
    // Local variable shadowing global variable with identical name
    const char* institutionName = "Local Finance Office Branch (Shadowed Local Scope)";

    printf("\n======================================================\n");
    printf("            RECONCILIATION STATEMENT                  \n");
    printf("======================================================\n");
    printf("Department (Shadowed Local): %s\n", institutionName);
    printf("------------------------------------------------------\n");
    printf("Student ID   : %s\n", acc->studentId);
    printf("Student Name : %s\n", acc->studentName);
    printf("Total Billed : KES %.2f\n", acc->totalBilled);
    printf("------------------------------------------------------\n");
    printf("Processed Transactions:\n");

    for (int i = 0; i < historyCount; i++) {
        printf("  - Tx #%d [%s] via %s: KES %.2f\n",
               txHistory[i].id,
               typeToString(txHistory[i].type),
               txHistory[i].channel,
               txHistory[i].amount);
    }

    printf("------------------------------------------------------\n");
    printf("Gross Payments   : KES %.2f\n", acc->grossPayments);
    printf("Approved Waivers : KES %.2f\n", acc->approvedWaivers);
    printf("Reversals        : KES %.2f\n", acc->reversals);
    printf("Final Balance    : KES %.2f\n", acc->currentBalance);
    printf("Clearance Status : %s\n", isCleared(acc) ? "CLEARED" : "UNCLEARED (PENDING)");
    printf("======================================================\n");
}

// Helper structure to map transactions to student IDs
typedef struct {
    char studentId[ID_LEN];
    Transaction tx;
} TransactionRecord;

// ============================================================================
// MAIN EXECUTION & TESTS
// ============================================================================
int main(void) {
    printf("======================================================\n");
    printf(" GROUP 2: FEE CLEARANCE & RECONCILIATION SYSTEM (C99) \n");
    printf("======================================================\n\n");

    // 1. Initialize Minimum Dataset: 5 Students
    StudentAccount students[5];
    initStudentAccount(&students[0], "STU001", "Mathew Ochieng");
    initStudentAccount(&students[1], "STU002", "Frank Mutua");
    initStudentAccount(&students[2], "STU003", "Levy Kiprop");
    initStudentAccount(&students[3], "STU004", "Isaac Newton");
    initStudentAccount(&students[4], "STU005", "Clement Wekesa");

    // 2. Initialize Minimum Dataset: 14 Transactions
    TransactionRecord transactionLog[] = {
        {"STU001", {101, "M-Pesa", 50000.0, TX_PAYMENT}},
        {"STU001", {102, "Bank Wire", 55000.0, TX_PAYMENT}},
        {"STU001", {103, "Bursary", 5000.0, TX_WAIVER}}, // Cleared

        {"STU002", {104, "M-Pesa", 105000.0, TX_PAYMENT}}, // Cleared
        {"STU002", {104, "M-Pesa", 105000.0, TX_PAYMENT}}, // Duplicate Tx ID -> REJECTED

        {"STU003", {105, "Bank Wire", 40000.0, TX_PAYMENT}},
        {"STU003", {106, "Bank Wire", -15000.0, TX_PAYMENT}}, // Negative amount -> REJECTED
        {"STU003", {107, "Bursary", 20000.0, TX_WAIVER}}, // Uncleared

        {"STU004", {108, "M-Pesa", 105000.0, TX_PAYMENT}},
        {"STU004", {109, "System Error", 105000.0, TX_REVERSAL}}, // Balance restored
        {"STU004", {110, "Bank Wire", 60000.0, TX_PAYMENT}}, // Uncleared

        {"STU005", {111, "M-Pesa", 50000.0, TX_PAYMENT}},
        {"STU005", {112, "Work-Study", 55000.0, TX_WAIVER}}, // Cleared
        {"STU005", {113, "Card", 1000.0, TX_DUPLICATE}} // Duplicate type -> REJECTED
    };
    int totalLogEntries = sizeof(transactionLog) / sizeof(transactionLog[0]);

    int seenTxIds[MAX_TRANSACTIONS];
    int seenCount = 0;
    int rejectedDuplicates = 0;
    double totalValidPaymentsProcessed = 0.0;

    // --- TASK C: ALIASING & REFERENCE PROOF EXPERIMENT ---
    printf("--- TASK C: ALIASING & POINTER PROOF EXPERIMENT ---\n");
    printf("Initial STU001 Balance: KES %.2f\n", students[0].currentBalance);

    Transaction testTx = {999, "Direct Pointer Test", 20000.0, TX_PAYMENT};
    // Passing pointer &students[0] creates an alias inside function
    applyTransaction(&students[0], &testTx, seenTxIds, &seenCount, &rejectedDuplicates);

    printf("Balance AFTER applyTransaction(&students[0], &testTx): KES %.2f\n", students[0].currentBalance);
    printf("-> PROOF: The memory at address %p was modified directly through the pointer alias.\n", (void*)&students[0]);
    printf("------------------------------------------------------\n\n");

    // --- TASK B: TRACE TABLE FOR BALANCE VALUE-BINDING (STU004) ---
    printf("--- TASK B: BALANCE VALUE-BINDING TRACE TABLE (STU004) ---\n");
    printf("%-10s %-15s %-15s %-20s\n", "Tx ID", "Type", "Amount (KES)", "Bound Balance (KES)");
    printf("------------------------------------------------------------\n");

    StudentAccount traceAcc;
    initStudentAccount(&traceAcc, "STU004", "Isaac Newton");
    printf("%-10s %-15s %-15.2f %-20.2f\n", "INIT", "INITIAL", 0.0, traceAcc.currentBalance);

    int dummyIds[10];
    int dummyCount = 0;
    int dummyRejects = 0;

    Transaction stu4Txs[] = {
        {108, "M-Pesa", 105000.0, TX_PAYMENT},
        {109, "System Error", 105000.0, TX_REVERSAL},
        {110, "Bank Wire", 60000.0, TX_PAYMENT}
    };

    for (int i = 0; i < 3; i++) {
        applyTransaction(&traceAcc, &stu4Txs[i], dummyIds, &dummyCount, &dummyRejects);
        printf("%-10d %-15s %-15.2f %-20.2f\n",
               stu4Txs[i].id,
               typeToString(stu4Txs[i].type),
               stu4Txs[i].amount,
               traceAcc.currentBalance);
    }
    printf("------------------------------------------------------------\n\n");

    // --- MAIN TRANSACTION PROCESSING LOOP ---
    printf("--- PROCESSING SYSTEM TRANSACTIONS ---\n");
    for (int s = 0; s < 5; s++) {
        Transaction studentHistory[MAX_TRANSACTIONS];
        int historyCount = 0;

        for (int t = 0; t < totalLogEntries; t++) {
            if (strcmp(transactionLog[t].studentId, students[s].studentId) == 0) {
                if (applyTransaction(&students[s], &transactionLog[t].tx, seenTxIds, &seenCount, &rejectedDuplicates)) {
                    studentHistory[historyCount++] = transactionLog[t].tx;
                    if (transactionLog[t].tx.type == TX_PAYMENT) {
                        totalValidPaymentsProcessed += transactionLog[t].tx.amount;
                    }
                }
            }
        }
        printReconciliationReport(&students[s], studentHistory, historyCount);
    }

    // --- EXECUTIVE SUMMARY ---
    int clearedCount = 0;
    int unclearedCount = 0;
    for (int i = 0; i < 5; i++) {
        if (isCleared(&students[i])) clearedCount++;
        else unclearedCount++;
    }

    printf("\n======================================================\n");
    printf("             FINAL RECONCILIATION SUMMARY             \n");
    printf("======================================================\n");
    printf("Total Students Processed : 5\n");
    printf("Cleared Accounts: %d\n", clearedCount);
    printf("Uncleared Accounts: %d\n", unclearedCount);
    printf("Duplicates/Errors Blocked: %d\n", rejectedDuplicates);
    printf("Total Valid Payments: KES %.2f\n",totalValidPaymentsProcessed);
    printf("======================================================\n");

    return 0;
}
