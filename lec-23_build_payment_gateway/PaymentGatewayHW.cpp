#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <chrono>
#include <functional>
#include <random>
using namespace std;

// ============================================================
// MODEL
// ============================================================
class PaymentRequest {
    string sender, receiver;
    double amount;
    string currency;
public:
    PaymentRequest(string s, string r, double a, string c = "INR")
        : sender(s), receiver(r), amount(a), currency(c) {}
    string getSender() const { return sender; }
    string getReceiver() const { return receiver; }
    double getAmount() const { return amount; }
    string getCurrency() const { return currency; }
};

// ============================================================
// BANKING SYSTEM (simulated remote)
// ============================================================
class IBankingSystem {
public:
    virtual bool processPayment(double amount) = 0;
    virtual ~IBankingSystem() {}
};

class PaytmBankingSystem : public IBankingSystem {
public:
    bool processPayment(double amount) override {
        static mt19937 rng(random_device{}());
        uniform_int_distribution<int> dist(1, 100);
        return dist(rng) <= 30;  // 30% success
    }
};

class RazorpayBankingSystem : public IBankingSystem {
public:
    bool processPayment(double amount) override {
        static mt19937 rng(random_device{}());
        uniform_int_distribution<int> dist(1, 100);
        return dist(rng) <= 90;  // 90% success
    }
};

// ============================================================
// HOMEWORK 1: RETRY STRATEGY (Strategy Pattern)
// ============================================================
class IRetryStrategy {
public:
    virtual bool executeWithRetry(
        function<bool()> operation,
        int maxRetries,
        const string& context) = 0;
    virtual ~IRetryStrategy() {}
};

// Linear retry: fixed delay between attempts
class LinearRetryStrategy : public IRetryStrategy {
    int delayMs;
public:
    LinearRetryStrategy(int delay = 0) : delayMs(delay) {}

    bool executeWithRetry(function<bool()> op, int maxRetries,
                          const string& context) override {
        for (int attempt = 1; attempt <= maxRetries; ++attempt) {
            cout << "[LinearRetry] " << context
                 << " attempt " << attempt << "/" << maxRetries << "\n";
            try {
                if (op()) return true;
            } catch (const exception& e) {
                cout << "[LinearRetry] Attempt " << attempt
                     << " error: " << e.what() << "\n";
            }
            if (delayMs > 0 && attempt < maxRetries) {
                this_thread::sleep_for(chrono::milliseconds(delayMs));
            }
        }
        return false;
    }
};

// Exponential backoff: delay doubles each attempt
class ExponentialRetryStrategy : public IRetryStrategy {
    int baseDelayMs;
public:
    ExponentialRetryStrategy(int baseDelay = 100) : baseDelayMs(baseDelay) {}

    bool executeWithRetry(function<bool()> op, int maxRetries,
                          const string& context) override {
        for (int attempt = 1; attempt <= maxRetries; ++attempt) {
            cout << "[ExpRetry] " << context
                 << " attempt " << attempt << "/" << maxRetries << "\n";
            try {
                if (op()) return true;
            } catch (const exception& e) {
                cout << "[ExpRetry] Attempt " << attempt
                     << " error: " << e.what() << "\n";
            }
            if (attempt < maxRetries) {
                int delay = baseDelayMs * (1 << (attempt - 1));  // 100, 200, 400...
                cout << "[ExpRetry] Backing off " << delay << "ms\n";
                this_thread::sleep_for(chrono::milliseconds(delay));
            }
        }
        return false;
    }
};

// ============================================================
// HOMEWORK 2: PAYMENT GATEWAY (Template Method)
// Extended with recurring payment support
// ============================================================
class PaymentGateway {
protected:
    IBankingSystem* bankingSystem;
public:
    PaymentGateway() : bankingSystem(nullptr) {}
    virtual ~PaymentGateway() { delete bankingSystem; }

    // ---- TEMPLATE METHOD 1: One-time payment ----
    virtual bool processPayment(const PaymentRequest& req) {
        if (!validatePayment(req)) {
            throw runtime_error("Validation Failed for " + req.getSender());
        }
        if (!initiatePayment(req)) {
            throw runtime_error("Initiation Failed for " + req.getSender());
        }
        if (!confirmPayment(req)) {
            throw runtime_error("Confirmation Failed for " + req.getSender());
        }
        return true;
    }

    // ---- TEMPLATE METHOD 2: Recurring payment (subscription) ----
    virtual bool processRecurringPayment(const PaymentRequest& req,
                                        int months) {
        cout << "\n=== Recurring Payment: " << months << " months for "
             << req.getSender() << " ===\n";

        if (!validateRecurringSetup(req)) {
            throw runtime_error("Recurring setup validation failed");
        }

        // Charge for first month immediately
        double totalCharged = 0;
        for (int month = 1; month <= months; ++month) {
            cout << "\n--- Month " << month << " ---\n";
            if (!processPayment(req)) {  // reuse one-time flow
                throw runtime_error("Recurring payment failed at month "
                                    + to_string(month));
            }
            totalCharged += req.getAmount();
        }

        if (!confirmRecurring(req, totalCharged)) {
            throw runtime_error("Recurring confirmation failed");
        }
        return true;
    }

    // ---- Steps (virtual, overridable) ----
    virtual bool validatePayment(const PaymentRequest&) = 0;
    virtual bool initiatePayment(const PaymentRequest&) = 0;
    virtual bool confirmPayment(const PaymentRequest&) = 0;

    // ---- Recurring-specific hooks (can be overridden) ----
    virtual bool validateRecurringSetup(const PaymentRequest& req) {
        cout << "[Default] Validating recurring setup\n";
        return req.getAmount() > 0;
    }

    virtual bool confirmRecurring(const PaymentRequest& req, double total) {
        cout << "[Default] Recurring confirmed: total ₹" << total << "\n";
        return true;
    }
};

// ============ CONCRETE: Paytm ============
class PaytmGateway : public PaymentGateway {
public:
    PaytmGateway() { bankingSystem = new PaytmBankingSystem(); }

    bool validatePayment(const PaymentRequest& req) override {
        cout << "[Paytm] Validating " << req.getSender() << "\n";
        return req.getAmount() > 0 && req.getCurrency() == "INR";
    }
    bool initiatePayment(const PaymentRequest& req) override {
        cout << "[Paytm] Initiating ₹" << req.getAmount()
             << " from " << req.getSender() << "\n";
        return bankingSystem->processPayment(req.getAmount());
    }
    bool confirmPayment(const PaymentRequest&) override {
        cout << "[Paytm] Confirmed\n";
        return true;
    }

    bool validateRecurringSetup(const PaymentRequest& req) override {
        cout << "[Paytm] Validating recurring (mandate registration)\n";
        return req.getAmount() > 0 && req.getCurrency() == "INR";
    }
};

// ============ CONCRETE: Razorpay ============
class RazorpayGateway : public PaymentGateway {
public:
    RazorpayGateway() { bankingSystem = new RazorpayBankingSystem(); }

    bool validatePayment(const PaymentRequest& req) override {
        cout << "[Razorpay] Validating " << req.getSender() << "\n";
        return req.getAmount() > 0;
    }
    bool initiatePayment(const PaymentRequest& req) override {
        cout << "[Razorpay] Initiating " << req.getCurrency() << " "
             << req.getAmount() << "\n";
        return bankingSystem->processPayment(req.getAmount());
    }
    bool confirmPayment(const PaymentRequest&) override {
        cout << "[Razorpay] Confirmed\n";
        return true;
    }

    bool validateRecurringSetup(const PaymentRequest& req) override {
        cout << "[Razorpay] Validating recurring (subscription plan)\n";
        return req.getAmount() > 0;
    }

    bool confirmRecurring(const PaymentRequest& req, double total) override {
        cout << "[Razorpay] Recurring total ₹" << total
             << " for " << req.getSender() << "\n";
        return true;
    }
};

// ============================================================
// PROXY: retry using pluggable strategy
// ============================================================
class PaymentGatewayProxy : public PaymentGateway {
private:
    PaymentGateway* realGateway;
    int maxRetries;
    IRetryStrategy* retryStrategy;

public:
    PaymentGatewayProxy(PaymentGateway* gateway, int retries,
                        IRetryStrategy* strategy)
        : realGateway(gateway), maxRetries(retries), retryStrategy(strategy) {}

    bool processPayment(const PaymentRequest& req) override {
        auto op = [this, &req]() {
            return realGateway->processPayment(req);
        };
        bool ok = retryStrategy->executeWithRetry(
            op, maxRetries, "Payment for " + req.getSender());
        if (!ok) {
            throw runtime_error("Payment failed after retries for "
                                + req.getSender());
        }
        return true;
    }

    bool processRecurringPayment(const PaymentRequest& req,
                                 int months) override {
        auto op = [this, &req, months]() {
            return realGateway->processRecurringPayment(req, months);
        };
        // Fewer retries for recurring — mandate is risky
        bool ok = retryStrategy->executeWithRetry(
            op, maxRetries, "Recurring for " + req.getSender());
        if (!ok) {
            throw runtime_error("Recurring payment failed after retries");
        }
        return true;
    }

    bool validatePayment(const PaymentRequest& r) override {
        return realGateway->validatePayment(r);
    }
    bool initiatePayment(const PaymentRequest& r) override {
        return realGateway->initiatePayment(r);
    }
    bool confirmPayment(const PaymentRequest& r) override {
        return realGateway->confirmPayment(r);
    }

    ~PaymentGatewayProxy() {
        delete realGateway;
        delete retryStrategy;
    }
};

// ============================================================
// FACTORY (Singleton)
// ============================================================
enum class GatewayType { PAYTM, RAZORPAY };
enum class RetryType { LINEAR, EXPONENTIAL };

class GatewayFactory {
private:
    static GatewayFactory* instance;
    static mutex mtx;
    GatewayFactory() {}
public:
    static GatewayFactory* getInstance() {
        lock_guard<mutex> lock(mtx);
        if (!instance) instance = new GatewayFactory();
        return instance;
    }

    PaymentGateway* getGateway(GatewayType gType, RetryType rType,
                               int maxRetries) {
        IRetryStrategy* strategy = nullptr;
        if (rType == RetryType::LINEAR) {
            strategy = new LinearRetryStrategy(50);
        } else {
            strategy = new ExponentialRetryStrategy(100);
        }

        PaymentGateway* real = (gType == GatewayType::PAYTM)
            ? static_cast<PaymentGateway*>(new PaytmGateway())
            : static_cast<PaymentGateway*>(new RazorpayGateway());

        return new PaymentGatewayProxy(real, maxRetries, strategy);
    }
};
GatewayFactory* GatewayFactory::instance = nullptr;
mutex GatewayFactory::mtx;

// ============================================================
// SERVICE + CONTROLLER
// ============================================================
class PaymentService {
private:
    static PaymentService* instance;
    static mutex mtx;
    PaymentGateway* gateway;
    PaymentService() : gateway(nullptr) {}
public:
    static PaymentService* getInstance() {
        lock_guard<mutex> lock(mtx);
        if (!instance) instance = new PaymentService();
        return instance;
    }
    void setGateway(PaymentGateway* g) {
        if (gateway) delete gateway;
        gateway = g;
    }
    bool processPayment(const PaymentRequest& r) {
        return gateway->processPayment(r);
    }
    bool processRecurring(const PaymentRequest& r, int months) {
        return gateway->processRecurringPayment(r, months);
    }
};
PaymentService* PaymentService::instance = nullptr;
mutex PaymentService::mtx;

class PaymentController {
private:
    static PaymentController* instance;
    static mutex mtx;
    PaymentService* service;
    PaymentController() : service(PaymentService::getInstance()) {}
public:
    static PaymentController* getInstance() {
        lock_guard<mutex> lock(mtx);
        if (!instance) instance = new PaymentController();
        return instance;
    }
    bool handlePayment(GatewayType gt, RetryType rt, int retries,
                       const PaymentRequest& r) {
        auto* pg = GatewayFactory::getInstance()->getGateway(gt, rt, retries);
        service->setGateway(pg);
        return service->processPayment(r);
    }
    bool handleRecurring(GatewayType gt, RetryType rt, int retries,
                         const PaymentRequest& r, int months) {
        auto* pg = GatewayFactory::getInstance()->getGateway(gt, rt, retries);
        service->setGateway(pg);
        return service->processRecurring(r, months);
    }
};
PaymentController* PaymentController::instance = nullptr;
mutex PaymentController::mtx;

// ============================================================
// CLIENT
// ============================================================
int main() {
    PaymentRequest req("Aditya", "Netflix", 199, "INR");

    cout << "========== ONE-TIME PAYMENT (Linear Retry) ==========\n";
    try {
        bool ok = PaymentController::getInstance()->handlePayment(
            GatewayType::PAYTM, RetryType::LINEAR, 3, req);
        cout << "Result: " << (ok ? "SUCCESS" : "FAIL") << "\n";
    } catch (exception& e) {
        cout << "Result: FAILED — " << e.what() << "\n";
    }

    cout << "\n========== RECURRING PAYMENT (Exponential Retry) ==========\n";
    PaymentRequest sub("Aditya", "YouTube", 129, "INR");
    try {
        bool ok = PaymentController::getInstance()->handleRecurring(
            GatewayType::RAZORPAY, RetryType::EXPONENTIAL, 2, sub, 3);
        cout << "Result: " << (ok ? "SUCCESS" : "FAIL") << "\n";
    } catch (exception& e) {
        cout << "Result: FAILED — " << e.what() << "\n";
    }
    return 0;
}