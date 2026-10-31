// zepto.cpp — Zepto / 10-minute delivery clone
// Integrates: inventory mgmt, replenish strategies, order splitting,
// rider mapping (Extension 1), payment + coupon (Extension 2).
// (Extension 3 — modularization — is orthogonal; single-file here for convenience.)

#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <string>
#include <algorithm>
using namespace std;

// ============================================================
//  MODELS
// ============================================================

class Product {
    int sku;
    string name;
    double price;
public:
    Product(int sku, string name, double price)
        : sku(sku), name(name), price(price) {}
    int getSku() const { return sku; }
    string getName() const { return name; }
    double getPrice() const { return price; }
};

class ProductFactory {
public:
    static Product* createProduct(int sku) {
        if (sku == 1001) return new Product(1001, "Apple", 20);
        if (sku == 1002) return new Product(1002, "Banana", 10);
        if (sku == 1003) return new Product(1003, "Chocolate", 50);
        if (sku == 2001) return new Product(2001, "T-Shirt", 500);
        if (sku == 2002) return new Product(2002, "Jeans", 1000);
        return new Product(sku, "Item-" + to_string(sku), 100);
    }
};

// ============================================================
//  INVENTORY — abstract store, DB impl, manager, strategies
// ============================================================

class InventoryStore {
public:
    virtual void addProduct(Product* p, int qty) = 0;
    virtual void removeProduct(int sku, int qty) = 0;
    virtual int  checkStock(int sku) = 0;
    virtual vector<Product*> listAvailableProducts() = 0;
    virtual ~InventoryStore() {}
};

class DBInventoryStore : public InventoryStore {
    map<int, int> stocks;              // SKU -> qty
    map<int, Product*> products;       // SKU -> Product*
public:
    void addProduct(Product* p, int qty) override {
        int sku = p->getSku();
        if (!products.count(sku)) products[sku] = p;
        stocks[sku] += qty;
    }
    void removeProduct(int sku, int qty) override {
        if (!stocks.count(sku)) return;
        int rem = stocks[sku] - qty;
        if (rem > 0) stocks[sku] = rem;
        else         stocks.erase(sku);
    }
    int checkStock(int sku) override {
        return stocks.count(sku) ? stocks[sku] : 0;
    }
    vector<Product*> listAvailableProducts() override {
        vector<Product*> out;
        for (auto& kv : stocks)
            if (kv.second > 0 && products.count(kv.first))
                out.push_back(products[kv.first]);
        return out;
    }
    ~DBInventoryStore() override {
        for (auto& kv : products) delete kv.second;
    }
};

class InventoryManager {
    InventoryStore* store;
public:
    InventoryManager(InventoryStore* s) : store(s) {}
    void addStock(int sku, int qty) {
        Product* p = ProductFactory::createProduct(sku);
        store->addProduct(p, qty);
    }
    void removeStock(int sku, int qty) { store->removeProduct(sku, qty); }
    int  checkStock(int sku)           { return store->checkStock(sku); }
    vector<Product*> listAvailable()   { return store->listAvailableProducts(); }
};

class ReplenishStrategy {
public:
    virtual void replenish(map<int,int>& items, InventoryManager* mgr) = 0;
    virtual ~ReplenishStrategy() {}
};

class ThresholdReplenishStrategy : public ReplenishStrategy {
    int threshold;
public:
    ThresholdReplenishStrategy(int t) : threshold(t) {}
    void replenish(map<int,int>& items, InventoryManager* mgr) override {
        for (auto& kv : items)
            if (mgr->checkStock(kv.first) < threshold)
                mgr->addStock(kv.first, kv.second);
    }
};

class WeeklyReplenishStrategy : public ReplenishStrategy {
public:
    void replenish(map<int,int>& items, InventoryManager* mgr) override {
        for (auto& kv : items) mgr->addStock(kv.first, kv.second);
    }
};

// ============================================================
//  DARK STORE + REGISTRY
// ============================================================

class DarkStore {
    string name;
    double x, y;
    InventoryManager* inventoryManager;
    ReplenishStrategy* replenishStrategy;
public:
    DarkStore(string n, double x, double y)
        : name(n), x(x), y(y), replenishStrategy(nullptr) {
        inventoryManager = new InventoryManager(new DBInventoryStore());
    }
    ~DarkStore() {
        delete inventoryManager;
        delete replenishStrategy;
    }
    double distanceTo(double ux, double uy) {
        return sqrt((x-ux)*(x-ux) + (y-uy)*(y-uy));
    }
    void setReplenishStrategy(ReplenishStrategy* s) { replenishStrategy = s; }
    void runReplenish(map<int,int>& items) {
        if (replenishStrategy)
            replenishStrategy->replenish(items, inventoryManager);
    }
    InventoryManager* getInventoryManager() { return inventoryManager; }
    string getName() { return name; }
    double getX()    { return x; }
    double getY()    { return y; }
};

class DarkStoreManager {
    static DarkStoreManager* instance;
    vector<DarkStore*> darkStores;
    DarkStoreManager() {}
public:
    static DarkStoreManager* getInstance() {
        if (!instance) instance = new DarkStoreManager();
        return instance;
    }
    void registerDarkStore(DarkStore* ds) { darkStores.push_back(ds); }
    vector<DarkStore*> getNearbyStores(double ux, double uy, double maxDist) {
        vector<pair<double, DarkStore*>> cands;
        for (auto* ds : darkStores) {
            double d = ds->distanceTo(ux, uy);
            if (d <= maxDist) cands.push_back({d, ds});
        }
        stable_sort(cands.begin(), cands.end(),
                    [](auto& a, auto& b){ return a.first < b.first; });
        vector<DarkStore*> out;
        for (auto& kv : cands) out.push_back(kv.second);
        return out;
    }
};
DarkStoreManager* DarkStoreManager::instance = nullptr;

// ============================================================
//  USER + CART
// ============================================================

class Cart {
    vector<pair<Product*, int>> items;
    double total = 0;
public:
    void addItem(int sku, int qty) {
        Product* p = ProductFactory::createProduct(sku);
        items.push_back({p, qty});
        total += p->getPrice() * qty;
    }
    double getTotal() { return total; }
    vector<pair<Product*, int>>& getItems() { return items; }
};

class User {
    string name;
    double x, y;
    Cart* cart;
public:
    User(string n, double x, double y) : name(n), x(x), y(y) {
        cart = new Cart();
    }
    ~User() { delete cart; }
    Cart* getCart()    { return cart; }
    string getName()   { return name; }
    double getX()      { return x; }
    double getY()      { return y; }
};

// ============================================================
//  DELIVERY PARTNER + ORDER
// ============================================================

class DeliveryPartner {
    int id;
    string name;
    double x, y;
    double rating;
    bool available;
public:
    DeliveryPartner(int id, string n, double x, double y, double r)
        : id(id), name(n), x(x), y(y), rating(r), available(true) {}
    int getId()          { return id; }
    string getName()     { return name; }
    double getX()        { return x; }
    double getY()        { return y; }
    double getRating()   { return rating; }
    bool isAvailable()   { return available; }
    void setAvailable(bool a) { available = a; }
    double distanceTo(double sx, double sy) {
        return sqrt((x-sx)*(x-sx) + (y-sy)*(y-sy));
    }
};

struct DeliveryAssignment {
    DeliveryPartner* rider;
    DarkStore* store;
    vector<pair<Product*, int>> items;
};

class Order {
    static int nextId;
    int orderId;
    User* user;
    vector<DeliveryAssignment> assignments;
    double subtotal, discount, total;
public:
    Order(User* u) : user(u), orderId(++nextId),
                     subtotal(0), discount(0), total(0) {}
    int getOrderId()                    { return orderId; }
    User* getUser()                     { return user; }
    void addAssignment(DeliveryAssignment a) { assignments.push_back(a); }
    vector<DeliveryAssignment>& getAssignments() { return assignments; }
    void setSubtotal(double s)          { subtotal = s; }
    void setDiscount(double d)          { discount = d; }
    void setTotal(double t)             { total    = t; }
    double getSubtotal()                { return subtotal; }
    double getDiscount()                { return discount; }
    double getTotal()                   { return total; }
};
int Order::nextId = 0;

// ============================================================
//  EXTENSION 1 — RIDER MAPPING
// ============================================================

class RiderMappingStrategy {
public:
    virtual DeliveryPartner* select(vector<DeliveryPartner*>& avail,
                                    DarkStore* store) = 0;
    virtual ~RiderMappingStrategy() {}
};

class NearestRiderStrategy : public RiderMappingStrategy {
public:
    DeliveryPartner* select(vector<DeliveryPartner*>& avail,
                            DarkStore* store) override {
        DeliveryPartner* best = nullptr;
        double bestD = 1e9;
        for (auto* r : avail) {
            double d = r->distanceTo(store->getX(), store->getY());
            if (d < bestD) { bestD = d; best = r; }
        }
        return best;
    }
};

class HighestRatedRiderStrategy : public RiderMappingStrategy {
public:
    DeliveryPartner* select(vector<DeliveryPartner*>& avail,
                            DarkStore* store) override {
        DeliveryPartner* best = nullptr;
        for (auto* r : avail) {
            if (!best
                || r->getRating() > best->getRating()
                || (r->getRating() == best->getRating()
                    && r->distanceTo(store->getX(), store->getY())
                       < best->distanceTo(store->getX(), store->getY()))) {
                best = r;
            }
        }
        return best;
    }
};

class RiderManager {
    static RiderManager* instance;
    vector<DeliveryPartner*> riders;
    RiderManager() {}
public:
    static RiderManager* getInstance() {
        if (!instance) instance = new RiderManager();
        return instance;
    }
    void registerRider(DeliveryPartner* r) { riders.push_back(r); }
    DeliveryPartner* assignRider(DarkStore* store,
                                 RiderMappingStrategy* strategy) {
        vector<DeliveryPartner*> free;
        for (auto* r : riders) if (r->isAvailable()) free.push_back(r);
        if (free.empty()) return nullptr;
        DeliveryPartner* picked = strategy->select(free, store);
        if (picked) picked->setAvailable(false);
        return picked;
    }
    void releaseRider(DeliveryPartner* r) { if (r) r->setAvailable(true); }
};
RiderManager* RiderManager::instance = nullptr;

// ============================================================
//  EXTENSION 2 — PAYMENT + COUPON
// ============================================================

class PaymentGateway {
public:
    virtual bool processPayment(double amount, User* user) = 0;
    virtual string getName() = 0;
    virtual ~PaymentGateway() {}
};

class RazorpayGateway : public PaymentGateway {
    string apiKey;
public:
    RazorpayGateway(string k) : apiKey(k) {}
    bool processPayment(double amount, User* user) override {
        cout << "[Razorpay] Charging Rs." << amount
             << " to " << user->getName() << " ... SUCCESS\n";
        return true;
    }
    string getName() override { return "Razorpay"; }
};

class StripeGateway : public PaymentGateway {
public:
    bool processPayment(double amount, User* user) override {
        cout << "[Stripe] Charging Rs." << amount
             << " to " << user->getName() << " ... SUCCESS\n";
        return true;
    }
    string getName() override { return "Stripe"; }
};

class PaymentManager {
    PaymentGateway* gateway;
public:
    PaymentManager(PaymentGateway* g) : gateway(g) {}
    bool pay(double amount, User* user) {
        bool ok = gateway->processPayment(amount, user);
        if (!ok) cout << "[Payment] FAILED via " << gateway->getName() << "\n";
        return ok;
    }
};

class Coupon {
public:
    virtual bool isApplicable(double amt) = 0;
    virtual double apply(double amt) = 0;
    virtual string getCode() = 0;
    virtual ~Coupon() {}
};

class PercentageCoupon : public Coupon {
    string code;
    double percent, maxDiscount, minAmount;
public:
    PercentageCoupon(string c, double p, double md, double ma)
        : code(c), percent(p), maxDiscount(md), minAmount(ma) {}
    bool isApplicable(double amt) override { return amt >= minAmount; }
    double apply(double amt) override {
        double d = amt * percent / 100.0;
        if (d > maxDiscount) d = maxDiscount;
        return amt - d;
    }
    string getCode() override { return code; }
};

class FlatCoupon : public Coupon {
    string code;
    double flatOff, minAmount;
public:
    FlatCoupon(string c, double f, double ma)
        : code(c), flatOff(f), minAmount(ma) {}
    bool isApplicable(double amt) override { return amt >= minAmount; }
    double apply(double amt) override {
        double r = amt - flatOff;
        return r > 0 ? r : 0;
    }
    string getCode() override { return code; }
};

// ============================================================
//  ORDER MANAGER — the core algorithm
// ============================================================

class OrderManager {
    static OrderManager* instance;
    vector<Order*> orders;
    OrderManager() {}
public:
    static OrderManager* getInstance() {
        if (!instance) instance = new OrderManager();
        return instance;
    }

    Order* placeOrder(User* user, Cart* cart,
                      DarkStoreManager* dsm,
                      RiderManager* riderMgr,
                      RiderMappingStrategy* riderStrategy,
                      PaymentManager* paymentMgr,
                      Coupon* coupon) {
        // ---- Step 1: price ----
        double subtotal = cart->getTotal();
        double discount = 0.0, payable = subtotal;
        if (coupon && coupon->isApplicable(subtotal)) {
            payable  = coupon->apply(subtotal);
            discount = subtotal - payable;
            cout << "Coupon " << coupon->getCode()
                 << " applied. Discount: Rs." << discount << "\n";
        }

        // ---- Step 2: payment (BEFORE touching stock/riders) ----
        if (!paymentMgr->pay(payable, user)) {
            cout << "Order aborted - payment failed.\n";
            return nullptr;
        }

        // ---- Step 3: nearby dark stores ----
        auto nearby = dsm->getNearbyStores(user->getX(), user->getY(), 5.0);
        if (nearby.empty()) {
            cout << "No dark store nearby.\n";
            return nullptr;
        }
        auto requested = cart->getItems();

        Order* order = new Order(user);
        order->setSubtotal(subtotal);
        order->setDiscount(discount);
        order->setTotal(payable);

        // ---- Step 4: fast path — everything from nearest store ----
        DarkStore* first = nearby[0];
        bool allInFirst = true;
        for (auto& kv : requested) {
            if (first->getInventoryManager()->checkStock(kv.first->getSku())
                < kv.second) {
                allInFirst = false;
                break;
            }
        }

        if (allInFirst) {
            DeliveryPartner* rider = riderMgr->assignRider(first, riderStrategy);
            if (!rider) { cout << "No rider available.\n"; return nullptr; }

            vector<pair<Product*, int>> storeItems;
            for (auto& kv : requested) {
                first->getInventoryManager()->removeStock(kv.first->getSku(),
                                                         kv.second);
                storeItems.push_back(kv);
            }
            order->addAssignment({rider, first, storeItems});
            orders.push_back(order);
            return order;
        }

        // ---- Step 5: split path — iterate stores, take min(avail, needed) ----
        map<int, int> allItems;
        for (auto& kv : requested) allItems[kv.first->getSku()] = kv.second;

        for (DarkStore* ds : nearby) {
            if (allItems.empty()) break;

            vector<pair<Product*, int>> storeItems;
            vector<int> satisfied;

            for (auto& kv : allItems) {
                int sku = kv.first, qtyNeeded = kv.second;
                int avail = ds->getInventoryManager()->checkStock(sku);
                if (avail <= 0) continue;

                int take = min(avail, qtyNeeded);
                ds->getInventoryManager()->removeStock(sku, take);
                storeItems.push_back({ProductFactory::createProduct(sku), take});

                if (take < qtyNeeded) allItems[sku] = qtyNeeded - take;
                else                  satisfied.push_back(sku);
            }
            for (int sku : satisfied) allItems.erase(sku);

            if (!storeItems.empty()) {
                DeliveryPartner* rider =
                    riderMgr->assignRider(ds, riderStrategy);
                if (!rider) {
                    cout << "No rider for store " << ds->getName() << "\n";
                    continue;
                }
                order->addAssignment({rider, ds, storeItems});
            }
        }

        if (!allItems.empty()) {
            cout << "WARNING: Could not fulfill - ";
            for (auto& kv : allItems)
                cout << "SKU " << kv.first << "(" << kv.second << ") ";
            cout << "\n";
        }

        // ---- Step 6: recompute total based on actual items ----
        double actual = 0;
        for (auto& a : order->getAssignments())
            for (auto& kv : a.items) actual += kv.first->getPrice() * kv.second;

        double finalTotal = actual;
        if (coupon && coupon->isApplicable(actual))
            finalTotal = coupon->apply(actual);
        order->setTotal(finalTotal);
        // (Production: refund [payable - finalTotal] here.)

        orders.push_back(order);
        return order;
    }

    vector<Order*> getAllOrders() { return orders; }
};
OrderManager* OrderManager::instance = nullptr;

// ============================================================
//  ORCHESTRATOR — Zepto facade
// ============================================================

class Zepto {
public:
    static void initialize() {
        DarkStore* a = new DarkStore("DarkStore-A", 0, 0);
        a->setReplenishStrategy(new ThresholdReplenishStrategy(10));
        a->getInventoryManager()->addStock(1001, 5);   // Apple x5
        a->getInventoryManager()->addStock(1002, 2);   // Banana x2
        cout << "Added stock to " << a->getName() << "\n";

        DarkStore* b = new DarkStore("DarkStore-B", 1, 1);
        b->getInventoryManager()->addStock(1001, 3);   // Apple x3
        b->getInventoryManager()->addStock(1003, 10);  // Chocolate x10
        cout << "Added stock to " << b->getName() << "\n";

        DarkStore* c = new DarkStore("DarkStore-C", 2, 2);
        c->getInventoryManager()->addStock(1002, 5);   // Banana x5
        c->getInventoryManager()->addStock(2001, 10);  // T-Shirt x10
        cout << "Added stock to " << c->getName() << "\n";

        DarkStoreManager::getInstance()->registerDarkStore(a);
        DarkStoreManager::getInstance()->registerDarkStore(b);
        DarkStoreManager::getInstance()->registerDarkStore(c);
    }

    static void showAllItems(User* user) {
        auto stores = DarkStoreManager::getInstance()
                        ->getNearbyStores(user->getX(), user->getY(), 5.0);
        for (auto* ds : stores)
            for (auto* p : ds->getInventoryManager()->listAvailable())
                cout << p->getSku() << " " << p->getName()
                     << " Rs." << p->getPrice() << "\n";
    }
};

// ============================================================
//  HAPPY FLOW DRIVER
// ============================================================

int main() {
    Zepto::initialize();

    // --- seed riders ---
    auto* rm = RiderManager::getInstance();
    rm->registerRider(new DeliveryPartner(1, "Ravi",  0.5, 0.5, 4.2));
    rm->registerRider(new DeliveryPartner(2, "Sita",  1.5, 1.5, 4.8));
    rm->registerRider(new DeliveryPartner(3, "Mohan", 2.5, 2.5, 3.9));
    rm->registerRider(new DeliveryPartner(4, "Priya", 0.2, 0.9, 4.9));

    // --- user + cart ---
    User* u = new User("Aditya", 1.0, 0.0);
    cout << "\nUser " << u->getName() << " comes on platform\n\n";

    Cart* c = u->getCart();
    c->addItem(1001, 4);   // Apple x4
    c->addItem(1002, 3);   // Banana x3
    c->addItem(1003, 2);   // Chocolate x2

    // --- coupon / payment / rider strategy ---
    Coupon*               coupon   = new PercentageCoupon("SAVE10", 10, 50, 100);
    PaymentManager*       pm       = new PaymentManager(new RazorpayGateway("rzp_test"));
    RiderMappingStrategy* strategy = new HighestRatedRiderStrategy();

    cout << "All available items near user:\n";
    Zepto::showAllItems(u);
    cout << "\n";

    // --- place order ---
    Order* o = OrderManager::getInstance()->placeOrder(
        u, c,
        DarkStoreManager::getInstance(),
        rm,
        strategy,
        pm,
        coupon);

    if (o) {
        cout << "\n===== ORDER #" << o->getOrderId() << " =====\n";
        cout << "Subtotal: Rs." << o->getSubtotal()
             << "  Discount: Rs." << o->getDiscount()
             << "  Paid: Rs."     << o->getTotal() << "\n";
        for (auto& a : o->getAssignments()) {
            cout << "  " << a.rider->getName() << " -> "
                 << a.store->getName() << " : ";
            for (auto& kv : a.items)
                cout << kv.first->getName() << " x" << kv.second << "  ";
            cout << "\n";
        }
    }
    return 0;
}