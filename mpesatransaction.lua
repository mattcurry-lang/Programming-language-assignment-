--[[
====================================================================
  CCS 2105 - PROGRAMMING LANGUAGES
  Question 2: M-Pesa Transaction Verification and Recovery (20 Marks)
====================================================================
  Fictional M-Pesa agent system. Each transaction moves through five
  stages:
      1. request received
      2. customer details checked
      3. balance verified   <-- may FAIL here (insufficient funds)
      4. transaction authorized
      5. receipt generated

  A transaction that fails balance verification must NOT continue to
  authorization. Multiple transactions are processed concurrently
  (quasi-concurrently) using coroutines, each yielding after every
  stage so the scheduler can interleave them.
====================================================================
]]

-- ------------------------------------------------------------------
-- (a) Each transaction is an INDEPENDENT coroutine that maintains
--     its own state (id, customer, amount, balance, stage, status).
-- ------------------------------------------------------------------
local function makeTransaction(txn)
    -- txn = { id, customer, amount, balance }
    return coroutine.create(function()

        -- STAGE 1: request received
        coroutine.yield({
            id = txn.id, stage = "request_received",
            status = "in_progress",
            message = string.format("Request received from %s for KSh %d",
                txn.customer, txn.amount)
        })

        -- STAGE 2: customer details checked
        -- (fictional check: customer name must not be empty)
        local customerOk = txn.customer ~= nil and txn.customer ~= ""
        if not customerOk then
            coroutine.yield({
                id = txn.id, stage = "customer_check",
                status = "failed",
                message = "Invalid customer details"
            })
            return "failed" -- coroutine ends -> becomes dead
        end

        coroutine.yield({
            id = txn.id, stage = "customer_check",
            status = "in_progress",
            message = "Customer details verified"
        })

        -- STAGE 3: balance verified (the critical decision point)
        if txn.balance < txn.amount then
            -- (b)/(c) Yield failure status; scheduler rejects this
            -- transaction WITHOUT affecting any other coroutine,
            -- and processing must NOT continue to authorization.
            coroutine.yield({
                id = txn.id, stage = "balance_verification",
                status = "failed",
                message = string.format(
                    "Insufficient funds (balance KSh %d < amount KSh %d)",
                    txn.balance, txn.amount)
            })
            return "failed" -- coroutine terminates here -> dead state
        end

        coroutine.yield({
            id = txn.id, stage = "balance_verification",
            status = "in_progress",
            message = "Sufficient balance confirmed"
        })

        -- STAGE 4: transaction authorized
        coroutine.yield({
            id = txn.id, stage = "authorization",
            status = "in_progress",
            message = "Transaction authorized"
        })

        -- STAGE 5: receipt generated
        coroutine.yield({
            id = txn.id, stage = "receipt_generated",
            status = "success",
            message = "Receipt generated successfully"
        })

        return "success"
    end)
end

-- ------------------------------------------------------------------
-- (d) MAIN SCHEDULER
--     Round-robin resumes every active coroutine, reads its status
--     table, checks coroutine.resume()'s own success flag, retires
--     coroutines that die (successfully or by failure), and NEVER
--     resumes a coroutine already in the "dead" state.
-- ------------------------------------------------------------------
local function runScheduler(transactionData)
    local jobs = {}      -- list of { co = coroutine, data = txn }
    local summary = {}   -- final results per transaction id

    for _, txn in ipairs(transactionData) do
        table.insert(jobs, { co = makeTransaction(txn), id = txn.id })
    end

    print("=== M-PESA TRANSACTION SCHEDULER STARTED ===\n")

    local activeCount = #jobs
    while activeCount > 0 do
        for _, job in ipairs(jobs) do
            -- Skip coroutines that are already finished/dead
            if coroutine.status(job.co) ~= "dead" then

                local ok, result = coroutine.resume(job.co)

                if not ok then
                    -- A runtime error occurred inside the coroutine.
                    -- Log it but keep the scheduler running for others.
                    print(string.format(
                        "[ERROR] Transaction %s crashed: %s",
                        job.id, tostring(result)))
                    summary[job.id] = "error"

                elseif coroutine.status(job.co) == "dead" then
                    -- Coroutine has just finished (success or failed
                    -- return value). Record final outcome.
                    if type(result) == "table" then
                        print(string.format("[%s] %-22s -> %s (%s)",
                            job.id, result.stage, result.status, result.message))
                        summary[job.id] = result.status
                    else
                        summary[job.id] = result -- "success" or "failed"
                    end

                else
                    -- Still in progress: print the yielded status.
                    local r = result
                    print(string.format("[%s] %-22s -> %s (%s)",
                        job.id, r.stage, r.status, r.message))
                end
            end
        end

        -- Recompute how many jobs are still alive
        activeCount = 0
        for _, job in ipairs(jobs) do
            if coroutine.status(job.co) ~= "dead" then
                activeCount = activeCount + 1
            end
        end
        print("---- scheduling cycle complete ----")
    end

    -- ------------------------------------------------------------
    -- (d continued) Final summary of successful vs failed transactions
    -- ------------------------------------------------------------
    print("\n=== FINAL TRANSACTION SUMMARY ===")
    for id, status in pairs(summary) do
        print(string.format("Transaction %s : %s", id, status))
    end
end

-- ------------------------------------------------------------------
-- Fictional sample data (no real personal/confidential information)
-- ------------------------------------------------------------------
local transactions = {
    { id = "TXN001", customer = "Wanjiru M.", amount = 2500, balance = 5000 },
    { id = "TXN002", customer = "Otieno K.",  amount = 8000, balance = 1500 }, -- insufficient funds
    { id = "TXN003", customer = "Kiptoo A.",  amount = 1200, balance = 1200 },
    { id = "TXN004", customer = "Achieng B.", amount = 3000, balance = 2999 }, -- insufficient funds
    { id = "TXN005", customer = "Mutua J.",   amount = 500,  balance = 10000 },
}

runScheduler(transactions)

--[[
====================================================================
(e) WHY STATE IS PRESERVED WHEN A COROUTINE RESUMES AFTER yield()
====================================================================
Unlike an ordinary function call -- which loses its entire call stack
and local variables the moment it returns -- a Lua coroutine keeps
its own independent execution stack. When coroutine.yield() is
called, the coroutine's stack (its local variables, current
instruction pointer, and call position) is frozen exactly as it was,
and control simply returns to whoever called coroutine.resume().

When that coroutine is resumed again, Lua does not re-run it from the
top: execution continues from the exact point right after the
yield() call, with every local variable (txn.id, txn.balance, the
current stage, etc.) still holding its previous value. This is what
allows each transaction in this program to "remember" which stage it
reached and what data it was working with, without the scheduler
having to manually save and restore that state itself. It is this
property that makes coroutines a natural fit for modelling multi-
stage workflows such as this M-Pesa transaction pipeline.
====================================================================
]]
