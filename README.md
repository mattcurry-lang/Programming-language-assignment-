Programming-language-assignment-

Question 2: M-Pesa Transaction Verification and Recovery

GROUP 2 --- REPORT FROM LUA LANGUAGE MODEL

Institution: Dedan Kimathi University of Technology
School: School of Computer Science and IT
Department: Department of Computer Science
Unit: Programming Languages
Unit Code: CCS 2105
Group: Group 2

Group Members

Mathew Kibet Mutai --- C026-01-0961/2025

Isaac Opiyo Wafula --- C026-01-0976/2025

Victor Mawira --- C026-01-0936/2025

CCS 2105 --- Programming Languages

Question 2: M-Pesa Transaction Verification and Recovery

GROUP 2 --- REPORT FROM LUA LANGUAGE MODEL

Group Members

Mathew Kibet Mutai --- C026-01-0961/2025

Isaac Opiyo Wafula --- C026-01-0976/2025

Victor Mawira --- C026-01-0936/2025

1. Introduction

Fictional M-Pesa agent system modelled using Lua coroutines. A real
M-Pesa transaction does not happen instantly; it moves through a
sequence of checks before it is finally confirmed.

In this exercise, that sequence is broken into five stages:

The request being received.

The customer's details being checked.

The customer's balance being verified.

The transaction being authorized.

A receipt being generated.

The key rule given in the scenario is that a transaction which fails the
balance-verification stage must stop there and must not be allowed to
continue on to authorization.

Several transactions also need to be processed at the same time, in an
interleaved fashion, without one transaction's progress interfering with
another's.

Lua coroutines are a natural fit for this kind of problem because each
coroutine can pause itself midway through a multi-step process using
coroutine.yield(), hand control back to a central scheduler, and then
pick up again from exactly where it left off using coroutine.resume().

This makes it possible to model five separate transactions as five
separate "mini programs" that all appear to be running side by side,
even though Lua itself only ever executes one of them at any given
instant.

2. Modelling a Transaction as an Independent Coroutine

Each transaction in the system is represented by its own coroutine,
created inside the makeTransaction() function.

When this function is called, it is given a small table describing the
transaction --- an ID, the customer's name, the amount being sent, and
the customer's current balance --- and it wraps a new function around
that data using coroutine.create().

Because Lua closures capture the variables around them, that single
coroutine effectively "remembers" its own ID, customer, amount and
balance for as long as it exists, completely separately from any other
transaction running at the same time.

Nothing is shared between coroutines except what the scheduler
explicitly manages, so there is no risk of one transaction's data
leaking into another's.

3. Yielding After Every Stage

After every one of the five processing stages, the coroutine calls
coroutine.yield() and passes back a small status table containing:

The transaction's ID.

The name of the stage just completed.

A status value such as "in_progress", "success" or "failed".

A short human-readable message describing what happened.

This allows the scheduler to see the transaction's progress one step at
a time rather than only finding out the final result.

It is also what makes the interleaving possible: because the coroutine
gives up control after each stage instead of running straight through to
the end, the scheduler gets a chance to move on and give another
transaction a turn before coming back.

4. Rejecting a Transaction Without Affecting Others

The balance-verification stage is where the scenario's central rule is
enforced.

Before allowing the transaction to continue, the coroutine checks
whether the customer's balance is at least equal to the amount being
sent.

If it is not, the coroutine yields one final failure status explaining
that funds are insufficient, and then returns the string "failed"
instead of carrying on to the authorization stage.

Returning from the coroutine function is what causes Lua to mark that
coroutine as dead, so a rejected transaction simply stops existing as an
active task from that point onward.

Because every transaction runs inside its own coroutine with its own
private state, rejecting one transaction this way has no effect
whatsoever on the coroutines belonging to the other transactions, which
keep running and yielding exactly as before.

5. The Scheduler

The runScheduler() function is the central program that drives all
five transactions forward together.

It first creates a coroutine for every transaction in the list, then
enters a loop that keeps going for as long as at least one coroutine is
still active.

On every pass through that loop, it goes through each job in turn and,
importantly, checks coroutine.status() first. If a coroutine is
already "dead", it is skipped completely, so a finished or rejected
transaction is never accidentally resumed again.

For every coroutine that is still alive, the scheduler calls
coroutine.resume() and looks at both values it returns:

The first is a true/false flag telling it whether the coroutine ran
without crashing.

The second is either the yielded status table or the final return
value.

If the flag comes back false, the scheduler prints an error message for
that one transaction and moves on, rather than letting the whole program
crash.

Once every coroutine has become dead, the loop ends and a final summary
table is printed showing the outcome --- success or failed --- for every
transaction that was processed.

6. Why State Is Preserved Across yield() and resume()

An ordinary Lua function loses everything about its progress the moment
it returns; calling it again starts it from the very top with fresh
local variables.

A coroutine behaves differently because it keeps its own separate
execution stack.

When coroutine.yield() runs, Lua freezes that stack exactly as it is
--- every local variable, including things like the transaction's ID and
balance, and the exact point in the code the coroutine had reached ---
and hands control back to whoever called resume().

The next time that same coroutine is resumed, execution does not restart
from the beginning; it carries on immediately after the yield() call
with all of those local variables still holding the values they had
before.

This is precisely why each transaction in this program can be paused
after, say, the customer-check stage and picked up again later at the
balance-verification stage without the scheduler having to manually save
and reload any of its data itself --- the coroutine does that
automatically.

GROUP 2 --- Programming Languages Laboratory

Group Members

Mathew Kibet Mutai --- C026-01-0961/2025

Isaac Opiyo Wafula --- C026-01-0976/2025

Victor Mawira --- C026-01-0936/2025

Course: Programming Languages Laboratory
Unit Code: CCS 2105
Topic: Names, Bindings, Scopes, and Storage Semantics in C
Group Case: Fee Clearance & Payment Reconciliation System

TRACE TABLE

Transaction   Type             Amount (KES)    Bound Balance Notes / Trigger
ID                                               Value (KES)

INIT      Initial                  0.00   105,000.00 Value bound at initialization
State                                          (REQUIRED_FEE + REGISTRATION_CHARGE)

108       PAYMENT            105,000.00         0.00 Balance reduced dynamically at run-time
upon valid transaction

109       REVERSAL           105,000.00   105,000.00 Reversal adds amount back; balance
restored

A. Binding Times & Binding Events

1. Compile Time

Types: Data structures (StudentAccount, Transaction) and
primitive types (double, int) are bound to sizes and alignment
rules by the C compiler.

Named Constants: Macros #define REQUIRED_FEE 100000.0 and
#define REGISTRATION_CHARGE 5000.0 are bound during
pre-processing/compilation.

2. Load Time

Global Storage: institutionName memory location and static
program structures are bound when the OS loads the binary into
memory.

3. Run Time

Value Binding: The value bound to account->currentBalance
updates dynamically at execution time following transaction
arithmetic operations.

Stack Allocation: Local variables (blockLocalAdjustment,
testTx) are dynamically bound to memory addresses upon entering
their respective execution frames.

B. Aliasing and Memory Modification (L-value / R-value)

Parameter Aliasing

The function:

applyTransaction(StudentAccount *account, ...)

receives the address &students[0].

The formal pointer variable account acts as an alias for
students[0].

L-value vs R-value

In:

account->currentBalance = account->currentBalance - t->amount;

account->currentBalance on the left side acts as an
l-value --- a locator representing a specific memory location
where a value can be stored.

account->currentBalance - t->amount on the right side acts as
an r-value --- the evaluated numerical value computed during the
operation.

C. Scope, Shadowing, and Variable Lifetimes

Global vs Local Scope

The global variable institutionName resides at file scope.

Inside printReconciliationReport(), defining:

const char *institutionName;

creates a local variable that shadows the global variable inside
that function block.

Block Lifetime

The variable blockLocalAdjustment inside applyTransaction() has
block scope and stack-dynamic lifetime.

Storage is allocated when entering the TX_REVERSAL block case and
discarded immediately when exiting the block brace }.

D. Core Reflection Questions

1. Binding Name to Type vs. Storage vs. Value

Type Binding: Determines how many bytes to allocate and which
operations are legal.

Storage Binding: Connects the identifier to a specific RAM/stack
memory address.

Value Binding: Populates or changes the binary data held in that
storage address during execution.

2. Same Name in Different Scopes vs. Two Names for One Location

Two variables with the same name in different scopes do not
share a memory location; they represent distinct bindings resolved
by scope rules.

Two different names sharing one location are aliases. For
example, the actual argument students[0] and the formal pointer
parameter *account can refer to the same underlying object.
Modifying the object through one reference changes the same object
seen through the other reference.

Submission Information

Institution: Dedan Kimathi University of Technology
School: School of Computer Science and IT
Department: Department of Computer Science
Unit: Programming Languages
Unit Code: CCS 2105
Group: Group 2

Group Members

Name                 Registration Number

Mathew Kibet Mutai   C026-01-0961/2025
Isaac Opiyo Wafula   C026-01-0976/2025
Victor Mawira        C026-01-0936/2025
