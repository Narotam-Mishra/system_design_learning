
# System Design Learning

## 01. Introduction To System Design (24:46)

## Introduction to Low-Level Design (LLD)

## Core Concept Overview

This tutorial introduces **Low-Level Design (LLD)** - the process of designing the internal structure of an application, focusing on how different components, objects, and entities interact with each other.

### Key Definition

> **LLD** is the complete application design built around DSA concepts - when you take multiple DSA algorithms and build a complete, working application structure around them.

---

## The Anurag vs. Maurya Story: LLD vs DSA

### The Scenario

Two friends join **QuickRide** (an Ola/Uber-like ride booking company):
- **Anurag**: Expert in DSA, knows nothing about LLD
- **Maurya**: Expert in both DSA and LLD

### Problem 1: Finding Shortest Route

**Anurag's Approach (DSA-focused):**
```
1. Identify problem: User needs to go from source to destination
2. Model city as graph:
   - Intersections = Nodes
   - Roads = Edges
3. Apply Dijkstra's Algorithm for shortest path
4. Done!
```

**What Anurag Missed:**
- No object/entity identification
- No relationships between components
- No data security considerations
- No notification system design
- No payment gateway integration
- No scalability planning

**Maurya's Approach (LLD-focused):**

```
Step 1: Identify Objects/Entities
- User
- Rider
- Location
- Notification
- Payment Gateway

Step 2: Define Relationships
- User books ride → Rider assigned
- Rider has Location
- User receives Notification
- Payment processes transaction

Step 3: Consider Non-Functional Requirements
- Data Security (phone number masking)
- Scalability (millions of users)
- Maintainability

Step 4: THEN apply DSA
- Dijkstra for routing
- Priority Queue for nearest rider
```

### Problem 2: Rider Assignment

**Anurag's DSA Solution:**
```java
// Simple Priority Queue approach
PriorityQueue<Rider> nearestRiders = new PriorityQueue<>(
    Comparator.comparingDouble(r -> r.distanceTo(user))
);

// Add all riders within radius
for (Rider r : availableRiders) {
    if (r.distanceTo(user) <= RADIUS) {
        nearestRiders.add(r);
    }
}

// Assign closest rider
Rider assignedRider = nearestRiders.poll();
```

**Maurya's LLD Approach:**
```java
// 1. Define the RiderMapping interface (Reusable)
public interface RiderMappingStrategy {
    Rider findBestRider(User user, List<Rider> availableRiders);
}

// 2. Implement specific strategies
public class NearestRiderStrategy implements RiderMappingStrategy {
    @Override
    public Rider findBestRider(User user, List<Rider> availableRiders) {
        return availableRiders.stream()
            .filter(r -> r.isAvailable())
            .min(Comparator.comparingDouble(r -> r.distanceTo(user)))
            .orElse(null);
    }
}

// 3. Make it plug-and-play for any application
public class DeliveryAssignmentService {
    private RiderMappingStrategy strategy;
    
    public DeliveryAssignmentService(RiderMappingStrategy strategy) {
        this.strategy = strategy;
    }
    
    public void assignDelivery(Order order, List<DeliveryPartner> partners) {
        // Same algorithm works for Zomato, Swiggy, Amazon, etc.
    }
}
```

---

## Three Pillars of LLD

### 1. Scalability

**Definition:** How well your application can handle growth in users, data, and traffic.

```java
// Example: Database Connection Pooling for Scalability
public class ConnectionPool {
    private static final int MAX_CONNECTIONS = 100;
    private BlockingQueue<Connection> availableConnections;
    
    public Connection getConnection() throws InterruptedException {
        if (availableConnections.isEmpty()) {
            // Create new connection or wait
            return createNewConnection();
        }
        return availableConnections.take();
    }
    
    public void releaseConnection(Connection conn) {
        availableConnections.offer(conn);
    }
}
```

**Key Points:**
- Handle millions of users simultaneously
- Application should not crash under load
- Easy to add new features without breaking existing ones

---

### 2. Maintainability

**Definition:** How easily code can be debugged, modified, and extended.

```java
// BAD: Hard to maintain
public class RideService {
    public void bookRide(User u, String paymentType) {
        // 500 lines of code doing everything
        if (paymentType.equals("credit")) {
            // process credit card
        } else if (paymentType.equals("debit")) {
            // process debit card
        } else if (paymentType.equals("upi")) {
            // process UPI
        }
        // ... notification code
        // ... rider assignment code
        // ... route calculation code
    }
}

// GOOD: Maintainable with Single Responsibility
public class RideService {
    private PaymentProcessor paymentProcessor;
    private NotificationService notificationService;
    private RiderAssignmentService riderService;
    private RouteCalculator routeCalculator;
    
    public Ride bookRide(RideRequest request) {
        Payment payment = paymentProcessor.process(request.getPaymentDetails());
        Rider rider = riderService.assignRider(request.getUser());
        Route route = routeCalculator.calculate(request.getSource(), request.getDestination());
        
        Ride ride = new Ride(request.getUser(), rider, route, payment);
        notificationService.notifyRideConfirmed(ride);
        
        return ride;
    }
}
```

**Key Points:**
- New features shouldn't break old ones
- Easy to find and fix bugs
- Code should be self-documenting

---

### 3. Reusability

**Definition:** Code should be easily reusable across different applications (Plug-and-Play model).

```
┌─────────────────────────────────────────────────────────────┐
│                    REUSABLE COMPONENTS                       │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐       │
│  │ Notification │  │   Payment    │  │    Rider     │       │
│  │   Service    │  │   Gateway    │  │   Mapping    │       │
│  └──────────────┘  └──────────────┘  └──────────────┘       │
│         │                 │                 │               │
│         ▼                 ▼                 ▼               │
│  ┌─────────────────────────────────────────────────┐        │
│  │              QuickRide Application               │        │
│  └─────────────────────────────────────────────────┘        │
│                                                              │
│  ┌─────────────────────────────────────────────────┐        │
│  │              Zomato Application                  │        │
│  └─────────────────────────────────────────────────┘        │
│                                                              │
│  ┌─────────────────────────────────────────────────┐        │
│  │              Amazon Application                  │        │
│  └─────────────────────────────────────────────────┘        │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

```java
// Reusable Payment Interface
public interface PaymentGateway {
    PaymentResult processPayment(PaymentRequest request);
    RefundResult processRefund(RefundRequest request);
}

// Implementations can be swapped
public class StripePaymentGateway implements PaymentGateway { ... }
public class RazorpayPaymentGateway implements PaymentGateway { ... }
public class PaytmPaymentGateway implements PaymentGateway { ... }

// Any application can use any gateway
public class QuickRide {
    private PaymentGateway paymentGateway;
    
    public QuickRide(PaymentGateway gateway) {
        this.paymentGateway = gateway;  // Dependency Injection
    }
}
```

**Key Point:** Avoid **Tight Coupling** - code should not be fixed to a specific application.

---

## LLD vs HLD vs DSA

### Comparison Table

| Aspect | DSA | LLD | HLD |
|--------|-----|-----|-----|
| **Focus** | Algorithms & Data Structures | Code Structure & Object Design | System Architecture |
| **Scope** | Isolated problems | Single application internals | Multiple systems integration |
| **Output** | Algorithm/Function | Class Diagrams, Code | System Design Diagrams |
| **Example** | Dijkstra's Algorithm | Class design for QuickRide | Server setup, Database choice |
| **Code Written** | Yes (algorithm) | Yes (full application) | Minimal/Negligible |

### Visual Representation

```
┌─────────────────────────────────────────────────────────────────┐
│                     APPLICATION BUILDING                         │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│   ┌─────────────┐                                               │
│   │    HLD      │  ← System Architecture                        │
│   │             │    • Tech Stack (Java/Spring Boot)            │
│   │             │    • Database (SQL/NoSQL)                     │
│   │             │    • Server Scaling                           │
│   │             │    • Cost Optimization                        │
│   └─────────────┘                                               │
│          │                                                       │
│          ▼                                                       │
│   ┌─────────────┐                                               │
│   │    LLD      │  ← Code Structure                             │
│   │             │    • Classes & Objects                        │
│   │             │    • Relationships                            │
│   │             │    • Design Patterns                          │
│   │             │    • Scalability, Maintainability, Reusability│
│   └─────────────┘                                               │
│          │                                                       │
│          ▼                                                       │
│   ┌─────────────┐                                               │
│   │    DSA      │  ← Brain of Application                       │
│   │             │    • Algorithms (Dijkstra, Sorting)           │
│   │             │    • Data Structures (Heap, Graph)            │
│   └─────────────┘                                               │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

### The Golden Rule

```
┌─────────────────────────────────────────────────────────────────┐
│                                                                  │
│   "If DSA is the BRAIN of an application,                        │
│    LLD is the SKELETON."                                         │
│                                                                  │
│   • Brain (DSA): Thinks and processes                           │
│   • Skeleton (LLD): Provides structure and support              │
│   • Both needed for a working application                       │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

---

## Complete QuickRide LLD Example

### Class Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                         QUICKRIDE SYSTEM                         │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌──────────────┐         ┌──────────────┐                      │
│  │    User      │         │    Rider     │                      │
│  ├──────────────┤         ├──────────────┤                      │
│  │ - id         │         │ - id         │                      │
│  │ - name       │         │ - name       │                      │
│  │ - phone      │         │ - phone      │                      │
│  │ - location   │         │ - location   │                      │
│  ├──────────────┤         ├──────────────┤                      │
│  │ + bookRide() │         │ + acceptRide()│                     │
│  │ + makePayment│         │ + updateLoc() │                     │
│  └──────────────┘         └──────────────┘                      │
│         │                        │                               │
│         │         ┌──────────────┴──────────────┐               │
│         │         │                             │               │
│         ▼         ▼                             ▼               │
│  ┌──────────────────────┐              ┌──────────────┐         │
│  │        Ride          │              │   Location   │         │
│  ├──────────────────────┤              ├──────────────┤         │
│  │ - id                 │              │ - latitude   │         │
│  │ - user               │              │ - longitude  │         │
│  │ - rider              │              ├──────────────┤         │
│  │ - source             │              │ + distanceTo()│        │
│  │ - destination        │              └──────────────┘         │
│  │ - status             │                                       │
│  │ - fare               │                                       │
│  ├──────────────────────┤                                       │
│  │ + start()            │                                       │
│  │ + complete()         │                                       │
│  │ + cancel()           │                                       │
│  └──────────────────────┘                                       │
│                                                                  │
│  ┌──────────────────────┐    ┌──────────────────────┐          │
│  │  PaymentService      │    │  NotificationService │          │
│  ├──────────────────────┤    ├──────────────────────┤          │
│  │ + processPayment()   │    │ + sendNotification() │          │
│  │ + processRefund()    │    │ + notifyRider()      │          │
│  └──────────────────────┘    │ + notifyUser()       │          │
│                               └──────────────────────┘          │
│                                                                  │
│  ┌──────────────────────┐    ┌──────────────────────┐          │
│  │ RiderMappingService  │    │  RouteCalculator     │          │
│  ├──────────────────────┤    ├──────────────────────┤          │
│  │ + findNearestRider() │    │ + findShortestPath() │          │
│  │ + assignRider()      │    │ + calculateETA()     │          │
│  └──────────────────────┘    └──────────────────────┘          │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

### Complete Implementation

```java
// ==================== ENTITIES ====================

public class Location {
    private double latitude;
    private double longitude;
    
    public double distanceTo(Location other) {
        // Haversine formula
        double latDiff = Math.toRadians(other.latitude - this.latitude);
        double lonDiff = Math.toRadians(other.longitude - this.longitude);
        double a = Math.sin(latDiff/2) * Math.sin(latDiff/2) +
                   Math.cos(Math.toRadians(this.latitude)) * 
                   Math.cos(Math.toRadians(other.latitude)) *
                   Math.sin(lonDiff/2) * Math.sin(lonDiff/2);
        double c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a));
        return 6371 * c; // Earth's radius in km
    }
}

public class User {
    private String id;
    private String name;
    private String phoneNumber;  // Sensitive - should be masked
    private Location currentLocation;
    
    public Ride bookRide(Location destination) {
        RideRequest request = new RideRequest(this, currentLocation, destination);
        return RideService.getInstance().processRideRequest(request);
    }
}

public class Rider {
    private String id;
    private String name;
    private String phoneNumber;  // Sensitive - should be masked
    private Location currentLocation;
    private boolean isAvailable;
    private double rating;
    
    public void acceptRide(Ride ride) {
        this.isAvailable = false;
        ride.setRider(this);
        ride.setStatus(RideStatus.ACCEPTED);
    }
    
    public void updateLocation(Location newLocation) {
        this.currentLocation = newLocation;
    }
}

// ==================== SERVICES ====================

// Strategy Pattern for Rider Mapping (Reusable)
public interface RiderMappingStrategy {
    Rider findBestRider(Location userLocation, List<Rider> availableRiders);
}

public class NearestRiderStrategy implements RiderMappingStrategy {
    @Override
    public Rider findBestRider(Location userLocation, List<Rider> availableRiders) {
        PriorityQueue<RiderDistance> pq = new PriorityQueue<>(
            Comparator.comparingDouble(RiderDistance::getDistance)
        );
        
        for (Rider rider : availableRiders) {
            if (rider.isAvailable()) {
                double distance = userLocation.distanceTo(rider.getCurrentLocation());
                pq.offer(new RiderDistance(rider, distance));
            }
        }
        
        RiderDistance best = pq.poll();
        return best != null ? best.getRider() : null;
    }
}

// Route Calculator using Dijkstra (DSA inside LLD)
public class RouteCalculator {
    private Graph cityGraph;
    
    public Route findShortestPath(Location source, Location destination) {
        Node sourceNode = cityGraph.findNearestNode(source);
        Node destNode = cityGraph.findNearestNode(destination);
        
        Map<Node, Double> distances = new HashMap<>();
        Map<Node, Node> previous = new HashMap<>();
        PriorityQueue<NodeDistance> pq = new PriorityQueue<>(
            Comparator.comparingDouble(NodeDistance::getDistance)
        );
        
        distances.put(sourceNode, 0.0);
        pq.offer(new NodeDistance(sourceNode, 0.0));
        
        while (!pq.isEmpty()) {
            NodeDistance current = pq.poll();
            Node currentNode = current.getNode();
            
            if (currentNode.equals(destNode)) break;
            
            for (Edge edge : currentNode.getEdges()) {
                Node neighbor = edge.getDestination();
                double newDistance = distances.get(currentNode) + edge.getWeight();
                
                if (newDistance < distances.getOrDefault(neighbor, Double.MAX_VALUE)) {
                    distances.put(neighbor, newDistance);
                    previous.put(neighbor, currentNode);
                    pq.offer(new NodeDistance(neighbor, newDistance));
                }
            }
        }
        
        return buildRoute(previous, sourceNode, destNode);
    }
}

// Payment Service (Reusable across applications)
public interface PaymentGateway {
    PaymentResult processPayment(PaymentRequest request);
}

public class PaymentService {
    private PaymentGateway gateway;
    
    public PaymentService(PaymentGateway gateway) {
        this.gateway = gateway;
    }
    
    public Payment processPayment(Ride ride, PaymentMethod method) {
        PaymentRequest request = new PaymentRequest(
            ride.getFare(),
            method,
            ride.getUser().getId()
        );
        
        PaymentResult result = gateway.processPayment(request);
        
        if (result.isSuccess()) {
            return new Payment(result.getTransactionId(), ride.getFare(), PaymentStatus.COMPLETED);
        } else {
            throw new PaymentFailedException(result.getErrorMessage());
        }
    }
}

// Notification Service (Reusable)
public interface NotificationChannel {
    void send(String recipient, String message);
}

public class NotificationService {
    private List<NotificationChannel> channels;
    
    public NotificationService(List<NotificationChannel> channels) {
        this.channels = channels;
    }
    
    public void notifyRideConfirmed(Ride ride) {
        String userMessage = String.format(
            "Your ride is confirmed! Rider %s will arrive in %d minutes.",
            ride.getRider().getName(),
            ride.getETA()
        );
        
        String riderMessage = String.format(
            "New ride assigned. Pickup at %s.",
            ride.getSource().getAddress()
        );
        
        for (NotificationChannel channel : channels) {
            channel.send(ride.getUser().getPhoneNumber(), userMessage);
            channel.send(ride.getRider().getPhoneNumber(), riderMessage);
        }
    }
}

// ==================== MAIN SERVICE (Facade) ====================

public class RideService {
    private static RideService instance;
    
    private RiderMappingStrategy riderMappingStrategy;
    private RouteCalculator routeCalculator;
    private PaymentService paymentService;
    private NotificationService notificationService;
    private RiderRepository riderRepository;
    private RideRepository rideRepository;
    
    private RideService() {
        // Initialize with default strategies
        this.riderMappingStrategy = new NearestRiderStrategy();
        this.routeCalculator = new RouteCalculator();
        this.paymentService = new PaymentService(new StripePaymentGateway());
        this.notificationService = new NotificationService(
            Arrays.asList(new PushNotificationChannel(), new SMSChannel())
        );
    }
    
    public static synchronized RideService getInstance() {
        if (instance == null) {
            instance = new RideService();
        }
        return instance;
    }
    
    public Ride processRideRequest(RideRequest request) {
        // Step 1: Find available riders
        List<Rider> availableRiders = riderRepository.findAvailableRiders(
            request.getSource(), 
            RADIUS_KM
        );
        
        // Step 2: Assign best rider (DSA - Priority Queue)
        Rider rider = riderMappingStrategy.findBestRider(
            request.getSource(), 
            availableRiders
        );
        
        if (rider == null) {
            throw new NoRiderAvailableException();
        }
        
        // Step 3: Calculate route (DSA - Dijkstra)
        Route route = routeCalculator.findShortestPath(
            request.getSource(), 
            request.getDestination()
        );
        
        // Step 4: Create ride
        Ride ride = new Ride(
            generateRideId(),
            request.getUser(),
            rider,
            request.getSource(),
            request.getDestination(),
            route,
            calculateFare(route)
        );
        
        // Step 5: Save ride
        rideRepository.save(ride);
        
        // Step 6: Notify user and rider
        notificationService.notifyRideConfirmed(ride);
        
        return ride;
    }
    
    public void completeRide(Ride ride) {
        ride.setStatus(RideStatus.COMPLETED);
        
        // Process payment
        Payment payment = paymentService.processPayment(
            ride, 
            ride.getUser().getPreferredPaymentMethod()
        );
        
        ride.setPayment(payment);
        rideRepository.update(ride);
        
        // Notify both parties
        notificationService.notifyRideCompleted(ride);
    }
}
```

---

## Key Takeaways

### 1. LLD is About Structure, Not Just Algorithms

```
DSA solves: "How to find shortest path?"
LLD solves: "How to build a ride-booking application that uses shortest path finding?"
```

### 2. The Three Pillars Must Be Balanced

```
                    SCALABILITY
                        ▲
                       / \
                      /   \
                     /     \
                    /       \
                   /         \
                  /           \
                 /             \
                ▼               ▼
        MAINTAINABILITY ◄────► REUSABILITY
```

### 3. Think Before You Code

```
Wrong Order: Problem → Algorithm → Application
Right Order: Problem → Entities → Relationships → Design → Algorithm → Application
```

### 4. Reusability Through Abstraction

- **Interfaces** for plug-and-play components
- **Dependency Injection** for flexibility
- **Strategy Pattern** for interchangeable algorithms

### 5. The Ultimate Summary

```
┌─────────────────────────────────────────────────────────────────┐
│                                                                  │
│   DSA = Brain (Processing Logic)                                │
│   LLD = Skeleton (Structure & Organization)                     │
│   HLD = Body Systems (Infrastructure & Scaling)                 │
│                                                                  │
│   All three together = Complete Working Application             │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

---

## Prerequisites & Course Structure

- **Language:** C++ (Java source code also provided)
- **Duration:** 30-35 videos
- **Coverage:** Complete LLD concepts from beginner to advanced
- **Resources:** Full notes + GitHub repository with all code
- **Outcome:** Confident in LLD interviews, ready for top companies

---

## Conclusion

This tutorial establishes that **LLD is the critical bridge** between knowing algorithms (DSA) and building real-world applications. It teaches you to:

1. **Identify** objects and entities in a system
2. **Design** relationships between components
3. **Apply** design patterns for scalability, maintainability, and reusability
4. **Integrate** DSA algorithms as tools within a well-designed structure

The QuickRide example demonstrates that a successful application requires:
- **HLD** for system architecture decisions
- **LLD** for code structure and object design
- **DSA** for efficient problem-solving within that structure

---

## 02. OOPs Real-World Examples | OOPs Pillars | Abstraction | Encapsulation (54:00)

## Overview

This lecture covers the **foundation of LLD - Object-Oriented Programming (OOP)**. It explains:
1. History of programming paradigms
2. Why OOPs was needed
3. OOPs vs Procedural Programming
4. The Ideology of OOPs
5. **Abstraction** (with code)
6. **Encapsulation** (with code)

> **Note:** Inheritance and Polymorphism are covered in Part 2 of this lecture.

---

## 1. History of Programming Paradigms

### Evolution Timeline

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                      EVOLUTION OF PROGRAMMING                                │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  ┌─────────────────┐                                                         │
│  │ MACHINE LANGUAGE │  ← Binary (0s and 1s), Direct CPU interaction          │
│  │    (1940s)       │     Example: 01010110 10110101                         │
│  └────────┬────────┘                                                         │
│           │ Problems: Very error-prone, tedious, not scalable               │
│           ▼                                                                  │
│  ┌─────────────────┐                                                         │
│  │ ASSEMBLY LANGUAGE│  ← Mnemonics (MOV, ADD), English keywords              │
│  │    (1950s)       │     Example: MOV A, 61H                                │
│  └────────┬────────┘                                                         │
│           │ Problems: Tightly coupled with hardware, still error-prone      │
│           ▼                                                                  │
│  ┌─────────────────┐                                                         │
│  │   PROCEDURAL     │  ← Functions, Loops, If-Else, Switch                   │
│  │   PROGRAMMING    │     Example: C Language                                │
│  │    (1960s-70s)   │     Recipe-book style: "Do this, then do this"        │
│  └────────┬────────┘                                                         │
│           │ Problems: Not scalable for large apps, no real-world modeling   │
│           ▼                                                                  │
│  ┌─────────────────┐                                                         │
│  │ OBJECT-ORIENTED  │  ← Classes, Objects, Inheritance, Polymorphism         │
│  │   PROGRAMMING    │     Example: C++, Java, Python                         │
│  │    (1980s+)      │     Models real-world entities                         │
│  └─────────────────┘                                                         │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### Detailed Comparison

| Aspect | Machine Language | Assembly | Procedural | OOP |
|--------|-----------------|----------|------------|-----|
| **Unit** | Binary instructions | Mnemonics | Functions | Objects |
| **Readability** | Extremely poor | Poor | Good | Excellent |
| **Error Prone** | Very high | High | Medium | Low |
| **Scalability** | None | Very low | Low | High |
| **Real-world modeling** | No | No | No | Yes |
| **Reusability** | None | None | Low | High |
| **Hardware coupling** | Tight | Tight | Loose | Very loose |

---

## 2. Why OOPs Matter: Real-World Modeling

### The Core Ideology

> **"Just like your real world works, programming should work the same way."**

In the real world:
- **Everything is an object** (You, me, mic, laptop, car, TV)
- **Objects interact with each other** (You interact with laptop, I interact with mic)

So in programming:
- We should **declare objects**
- We should **make objects interact** with each other

### The Fundamental Truth

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                                                                              │
│   Whenever you see OOP code, just understand:                               │
│                                                                              │
│   "Nothing is happening. A bunch of objects are interacting with each       │
│    other. One object calls another object's method, or one object is        │
│    passed as a parameter to another object. That's it."                     │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### What is an Object?

An object has **two things**:

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              OBJECT                                          │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│   ┌─────────────────────────┐    ┌─────────────────────────┐                │
│   │    CHARACTERISTICS      │    │       BEHAVIOR          │                │
│   │    (Attributes/Data)    │    │    (Methods/Functions)  │                │
│   ├─────────────────────────┤    ├─────────────────────────┤                │
│   │ • Brand                 │    │ • startEngine()         │                │
│   │ • Model                 │    │ • shiftGear()           │                │
│   │ • isEngineOn            │    │ • accelerate()          │                │
│   │ • currentSpeed          │    │ • brake()               │                │
│   │ • currentGear           │    │ • stopEngine()          │                │
│   └─────────────────────────┘    └─────────────────────────┘                │
│                                                                              │
│   Real-life Car Example:                                                     │
│   • Characteristics: Brand=Ford, Model=Mustang, Engine=On                    │
│   • Behavior: Start, Stop, Accelerate, Brake, Shift Gear                    │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 3. OOPs vs Procedural Programming

### The Car & Owner Example

**Scenario:** An owner owns a car and drives it.

### Procedural Approach (Without OOPs)

```java
// PROBLEM: No classes, only variables and functions

// Car characteristics as separate variables
String brand1 = "Ford";
String model1 = "Mustang";
boolean isEngineOn1 = false;

String brand2 = "Toyota";
String model2 = "Camry";
boolean isEngineOn2 = false;

// Owner characteristics as separate variables
String owner1Name = "John";
String owner2Name = "Jane";

// Functions for behavior
void start(String brand, String model) {
    System.out.println("Starting " + brand + " " + model);
}

void stop(String brand, String model) {
    System.out.println("Stopping " + brand + " " + model);
}

void shiftGear(String brand, String model, int gear) {
    System.out.println("Shifting " + brand + " to gear " + gear);
}

// Owner drives car
void drive(String ownerName, String brand, String model) {
    start(brand, model);
    shiftGear(brand, model, 1);
    // accelerate...
    stop(brand, model);
}

// Calling
drive(owner1Name, brand1, model1);
```

**Problems with Procedural Approach:**

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    PROCEDURAL PROGRAMMING PROBLEMS                          │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  1. POOR REAL-WORLD MODELING                                                │
│     • Can't represent "Owner owns Car" naturally                            │
│     • Just a set of methods calling each other                              │
│     • Hard to understand the actual relationship                            │
│                                                                              │
│  2. NO DATA SECURITY                                                        │
│     • All variables are accessible from anywhere                            │
│     • Anyone can change engine state, speed, etc.                           │
│                                                                              │
│  3. NOT SCALABLE                                                            │
│     • Adding a new car means duplicating all variables                      │
│     • brand3, model3, isEngineOn3... and so on                              │
│                                                                              │
│  4. NOT REUSABLE                                                            │
│     • Can't easily reuse car logic in another application                   │
│     • Tightly coupled with specific variable names                          │
│                                                                              │
│  5. HARD TO MAINTAIN                                                        │
│     • Changing one thing requires changes in many places                    │
│     • No encapsulation means bugs spread easily                             │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### OOP Approach

```java
// SOLUTION: Use classes and objects

// Car class - blueprint for all cars
class Car {
    // Characteristics (Attributes)
    private String brand;
    private String model;
    private boolean isEngineOn;
    private int currentSpeed;
    private int currentGear;
    
    // Constructor
    public Car(String brand, String model) {
        this.brand = brand;
        this.model = model;
        this.isEngineOn = false;
        this.currentSpeed = 0;
        this.currentGear = 0;
    }
    
    // Behaviors (Methods)
    public void startEngine() {
        isEngineOn = true;
        System.out.println(brand + " " + model + ": Engine started");
    }
    
    public void shiftGear(int gear) {
        if (!isEngineOn) {
            System.out.println("Engine is off. Can't shift gear.");
            return;
        }
        this.currentGear = gear;
        System.out.println("Shifted to gear " + gear);
    }
    
    public void accelerate() {
        if (!isEngineOn) {
            System.out.println("Engine is off. Can't accelerate.");
            return;
        }
        currentSpeed += 20;
        System.out.println("Accelerating to " + currentSpeed + " km/h");
    }
    
    public void brake() {
        currentSpeed = Math.max(0, currentSpeed - 20);
        System.out.println("Braking. Speed: " + currentSpeed + " km/h");
    }
    
    public void stopEngine() {
        isEngineOn = false;
        currentSpeed = 0;
        currentGear = 0;
        System.out.println("Engine turned off");
    }
    
    // Getters
    public int getCurrentSpeed() { return currentSpeed; }
    public String getBrand() { return brand; }
    public String getModel() { return model; }
}

// Owner class
class Owner {
    private String name;
    private Car car;  // Owner HAS-A car (Composition)
    
    public Owner(String name, Car car) {
        this.name = name;
        this.car = car;
    }
    
    public void drive() {
        System.out.println(name + " is driving...");
        car.startEngine();
        car.shiftGear(1);
        car.accelerate();
        car.shiftGear(2);
        car.accelerate();
        car.brake();
        car.stopEngine();
    }
}

// Main
public class Main {
    public static void main(String[] args) {
        Car myCar = new Car("Ford", "Mustang");
        Owner owner = new Owner("John", myCar);
        owner.drive();
    }
}
```

**Benefits of OOP Approach:**

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                       OOP BENEFITS                                           │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  1. REAL-WORLD MODELING                                                     │
│     • Owner owns Car - naturally represented                                │
│     • Car has brand, model, engine state                                    │
│     • Owner drives Car - clear interaction                                  │
│                                                                              │
│  2. DATA SECURITY                                                           │
│     • Private variables - can't be changed directly                         │
│     • Controlled access through methods                                     │
│                                                                              │
│  3. SCALABLE                                                                │
│     • Create as many Car objects as needed                                  │
│     • Car car1 = new Car("Ford", "Mustang");                                │
│     • Car car2 = new Car("Toyota", "Camry");                                │
│                                                                              │
│  4. REUSABLE                                                                │
│     • Car class can be used in any application                              │
│     • Plug-and-play model                                                   │
│                                                                              │
│  5. MAINTAINABLE                                                            │
│     • Changes in one place don't break everything                           │
│     • Easy to debug                                                         │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 4. Pillars of OOPs

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         FOUR PILLARS OF OOPS                                 │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│                    ┌─────────────────────────┐                              │
│                    │      ABSTRACTION        │                              │
│                    │   (Hide unnecessary     │                              │
│                    │    details from client)  │                              │
│                    └───────────┬─────────────┘                              │
│                                │                                             │
│                    ┌───────────▼─────────────┐                              │
│                    │     ENCAPSULATION       │                              │
│                    │   (Wrap data & methods  │                              │
│                    │    + Data Security)     │                              │
│                    └───────────┬─────────────┘                              │
│                                │                                             │
│                    ┌───────────▼─────────────┐                              │
│                    │      INHERITANCE        │                              │
│                    │   (Child class inherits │                              │
│                    │    parent properties)   │                              │
│                    └───────────┬─────────────┘                              │
│                                │                                             │
│                    ┌───────────▼─────────────┐                              │
│                    │      POLYMORPHISM       │                              │
│                    │   (Many forms - same    │                              │
│                    │    method, different    │                              │
│                    │    behavior)            │                              │
│                    └─────────────────────────┘                              │
│                                                                              │
│   Note: This lecture covers Abstraction & Encapsulation                     │
│         Next part covers Inheritance & Polymorphism                         │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 5. Abstraction (Deep Dive)

### Definition

> **Abstraction hides unnecessary details from a client and showcases only what is necessary.**

### Real-World Analogy: Driving a Car

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    DRIVING A CAR - WHAT YOU NEED TO KNOW                     │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│   YOU NEED TO KNOW:                    YOU DON'T NEED TO KNOW:              │
│   ┌─────────────────────┐              ┌─────────────────────┐              │
│   │ • Turn key/press    │              │ • How engine works  │              │
│   │   start button      │              │ • How gearbox works │              │
│   │ • Press clutch      │              │ • How fuel injection│              │
│   │ • Shift gear        │              │   works             │              │
│   │ • Press accelerator │              │ • How brakes work   │              │
│   │ • Press brake       │              │   internally        │              │
│   └─────────────────────┘              └─────────────────────┘              │
│                                                                              │
│   The car provides an INTERFACE (steering, pedals, gear)                    │
│   You interact with the interface, not the internal complexity              │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### More Examples

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    ABSTRACTION IN DAILY LIFE                                 │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│   ┌──────────────┐    ┌──────────────┐    ┌──────────────┐                 │
│   │   LAPTOP     │    │     TV       │    │   MOBILE     │                 │
│   ├──────────────┤    ├──────────────┤    ├──────────────┤                 │
│   │ Interface:   │    │ Interface:   │    │ Interface:   │                 │
│   │ • Screen     │    │ • Remote     │    │ • Touchscreen│                 │
│   │ • Keyboard   │    │ • Power btn  │    │ • Buttons    │                 │
│   │ • Touchpad   │    │ • Volume btn │    │ • Apps       │                 │
│   ├──────────────┤    ├──────────────┤    ├──────────────┤                 │
│   │ Hidden:      │    │ Hidden:      │    │ Hidden:      │                 │
│   │ • CPU arch   │    │ • Wiring     │    │ • OS kernel  │                 │
│   │ • RAM timing │    │ • Signal     │    │ • Hardware   │                 │
│   │ • Motherboard│    │   processing │    │   drivers    │                 │
│   └──────────────┘    └──────────────┘    └──────────────┘                 │
│                                                                              │
│   Programming Languages themselves are an abstraction!                      │
│   • You write: if (x > 0) { ... }                                          │
│   • Compiler converts to: 01010110 10110101 ...                            │
│   • You don't need to know how binary works                                 │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### Abstraction: Two Objects Interacting

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    ABSTRACTION BETWEEN OBJECTS                               │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│   ┌─────────────────────┐              ┌─────────────────────┐              │
│   │     OBJECT 1        │              │     OBJECT 2        │              │
│   │    (Client)         │              │    (Service)        │              │
│   ├─────────────────────┤              ├─────────────────────┤              │
│   │                     │              │                     │              │
│   │  Only needs to know │─────────────▶│  • Hidden Data      │              │
│   │  about behaviors:   │              │  • Hidden Methods   │              │
│   │  • B1               │              │                     │              │
│   │  • B2               │              │  • Public Behavior  │              │
│   │                     │              │    B1, B2           │              │
│   │  Doesn't need to    │              │                     │              │
│   │  know HOW B1, B2    │              │  "I'll handle the   │              │
│   │  are implemented    │              │   implementation"   │              │
│   │                     │              │                     │              │
│   └─────────────────────┘              └─────────────────────┘              │
│                                                                              │
│   Key: Client only knows WHAT the object can do,                            │
│        not HOW it does it                                                   │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### Code Example: Abstraction with Abstract Class

```java
// ABSTRACT CLASS - The Interface (Blueprint)
abstract class Car {
    // Abstract methods - only declarations, no implementation
    // Child class MUST implement these
    public abstract void startEngine();
    public abstract void shiftGear(int gear);
    public abstract void accelerate();
    public abstract void brake();
    public abstract void stopEngine();
    
    // Destructor (C++ concept)
    // In Java, we use finalize() or AutoCloseable
}

// CONCRETE CLASS - Actual implementation
class SportsCar extends Car {
    private String brand;
    private String model;
    private boolean isEngineOn;
    private int currentSpeed;
    private int currentGear;
    
    public SportsCar(String brand, String model) {
        this.brand = brand;
        this.model = model;
        this.isEngineOn = false;
        this.currentSpeed = 0;
        this.currentGear = 0;
    }
    
    @Override
    public void startEngine() {
        isEngineOn = true;
        System.out.println(brand + " " + model + ": Engine starts with a roar!");
    }
    
    @Override
    public void shiftGear(int gear) {
        if (!isEngineOn) {
            System.out.println("Engine is off. Can't shift gear.");
            return;
        }
        this.currentGear = gear;
        System.out.println("Shifted to gear " + gear);
    }
    
    @Override
    public void accelerate() {
        if (!isEngineOn) {
            System.out.println("Engine is off. Can't accelerate.");
            return;
        }
        currentSpeed += 20;
        System.out.println("Accelerating to " + currentSpeed + " km/h");
    }
    
    @Override
    public void brake() {
        currentSpeed = Math.max(0, currentSpeed - 20);
        System.out.println("Braking. Speed: " + currentSpeed + " km/h");
    }
    
    @Override
    public void stopEngine() {
        isEngineOn = false;
        currentSpeed = 0;
        currentGear = 0;
        System.out.println("Engine turned off");
    }
}

// MAIN - Client Code
public class Main {
    public static void main(String[] args) {
        // Parent reference pointing to child object
        Car myCar = new SportsCar("Ford", "Mustang");
        
        // Client only knows the interface (abstract methods)
        // Doesn't need to know HOW these are implemented
        myCar.startEngine();    // Output: Ford Mustang: Engine starts with a roar!
        myCar.shiftGear(1);     // Output: Shifted to gear 1
        myCar.accelerate();     // Output: Accelerating to 20 km/h
        myCar.shiftGear(2);     // Output: Shifted to gear 2
        myCar.accelerate();     // Output: Accelerating to 40 km/h
        myCar.brake();          // Output: Braking. Speed: 20 km/h
        myCar.stopEngine();     // Output: Engine turned off
    }
}
```

### Abstraction Flow Diagram

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         ABSTRACTION FLOW                                     │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │                      ABSTRACT CLASS: Car                             │   │
│   │  ┌─────────────────────────────────────────────────────────────┐   │   │
│   │  │  virtual void startEngine() = 0;  // Pure virtual           │   │   │
│   │  │  virtual void shiftGear(int) = 0; // Pure virtual           │   │   │
│   │  │  virtual void accelerate() = 0;   // Pure virtual           │   │   │
│   │  │  virtual void brake() = 0;        // Pure virtual           │   │   │
│   │  │  virtual void stopEngine() = 0;   // Pure virtual           │   │   │
│   │  └─────────────────────────────────────────────────────────────┘   │   │
│   │                                                                      │   │
│   │  "I only tell WHAT to do, not HOW to do it"                         │   │
│   │  "Child classes will provide the HOW"                               │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│                                    │                                         │
│                                    │ extends/inherits                        │
│                                    ▼                                         │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │                    CONCRETE CLASS: SportsCar                         │   │
│   │  ┌─────────────────────────────────────────────────────────────┐   │   │
│   │  │  void startEngine() { isEngineOn = true; ... }              │   │   │
│   │  │  void shiftGear(int g) { currentGear = g; ... }             │   │   │
│   │  │  void accelerate() { currentSpeed += 20; ... }              │   │   │
│   │  │  void brake() { currentSpeed -= 20; ... }                   │   │   │
│   │  │  void stopEngine() { isEngineOn = false; ... }              │   │   │
│   │  └─────────────────────────────────────────────────────────────┘   │   │
│   │                                                                      │   │
│   │  "I provide the actual implementation of all abstract methods"      │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│                                    │                                         │
│                                    │ Client uses                            │
│                                    ▼                                         │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │                         CLIENT CODE                                  │   │
│   │                                                                      │   │
│   │   Car myCar = new SportsCar("Ford", "Mustang");                     │   │
│   │   myCar.startEngine();  // Client doesn't know internals            │   │
│   │   myCar.accelerate();   // Just calls the interface                 │   │
│   │   myCar.brake();                                                     │   │
│   │                                                                      │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 6. Encapsulation (Deep Dive)

### Definition

> **Encapsulation is the bundling of data (characteristics) and methods (behaviors) that operate on that data into a single unit (class), AND providing data security.**

### The Capsule Analogy

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         CAPSULE ANALOGY                                      │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │                         MEDICINE CAPSULE                             │   │
│   │  ┌─────────────────────────────────────────────────────────────┐   │   │
│   │  │                                                              │   │   │
│   │  │   ┌─────────────────────────────────────────────────────┐   │   │   │
│   │  │   │              PROTECTED LAYER                        │   │   │   │
│   │  │   │   ┌─────────────────────────────────────────────┐   │   │   │   │
│   │  │   │   │           MEDICINE (Data)                   │   │   │   │   │
│   │  │   │   │                                             │   │   │   │   │
│   │  │   │   │   • Can't be accessed from outside          │   │   │   │   │
│   │  │   │   │   • Must go through the capsule layer       │   │   │   │   │
│   │  │   │   │                                             │   │   │   │   │
│   │  │   │   └─────────────────────────────────────────────┘   │   │   │   │
│   │  │   │                                                      │   │   │   │
│   │  │   └─────────────────────────────────────────────────────┘   │   │   │
│   │  │                                                              │   │   │
│   │  └─────────────────────────────────────────────────────────────┘   │   │
│   │                                                                      │   │
│   │   Similarly in OOP:                                                  │   │
│   │   • Class = Capsule                                                  │   │
│   │   • Private Data = Medicine (protected)                              │   │
│   │   • Public Methods = The way to interact with the capsule            │   │
│   │                                                                      │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### Encapsulation Has TWO Requirements

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    TWO REQUIREMENTS OF ENCAPSULATION                         │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│   REQUIREMENT 1: BUNDLING                                                    │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │  All characteristics AND behaviors of an object                     │   │
│   │  must be bundled together in a single class.                        │   │
│   │                                                                      │   │
│   │  ┌─────────────────────────────────────────────────────────────┐   │   │
│   │  │                    CLASS: Car                                │   │   │
│   │  │  ┌─────────────────────┐  ┌─────────────────────────────┐  │   │   │
│   │  │  │   CHARACTERISTICS   │  │        BEHAVIORS            │  │   │   │
│   │  │  │   (Variables)       │  │        (Methods)            │  │   │   │
│   │  │  ├─────────────────────┤  ├─────────────────────────────┤  │   │   │
│   │  │  │ • brand             │  │ • startEngine()             │  │   │   │
│   │  │  │ • model             │  │ • shiftGear()               │  │   │   │
│   │  │  │ • isEngineOn        │  │ • accelerate()              │  │   │   │
│   │  │  │ • currentSpeed      │  │ • brake()                   │  │   │   │
│   │  │  │ • currentGear       │  │ • stopEngine()              │  │   │   │
│   │  │  └─────────────────────┘  └─────────────────────────────┘  │   │   │
│   │  └─────────────────────────────────────────────────────────────┘   │   │
│   │                                                                      │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│                                                                              │
│   REQUIREMENT 2: DATA SECURITY                                               │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │  Some data must be protected from outside access.                   │   │
│   │  Use access modifiers to control visibility.                        │   │
│   │                                                                      │   │
│   │  ┌─────────────────────────────────────────────────────────────┐   │   │
│   │  │  private int currentSpeed;  // Can't be accessed directly   │   │   │
│   │  │  public int getCurrentSpeed() { return currentSpeed; }      │   │   │
│   │  │  // Controlled access through getter                        │   │   │
│   │  └─────────────────────────────────────────────────────────────┘   │   │
│   │                                                                      │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### Abstraction vs Encapsulation: Key Difference

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    ABSTRACTION vs ENCAPSULATION                              │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│   ┌─────────────────────────────┐    ┌─────────────────────────────┐        │
│   │        ABSTRACTION          │    │       ENCAPSULATION         │        │
│   ├─────────────────────────────┤    ├─────────────────────────────┤        │
│   │                             │    │                             │        │
│   │  Focus: DATA HIDING         │    │  Focus: DATA SECURITY       │        │
│   │                             │    │                             │        │
│   │  "You don't NEED to know    │    │  "You MUST NOT know         │        │
│   │   how engine works"         │    │   the odometer value"       │        │
│   │                             │    │                             │        │
│   │  If you find out,           │    │  If you find out,           │        │
│   │  no harm done               │    │  security threat!           │        │
│   │                             │    │                             │        │
│   │  Example:                   │    │  Example:                   │        │
│   │  • Car engine internals     │    │  • Odometer reading         │        │
│   │  • TV wiring                │    │  • User passwords           │        │
│   │  • Laptop motherboard       │    │  • Bank account balance     │        │
│   │                             │    │                             │        │
│   │  Implementation:            │    │  Implementation:            │        │
│   │  • Abstract classes         │    │  • Private variables        │        │
│   │  • Interfaces               │    │  • Getters/Setters          │        │
│   │                             │    │  • Access modifiers         │        │
│   │                             │    │                             │        │
│   └─────────────────────────────┘    └─────────────────────────────┘        │
│                                                                              │
│   KEY INSIGHT:                                                               │
│   Abstraction is about DESIGN (what to show)                                │
│   Encapsulation is about IMPLEMENTATION (how to protect)                    │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### Access Modifiers

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                         ACCESS MODIFIERS (C++/Java)                          │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│   ┌──────────────┬─────────────────────────────────────────────────────┐    │
│   │  MODIFIER    │                  ACCESS LEVEL                        │    │
│   ├──────────────┼─────────────────────────────────────────────────────┤    │
│   │              │  Same Class │ Same Package │ Child Class │ World   │    │
│   │  public      │     ✓       │      ✓      │      ✓      │   ✓     │    │
│   │  protected   │     ✓       │      ✓      │      ✓      │   ✗     │    │
│   │  private     │     ✓       │      ✗      │      ✗      │   ✗     │    │
│   │  default     │     ✓       │      ✓      │      ✗      │   ✗     │    │
│   └──────────────┴─────────────────────────────────────────────────────┘    │
│                                                                              │
│   In C++: public, private, protected                                         │
│   In Java: public, private, protected, default (package-private)            │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

### Code Example: Encapsulation with Getters & Setters

```java
// ENCAPSULATION EXAMPLE

public class SportsCar {
    // CHARACTERISTICS - All private (Data Security)
    private String brand;
    private String model;
    private boolean isEngineOn;
    private int currentSpeed;
    private int currentGear;
    private String tyre;  // New characteristic
    
    // Constructor
    public SportsCar(String brand, String model) {
        this.brand = brand;
        this.model = model;
        this.isEngineOn = false;
        this.currentSpeed = 0;
        this.currentGear = 0;
        this.tyre = "MRF";  // Default tyre
    }
    
    // ==================== BEHAVIORS (Public Methods) ====================
    
    public void startEngine() {
        isEngineOn = true;
        System.out.println(brand + " " + model + ": Engine started");
    }
    
    public void shiftGear(int gear) {
        if (!isEngineOn) {
            System.out.println("Engine is off. Can't shift gear.");
            return;
        }
        this.currentGear = gear;
        System.out.println("Shifted to gear " + gear);
    }
    
    public void accelerate() {
        if (!isEngineOn) {
            System.out.println("Engine is off. Can't accelerate.");
            return;
        }
        currentSpeed += 20;
        System.out.println("Accelerating to " + currentSpeed + " km/h");
    }
    
    public void brake() {
        currentSpeed = Math.max(0, currentSpeed - 20);
        System.out.println("Braking. Speed: " + currentSpeed + " km/h");
    }
    
    public void stopEngine() {
        isEngineOn = false;
        currentSpeed = 0;
        currentGear = 0;
        System.out.println("Engine turned off");
    }
    
    // ==================== GETTERS & SETTERS ====================
    
    // GETTER for currentSpeed (Read-only - no setter!)
    public int getCurrentSpeed() {
        return this.currentSpeed;
    }
    // Note: No setCurrentSpeed() - you can't directly set speed
    // You must use accelerate() or brake()
    
    // GETTER and SETTER for tyre (Both allowed - with validation)
    public String getTyre() {
        return this.tyre;
    }
    
    public void setTyre(String tyre) {
        // VALIDATION: Check if tyre is valid
        if (tyre == null || tyre.isEmpty()) {
            System.out.println("Invalid tyre name!");
            return;
        }
        
        // VALIDATION: Check if tyre brand exists
        if (!isValidTyreBrand(tyre)) {
            System.out.println("Unknown tyre brand: " + tyre);
            return;
        }
        
        this.tyre = tyre;
        System.out.println("Tyre changed to: " + tyre);
    }
    
    private boolean isValidTyreBrand(String tyre) {
        return tyre.equals("MRF") || tyre.equals("CEAT") || 
               tyre.equals("Apollo") || tyre.equals("Michelin");
    }
    
    // Getters for brand and model (read-only)
    public String getBrand() { return brand; }
    public String getModel() { return model; }
}

// MAIN - Demonstrating Encapsulation
public class Main {
    public static void main(String[] args) {
        SportsCar myCar = new SportsCar("Ford", "Mustang");
        
        // ✅ ALLOWED: Using public methods
        myCar.startEngine();
        myCar.shiftGear(1);
        myCar.accelerate();
        myCar.shiftGear(2);
        myCar.accelerate();
        myCar.brake();
        
        // ✅ ALLOWED: Reading current speed through getter
        System.out.println("Current Speed: " + myCar.getCurrentSpeed());
        
        // ❌ NOT ALLOWED: Direct access to private variable
        // myCar.currentSpeed = 500;  // ERROR: currentSpeed has private access
        
        // ✅ ALLOWED: Using setter with validation
        myCar.setTyre("Michelin");  // Valid
        myCar.setTyre("FakeBrand"); // Invalid - will be rejected
        
        myCar.stopEngine();
    }
}
```

### What Happens Without Encapsulation

```java
// BAD: All public - No encapsulation
class SportsCarBad {
    public String brand;
    public String model;
    public boolean isEngineOn;
    public int currentSpeed;  // PUBLIC - Anyone can change!
    public int currentGear;
    
    // ...
}

// MAIN - Problems
public class Main {
    public static void main(String[] args) {
        SportsCarBad myCar = new SportsCarBad();
        
        // PROBLEM: Directly setting impossible speed!
        myCar.currentSpeed = 500;  // A sports car going 500 km/h?!
        System.out.println("Speed: " + myCar.currentSpeed);  // Output: 500
        
        // PROBLEM: Setting speed to negative!
        myCar.currentSpeed = -100;  // Negative speed?!
        
        // PROBLEM: Changing engine state directly!
        myCar.isEngineOn = true;  // Without proper initialization!
        
        // This is why we need ENCAPSULATION!
    }
}
```

### Encapsulation with Real-World Example: Odometer

```java
public class Car {
    // Private - can't be accessed directly
    private int odometerReading;  // Total km driven
    private int currentSpeed;
    
    public Car() {
        this.odometerReading = 0;
        this.currentSpeed = 0;
    }
    
    // Odometer can only be READ, not written directly
    public int getOdometerReading() {
        return this.odometerReading;
    }
    
    // No setOdometerReading() method!
    // Odometer increases automatically as car moves
    
    public void accelerate() {
        if (currentSpeed < 200) {
            currentSpeed += 10;
            // Odometer increases based on speed and time
            odometerReading += 1;  // Simplified: 1 km per acceleration
        }
    }
    
    public void brake() {
        currentSpeed = Math.max(0, currentSpeed - 10);
    }
}

// Usage
public class Main {
    public static void main(String[] args) {
        Car car = new Car();
        
        // Can READ odometer
        System.out.println("Odometer: " + car.getOdometerReading());  // 0
        
        // Can't directly SET odometer
        // car.odometerReading = 25000;  // ERROR: private access
        
        // Odometer changes through legitimate use
        car.accelerate();  // Odometer becomes 1
        car.accelerate();  // Odometer becomes 2
        car.accelerate();  // Odometer becomes 3
        
        System.out.println("Odometer: " + car.getOdometerReading());  // 3
    }
}
```

---

## 7. Complete Summary Diagram

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    OOP CONCEPTS - COMPLETE OVERVIEW                          │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │                         WHY OOPs?                                    │   │
│   │  • Real-world modeling                                               │   │
│   │  • Data security                                                     │   │
│   │  • Scalability                                                       │   │
│   │  • Reusability                                                       │   │
│   │  • Maintainability                                                   │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│                                    │                                         │
│                                    ▼                                         │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │                      IDEOLOGY OF OOPs                                │   │
│   │  "Just like your real world works, programming should work."        │   │
│   │  Objects exist and interact with each other.                        │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│                                    │                                         │
│                                    ▼                                         │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │                        OBJECT = DATA + BEHAVIOR                      │   │
│   │  ┌─────────────────────────┐  ┌─────────────────────────┐          │   │
│   │  │   CHARACTERISTICS       │  │       BEHAVIOR          │          │   │
│   │  │   (Variables)           │  │       (Methods)         │          │   │
│   │  │   • brand               │  │       • startEngine()   │          │   │
│   │  │   • model               │  │       • accelerate()    │          │   │
│   │  │   • currentSpeed        │  │       • brake()         │          │   │
│   │  └─────────────────────────┘  └─────────────────────────┘          │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│                                    │                                         │
│                                    ▼                                         │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │                         FOUR PILLARS                                 │   │
│   │                                                                      │   │
│   │  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  ┌──────────┐│   │
│   │  │ ABSTRACTION  │  │ENCAPSULATION │  │ INHERITANCE  │  │POLYMORPH ││   │
│   │  ├──────────────┤  ├──────────────┤  ├──────────────┤  ├──────────┤│   │
│   │  │ Hide         │  │ Bundle data  │  │ Child class  │  │ Many     ││   │
│   │  │ unnecessary  │  │ + methods    │  │ inherits     │  │ forms    ││   │
│   │  │ details      │  │ + Security   │  │ parent       │  │ of same  ││   │
│   │  │              │  │              │  │ properties   │  │ method   ││   │
│   │  │ Focus:       │  │ Focus:       │  │              │  │          ││   │
│   │  │ DATA HIDING  │  │ DATA SECURITY│  │              │  │          ││   │
│   │  │              │  │              │  │              │  │          ││   │
│   │  │ Impl:        │  │ Impl:        │  │ Impl:        │  │ Impl:    ││   │
│   │  │ Abstract     │  │ Private vars │  │ extends      │  │ Override ││   │
│   │  │ classes,     │  │ + Getters/   │  │ keyword      │  │ methods  ││   │
│   │  │ Interfaces   │  │ Setters      │  │              │  │          ││   │
│   │  └──────────────┘  └──────────────┘  └──────────────┘  └──────────┘│   │
│   │                                                                      │   │
│   │  ◀─── COVERED IN THIS LECTURE ───▶  ◀── NEXT LECTURE ──▶            │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 8. Key Takeaways

### Abstraction
- **What:** Hides unnecessary details, shows only what's needed
- **Why:** Simplifies interaction, reduces complexity
- **How:** Abstract classes, interfaces, pure virtual functions
- **Real-world:** Car pedals/steering, TV remote, laptop screen
- **Focus:** DATA HIDING (design level)

### Encapsulation
- **What:** Bundles data + methods in a class AND provides data security
- **Why:** Protects sensitive data, controls access, enables validation
- **How:** Access modifiers (private/public/protected), getters/setters
- **Real-world:** Car odometer (read-only), speed (controlled via accelerate/brake)
- **Focus:** DATA SECURITY (implementation level)

### The Golden Difference
```
Abstraction: "You don't NEED to know" → Design decision
Encapsulation: "You MUST NOT know" → Security decision
```

### Prerequisites for Next Lecture
- Inheritance (Child class extends Parent class)
- Polymorphism (Same method, different behavior)
- Types: Compile-time (overloading) vs Runtime (overriding)

---

## 03. Inheritance & Polymorphism in OOPs (44:38)

This lecture completes the four pillars of Object-Oriented Programming (OOP) for Low-Level Design (LLD). It covers **Inheritance** and **Polymorphism**, building on the previous lecture's **Abstraction** and **Encapsulation**. The same `Car` example is extended throughout.

---

## 1. Inheritance

### Concept

Inheritance models **parent-child relationships** between real-world objects. A child class inherits characteristics and behaviors from a parent class, and can add its own specific features.

**Real-world example:**
- Parent: `Car` (generic)
- Children: `ManualCar`, `ElectricCar`

```
┌─────────────────────────────────────────────────────────────┐
│                          Car                                │
│  Characteristics: brand, model, isEngineOn, currentSpeed    │
│  Behaviors: startEngine(), stopEngine(), accelerate(),      │
│             brake()                                         │
└──────────────────────────┬──────────────────────────────────┘
                           │
           ┌───────────────┴───────────────┐
           │                               │
           ▼                               ▼
┌─────────────────────┐         ┌─────────────────────┐
│    ManualCar        │         │    ElectricCar      │
│  + currentGear      │         │  + batteryPercentage│
│  + shiftGear()      │         │  + chargeBattery()  │
└─────────────────────┘         └─────────────────────┘
```

### Code Example (C++)

```cpp
#include <iostream>
#include <string>
using namespace std;

class Car {
protected:  // accessible by child classes, not outside
    string brand;
    string model;
    bool isEngineOn;
    int currentSpeed;

public:
    Car(string b, string m) : brand(b), model(m), isEngineOn(false), currentSpeed(0) {}

    void startEngine() {
        isEngineOn = true;
        cout << brand << " " << model << ": Engine started\n";
    }

    void stopEngine() {
        isEngineOn = false;
        currentSpeed = 0;
        cout << "Engine turned off\n";
    }

    void accelerate() {
        if (!isEngineOn) { cout << "Engine is off. Can't accelerate.\n"; return; }
        currentSpeed += 10;
        cout << "Accelerating to " << currentSpeed << " km/h\n";
    }

    void brake() {
        currentSpeed = max(0, currentSpeed - 10);
        cout << "Braking. Speed: " << currentSpeed << " km/h\n";
    }
};

class ManualCar : public Car {
private:
    int currentGear;
public:
    ManualCar(string b, string m) : Car(b, m), currentGear(0) {}

    void shiftGear(int gear) {
        currentGear = gear;
        cout << "Shifted to gear " << gear << "\n";
    }
};

class ElectricCar : public Car {
private:
    int batteryPercentage;
public:
    ElectricCar(string b, string m) : Car(b, m), batteryPercentage(100) {}

    void chargeBattery() {
        batteryPercentage = 100;
        cout << "Battery fully charged\n";
    }
};

int main() {
    ManualCar wagonR("Suzuki", "Wagon R");
    wagonR.startEngine();
    wagonR.shiftGear(1);
    wagonR.accelerate();
    wagonR.brake();
    wagonR.stopEngine();

    ElectricCar tesla("Tesla", "Model S");
    tesla.startEngine();
    tesla.accelerate();
    tesla.chargeBattery();
    tesla.stopEngine();
}
```

**Output:**
```
Suzuki Wagon R: Engine started
Shifted to gear 1
Accelerating to 10 km/h
Braking. Speed: 0 km/h
Engine turned off
Tesla Model S: Engine started
Accelerating to 10 km/h
Battery fully charged
Engine turned off
```

### Access Modifiers & Inheritance Types

| Modifier | Same Class | Child Class | Outside |
|----------|------------|-------------|---------|
| `public` | ✓ | ✓ | ✓ |
| `protected` | ✓ | ✓ | ✗ |
| `private` | ✓ | ✗ | ✗ |

**Inheritance access specifiers:**

| Inheritance Type | `public` members in parent become | `protected` members become |
|------------------|-----------------------------------|----------------------------|
| `public` | `public` | `protected` |
| `protected` | `protected` | `protected` |
| `private` | `private` | `private` |

> **Practical note:** In 99% of real-world LLD and system design, **public inheritance** is used. Private/protected inheritance breaks the "is-a" relationship and is rarely used.

---

## 2. Polymorphism

### Concept

**Polymorphism** = "many forms". The same method call can behave differently depending on the object or parameters.

**Two real-world scenarios:**
1. Different animals (`Duck`, `Human`, `Tiger`) all have `run()`, but each runs differently.
2. The same `Human` runs differently depending on situation (tired vs. chased by a tiger) — same object, different parameter.

### Types of Polymorphism

```
┌─────────────────────────────────────────────────────────────┐
│                      POLYMORPHISM                           │
├──────────────────────────┬──────────────────────────────────┤
│   Dynamic Polymorphism   │     Static Polymorphism          │
│   (Runtime)              │     (Compile-time)               │
│   Method Overriding      │     Method Overloading           │
│   Virtual functions      │     Same name, different params  │
└──────────────────────────┴──────────────────────────────────┘
```

---

### Dynamic Polymorphism (Method Overriding)

**Definition:** A child class provides a specific implementation of a method already declared in the parent class. The method signature must be identical. In C++, the parent method must be `virtual`.

**Car Example:**
- `Car` declares `virtual void accelerate()` and `virtual void brake()`.
- `ManualCar` overrides `accelerate()` to increase speed by 20 km/h.
- `ElectricCar` overrides `accelerate()` to increase speed by 15 km/h and decrease battery.

```cpp
class Car {
protected:
    string brand, model;
    bool isEngineOn;
    int currentSpeed;
public:
    Car(string b, string m) : brand(b), model(m), isEngineOn(false), currentSpeed(0) {}

    void startEngine() { isEngineOn = true; cout << brand << " " << model << ": Engine started\n"; }
    void stopEngine() { isEngineOn = false; currentSpeed = 0; cout << "Engine turned off\n"; }

    virtual void accelerate() = 0;  // pure virtual
    virtual void brake() = 0;
};

class ManualCar : public Car {
private:
    int currentGear;
public:
    ManualCar(string b, string m) : Car(b, m), currentGear(0) {}

    void shiftGear(int gear) { currentGear = gear; cout << "Shifted to gear " << gear << "\n"; }

    void accelerate() override {
        if (!isEngineOn) return;
        currentSpeed += 20;
        cout << "Manual accelerating to " << currentSpeed << " km/h\n";
    }

    void brake() override {
        currentSpeed = max(0, currentSpeed - 20);
        cout << "Manual braking. Speed: " << currentSpeed << " km/h\n";
    }
};

class ElectricCar : public Car {
private:
    int batteryPercentage;
public:
    ElectricCar(string b, string m) : Car(b, m), batteryPercentage(100) {}

    void chargeBattery() { batteryPercentage = 100; cout << "Battery fully charged\n"; }

    void accelerate() override {
        if (!isEngineOn || batteryPercentage <= 0) return;
        batteryPercentage -= 5;
        currentSpeed += 15;
        cout << "Electric accelerating to " << currentSpeed << " km/h, battery: "
             << batteryPercentage << "%\n";
    }

    void brake() override {
        currentSpeed = max(0, currentSpeed - 15);
        cout << "Regenerative braking. Speed: " << currentSpeed << " km/h\n";
    }
};

int main() {
    ManualCar wagonR("Suzuki", "Wagon R");
    ElectricCar tesla("Tesla", "Model S");

    wagonR.startEngine();
    wagonR.accelerate();  // 20 km/h
    wagonR.accelerate();  // 40 km/h
    wagonR.brake();       // 20 km/h
    wagonR.stopEngine();

    tesla.startEngine();
    tesla.accelerate();   // 15 km/h, battery 95%
    tesla.accelerate();   // 30 km/h, battery 90%
    tesla.brake();        // 15 km/h
    tesla.stopEngine();
}
```

**Output:**
```
Suzuki Wagon R: Engine started
Manual accelerating to 20 km/h
Manual accelerating to 40 km/h
Manual braking. Speed: 20 km/h
Engine turned off
Tesla Model S: Engine started
Electric accelerating to 15 km/h, battery: 95%
Electric accelerating to 30 km/h, battery: 90%
Regenerative braking. Speed: 15 km/h
Engine turned off
```

---

### Static Polymorphism (Method Overloading)

**Definition:** Multiple methods with the **same name** but **different parameter lists** (different number or types of arguments). Resolved at compile time.

**Car Example:**
- `ManualCar` has two `accelerate()` methods:
  - `accelerate()` → default acceleration (+20 km/h)
  - `accelerate(int speed)` → custom acceleration

```cpp
class ManualCar {
private:
    string brand, model;
    bool isEngineOn;
    int currentSpeed;
    int currentGear;
public:
    ManualCar(string b, string m) : brand(b), model(m), isEngineOn(false),
                                    currentSpeed(0), currentGear(0) {}

    void startEngine() { isEngineOn = true; cout << "Engine started\n"; }
    void stopEngine() { isEngineOn = false; currentSpeed = 0; cout << "Engine turned off\n"; }

    // Overloaded methods
    void accelerate() {
        if (!isEngineOn) return;
        currentSpeed += 20;
        cout << "Accelerating to " << currentSpeed << " km/h\n";
    }

    void accelerate(int speed) {
        if (!isEngineOn) return;
        currentSpeed += speed;
        cout << "Accelerating by " << speed << " to " << currentSpeed << " km/h\n";
    }

    void brake() {
        currentSpeed = max(0, currentSpeed - 20);
        cout << "Braking. Speed: " << currentSpeed << " km/h\n";
    }

    void shiftGear(int gear) {
        currentGear = gear;
        cout << "Shifted to gear " << gear << "\n";
    }
};

int main() {
    ManualCar car("Suzuki", "Wagon R");
    car.startEngine();
    car.accelerate();      // +20
    car.accelerate(40);    // +40 → total 60
    car.brake();           // -20 → 40
    car.stopEngine();
}
```

**Output:**
```
Engine started
Accelerating to 20 km/h
Accelerating by 40 to 60 km/h
Braking. Speed: 40 km/h
Engine turned off
```

---

## 3. Combined Example: All Four Pillars

The lecture ends with a single program that demonstrates **Abstraction, Encapsulation, Inheritance, and both types of Polymorphism**.

```cpp
// Abstract base class (Abstraction)
class Car {
protected:  // Encapsulation (protected access)
    string brand, model;
    bool isEngineOn;
    int currentSpeed;
public:
    Car(string b, string m) : brand(b), model(m), isEngineOn(false), currentSpeed(0) {}

    void startEngine() { isEngineOn = true; cout << brand << " " << model << ": Engine started\n"; }
    void stopEngine() { isEngineOn = false; currentSpeed = 0; cout << "Engine turned off\n"; }

    // Dynamic polymorphism (virtual + overloading)
    virtual void accelerate() = 0;
    virtual void accelerate(int speed) = 0;
    virtual void brake() = 0;
};

class ManualCar : public Car {  // Inheritance
private:
    int currentGear;
public:
    ManualCar(string b, string m) : Car(b, m), currentGear(0) {}

    void shiftGear(int gear) { currentGear = gear; cout << "Shifted to gear " << gear << "\n"; }

    void accelerate() override { if (!isEngineOn) return; currentSpeed += 20; cout << "Manual accelerating to " << currentSpeed << " km/h\n"; }
    void accelerate(int speed) override { if (!isEngineOn) return; currentSpeed += speed; cout << "Manual accelerating by " << speed << " to " << currentSpeed << " km/h\n"; }
    void brake() override { currentSpeed = max(0, currentSpeed - 20); cout << "Manual braking. Speed: " << currentSpeed << " km/h\n"; }
};

class ElectricCar : public Car {
private:
    int batteryPercentage;
public:
    ElectricCar(string b, string m) : Car(b, m), batteryPercentage(100) {}

    void chargeBattery() { batteryPercentage = 100; cout << "Battery fully charged\n"; }

    void accelerate() override { if (!isEngineOn || batteryPercentage <= 0) return; batteryPercentage -= 5; currentSpeed += 15; cout << "Electric accelerating to " << currentSpeed << " km/h, battery: " << batteryPercentage << "%\n"; }
    void accelerate(int speed) override { if (!isEngineOn || batteryPercentage <= 0) return; batteryPercentage -= 5; currentSpeed += speed; cout << "Electric accelerating by " << speed << " to " << currentSpeed << " km/h, battery: " << batteryPercentage << "%\n"; }
    void brake() override { currentSpeed = max(0, currentSpeed - 15); cout << "Regenerative braking. Speed: " << currentSpeed << " km/h\n"; }
};

int main() {
    ManualCar wagonR("Suzuki", "Wagon R");
    ElectricCar tesla("Tesla", "Model S");

    wagonR.startEngine();
    wagonR.accelerate();      // overridden + overloaded
    wagonR.accelerate(40);
    wagonR.brake();
    wagonR.stopEngine();

    tesla.startEngine();
    tesla.accelerate();
    tesla.accelerate(30);
    tesla.brake();
    tesla.stopEngine();
}
```

**This single example demonstrates:**
- **Abstraction:** `Car` hides implementation details behind pure virtual functions.
- **Encapsulation:** All data members are `protected` or `private`.
- **Inheritance:** `ManualCar` and `ElectricCar` inherit from `Car`.
- **Dynamic Polymorphism:** `accelerate()` and `brake()` are overridden.
- **Static Polymorphism:** `accelerate()` and `accelerate(int)` are overloaded.

---

## 4. Key Takeaways

| Concept | Definition | Key Mechanism |
|---------|------------|---------------|
| **Inheritance** | Child class acquires properties of parent | `class Child : public Parent` |
| **Dynamic Polymorphism** | Same method signature, different behavior at runtime | `virtual` + `override` |
| **Static Polymorphism** | Same method name, different parameters at compile time | Method overloading |
| **Access Modifiers** | Control visibility | `public`, `protected`, `private` |

**Method Overloading vs Overriding:**

| Feature | Overloading | Overriding |
|---------|-------------|------------|
| Signature | Same name, different parameters | Same name, same parameters |
| Class | Same class (or across inheritance) | Parent-child |
| Binding | Compile-time | Runtime |
| Keyword | — | `virtual` / `override` |

**Practical note:** Always use **public inheritance** in real LLD. Private/protected inheritance is rare and breaks the "is-a" relationship.

---

## 5. Homework (from the lecture)

1. **What is operator overloading in C++?** Provide examples.
2. **Why do Java and Python not support operator overloading, while C++ does?** Discuss design trade-offs.

---

## 6. Summary Diagram: All Four Pillars Together

```
┌─────────────────────────────────────────────────────────────────────┐
│                         OOP PILLARS                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   ┌──────────────┐    ┌──────────────┐    ┌──────────────┐          │
│   │ ABSTRACTION  │    │ENCAPSULATION │    │ INHERITANCE  │          │
│   │              │    │              │    │              │          │
│   │ Hide details │    │ Bundle data  │    │ Child gets   │          │
│   │ Show only    │    │ + methods    │    │ parent props │          │
│   │ what's needed│    │ + security   │    │ + own props  │          │
│   │              │    │              │    │              │          │
│   │ e.g., pure   │    │ e.g., private│    │ e.g., Manual │          │
│   │ virtual      │    │ members +    │    │ Car : Car    │          │
│   │ functions    │    │ getters/     │    │              │          │
│   │              │    │ setters      │    │              │          │
│   └──────────────┘    └──────────────┘    └──────────────┘          │
│                                                                      │
│   ┌──────────────────────────────────────────────────────────────┐  │
│   │                       POLYMORPHISM                            │  │
│   │                                                               │  │
│   │   ┌─────────────────────┐    ┌─────────────────────────┐     │  │
│   │   │ Dynamic (Runtime)   │    │ Static (Compile-time)   │     │  │
│   │   │ Method Overriding   │    │ Method Overloading      │     │  │
│   │   │ virtual + override  │    │ same name, diff params  │     │  │
│   │   └─────────────────────┘    └─────────────────────────┘     │  │
│   └──────────────────────────────────────────────────────────────┘  │
│                                                                      │
│   All four together = Complete OOP foundation for LLD               │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 04. What is UML Diagrams | Class & Sequence Diagrams with Real Examples (1:12:09)

This lecture introduces **UML (Unified Modeling Language) Diagrams**, focusing on the two most important types for LLD: **Class Diagrams** (Structural) and **Sequence Diagrams** (Behavioral).

---

## 1. What are UML Diagrams?

**UML Diagrams** are a visual way to express an application's design — what components/objects exist, how they're connected, and how they interact.

- UML stands for Unified Modeling Language.

**Why use diagrams instead of paragraphs?**
- Intuitive and easy to understand
- Clearly shows components, objects, and their interactions
- Standardized notation understood across the industry

---

## 2. Types of UML Diagrams

UML diagrams are split into **two categories**:

```
┌─────────────────────────────────────────────────────────────────────┐
│                         UML DIAGRAMS                                 │
├──────────────────────────────┬──────────────────────────────────────┤
│       STRUCTURAL             │          BEHAVIORAL                  │
│       (Static)               │          (Dynamic)                   │
├──────────────────────────────┼──────────────────────────────────────┤
│ • Show application structure │ • Show how components interact       │
│ • What components exist      │ • How objects send messages          │
│ • How they're connected      │ • Object interactions over time      │
│                              │                                      │
│ ⭐ Class Diagram             │ ⭐ Sequence Diagram                   │
│   (99% of LLD interviews)    │   (Important for specific use cases) │
├──────────────────────────────┼──────────────────────────────────────┤
│ 7 types total                │ 7 types total                        │
│ (Only Class Diagram needed)  │ (Only Sequence Diagram needed)       │
└──────────────────────────────┴──────────────────────────────────────┘
```

**Note:** Of the 14 total UML diagram types, only **2 are needed** for LLD: **Class Diagram** and **Sequence Diagram**. The rest are highly use-case specific.

---

## 3. Class Diagrams

### 3.1 Representing a Class

A class is represented as a **rectangle divided into 3 parts**:

```
┌─────────────────────────────────────┐
│           <<abstract>>              │  ← Optional: abstract marker
│              Car                    │  ← PART 1: Class Name
├─────────────────────────────────────┤
│ - brand: String                     │  ← PART 2: Characteristics
│ - model: String                     │     (Variables/Attributes)
│ - engineCC: int                     │
├─────────────────────────────────────┤
│ + startEngine(): void               │  ← PART 3: Behaviors
│ + stopEngine(): void                │     (Methods/Functions)
│ + accelerate(): void                │
│ + brake(): void                     │
└─────────────────────────────────────┘
```

**Format:** `accessModifier variableName: dataType` and `accessModifier methodName(): returnType`

### 3.2 Access Modifiers in UML

| Access Modifier | UML Symbol | Accessible From |
|-----------------|------------|-----------------|
| `public` | `+` | Everywhere |
| `protected` | `#` | Same class + Child classes |
| `private` | `-` | Same class only |

**Example with mixed modifiers:**
```
┌─────────────────────────────────────┐
│              Car                    │
├─────────────────────────────────────┤
│ - brand: String          (private)  │
│ # model: String          (protected)│
│ + engineCC: int          (public)   │
├─────────────────────────────────────┤
│ + startEngine(): void               │
│ - validateEngine(): boolean         │
│ # internalCheck(): void             │
└─────────────────────────────────────┘
```

### 3.3 Class Diagram Example (C++ Code)

```cpp
class Car {
private:
    string brand;
    string model;
    int engineCC;

public:
    void startEngine() { /* ... */ }
    void stopEngine() { /* ... */ }
    void accelerate() { /* ... */ }
    void brake() { /* ... */ }
};
```

**UML Representation:**
```
┌─────────────────────────────────────┐
│              Car                    │
├─────────────────────────────────────┤
│ - brand: String                     │
│ - model: String                     │
│ - engineCC: int                     │
├─────────────────────────────────────┤
│ + startEngine(): void               │
│ + stopEngine(): void                │
│ + accelerate(): void                │
│ + brake(): void                     │
└─────────────────────────────────────┘
```

---

## 4. Class Associations

### 4.1 Association Hierarchy

```
┌─────────────────────────────────────────────────────────────────────┐
│                         ASSOCIATIONS                                 │
├──────────────────────────────────┬──────────────────────────────────┤
│       CLASS ASSOCIATION          │      OBJECT ASSOCIATION          │
│                                  │                                  │
│   ⭐ Inheritance                 │   ⭐ Simple Association          │
│      (IS-A relationship)         │   ⭐ Aggregation                 │
│                                  │   ⭐ Composition                 │
│                                  │      (all are HAS-A)             │
└──────────────────────────────────┴──────────────────────────────────┘
```

**Key insight:** Simple Association, Aggregation, and Composition are all **combined into one "Composition" concept** in programming — they differ only theoretically/conceptually.

---

### 4.2 Inheritance (IS-A Relationship)

**Definition:** Child class inherits parent class properties and behaviors.

**Real-world examples:**
- Cow IS-A Animal
- Tiger IS-A Animal
- ManualCar IS-A Car
- ElectricCar IS-A Car

**UML Notation:** Solid line with a **closed (hollow) arrowhead** pointing to parent.

```
┌──────────────┐
│    Animal    │
└──────┬───────┘
       △
       │  (closed arrow = inheritance)
       │
  ┌────┴────┬──────────┐
  │         │          │
┌─┴──┐   ┌──┴──┐   ┌───┴───┐
│Cow │   │Tiger│   │Human  │
└────┘   └─────┘   └───────┘
```

**Code:**
```cpp
class Animal {
public:
    void eat() { /* ... */ }
};

class Cow : public Animal {
public:
    void moo() { /* ... */ }
};
```

---

### 4.3 Simple Association (Weakest HAS-A)

**Definition:** Two classes are related through a simple link — one object uses/has another, but no ownership.

**Real-world example:** Arjun lives in a House (Arjun HAS-A House)

**UML Notation:** Solid line with an **open arrow**.

```
┌──────────┐         ┌──────────┐
│  Arjun   │────────▶│  House   │
└──────────┘  open   └──────────┘
              arrow
```

---

### 4.4 Aggregation (Container HAS-A)

**Definition:** A container object holds multiple other objects. The contained objects **can exist independently** of the container.

**Real-world example:** Room HAS-A Sofa, Bed, Chair (but Sofa/Bed/Chair can exist without Room)

**UML Notation:** Solid line with a **hollow (unfilled) diamond** on the container side.

```
┌──────────┐
│   Room   │
└────┬─────┘
     ◇ (hollow diamond)
     │
  ┌──┴────┬─────────┐
  │       │         │
┌─┴──┐ ┌──┴──┐ ┌───┴────┐
│Sofa│ │Bed  │ │Chair   │
└────┘ └─────┘ └────────┘

Note: Diamond points toward the CONTAINER (Room)
     "Sofa IS PART OF Room"
```

**Code:**
```cpp
class Room {
private:
    Sofa* sofa;    // pointer - can exist independently
    Bed* bed;
    Chair* chair;
public:
    Room(Sofa* s, Bed* b, Chair* c) : sofa(s), bed(b), chair(c) {}
};
```

---

### 4.5 Composition (Strongest HAS-A)

**Definition:** A strong ownership where the contained objects **cannot exist independently** of the container. If the container dies, the parts die too.

**Real-world example:** Chair HAS-A Seat, Arms, Wheels (these cannot exist without the chair)

**UML Notation:** Solid line with a **filled (solid) diamond** on the container side.

```
┌──────────┐
│  Chair   │
└────┬─────┘
     ◆ (filled diamond)
     │
  ┌──┴────┬─────────┐
  │       │         │
┌─┴────┐ ┌─┴──┐ ┌──┴─────┐
│Seat  │ │Arms│ │Wheels  │
└──────┘ └────┘ └────────┘

"Seat, Arms, Wheels cannot exist without Chair"
```

**Code:**
```cpp
class Chair {
private:
    Seat seat;      // value - tied to Chair's lifecycle
    Arms arms;
    Wheels wheels;
public:
    Chair() : seat(), arms(), wheels() {}
};
```

---

### 4.6 Composition in Code (General Pattern)

Regardless of whether it's Simple Association, Aggregation, or Composition, the **programmatic representation is the same**:

```cpp
class A {
public:
    void method1() { /* ... */ }
};

class B {
private:
    A* a;  // Reference to A (HAS-A relationship)
public:
    B() {
        a = new A();  // Or passed in constructor
    }
    
    void method2() {
        // Call method1 on A's object
        a->method1();
    }
};

int main() {
    B* b = new B();
    b->method2();  // Internally calls a->method1()
}
```

**Key insight:** Composition is **more important than inheritance** in LLD. Most real-world designs use composition.

---

### 4.7 Association Summary Table

| Relationship | Notation | Meaning | Example |
|-------------|----------|---------|---------|
| **Inheritance** | Closed arrow | IS-A | ManualCar IS-A Car |
| **Simple Association** | Open arrow | Uses / Lives-in | Arjun HAS-A House |
| **Aggregation** | Hollow diamond | Container HAS-A (independent) | Room HAS-A Sofa |
| **Composition** | Filled diamond | Strong HAS-A (dependent) | Chair HAS-A Seat |

**Visual Overview:**
```
INHERITANCE (IS-A):
    Child ──▶ Parent     (closed arrow)

SIMPLE ASSOCIATION (HAS-A):
    A ──▶ B              (open arrow)

AGGREGATION (HAS-A, weak ownership):
    Container ◇── Part   (hollow diamond)

COMPOSITION (HAS-A, strong ownership):
    Container ◆── Part   (filled diamond)
```

---

### 4.8 Exercise Problem

**Task:** Draw a class diagram for:
- A `Car` class with 3-4 characteristics and 3-4 behaviors
- `ManualCar` and `ElectricCar` as children
- `ManualCar` has `shiftGear()` (specific)
- `ElectricCar` has `chargeBattery()` (specific)
- Decide: Inheritance or Composition?

**Solution:**
```
┌─────────────────────────────┐
│            Car              │
├─────────────────────────────┤
│ - brand: String             │
│ - model: String             │
│ - isEngineOn: boolean       │
│ - currentSpeed: int         │
├─────────────────────────────┤
│ + startEngine(): void       │
│ + stopEngine(): void        │
│ + accelerate(): void        │
│ + brake(): void             │
└──────────┬──────────────────┘
           △ (inheritance)
     ┌─────┴─────┐
     │           │
┌────┴─────┐ ┌───┴────────┐
│ManualCar │ │ElectricCar │
├──────────┤ ├────────────┤
│- currentG│ │- batteryPct│
│  ear: int│ │  : int     │
├──────────┤ ├────────────┤
│+ shiftGea│ │+ chargeBat │
│  r(): void│ │  tery():void│
└──────────┘ └────────────┘
```

---

### 4.9 Subjective Nature of Composition vs Aggregation

**Important note from the lecture:** The line between Simple Association, Aggregation, and Composition is **subjective**. It depends on how you design the application.

**Example — Zomato/Swiggy clone:**
- Restaurant HAS-A Menu
- **Question:** Does Menu exist independently of Restaurant?
  - If **yes** → Aggregation (hollow diamond)
  - If **no** → Composition (filled diamond)

There's no single correct answer — it depends on your design choice.

---

## 5. Sequence Diagrams

### 5.1 What is a Sequence Diagram?

**Definition:** A **behavioral (dynamic)** diagram that shows how objects interact with each other over time — specifically, the sequence of messages exchanged between objects.

**Purpose:** Shows the message flow for a **specific use case** (not the entire application).

**Note:** One application can have thousands of sequence diagrams — one per use case/flow.

---

### 5.2 Components of a Sequence Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                    SEQUENCE DIAGRAM COMPONENTS                       │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  1. OBJECTS (rectangles at top)                                     │
│     ┌──────────┐  ┌──────────┐  ┌──────────┐                       │
│     │  Object  │  │  Object  │  │  Object  │                       │
│     │    A     │  │    B     │  │    C     │                       │
│     └────┬─────┘  └────┬─────┘  └────┬─────┘                       │
│          │             │             │                              │
│  2. LIFELINE (dashed vertical line)                                 │
│          │             │             │                              │
│          │             │             │                              │
│  3. ACTIVATION BAR (rectangle on lifeline)                          │
│          ┃             ┃             ┃                              │
│          ┃             ┃             ┃                              │
│          ┃             ┃             ┃                              │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

**Detailed definitions:**

| Component | Description |
|-----------|-------------|
| **Object** | Represented by a rectangle at the top. Simple name (no variables/methods shown). |
| **Lifeline** | Dashed vertical line showing how long the object exists in the application. |
| **Activation Bar** | Solid rectangle on the lifeline showing when the object is **active** (can send/receive messages). |
| **Message** | Arrow between lifelines showing communication. |

---

### 5.3 Message Types

#### Synchronous vs Asynchronous Messages

```
SYNCHRONOUS MESSAGE (waits for response):
┌────────┐                    ┌────────┐
│   A    │                    │   B    │
└───┬────┘                    └───┬────┘
    ┃                             ┃
    ┃──── message() ─────────────▶┃   (solid closed arrow)
    ┃                             ┃
    ┃◀──── response ──────────────┃   (dashed line)
    ┃                             ┃

ASYNCHRONOUS MESSAGE (doesn't wait):
┌────────┐                    ┌────────┐
│   A    │                    │   B    │
└───┬────┘                    └───┬────┘
    ┃                             ┃
    ┃──── message1() ────────────▶┃   (open arrow)
    ┃──── message2() ────────────▶┃
    ┃──── message3() ────────────▶┃
    ┃                             ┃
```

| Message Type | Arrow Style | Waits for Response? |
|-------------|-------------|---------------------|
| **Synchronous** | Solid line, closed arrowhead | Yes — waits |
| **Asynchronous** | Solid line, open arrowhead | No — fires and forgets |
| **Response** | Dashed line | Return value from synchronous |

---

#### Create, Destroy, Lost, and Found Messages

```
CREATE MESSAGE:
┌────────┐                    ┌────────┐
│   A    │                    │   B    │
└───┬────┘                    └───┬────┘
    ┃                             │
    ┃──── <<create>> ────────────▶┃  (new object created)
    ┃                             ┃
    
DESTROY MESSAGE:
┌────────┐                    ┌────────┐
│   A    │                    │   B    │
└───┬────┘                    └───┬────┘
    ┃                             ┃
    ┃──── <<destroy>> ───────────▶┃
    ┃                             ✗  (lifeline ends)

LOST MESSAGE:
┌────────┐                    ┌────────┐
│   A    │                    │   B    │
└───┬────┘                    └───┬────┘
    ┃                             │
    ┃──── message() ────────────▶○  (goes into void)
    ┃                             │
    
FOUND MESSAGE:
┌────────┐                    ┌────────┐
│   A    │                    │   B    │
└───┬────┘                    └───┬────┘
    ┃                             │
    ○◀──── message() ─────────────┃  (from unknown source)
    ┃                             │
```

| Message Type | Meaning |
|-------------|---------|
| **Create** | A new object is created |
| **Destroy** | An existing object is destroyed (lifeline ends) |
| **Lost** | Message sent but never reached the target |
| **Found** | Message received from an unknown source |

---

### 5.4 How to Draw a Sequence Diagram — ATM Example

**Step 1: Identify the Use Case (Flow)**
A user goes to an ATM to withdraw cash:
1. User inserts account number and amount
2. ATM creates a transaction
3. Transaction verifies sufficient funds
4. Cash dispenser dispenses cash
5. User receives cash

**Step 2: Identify Objects Involved**
- `User`
- `ATM`
- `Transaction`
- `Account`
- `CashDispenser`

**Step 3: Draw the Sequence Diagram**

```
┌────────┐    ┌────────┐    ┌────────────┐    ┌─────────┐    ┌──────────────┐
│  User  │    │  ATM   │    │Transaction │    │ Account │    │CashDispenser │
└───┬────┘    └───┬────┘    └─────┬──────┘    └────┬────┘    └──────┬───────┘
    ┃             ┃               │                │                │
    ┃             ┃               │                │                │
    ┃──withdraw(accountNo,amt)───▶┃               │                │
    ┃             ┃               │                │                │
    ┃             ┃──<<create>>──▶┃                │                │
    ┃             ┃               ┃                │                │
    ┃             ┃               ┃──checkAmount(amt)──▶┃           │
    ┃             ┃               ┃                ┃   │            │
    ┃             ┃               ┃◀────true────────┃  │            │
    ┃             ┃               ┃                │                │
    ┃             ┃◀────true──────┃                │                │
    ┃             ┃               ✗                │                │
    ┃             ┃               │                │                │
    ┃             ┃──withdrawCash(amt)─────────────────────────────▶┃
    ┃             ┃               │                │                ┃
    ┃             ┃◀────cash──────────────────────────────────────┃
    ┃             ┃               │                │                ✗
    ┃◀───cash─────┃               │                │                │
    ✗             ✗               │                │                │

Legend:
  ━━▶  = synchronous message (waits for response)
  ◀╌╌  = response (dashed line)
  ✗    = object destroyed / lifeline ends
```

**Step-by-step message flow:**

| Step | From | To | Message | Type |
|------|------|----|---------|------|
| 1 | User | ATM | `withdraw(accountNo, amount)` | Synchronous |
| 2 | ATM | Transaction | `<<create>>` | Create |
| 3 | Transaction | Account | `checkAmount(amount)` | Synchronous |
| 4 | Account | Transaction | `true` (sufficient funds) | Response |
| 5 | Transaction | ATM | `true` | Response |
| 6 | ATM | CashDispenser | `withdrawCash(amount)` | Synchronous |
| 7 | CashDispenser | ATM | `cash` | Response |
| 8 | ATM | User | `cash` | Response |

**Key observations:**
- **User** and **ATM** are active from start to end
- **Transaction** is created mid-flow and destroyed after use
- **Account** and **CashDispenser** are activated only when needed
- The transaction object gets destroyed after returning true (its work is done)

---

### 5.5 Additional Sequence Diagram Terms

| Term | Meaning | Example |
|------|---------|---------|
| **alt** | If-else block (alternate flows) | If balance is sufficient → dispense; else → show error |
| **opt** | Only-if block (no else) | If PIN is correct → proceed |
| **loop** | Iteration (for/while loop) | Retry PIN up to 3 times |

**Visual:**
```
alt (sufficient funds)              opt (PIN correct)              loop (3 times)
┌─────────────────────┐            ┌──────────────────┐           ┌──────────────────┐
│   withdraw cash     │            │   proceed        │           │  enter PIN       │
├─────────────────────┤            └──────────────────┘           └──────────────────┘
│   [else]            │
│   show error        │
└─────────────────────┘
```

**Note:** Happy flow diagrams (no alt/opt/loop) are most common in interviews.

---

## 6. Complete Overview Diagram

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           UML DIAGRAMS                                       │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│  ┌────────────────────────────┐    ┌────────────────────────────┐           │
│  │     STRUCTURAL (Static)    │    │    BEHAVIORAL (Dynamic)    │           │
│  ├────────────────────────────┤    ├────────────────────────────┤           │
│  │                            │    │                            │           │
│  │  ⭐ CLASS DIAGRAM           │    │  ⭐ SEQUENCE DIAGRAM        │           │
│  │                            │    │                            │           │
│  │  • Class Name              │    │  • Objects                 │           │
│  │  • Variables (Attributes)  │    │  • Lifelines               │           │
│  │  • Methods (Behaviors)     │    │  • Activation Bars         │           │
│  │  • Access Modifiers        │    │  • Messages:               │           │
│  │  • Abstract marker         │    │    - Synchronous           │           │
│  │                            │    │    - Asynchronous          │           │
│  │  ASSOCIATIONS:             │    │    - Create / Destroy      │           │
│  │  • Inheritance (IS-A)      │    │    - Lost / Found          │           │
│  │  • Simple Association      │    │  • alt / opt / loop        │           │
│  │  • Aggregation             │    │                            │           │
│  │  • Composition             │    │                            │           │
│  │                            │    │                            │           │
│  └────────────────────────────┘    └────────────────────────────┘           │
│                                                                              │
│  Both are essential for LLD interviews and real-world design                │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 7. Key Takeaways

| Concept | Key Point |
|---------|-----------|
| **UML Diagram** | Visual representation of application design (classes, interactions) |
| **Class Diagram** | Shows classes, their members, and associations (static structure) |
| **Sequence Diagram** | Shows message flow between objects for a specific use case (dynamic behavior) |
| **Inheritance** | IS-A relationship; closed arrow |
| **Simple Association** | HAS-A (weak); open arrow |
| **Aggregation** | HAS-A (container, independent parts); hollow diamond |
| **Composition** | HAS-A (strong, dependent parts); filled diamond |
| **Composition in Code** | Same as any HAS-A: store reference/object of another class |
| **Synchronous Message** | Waits for response; closed arrow + dashed response |
| **Asynchronous Message** | No wait; open arrow |
| **Create/Destroy** | Object lifecycle messages |
| **Lost/Found** | Message delivery failures / unknown sources |

---

## 8. Exercise

**Draw a class diagram for a Car hierarchy:**
- `Car` (parent): brand, model, isEngineOn, currentSpeed + startEngine(), stopEngine(), accelerate(), brake()
- `ManualCar` (child): currentGear + shiftGear()
- `ElectricCar` (child): batteryPercentage + chargeBattery()

**Decide:** Inheritance or Composition relationship?

**Answer:** Inheritance (IS-A) — because ManualCar IS-A Car, ElectricCar IS-A Car.

**Practice drawing both Class Diagram and Sequence Diagram for a use case of your choice.**

---

## 05. SOLID Design Principles | Complete Guide with Code Examples (1:07:31)

This lecture introduces **SOLID Design Principles** — five rules created by **Robert C. Martin (Uncle Bob)** in 2000 to solve common problems in real-world projects. Part 1 covers the first three: **SRP, OCP, and LSP**.

---

## 0. Why SOLID? — Problems Without Design Principles

### Real-World Analogy: A Messy House Wiring

```
┌─────────────────────────────────────────────────────────────────────┐
│                    A HOUSE WITH MESSY WIRING                        │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   Electrical wires ──┐                                               │
│   Internet wires   ──┼──▶ ALL from ONE point ──▶ TOTAL MESS         │
│   Water pipes      ──┘                                               │
│                                                                      │
│   If ONE wire fails:                                                │
│   • Hard to find which wire is faulty                               │
│   • Hard to replace the correct wire                                │
│   • Everything is TIGHTLY COUPLED                                   │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

Same problem in code when classes are **tightly coupled** and **messy**.

### Common Problems Without Design Principles

| Problem | Description |
|---------|-------------|
| **Maintainability** | New features can't be easily integrated; old features break |
| **Readability** | New engineers can't understand the code quickly |
| **Bugs & Debugging** | Many bugs introduced, hard to debug and resolve |
| **Monetary Impact** | Bugs in production cost real money |

---

## 1. SOLID Acronym Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                    SOLID DESIGN PRINCIPLES                          │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   S ── Single Responsibility Principle (SRP)                        │
│        "A class should have only one reason to change"              │
│                                                                      │
│   O ── Open/Closed Principle (OCP)                                  │
│        "Open for extension, closed for modification"                │
│                                                                      │
│   L ── Liskov Substitution Principle (LSP)                          │
│        "Subclasses should be substitutable for base classes"        │
│                                                                      │
│   I ── Interface Segregation Principle (ISP)                        │
│        (Covered in Part 2)                                          │
│                                                                      │
│   D ── Dependency Inversion Principle (DIP)                         │
│        (Covered in Part 2)                                          │
│                                                                      │
│   Created by: Robert C. Martin (Uncle Bob), 2000                    │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 2. Single Responsibility Principle (SRP)

### Definition

> **"A class should have only one reason to change."**
> **"A class should do only one thing."**

**Important clarification:** SRP does NOT mean a class should have only one method. It means all methods of a class should serve **one responsibility**.

### Real-World Analogy: TV Remote

```
┌─────────────────────────────────────────────────────────────────────┐
│                    TV REMOTE ANALOGY                                │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   GOOD:                             BAD:                            │
│   ┌─────────────────┐              ┌─────────────────────────┐     │
│   │  TV Remote      │              │  Universal Remote       │     │
│   │  • Power        │              │  • Power TV             │     │
│   │  • Volume       │              │  • Volume TV            │     │
│   │  • Channel      │              │  • Channel TV           │     │
│   └─────────────────┘              │  • Control Fridge       │     │
│                                    │  • Control AC           │     │
│   One responsibility:              │  • Control Washing M/C  │     │
│   "Control the TV"                 └─────────────────────────┘     │
│                                                                      │
│                                    Too many responsibilities!       │
│                                    Hard to maintain                 │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Example: Shopping Cart

**❌ BAD — Violates SRP:**

```
┌─────────────────────────────────────────────────────────────────────┐
│                    ShoppingCart (SRP VIOLATION)                     │
├─────────────────────────────────────────────────────────────────────┤
│  - products: List<Product>                                          │
├─────────────────────────────────────────────────────────────────────┤
│  + addProduct(Product): void                                        │
│  + getProducts(): List<Product>                                     │
│  + calculateTotalPrice(): double      ← Responsibility 1            │
│  + printInvoice(): void               ← Responsibility 2            │
│  + saveToDatabase(): void             ← Responsibility 3            │
└─────────────────────────────────────────────────────────────────────┘
```

**Problems:**
- 3 reasons to change: pricing logic, invoice format, DB persistence
- Change in DB → must modify `ShoppingCart`
- Change in invoice format → must modify `ShoppingCart`

**❌ Bad Code:**

```cpp
class Product {
    string name;
    double price;
public:
    Product(string n, double p) : name(n), price(p) {}
    string getName() { return name; }
    double getPrice() { return price; }
};

class ShoppingCart {
    vector<Product> products;
public:
    void addProduct(Product p) { products.push_back(p); }
    vector<Product> getProducts() { return products; }
    
    double calculateTotalPrice() {
        double total = 0;
        for (auto& p : products) total += p.getPrice();
        return total;
    }
    
    void printInvoice() {
        cout << "Invoice:\n";
        for (auto& p : products)
            cout << p.getName() << " - $" << p.getPrice() << "\n";
        cout << "Total: $" << calculateTotalPrice() << "\n";
    }
    
    void saveToDatabase() {
        cout << "Saving shopping cart to database...\n";
    }
};

int main() {
    ShoppingCart cart;
    cart.addProduct(Product("Laptop", 1500));
    cart.addProduct(Product("Mouse", 50));
    cart.printInvoice();
    cart.saveToDatabase();
}
```

**✅ GOOD — Follows SRP:**

```
┌────────────────────────┐
│       Product          │
├────────────────────────┤
│ - name: String         │
│ - price: double        │
├────────────────────────┤
│ + getName(): String    │
│ + getPrice(): double   │
└────────────────────────┘
           △ (has-a)
           │ 1..*
┌──────────┴─────────────┐
│     ShoppingCart       │
├────────────────────────┤
│ - products: List<...>  │
├────────────────────────┤
│ + addProduct()         │
│ + getProducts()        │
│ + calculateTotalPrice()│ ← Only ONE responsibility
└────────────────────────┘

┌────────────────────────┐    ┌────────────────────────┐
│ ShoppingCartInvoice    │    │ ShoppingCartStorage    │
│       Printer          │    │                        │
├────────────────────────┤    ├────────────────────────┤
│ - cart: ShoppingCart*  │    │ - cart: ShoppingCart*  │
├────────────────────────┤    ├────────────────────────┤
│ + printInvoice()       │    │ + saveToDB()           │
└────────────────────────┘    └────────────────────────┘
        │                              │
        └──────────────┬───────────────┘
                       │ (has-a)
                       ▼
                ShoppingCart
```

**✅ Good Code:**

```cpp
class Product {
    string name;
    double price;
public:
    Product(string n, double p) : name(n), price(p) {}
    string getName() { return name; }
    double getPrice() { return price; }
};

// Responsibility 1: Cart management + Price calculation
class ShoppingCart {
    vector<Product> products;
public:
    void addProduct(Product p) { products.push_back(p); }
    vector<Product> getProducts() { return products; }
    
    double calculateTotalPrice() {
        double total = 0;
        for (auto& p : products) total += p.getPrice();
        return total;
    }
};

// Responsibility 2: Invoice printing
class ShoppingCartInvoicePrinter {
    ShoppingCart* cart;
public:
    ShoppingCartInvoicePrinter(ShoppingCart* c) : cart(c) {}
    
    void printInvoice() {
        cout << "Invoice:\n";
        for (auto& p : cart->getProducts())
            cout << p.getName() << " - $" << p.getPrice() << "\n";
        cout << "Total: $" << cart->calculateTotalPrice() << "\n";
    }
};

// Responsibility 3: Database persistence
class ShoppingCartStorage {
    ShoppingCart* cart;
public:
    ShoppingCartStorage(ShoppingCart* c) : cart(c) {}
    
    void saveToDatabase() {
        cout << "Saving shopping cart to database...\n";
    }
};

int main() {
    ShoppingCart cart;
    cart.addProduct(Product("Laptop", 1500));
    cart.addProduct(Product("Mouse", 50));
    
    ShoppingCartInvoicePrinter printer(&cart);
    printer.printInvoice();
    
    ShoppingCartStorage storage(&cart);
    storage.saveToDatabase();
}
```

**Benefits:**
- Change DB logic → only modify `ShoppingCartStorage`
- Change invoice format → only modify `ShoppingCartInvoicePrinter`
- Change pricing logic → only modify `ShoppingCart`

---

## 3. Open/Closed Principle (OCP)

### Definition

> **"A class should be open for extension but closed for modification."**

- **Extension** = Adding new features
- **Modification** = Changing existing code
- **Rule:** Add new features **without touching existing classes** (use abstraction + inheritance + polymorphism)

### Problem Setup

**Scenario:** Initially, `ShoppingCartStorage` only saves to SQL. Now you want to add MongoDB and File storage.

**❌ BAD — Violates OCP (Adding methods to existing class):**

```cpp
class ShoppingCartStorage {
    ShoppingCart* cart;
public:
    ShoppingCartStorage(ShoppingCart* c) : cart(c) {}
    
    void saveToSqlDB() { /* SQL logic */ }
    void saveToMongoDB() { /* Mongo logic */ }  // ❌ Modified class
    void saveToFile() { /* File logic */ }       // ❌ Modified class
};
```

**Why it's bad:** Every new storage type requires modifying the existing `ShoppingCartStorage` class.

### ✅ Good — Using Abstraction + Inheritance + Polymorphism

```
                  ┌────────────────────────────┐
                  │    <<abstract>>            │
                  │   DBPersistence            │
                  ├────────────────────────────┤
                  │ - cart: ShoppingCart*      │
                  ├────────────────────────────┤
                  │ + save(): void = 0         │
                  └─────────────┬──────────────┘
                                △ (inheritance)
              ┌─────────────────┼─────────────────┐
              │                 │                 │
     ┌────────┴────────┐ ┌──────┴────────┐ ┌──────┴────────┐
     │ SqlPersistence  │ │ MongoPersist. │ │ FilePersist.  │
     ├─────────────────┤ ├───────────────┤ ├───────────────┤
     │ + save()        │ │ + save()      │ │ + save()      │
     └─────────────────┘ └───────────────┘ └───────────────┘
        (override)        (override)         (override)
```

**✅ Good Code:**

```cpp
class Product { /* same as before */ };

class ShoppingCart {
    vector<Product> products;
public:
    void addProduct(Product p) { products.push_back(p); }
    vector<Product> getProducts() { return products; }
    double calculateTotalPrice() { /* ... */ }
};

class ShoppingCartInvoicePrinter { /* same as before */ };

// ABSTRACT CLASS - The extension point
class DBPersistence {
protected:
    ShoppingCart* cart;
public:
    DBPersistence(ShoppingCart* c) : cart(c) {}
    virtual void save() = 0;  // Pure virtual
    virtual ~DBPersistence() {}
};

// Concrete implementations - each new one is a NEW class (not modification)
class SqlPersistence : public DBPersistence {
public:
    SqlPersistence(ShoppingCart* c) : DBPersistence(c) {}
    
    void save() override {
        cout << "Saving shopping cart to SQL database...\n";
    }
};

class MongoPersistence : public DBPersistence {
public:
    MongoPersistence(ShoppingCart* c) : DBPersistence(c) {}
    
    void save() override {
        cout << "Saving shopping cart to MongoDB...\n";
    }
};

class FilePersistence : public DBPersistence {
public:
    FilePersistence(ShoppingCart* c) : DBPersistence(c) {}
    
    void save() override {
        cout << "Saving shopping cart to File...\n";
    }
};

int main() {
    ShoppingCart cart;
    cart.addProduct(Product("Laptop", 1500));
    cart.addProduct(Product("Mouse", 50));
    
    ShoppingCartInvoicePrinter printer(&cart);
    printer.printInvoice();
    
    // Polymorphism in action — same save() call, different behavior
    DBPersistence* sql = new SqlPersistence(&cart);
    DBPersistence* mongo = new MongoPersistence(&cart);
    DBPersistence* file = new FilePersistence(&cart);
    
    sql->save();    // SQL behavior
    mongo->save();  // MongoDB behavior
    file->save();   // File behavior
}
```

**Output:**
```
Invoice:
Laptop - $1500
Mouse - $50
Total: $1550
Saving shopping cart to SQL database...
Saving shopping cart to MongoDB...
Saving shopping cart to File...
```

**Key Insight:** Adding a new persistence type (e.g., Cassandra) requires only a new class — **no existing class is modified**.

---

## 4. Liskov Substitution Principle (LSP)

### Definition

> **"Subclasses should be substitutable for their base classes."**

**In practice:** Wherever a base class object can be used, a subclass object should work without breaking the code.

### Why It's Important (and Often Violated)

Despite being a "basic" inheritance property, LSP is one of the most commonly violated principles in real-world projects — often unintentionally.

### The Rule Illustrated

```
┌─────────────────────────────────────────────────────────────────────┐
│                    LSP IN A NUTSHELL                                │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   Base Class A                       Subclass B                     │
│   ┌─────────────────┐               ┌─────────────────┐             │
│   │ + m1()          │               │ + m1()          │ (inherited) │
│   │ + m2()          │  ◀── inherits │ + m2()          │ (inherited) │
│   │ + m3()          │               │ + m3()          │ (inherited) │
│   └─────────────────┘               │ + m4()          │ (own)       │
│                                     │ + m5()          │ (own)       │
│   Client expects A                  └─────────────────┘             │
│   ────▶ Pass B instead                                              │
│   ────▶ Should STILL work — B extends A, doesn't restrict it        │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Client Code Pattern

```cpp
class A {
public:
    void m1() { /* ... */ }
    void m2() { /* ... */ }
    void m3() { /* ... */ }
};

class B : public A {
public:
    void m4() { /* ... */ }  // Specialized
    void m5() { /* ... */ }
};

// Client expects A
void randomMethod(A* a) {
    a->m1();
    a->m2();
    a->m3();
}

int main() {
    A* a = new A();
    randomMethod(a);           // ✅ Works
    
    A* b = new B();            // ✅ Substitution
    randomMethod(b);           // ✅ Should work — B has m1, m2, m3
}
```

The client only knows about `m1()`, `m2()`, `m3()` — it doesn't know (or care) about `m4()`, `m5()`.

### ❌ Violation Example: Bank Accounts

**Scenario:** Bank account hierarchy with Fixed Deposit accounts.

```
┌────────────────────────────────────┐
│       <<abstract>>                 │
│        Account                     │
├────────────────────────────────────┤
│ + deposit(): void = 0              │
│ + withdraw(): void = 0             │
└─────────────────┬──────────────────┘
                  △
      ┌───────────┼────────────┐
      │           │            │
┌─────┴─────┐ ┌───┴──────┐ ┌───┴───────────┐
│ Savings   │ │ Current  │ │ FixedDeposit  │
├───────────┤ ├──────────┤ ├───────────────┤
│ + deposit │ │ + deposit│ │ + deposit     │
│ + withdraw│ │ + withdraw│ │ + withdraw ❌ │
└───────────┘ └──────────┘ │  (throws!)    │
                           └───────────────┘
```

**Why this violates LSP:** The client expects to call `withdraw()` on any `Account`, but `FixedDepositAccount::withdraw()` throws an exception.

**❌ Bad Code:**

```cpp
class Account {
public:
    virtual void deposit(double amount) = 0;
    virtual void withdraw(double amount) = 0;
    virtual ~Account() {}
};

class SavingsAccount : public Account {
    double balance = 0;
public:
    void deposit(double amount) override {
        balance += amount;
        cout << "Deposited: " << amount << "\n";
    }
    void withdraw(double amount) override {
        if (amount <= balance) {
            balance -= amount;
            cout << "Withdrawn: " << amount << "\n";
        } else {
            cout << "Insufficient funds\n";
        }
    }
};

class CurrentAccount : public Account {
    // Similar to SavingsAccount
};

class FixedDepositAccount : public Account {
public:
    void deposit(double amount) override {
        cout << "Deposited: " << amount << "\n";
    }
    void withdraw(double amount) override {
        throw logic_error("Withdrawal not allowed in Fixed Deposit Account");
    }
};

class BankClient {
    vector<Account*> accounts;
public:
    BankClient(vector<Account*> accs) : accounts(accs) {}
    
    void processTransactions() {
        for (auto* acc : accounts) {
            acc->deposit(1000);
            acc->withdraw(500);  // ❌ Throws for FixedDeposit
        }
    }
};

int main() {
    vector<Account*> accounts = {
        new SavingsAccount(),
        new CurrentAccount(),
        new FixedDepositAccount()
    };
    BankClient client(accounts);
    client.processTransactions();  // Exception thrown!
}
```

### ❌ Wrong "Fix": Modify the Client

```cpp
void processTransactions() {
    for (auto* acc : accounts) {
        acc->deposit(1000);
        
        // ❌ BAD: Type checking — client is now tightly coupled
        if (typeid(*acc) != typeid(FixedDepositAccount)) {
            acc->withdraw(500);
        }
    }
}
```

**Why it's bad:**
- Client becomes **tightly coupled** to account types
- **Breaks OCP** — every new account type requires client modification
- Client shouldn't know about implementation details

### ✅ Correct Fix: Restructure Hierarchy

```
       ┌────────────────────────────────┐
       │     <<abstract>>               │
       │  DepositOnlyAccount            │
       ├────────────────────────────────┤
       │ + deposit(): void = 0          │
       └───────────────┬────────────────┘
                       △
                       │ (extends)
       ┌───────────────┴────────────────┐
       │     <<abstract>>               │
       │  WithdrawableAccount           │
       ├────────────────────────────────┤
       │ + withdraw(): void = 0         │
       └───────────────┬────────────────┘
                       △
              ┌────────┼─────────┐
              │                  │
       ┌──────┴──────┐    ┌──────┴──────┐
       │  Savings    │    │  Current    │
       └─────────────┘    └─────────────┘

       ┌─────────────────────────────┐
       │  FixedDepositAccount        │
       │  (extends DepositOnly)      │
       └─────────────────────────────┘
```

**✅ Good Code:**

```cpp
// Interface 1: Deposit only
class DepositOnlyAccount {
public:
    virtual void deposit(double amount) = 0;
    virtual ~DepositOnlyAccount() {}
};

// Interface 2: Deposit + Withdraw
class WithdrawableAccount : public DepositOnlyAccount {
public:
    virtual void withdraw(double amount) = 0;
    virtual ~WithdrawableAccount() {}
};

class SavingsAccount : public WithdrawableAccount {
    double balance = 0;
public:
    void deposit(double amount) override {
        balance += amount;
        cout << "Savings: Deposited " << amount << "\n";
    }
    void withdraw(double amount) override {
        if (amount <= balance) {
            balance -= amount;
            cout << "Savings: Withdrawn " << amount << "\n";
        } else {
            cout << "Savings: Insufficient funds\n";
        }
    }
};

class CurrentAccount : public WithdrawableAccount {
    // Same as Savings
};

class FixedDepositAccount : public DepositOnlyAccount {
public:
    void deposit(double amount) override {
        cout << "FixedDeposit: Deposited " << amount << "\n";
    }
    // No withdraw() method — it doesn't exist in parent
};

// Client with two separate lists — no type checking!
class BankClient {
    vector<WithdrawableAccount*> withdrawableAccounts;
    vector<DepositOnlyAccount*> depositOnlyAccounts;
public:
    BankClient(vector<WithdrawableAccount*> w, vector<DepositOnlyAccount*> d)
        : withdrawableAccounts(w), depositOnlyAccounts(d) {}
    
    void processTransactions() {
        // Withdrawable accounts: deposit + withdraw
        for (auto* acc : withdrawableAccounts) {
            acc->deposit(1000);
            acc->withdraw(500);
        }
        
        // Deposit-only accounts: deposit only
        for (auto* acc : depositOnlyAccounts) {
            acc->deposit(1000);
        }
    }
};

int main() {
    vector<WithdrawableAccount*> w = {
        new SavingsAccount(),
        new CurrentAccount()
    };
    vector<DepositOnlyAccount*> d = {
        new FixedDepositAccount()
    };
    
    BankClient client(w, d);
    client.processTransactions();
}
```

**Output:**
```
Savings: Deposited 1000
Savings: Withdrawn 500
Current: Deposited 1000
Current: Withdrawn 500
FixedDeposit: Deposited 1000
```

**Key Insight:** By splitting the interface into two levels, each subclass now correctly supports all methods of its parent. Client doesn't need any type checks.

---

## 5. Complete SOLID Overview Diagram

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                       SOLID DESIGN PRINCIPLES                                │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│   ┌────────────────────────────────────────────────────────────────────┐    │
│   │  S — SINGLE RESPONSIBILITY PRINCIPLE                               │    │
│   │  ────────────────────────────────────                              │    │
│   │  • One class = One responsibility                                  │    │
│   │  • One reason to change                                            │    │
│   │  • Split ShoppingCart → Cart + Printer + Storage                   │    │
│   └────────────────────────────────────────────────────────────────────┘    │
│                                                                              │
│   ┌────────────────────────────────────────────────────────────────────┐    │
│   │  O — OPEN/CLOSED PRINCIPLE                                         │    │
│   │  ───────────────────────────                                       │    │
│   │  • Open for extension                                              │    │
│   │  • Closed for modification                                         │    │
│   │  • Use abstraction + inheritance + polymorphism                    │    │
│   │  • New persistence types → new classes, no modifications           │    │
│   └────────────────────────────────────────────────────────────────────┘    │
│                                                                              │
│   ┌────────────────────────────────────────────────────────────────────┐    │
│   │  L — LISKOV SUBSTITUTION PRINCIPLE                                 │    │
│   │  ────────────────────────────────                                  │    │
│   │  • Subclass must be substitutable for base class                   │    │
│   │  • Extend, never restrict parent's contract                        │    │
│   │  • FixedDepositAccount → DepositOnlyAccount (not Account)          │    │
│   └────────────────────────────────────────────────────────────────────┘    │
│                                                                              │
│   ┌────────────────────────────────────────────────────────────────────┐    │
│   │  I — INTERFACE SEGREGATION PRINCIPLE                               │    │
│   │  D — DEPENDENCY INVERSION PRINCIPLE                                │    │
│   │  (Covered in Part 2)                                               │    │
│   └────────────────────────────────────────────────────────────────────┘    │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 6. Key Takeaways

| Principle | Core Idea | Key Technique | Warning Sign |
|-----------|-----------|---------------|--------------|
| **SRP** | One class = one responsibility | Split into multiple classes; use composition | A class has "and" in its responsibility description |
| **OCP** | Extend without modifying | Abstraction + inheritance + polymorphism | Adding a feature requires editing existing classes |
| **LSP** | Subclass substitutes base | Restructure hierarchy; don't narrow parent's contract | Child throws exceptions/returns null for parent's methods |

### Golden Rules
1. **SRP:** "A class should have only one reason to change."
2. **OCP:** "A class should be open for extension, closed for modification."
3. **LSP:** "A subclass must be substitutable for its base class."

### Common Pitfall: Solving One Principle by Breaking Another
In the LSP bank example, the "wrong fix" (adding type checks in client) violated **OCP** — this is a key lesson: always check that a fix doesn't break other principles.

---

## 06. SOLID Design Principles | part 2 (1:17:10)

This lecture completes the SOLID principles. It deep-dives into **LSP guidelines** (signature, property, and method rules), then covers **Interface Segregation Principle (ISP)** and **Dependency Inversion Principle (DIP)**.

---

## 1. LSP Guidelines (Deep Dive)

### Recap: LSP Definition

> **"Subclasses should be substitutable for their base classes."**

A child class must **behave like** the parent class — not just inherit its methods. The client should not notice any difference between parent and child.

### Why Guidelines Are Needed

```
┌─────────────────────────────────────────────────────────────────────┐
│                    LSP IS EASY TO BREAK                             │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   Inheritance is NOT enough.                                        │
│   Just overriding parent methods ≠ LSP compliance.                  │
│                                                                      │
│   Example: FixedDepositAccount overrides withdraw() but throws     │
│   exception → breaks LSP even though it "inherits" from Account    │
│                                                                      │
│   Key Rule: Child class must BEHAVE like parent class.             │
│   Client should NOT know parent vs child difference.                │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Three Categories of LSP Guidelines

```
┌─────────────────────────────────────────────────────────────────────┐
│                    LSP GUIDELINES                                   │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   1. SIGNATURE RULE                                                │
│      ├── Method Argument Rule                                       │
│      ├── Return Type Rule (Covariance)                              │
│      └── Exception Rule                                             │
│                                                                      │
│   2. PROPERTY RULE                                                 │
│      ├── Class Invariant                                            │
│      └── History Constraint                                         │
│                                                                      │
│   3. METHOD RULE                                                   │
│      ├── Pre-condition                                              │
│      └── Post-condition                                             │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Terminology: Broad vs Narrow

| Term | Meaning | Example |
|------|---------|---------|
| **Broad** | Parent class or any ancestor | Animal is broader than Dog |
| **Narrow** | Child class or any descendant | Dog is narrower than Animal |

---

## 2. Signature Rule

### 2.1 Method Argument Rule

**Rule:** The method argument in the child class must be **the same** or **broader** than the parent class's argument.

```
┌─────────────────────────────────────────────────────────────────────┐
│                 METHOD ARGUMENT RULE                                │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   Parent:  void solve(String s)                                     │
│                                                                      │
│   Child:   void solve(String s)     ✅ Same (allowed)               │
│            void solve(Object o)     ✅ Broader (allowed)            │
│            void solve(Integer i)    ❌ Narrower (NOT allowed)       │
│                                                                      │
│   Why? Client knows parent's contract: expects String.              │
│   If child expects Integer, client passing String will break.       │
│                                                                      │
│   Note: C++ enforces this automatically. Java also enforces this.   │
│         You cannot break this rule even if you wanted to.           │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

**Code Example:**

```cpp
class Parent {
public:
    virtual void print(string msg) {
        cout << "Parent: " << msg << "\n";
    }
};

class Child : public Parent {
public:
    // ✅ Correct: same argument type
    void print(string msg) override {
        cout << "Child: " << msg << "\n";
    }
    
    // ❌ Compile error: different argument type
    // void print(int msg) override { ... }
    // Error: does not override base class member
};

// Client
void processMessage(Parent* p) {
    p->print("Hello");  // Works for both Parent and Child
}

int main() {
    Parent p;
    Child c;
    processMessage(&p);  // ✅ Parent
    processMessage(&c);  // ✅ Child (substituted)
}
```

---

### 2.2 Return Type Rule (Covariance)

**Rule:** The return type in the child class must be **the same** or **narrower** than the parent class's return type. It must **never** be broader.

```
┌─────────────────────────────────────────────────────────────────────┐
│                    RETURN TYPE RULE                                 │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   Hierarchy:    Animal (broader) ◀── Dog (narrower)                 │
│                                                                      │
│   Parent:  Animal getAnimal()                                       │
│                                                                      │
│   Child:   Animal getAnimal()  ✅ Same (allowed)                    │
│            Dog getAnimal()     ✅ Narrower (allowed — Covariance)   │
│            Organism getAnimal() ❌ Broader (NOT allowed)            │
│                                                                      │
│   Why? Client expects Animal reference.                             │
│   If child returns Organism, client can't hold it in Animal ref.    │
│                                                                      │
│   This is called COVARIANCE.                                        │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

**Code Example:**

```cpp
class Animal {
public:
    virtual void speak() { cout << "Animal speaks\n"; }
};

class Dog : public Animal {
public:
    void speak() override { cout << "Dog barks\n"; }
};

class Parent {
public:
    virtual Animal* getAnimal() {
        cout << "Parent returning Animal instance\n";
        return new Animal();
    }
};

class Child : public Parent {
public:
    // ✅ Allowed: narrower return type (Dog is narrower than Animal)
    Dog* getAnimal() override {
        cout << "Child returning Dog instance\n";
        return new Dog();
    }
    
    // ❌ NOT allowed: broader return type
    // Organism* getAnimal() override { ... }
};

// Client
class Client {
    Parent* p;
public:
    Client(Parent* parent) : p(parent) {}
    
    Animal* takeAnimal() {
        return p->getAnimal();  // Works for Parent or Child
    }
};

int main() {
    Parent p;
    Child c;
    
    Client c1(&p);
    c1.takeAnimal();  // Output: Parent returning Animal instance
    
    Client c2(&c);
    c2.takeAnimal();  // Output: Child returning Dog instance
}
```

---

### 2.3 Exception Rule

**Rule:** If the parent method throws an exception of type `E`, the child method must throw **the same exception** or a **narrower (subclass)** exception. Never a broader one.

```
┌─────────────────────────────────────────────────────────────────────┐
│                    EXCEPTION HIERARCHY                              │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│                        Exception                                    │
│                       /         \                                   │
│               LogicError       RuntimeError                         │
│              /    |    \       /    |    \                          │
│         OutOfRange  ...  ...  ...  ...  ...                         │
│                                                                      │
│   Child can throw: OutOfRangeError (narrower) ✅                    │
│   Child can throw: LogicError (same) ✅                             │
│   Child cannot throw: RuntimeError (different/broader) ❌            │
│   Child cannot throw: Exception (broader) ❌                        │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

**Why?** The client has a `try-catch` block that only handles the parent's exception type. If the child throws something different, the client's catch block won't catch it.

**Code Example:**

```cpp
#include <iostream>
#include <stdexcept>
using namespace std;

class Parent {
public:
    // Contract: throws LogicError (or subclass)
    virtual int getValue() {
        throw LogicError("Parent logical error");
    }
};

class Child : public Parent {
public:
    // ✅ Allowed: narrower exception (OutOfRange is subclass of LogicError)
    int getValue() override {
        throw OutOfRange("Child out of range error");
    }
    
    // ❌ NOT allowed: different/broader exception
    // int getValue() override {
    //     throw RuntimeError("Child runtime error");  // Different class!
    // }
};

// Client
class Client {
    Parent* p;
public:
    Client(Parent* parent) : p(parent) {}
    
    void takeValue() {
        try {
            p->getValue();
        } catch (LogicError& e) {  // Client expects LogicError
            cout << "Logic error handled: " << e.what() << "\n";
        }
    }
};

int main() {
    Child c;
    Client client(&c);
    client.takeValue();  // ✅ OutOfRange caught by LogicError handler
}
```

---

## 3. Property Rule

### 3.1 Class Invariant

**Definition:** An **invariant** is a rule/fact that must **always be true** for a class throughout its lifetime.

**Rule:** The child class must **follow** the parent's invariant — either keep it as-is or **strengthen** it, but **never weaken** it.

```
┌─────────────────────────────────────────────────────────────────────┐
│                    CLASS INVARIANT RULE                             │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   Parent Class: Account                                             │
│   Invariant:  "Balance can never be negative"                      │
│                                                                      │
│   ✅ Child follows:                                                 │
│      • SavingsAccount — maintains non-negative balance             │
│      • CurrentAccount — maintains non-negative balance             │
│                                                                      │
│   ❌ Child violates:                                                │
│      • CheatAccount — allows negative balance                      │
│        → Breaks invariant → breaks LSP                              │
│                                                                      │
│   Note: Invariants are written as comments — C++ doesn't           │
│         enforce them. It's YOUR responsibility to follow.          │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

**Code Example:**

```cpp
class BankAccount {
protected:
    double balance;
    
    // INVARIANT: balance must never be negative
    // (This is a documented rule — not enforced by compiler)
    
public:
    BankAccount(double b) {
        if (b < 0) {
            throw invalid_argument("Balance can't be negative");
        }
        balance = b;
    }
    
    virtual void withdraw(double amount) {
        if (balance - amount < 0) {
            throw runtime_error("Insufficient funds");
        }
        balance -= amount;
        cout << "Withdrawn: " << amount << "\n";
    }
};

// ✅ Follows invariant
class SavingsAccount : public BankAccount {
public:
    SavingsAccount(double b) : BankAccount(b) {}
    // withdraw() inherited — maintains non-negative balance
};

// ❌ Violates invariant
class CheatAccount : public BankAccount {
public:
    CheatAccount(double b) : BankAccount(b) {}
    
    void withdraw(double amount) override {
        // BUG: No check for negative balance!
        balance -= amount;  // Can go negative → breaks invariant
        cout << "Withdrawn: " << amount << "\n";
    }
};
```

---

### 3.2 History Constraint

**Definition:** A **history constraint** is a rule about the object's behavior over time — that certain operations should **always be allowed** (or never allowed).

**Rule:** The child class must **not change the history** set by the parent. If parent says "withdrawal is always allowed," child cannot disable it.

```
┌─────────────────────────────────────────────────────────────────────┐
│                    HISTORY CONSTRAINT RULE                          │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   Parent Class: Account                                             │
│   History Constraint: "Withdrawal should always be allowed"         │
│                                                                      │
│   ✅ Child follows:                                                 │
│      • SavingsAccount — withdraw() works                            │
│      • CurrentAccount — withdraw() works                            │
│                                                                      │
│   ❌ Child violates:                                                │
│      • FixedDepositAccount — withdraw() throws exception            │
│        → Breaks history constraint → breaks LSP                     │
│                                                                      │
│   Also: Immutable classes/methods must stay immutable in child.    │
│   If parent marks something final/immutable, child must not        │
│   make it mutable.                                                  │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

**Code Example:**

```cpp
class BankAccount {
protected:
    double balance;
    
    // HISTORY CONSTRAINT: "Withdrawal should be allowed"
    // This means any BankAccount subclass MUST support withdraw()
    
public:
    BankAccount(double b) : balance(b) {}
    
    virtual void withdraw(double amount) {
        if (amount <= balance) {
            balance -= amount;
            cout << "Withdrawn: " << amount << "\n";
        } else {
            cout << "Insufficient funds\n";
        }
    }
};

// ✅ Follows history constraint
class SavingsAccount : public BankAccount {
public:
    SavingsAccount(double b) : BankAccount(b) {}
    // withdraw() works — as expected
};

// ❌ Violates history constraint
class FixedDepositAccount : public BankAccount {
public:
    FixedDepositAccount(double b) : BankAccount(b) {}
    
    void withdraw(double amount) override {
        // BREAKS history constraint — withdrawal is no longer allowed
        throw runtime_error("Withdrawal not allowed in Fixed Deposit");
    }
};

int main() {
    BankAccount* acc = new FixedDepositAccount(1000);
    // Client expects withdrawal to work (based on parent contract)
    acc->withdraw(500);  // ❌ Throws exception — client breaks!
}
```

**Bonus: Immutable Classes and Methods**

```cpp
// Immutable class — cannot be inherited
class FinalClass final {
    // ...
};

class Parent {
public:
    // Immutable method — cannot be overridden
    virtual void doSomething() final {
        cout << "This cannot be overridden\n";
    }
};

class Child : public Parent {
    // ❌ Compile error: cannot override final method
    // void doSomething() override { ... }
};
```

---

## 4. Method Rule

### 4.1 Pre-condition

**Definition:** A **pre-condition** is a condition that must be **true before** a method runs.

**Rule:** The child class may **weaken** (relax) the pre-condition or keep it **as-is**, but must **never strengthen** (tighten) it.

```
┌─────────────────────────────────────────────────────────────────────┐
│                    PRE-CONDITION RULE                               │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   Parent: createPassword()                                          │
│   Pre-condition: password.length() >= 8                             │
│                                                                      │
│   Child:                                                            │
│   ✅ Weakens:  password.length() >= 6  (more permissive)            │
│   ✅ Same:     password.length() >= 8                               │
│   ❌ Strengthens: password.length() >= 10 (more restrictive)        │
│                                                                      │
│   Why? Client expects to pass any value satisfying parent's        │
│   pre-condition. If child tightens it, valid inputs for parent     │
│   become invalid for child → breaks LSP.                            │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

**Code Example:**

```cpp
class User {
public:
    // PRE-CONDITION: password must be at least 8 characters
    virtual void setPassword(string password) {
        if (password.length() < 8) {
            throw invalid_argument("Password must be at least 8 characters");
        }
        cout << "Password set: " << password << "\n";
    }
};

// ✅ Child weakens pre-condition (allows shorter passwords)
class AdminUser : public User {
public:
    // PRE-CONDITION (weakened): password must be at least 6 characters
    void setPassword(string password) override {
        if (password.length() < 6) {
            throw invalid_argument("Password must be at least 6 characters");
        }
        cout << "Admin password set: " << password << "\n";
    }
};

// Client
void createUserPassword(User* user) {
    // Client follows parent contract: uses 8-char password
    user->setPassword("password123");  // 11 chars — fine for both
}

int main() {
    User u;
    AdminUser a;
    createUserPassword(&u);  // ✅ Works
    createUserPassword(&a);  // ✅ Works (weakened pre-condition)
}
```

---

### 4.2 Post-condition

**Definition:** A **post-condition** is a condition that must be **true after** a method runs.

**Rule:** The child class may **strengthen** the post-condition or keep it **as-is**, but must **never weaken** it.

```
┌─────────────────────────────────────────────────────────────────────┐
│                    POST-CONDITION RULE                              │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   Parent: brake()                                                   │
│   Post-condition: "Speed must decrease after brake"                 │
│                                                                      │
│   Child (ElectricCar):                                              │
│   ✅ Strengthens: "Speed decreases AND battery increases"           │
│   ✅ Same: "Speed decreases"                                       │
│   ❌ Weakens: "Speed stays same" (never allowed)                    │
│                                                                      │
│   Why? Client expects speed to decrease after calling brake.       │
│   If child doesn't decrease speed, client's expectation breaks.    │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

**Code Example:**

```cpp
class Car {
protected:
    int speed = 0;
public:
    virtual void accelerate() {
        speed += 20;
        cout << "Accelerating to " << speed << " km/h\n";
    }
    
    // POST-CONDITION: speed must decrease after brake
    virtual void brake() {
        speed = max(0, speed - 20);
        cout << "Braking. Speed: " << speed << " km/h\n";
    }
};

// ✅ Child strengthens post-condition
class HybridCar : public Car {
private:
    int charge = 0;
public:
    // POST-CONDITION (strengthened): speed decreases AND charge increases
    void brake() override {
        speed = max(0, speed - 20);
        charge += 10;
        cout << "Braking (regenerative). Speed: " << speed
             << " km/h, Charge: " << charge << "%\n";
    }
};

// ❌ Child weakens post-condition (DO NOT DO THIS)
class BrokenCar : public Car {
public:
    void brake() override {
        // Speed doesn't decrease — violates post-condition!
        cout << "Braking... but speed stays " << speed << " km/h\n";
    }
};

int main() {
    Car c;
    c.accelerate();  // 20
    c.brake();       // 0
    
    HybridCar h;
    h.accelerate();  // 20
    h.brake();       // 0, charge +10 — valid (strengthened)
    
    BrokenCar b;
    b.accelerate();  // 20
    b.brake();       // Speed still 20 — BROKEN!
}
```

---

## 5. LSP Guidelines Summary

```
┌─────────────────────────────────────────────────────────────────────┐
│                    LSP GUIDELINES SUMMARY                           │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   ┌─────────────────────────────────────────────────────────────┐   │
│   │  SIGNATURE RULE                                             │   │
│   │  ├── Method Argument: same or broader                       │   │
│   │  ├── Return Type: same or narrower (Covariance)             │   │
│   │  └── Exception: same or narrower                            │   │
│   └─────────────────────────────────────────────────────────────┘   │
│                                                                      │
│   ┌─────────────────────────────────────────────────────────────┐   │
│   │  PROPERTY RULE                                              │   │
│   │  ├── Class Invariant: maintain or strengthen (never weaken) │   │
│   │  └── History Constraint: never change history               │   │
│   └─────────────────────────────────────────────────────────────┘   │
│                                                                      │
│   ┌─────────────────────────────────────────────────────────────┐   │
│   │  METHOD RULE                                                │   │
│   │  ├── Pre-condition: same or weaker (never strengthen)       │   │
│   │  └── Post-condition: same or stronger (never weaken)        │   │
│   └─────────────────────────────────────────────────────────────┘   │
│                                                                      │
│   How to spot LSP violation in code:                                │
│   • Child throws exception for parent's method                      │
│   • Child leaves parent's method empty                              │
│   • Child hardcodes a value                                         │
│   • Child narrows parent's contract                                 │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 6. Interface Segregation Principle (ISP)

### Definition

> **"Many client-specific interfaces are better than one general-purpose interface."**
> **"Clients should not be forced to implement methods they don't need."**

### Problem Setup

**Scenario:** A `Shape` interface with `area()` and `volume()` methods.

```
┌─────────────────────────────────────────────────────────────────────┐
│                    ISP VIOLATION                                    │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│                    <<abstract>>                                     │
│                      Shape                                          │
│                    ┌──────────────┐                                 │
│                    │ + area()     │                                 │
│                    │ + volume()   │  ← Problem: 2D shapes          │
│                    └──────┬───────┘    don't have volume!           │
│                           △                                          │
│           ┌───────────────┼───────────────┐                         │
│           │               │               │                         │
│      ┌────┴────┐    ┌─────┴─────┐   ┌─────┴─────┐                  │
│      │ Square  │    │ Rectangle │   │   Cube    │                  │
│      ├─────────┤    ├───────────┤   ├───────────┤                  │
│      │ + area()│    │ + area()  │   │ + area()  │                  │
│      │ + volume│    │ + volume  │   │ + volume()│                  │
│      │  (throws)│   │  (throws) │   │           │                  │
│      └─────────┘    └───────────┘   └───────────┘                  │
│                                                                      │
│   Square & Rectangle are FORCED to implement volume() even though   │
│   they don't need it. → ISP VIOLATION                               │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### ✅ Solution: Segregate Interfaces

```
┌─────────────────────────────────────────────────────────────────────┐
│                    ISP SOLUTION                                     │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│            <<abstract>>          <<abstract>>                       │
│             TwoDShape             ThreeDShape                       │
│           ┌──────────┐           ┌──────────────┐                   │
│           │ + area() │           │ + area()     │                   │
│           └────┬─────┘           │ + volume()   │                   │
│                △                 └──────┬───────┘                   │
│        ┌───────┼───────┐                △                            │
│        │               │                │                            │
│   ┌────┴────┐   ┌──────┴──────┐   ┌─────┴─────┐                    │
│   │ Square  │   │ Rectangle   │   │   Cube    │                    │
│   ├─────────┤   ├─────────────┤   ├───────────┤                    │
│   │ + area()│   │ + area()    │   │ + area()  │                    │
│   └─────────┘   └─────────────┘   │ + volume()│                    │
│                                    └───────────┘                    │
│                                                                      │
│   Square & Rectangle only implement area() — no unnecessary volume() │
│   Cube implements both area() and volume()                          │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Code Example

**❌ Bad (ISP Violated):**

```cpp
class Shape {
public:
    virtual double area() = 0;
    virtual double volume() = 0;  // Problem: not all shapes have volume
    virtual ~Shape() {}
};

class Square : public Shape {
    double side;
public:
    Square(double s) : side(s) {}
    
    double area() override { return side * side; }
    
    double volume() override {
        throw logic_error("Volume not applicable for 2D shape");
    }
};

class Rectangle : public Shape {
    double length, width;
public:
    Rectangle(double l, double w) : length(l), width(w) {}
    
    double area() override { return length * width; }
    
    double volume() override {
        throw logic_error("Volume not applicable for 2D shape");
    }
};

class Cube : public Shape {
    double side;
public:
    Cube(double s) : side(s) {}
    
    double area() override { return 6 * side * side; }
    double volume() override { return side * side * side; }
};
```

**✅ Good (ISP Followed):**

```cpp
// Interface 1: 2D shapes
class TwoDShape {
public:
    virtual double area() = 0;
    virtual ~TwoDShape() {}
};

// Interface 2: 3D shapes (extends 2D)
class ThreeDShape : public TwoDShape {
public:
    virtual double volume() = 0;
    virtual ~ThreeDShape() {}
};

class Square : public TwoDShape {
    double side;
public:
    Square(double s) : side(s) {}
    double area() override { return side * side; }
    // No volume() — not needed!
};

class Rectangle : public TwoDShape {
    double length, width;
public:
    Rectangle(double l, double w) : length(l), width(w) {}
    double area() override { return length * width; }
    // No volume() — not needed!
};

class Cube : public ThreeDShape {
    double side;
public:
    Cube(double s) : side(s) {}
    double area() override { return 6 * side * side; }
    double volume() override { return side * side * side; }
};
```

---

## 7. Dependency Inversion Principle (DIP)

### Definition

> **"High-level modules should not depend on low-level modules. Both should depend on abstractions."**
> **"Abstractions should not depend on details. Details should depend on abstractions."**

### Terminology

| Module Type | Description | Example |
|-------------|-------------|---------|
| **High-level module** | Business logic | Application, UserService |
| **Low-level module** | System interaction | Database, File system, External API |

### Problem Setup

```
┌─────────────────────────────────────────────────────────────────────┐
│                    DIP VIOLATION                                    │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   ┌─────────────────────┐                                           │
│   │   Application       │  (HIGH-LEVEL MODULE)                      │
│   │   (Business Logic)  │                                           │
│   └──────┬──────────────┘                                           │
│          │                                                           │
│          │ Direct dependency (TIGHTLY COUPLED)                       │
│          │                                                           │
│   ┌──────┴──────┐    ┌──────────────┐                               │
│   │  MySQL      │    │  MongoDB     │  (LOW-LEVEL MODULES)          │
│   │  Database   │    │  Database    │                               │
│   └─────────────┘    └──────────────┘                               │
│                                                                      │
│   Problem: Application has direct references to both databases      │
│   To add Cassandra, must modify Application → breaks OCP            │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### ✅ Solution: Introduce Abstraction

```
┌─────────────────────────────────────────────────────────────────────┐
│                    DIP SOLUTION                                     │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   ┌─────────────────────┐                                           │
│   │   Application       │  (HIGH-LEVEL MODULE)                      │
│   │                     │                                           │
│   │   - db: Database*   │───────┐                                  │
│   └─────────────────────┘       │                                  │
│                                 │ (depends on abstraction)          │
│                                 ▼                                  │
│                    ┌────────────────────────┐                       │
│                    │  <<abstract>>          │                       │
│                    │  Database              │                       │
│                    ├────────────────────────┤                       │
│                    │ + save(): void = 0     │                       │
│                    └───────────┬────────────┘                       │
│                                △                                    │
│                    ┌───────────┼───────────┐                        │
│                    │           │           │                        │
│              ┌─────┴─────┐ ┌───┴─────┐ ┌───┴──────┐                 │
│              │ MySQL DB  │ │ MongoDB │ │ Cassandra│                 │
│              │ (details) │ │(details)│ │ (details)│                 │
│              └───────────┘ └─────────┘ └──────────┘                 │
│                                                                      │
│   Now Application depends on Database abstraction, not concrete     │
│   databases. Adding new DB = new class, no Application change.      │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Code Example

**❌ Bad (DIP Violated):**

```cpp
class MySQLDatabase {
public:
    void saveToSQL(string data) {
        cout << "Saving to MySQL: " << data << "\n";
    }
};

class MongoDBDatabase {
public:
    void saveToMongo(string data) {
        cout << "Saving to MongoDB: " << data << "\n";
    }
};

class UserService {
    MySQLDatabase* sqlDB;
    MongoDBDatabase* mongoDB;
public:
    UserService() {
        sqlDB = new MySQLDatabase();
        mongoDB = new MongoDBDatabase();
    }
    
    void storeUserToSQL(string user) {
        sqlDB->saveToSQL(user);
    }
    
    void storeUserToMongo(string user) {
        mongoDB->saveToMongo(user);
    }
    
    // Adding Cassandra → must modify UserService → breaks OCP
};
```

**✅ Good (DIP Followed):**

```cpp
// Abstraction
class Database {
public:
    virtual void save(string data) = 0;
    virtual ~Database() {}
};

// Low-level modules
class MySQLDatabase : public Database {
public:
    void save(string data) override {
        cout << "Saving to MySQL: " << data << "\n";
    }
};

class MongoDBDatabase : public Database {
public:
    void save(string data) override {
        cout << "Saving to MongoDB: " << data << "\n";
    }
};

class CassandraDatabase : public Database {
public:
    void save(string data) override {
        cout << "Saving to Cassandra: " << data << "\n";
    }
};

// High-level module (depends only on abstraction)
class UserService {
    Database* db;  // Dependency injection
public:
    UserService(Database* database) : db(database) {}
    
    void storeUser(string user) {
        db->save(user);  // Polymorphism — works for any DB
    }
};

int main() {
    // Runtime decides which DB to use
    UserService sqlService(new MySQLDatabase());
    sqlService.storeUser("Alice");     // Output: Saving to MySQL: Alice
    
    UserService mongoService(new MongoDBDatabase());
    mongoService.storeUser("Bob");     // Output: Saving to MongoDB: Bob
    
    UserService cassandraService(new CassandraDatabase());
    cassandraService.storeUser("Carol");  // Output: Saving to Cassandra: Carol
}
```

### Real-Life Analogy: CEO and Developers

```
┌─────────────────────────────────────────────────────────────────────┐
│                    DIP IN A COMPANY                                 │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   CEO (High-level) ──▶ Manager (Abstraction) ◀── Developers         │
│                                                        (Low-level)  │
│                                                                      │
│   • CEO doesn't talk directly to developers                         │
│   • CEO talks to Manager (interface/abstraction)                    │
│   • Developers talk to Manager, not CEO                             │
│   • If developers change, CEO doesn't need to know                  │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Key Quote from the Lecture

> **"If Open/Closed Principle is the target, then Dependency Inversion Principle is the solution."**

---

## 8. SOLID Principles — Complete Summary

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                       SOLID DESIGN PRINCIPLES                                │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│   ┌────────────────────────────────────────────────────────────────────┐    │
│   │  S — SINGLE RESPONSIBILITY PRINCIPLE (SRP)                         │    │
│   │  • A class should have only one reason to change                   │    │
│   │  • One class = One responsibility                                  │    │
│   │  • Split ShoppingCart → Cart + Printer + Storage                   │    │
│   └────────────────────────────────────────────────────────────────────┘    │
│                                                                              │
│   ┌────────────────────────────────────────────────────────────────────┐    │
│   │  O — OPEN/CLOSED PRINCIPLE (OCP)                                   │    │
│   │  • Open for extension, closed for modification                     │    │
│   │  • Use abstraction + inheritance + polymorphism                    │    │
│   │  • New persistence types → new classes, no modifications           │    │
│   └────────────────────────────────────────────────────────────────────┘    │
│                                                                              │
│   ┌────────────────────────────────────────────────────────────────────┐    │
│   │  L — LISKOV SUBSTITUTION PRINCIPLE (LSP)                           │    │
│   │  • Subclass must be substitutable for base class                   │    │
│   │  • Guidelines:                                                     │    │
│   │    - Signature Rule (args, return, exceptions)                     │    │
│   │    - Property Rule (invariant, history constraint)                 │    │
│   │    - Method Rule (pre-condition, post-condition)                   │    │
│   └────────────────────────────────────────────────────────────────────┘    │
│                                                                              │
│   ┌────────────────────────────────────────────────────────────────────┐    │
│   │  I — INTERFACE SEGREGATION PRINCIPLE (ISP)                         │    │
│   │  • Many client-specific interfaces > one general-purpose interface │    │
│   │  • Don't force clients to implement unused methods                 │    │
│   │  • Split Shape → TwoDShape + ThreeDShape                           │    │
│   └────────────────────────────────────────────────────────────────────┘    │
│                                                                              │
│   ┌────────────────────────────────────────────────────────────────────┐    │
│   │  D — DEPENDENCY INVERSION PRINCIPLE (DIP)                          │    │
│   │  • High-level and low-level modules depend on abstractions         │    │
│   │  • Use dependency injection + polymorphism                         │    │
│   │  • Application → Database abstraction → MySQL/Mongo/Cassandra      │    │
│   └────────────────────────────────────────────────────────────────────┘    │
│                                                                              │
│   Created by: Robert C. Martin (Uncle Bob), 2000                            │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 9. The Trade-off Reality

The lecture ends with a crucial point: **SOLID principles are ideals, not laws.**

```
┌─────────────────────────────────────────────────────────────────────┐
│                    SOLID IS A TRADE-OFF                             │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   • Following ALL principles perfectly is nearly impossible        │
│   • Business logic sometimes requires breaking a principle          │
│   • Just like DSA has time-space trade-offs, LLD has              │
│     SOLID vs business-logic trade-offs                              │
│                                                                      │
│   "At the end of the day, business logic is the main thing.        │
│    That's what brings money to the application."                    │
│                                                                      │
│   Goal: Try to follow SOLID as much as possible.                   │
│   It will make your code cleaner, more maintainable, and more      │
│   scalable — but perfection isn't always practical.                 │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 10. Key Takeaways

| Principle | Core Idea | Key Rule |
|-----------|-----------|----------|
| **LSP — Signature** | Method signature must match | Args same/broader; return same/narrower; exception same/narrower |
| **LSP — Property** | Class-level rules preserved | Invariant: maintain/strengthen; History: never change |
| **LSP — Method** | Method contracts preserved | Pre-condition: same/weaker; Post-condition: same/stronger |
| **ISP** | Don't force unused methods | Split interfaces by client needs |
| **DIP** | Depend on abstractions | High + low level both depend on interfaces |

**How to Spot LSP Violations:**
- Child throws exception for parent's method
- Child leaves parent's method empty
- Child hardcodes a value
- Child narrows parent's contract

**DIP in One Line:**
> "If OCP is the target, DIP is the solution."

---

## 07. Build Google Docs | A Real-World LLD Project (45:19)

This lecture applies **all OOP pillars** and **SOLID principles** to a real LLD problem: designing a **Document Editor** (like Google Docs). The instructor walks through three designs — Bad → Better → Final — showing the interview approach.

---

## 1. Problem Statement

**Design a Document Editor** that:
- Supports **text** and **images** (initially)
- Must be **scalable** — later support tables, videos, fonts, newlines, tabs, spaces
- Should allow users to add elements, render the document, and save it

### Two Approaches to Any LLD Problem

| Approach | Description | When to Use |
|----------|-------------|-------------|
| **Top-Down** | Design topmost object first, then dependencies | Some specific problems |
| **Bottom-Up** | Design small objects first, then compose larger ones | ✅ Most common in LLD interviews |

> The instructor prefers **Bottom-Up** for this problem.

---

## 2. Bad Design (Violates SRP & OCP)

### The Single-Class Approach

```
┌─────────────────────────────────────────────────────────────┐
│                     DocumentEditor                          │
├─────────────────────────────────────────────────────────────┤
│ - documentElements: vector<string>                          │
│ - renderedDocument: string                                  │
├─────────────────────────────────────────────────────────────┤
│ + addText(string): void                                     │
│ + addImage(string path): void                               │
│ + renderDocument(): string                                  │
│ + saveToFile(): void                                        │
└─────────────────────────────────────────────────────────────┘
```

### Bad Code

```cpp
class DocumentEditor {
    vector<string> documentElements;
    string renderedDocument;
    
public:
    void addText(string text) {
        documentElements.push_back(text);
    }
    
    void addImage(string imagePath) {
        documentElements.push_back(imagePath);
    }
    
    string renderDocument() {
        if (renderedDocument.empty()) {
            string result;
            for (auto element : documentElements) {
                // Hacky way to detect images
                if (element.size() > 4 &&
                    (element.substr(element.size() - 4) == ".jpg" ||
                     element.substr(element.size() - 4) == ".png")) {
                    result += "[Image: " + element + "]\n";
                } else {
                    result += element + "\n";
                }
            }
            renderedDocument = result;
        }
        return renderedDocument;
    }
    
    void saveToFile() {
        ofstream file("document.txt");
        if (file.is_open()) {
            file << renderDocument();
            file.close();
            cout << "Document saved to file.\n";
        } else {
            cout << "Unable to open file.\n";
        }
    }
};

int main() {
    DocumentEditor editor;
    editor.addText("Hello, World!");
    editor.addImage("picture.jpg");
    editor.addText("This is a document editor.");
    
    cout << editor.renderDocument() << endl;
    editor.saveToFile();
}
```

### Problems with Bad Design

```
┌─────────────────────────────────────────────────────────────┐
│                  PROBLEMS WITH BAD DESIGN                   │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ❌ SRP VIOLATION                                           │
│     • Handles text, images, rendering, AND file saving      │
│     • Multiple reasons to change                            │
│                                                             │
│  ❌ OCP VIOLATION                                           │
│     • Adding new element (video, table) requires            │
│       modifying DocumentEditor class                        │
│                                                             │
│  ❌ No abstraction / No polymorphism                        │
│     • Everything stored as string (hacky image detection)   │
│     • Can't scale to new element types                      │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## 3. Better Design (Applying SRP & OCP)

### Step 1: Extract DocumentElement Hierarchy

```
                    ┌──────────────────────────┐
                    │   <<abstract>>           │
                    │   DocumentElement        │
                    ├──────────────────────────┤
                    │ + render(): string = 0   │
                    └────────────┬─────────────┘
                                 △
              ┌──────────────────┼──────────────────┐
              │                  │                  │
     ┌────────┴────────┐ ┌───────┴────────┐ ┌───────┴────────┐
     │  TextElement    │ │  ImageElement  │ │ NewLineElement │
     ├─────────────────┤ ├────────────────┤ ├────────────────┤
     │ - text: string  │ │ - imagePath    │ │ + render()     │
     │ + render()      │ │ + render()     │ └────────────────┘
     └─────────────────┘ └────────────────┘
```

```cpp
// ABSTRACT CLASS — The extension point
class DocumentElement {
public:
    virtual string render() = 0;
    virtual ~DocumentElement() {}
};

class TextElement : public DocumentElement {
    string text;
public:
    TextElement(string t) : text(t) {}
    string render() override { return text; }
};

class ImageElement : public DocumentElement {
    string imagePath;
public:
    ImageElement(string path) : imagePath(path) {}
    string render() override {
        return "[Image: " + imagePath + "]";
    }
};

// NEW — Added without touching existing classes (OCP!)
class NewLineElement : public DocumentElement {
public:
    string render() override { return "\n"; }
};

class TabSpaceElement : public DocumentElement {
public:
    string render() override { return "\t"; }
};
```

### Step 2: Extract Document Class (CRUD Operations)

```cpp
class Document {
    vector<DocumentElement*> documentElements;
public:
    void addElement(DocumentElement* element) {
        documentElements.push_back(element);
    }
    
    // Needed for the renderer to access elements
    vector<DocumentElement*> getElements() {
        return documentElements;
    }
};
```

### Step 3: Extract Persistence Hierarchy

```
                    ┌──────────────────────────┐
                    │   <<abstract>>           │
                    │   Persistence            │
                    ├──────────────────────────┤
                    │ + save(string): void = 0 │
                    └────────────┬─────────────┘
                                 △
                    ┌────────────┴─────────────┐
                    │                          │
           ┌────────┴────────┐        ┌────────┴────────┐
           │  FileStorage    │        │   DBStorage     │
           ├─────────────────┤        ├─────────────────┤
           │ + save(string)  │        │ + save(string)  │
           └─────────────────┘        └─────────────────┘
```

```cpp
class Persistence {
public:
    virtual void save(string data) = 0;
    virtual ~Persistence() {}
};

class FileStorage : public Persistence {
public:
    void save(string data) override {
        ofstream file("document.txt");
        if (file.is_open()) {
            file << data;
            file.close();
            cout << "Document saved to file.\n";
        }
    }
};

class DBStorage : public Persistence {
public:
    void save(string data) override {
        // SQL/MongoDB connection logic here
        cout << "Document saved to DB.\n";
    }
};
```

### Step 4: DocumentEditor Delegates Everything

```cpp
class DocumentEditor {
    Document* document;
    Persistence* storage;
    string renderedDocument;
    
public:
    DocumentEditor(Document* doc, Persistence* store)
        : document(doc), storage(store) {}
    
    void addText(string text) {
        // Delegates to Document
        document->addElement(new TextElement(text));
    }
    
    void addImage(string imagePath) {
        document->addElement(new ImageElement(imagePath));
    }
    
    string renderDocument() {
        if (renderedDocument.empty()) {
            string result;
            for (auto element : document->getElements()) {
                result += element->render() + "\n";
            }
            renderedDocument = result;
        }
        return renderedDocument;
    }
    
    void saveDocument() {
        // Delegates to Persistence
        storage->save(renderDocument());
    }
};
```

### Diagram of Better Design

```
                          ┌──────────────────┐
                          │  DocumentEditor  │
                          ├──────────────────┤
                          │ - document       │
                          │ - storage        │
                          ├──────────────────┤
                          │ + addText()      │
                          │ + addImage()     │
                          │ + renderDoc()    │
                          │ + saveDoc()      │
                          └────────┬─────────┘
                                   │
                    ┌──────────────┴───────────────┐
                    │ (has-a)                       │ (has-a)
                    ▼                               ▼
            ┌──────────────┐              ┌────────────────┐
            │  Document    │              │  Persistence   │
            ├──────────────┤              │  <<abstract>>  │
            │ + addElement │              ├────────────────┤
            │ + getElements│              │ + save() = 0   │
            └───────┬──────┘              └───────┬────────┘
                    │                              △
                    │ (has-a)                      │
                    ▼                              │
            ┌──────────────┐              ┌────────┴────────┐
            │DocumentElement│             │                 │
            │  <<abstract>>│      ┌──────┴──────┐  ┌───────┴─────┐
            ├──────────────┤      │FileStorage  │  │  DBStorage  │
            │ + render()=0 │      ├─────────────┤  ├─────────────┤
            └───────┬──────┘      │ + save()    │  │  + save()   │
                    △             └─────────────┘  └─────────────┘
                    │
      ┌─────────────┼─────────────┬──────────────┐
      │             │             │              │
┌─────┴────┐ ┌─────┴────┐ ┌──────┴───┐ ┌────────┴─────┐
│TextElem  │ │ImageElem │ │NewLine   │ │TabSpace      │
└──────────┘ └──────────┘ └──────────┘ └──────────────┘
```

---

## 4. SOLID Compliance Checklist

| Principle | How It's Followed |
|-----------|-------------------|
| **S**RP | Each class has one job: `Document` (holds elements), `DocumentElement` (renders itself), `Persistence` (saves), `DocumentEditor` (delegates) |
| **O**CP | New element types = new subclasses (no modification) |
| **L**SP | All `DocumentElement` subclasses (`TextElement`, `ImageElement`) are substitutable |
| **I**SP | `DocumentElement` interface only has `render()`; `Persistence` only has `save()` — no unused methods |
| **D**IP | `DocumentEditor` depends on abstractions (`Document`, `Persistence`) not on concrete classes |

---

## 5. Final Design (Fixing Remaining Issues)

### The Counter-Question: Principle of Least Knowledge (Law of Demeter)

> **"You should only talk to your immediate friends."**

The **Better Design** still has an issue: `DocumentEditor.renderDocument()` fetches elements from `Document` and then calls `element->render()`. This is **talking to friends of friends** (DocumentEditor → Document → DocumentElement) → increases **coupling**.

### Fix: Extract DocumentRenderer

```
┌────────────────┐
│DocumentRenderer│
├────────────────┤
│ - document     │
├────────────────┤
│ + render()     │
└────────────────┘
```

```cpp
class DocumentRenderer {
    Document* document;
public:
    DocumentRenderer(Document* doc) : document(doc) {}
    
    string render() {
        string result;
        // Talks to Document (immediate friend), which returns elements
        for (auto element : document->getElements()) {
            result += element->render() + "\n";
        }
        return result;
    }
};
```

### Final Architecture

```
                    ┌──────────────────┐
                    │     Client       │
                    │   (main code)    │
                    └────────┬─────────┘
                             │ (uses all 4)
              ┌──────────────┼──────────────┬──────────────┐
              ▼              ▼              ▼              ▼
      ┌────────────┐ ┌──────────────┐ ┌────────────┐ ┌───────────┐
      │Document    │ │DocumentEditor│ │Document    │ │Persistence│
      │            │ │              │ │Renderer    │ │<<abstract>>│
      ├────────────┤ ├──────────────┤ ├────────────┤ ├───────────┤
      │+ addElement│ │+ addText     │ │ + render() │ │ + save()=0│
      │+ getElements│ │+ addImage    │ └──────┬─────┘ └─────┬─────┘
      └─────┬──────┘ └──────┬───────┘        │             │
            │               │ (has-a)         │             │
            │(has-a)        ▼                 │             │
            │      ┌──────────────┐           │             │
            │      │ Document     │◀──────────┘             │
            │      └──────┬───────┘                         │
            │             │                                  │
            │             │(has-a)                           │
            │             ▼                                  │
            │      ┌──────────────────┐         ┌───────────┴────────┐
            │      │ DocumentElement  │         │                    │
            │      │  <<abstract>>    │    ┌────┴──────┐  ┌──────────┴─┐
            │      ├──────────────────┤    │FileStorage│  │ DBStorage  │
            │      │ + render()=0     │    └───────────┘  └────────────┘
            │      └────────┬─────────┘
            │               △
            │  ┌────────────┼───────────┬────────────┐
            │  │            │           │            │
            │  ▼            ▼           ▼            ▼
            │┌────────┐ ┌────────┐ ┌─────────┐ ┌──────────┐
            ││TextElem│ │ImgElem │ │NewLine  │ │TabSpace  │
            │└────────┘ └────────┘ └─────────┘ └──────────┘
            │
            └────(elements stored here)
```

### Final Client Code

```cpp
int main() {
    // 1. Create element hierarchy
    Document* document = new Document();
    
    // 2. Create editor (only for editing)
    DocumentEditor* editor = new DocumentEditor(document);
    editor->addText("Hello, World!");
    editor->addImage("picture.jpg");
    editor->addText("This is a real-world document editor.");
    editor->addNewLine();
    editor->addTabSpace();
    editor->addText("Indented text");
    
    // 3. Render (separate responsibility)
    DocumentRenderer renderer(document);
    string output = renderer.render();
    cout << output << endl;
    
    // 4. Save (separate responsibility, pluggable storage)
    Persistence* storage = new FileStorage();
    storage->save(output);
    
    // Or use DBStorage:
    // Persistence* dbStorage = new DBStorage();
    // dbStorage->save(output);
    
    delete document;
    delete editor;
    delete storage;
}
```

**Output:**
```
Hello, World!
[Image: picture.jpg]
This is a real-world document editor.
    Indented text
Document saved to file.
```

---

## 6. Key Takeaways

### Design Evolution Summary

| Stage | Description | Principles Followed |
|-------|-------------|---------------------|
| **Bad** | One class does everything | ❌ SRP, OCP |
| **Better** | Extract abstractions, delegate | ✅ SRP, OCP, LSP, ISP, DIP |
| **Final** | Separate renderer; client orchestrates | ✅ All SOLID + LoD |

### Lessons from This Case Study

```
┌─────────────────────────────────────────────────────────────────┐
│                    INTERVIEW WISDOM                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  1. START WITH BAD DESIGN, then improve iteratively             │
│                                                                  │
│  2. DELEGATION is key — one class should not do everything      │
│                                                                  │
│  3. ABSTRACTION + POLYMORPHISM enables OCP                      │
│                                                                  │
│  4. CLIENT should orchestrate multiple services                 │
│     (Don't overload one class with all responsibilities)        │
│                                                                  │
│  5. PRINCIPLE OF LEAST KNOWLEDGE (Law of Demeter)              │
│     • Talk only to immediate friends                           │
│     • Don't call methods on objects returned by methods         │
│                                                                  │
│  6. SOLID ARE PRINCIPLES, NOT LAWS                              │
│     • Always a TRADE-OFF                                        │
│     • In interviews, discuss trade-offs with interviewer        │
│     • No perfect design exists in LLD                           │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

### The Trade-off Example

In this case study, two principles conflict:
- **SRP** says `DocumentRenderer` should only render (good)
- **LoD** says `DocumentRenderer` shouldn't reach through `Document` to `DocumentElement` (bad)

**Resolution:** Accept the small LoD violation because SRP benefit outweighs it. This is a **subjective design decision** — discuss it in interviews.

### Final Principle Checklist

| Design Decision | Enables |
|-----------------|---------|
| Abstract `DocumentElement` | OCP, LSP, DIP |
| Abstract `Persistence` | DIP, OCP |
| Separate `DocumentRenderer` | SRP, LoD |
| `DocumentEditor` delegates | SRP, DIP |
| Client orchestrates | LoD |
| Small interfaces | ISP |

---

## 7. Complete Code Files Reference

The lecture references these final classes:
1. `DocumentElement` (abstract) + `TextElement`, `ImageElement`, `NewLineElement`, `TabSpaceElement`
2. `Document` (CRUD on elements)
3. `DocumentRenderer` (renders document)
4. `Persistence` (abstract) + `FileStorage`, `DBStorage`
5. `DocumentEditor` (adds text/images only)
6. `Client` (main — orchestrates all)

---

## 08. Strategy Design Pattern Explained with Real-World Example (32:39)

This lecture introduces **Design Patterns** — reusable solutions to common software design problems — and deep-dives into the **Strategy Design Pattern**, proving why **composition is better than inheritance**.

---

## 1. Introduction to Design Patterns

### What Are Design Patterns?

> **Design Patterns are proven solutions to recurring problems in software design.**
> When many developers faced the same problem, they found a standard path to solve it. That path is called a design pattern.

### Why Do We Need Them?

```
┌─────────────────────────────────────────────────────────────────────┐
│              WHY DESIGN PATTERNS?                                   │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   Applications always evolve:                                       │
│   • New features keep coming                                        │
│   • Requirements keep changing                                      │
│   • "Change is the only constant"                                   │
│                                                                      │
│   Goal: A FLEXIBLE DESIGN that minimizes code changes               │
│         when new features are added                                 │
│                                                                      │
│   Tools for flexibility:                                            │
│   • OOP Principles (Abstraction, Encapsulation, Inheritance,        │
│     Polymorphism)                                                   │
│   • SOLID Principles                                                │
│   • Design Patterns  ← This series                                │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### The Core Idea Behind All Design Patterns

Every design pattern essentially does the same thing:

```
┌─────────────────────────────────────────────────────────────────────┐
│              THE FUNDAMENTAL DESIGN PATTERN IDEA                    │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   Application has TWO types of code:                                │
│                                                                      │
│   ┌─────────────────────┐    ┌─────────────────────┐                │
│   │   STATIC PART       │    │   DYNAMIC PART      │                │
│   │   (Doesn't change)  │    │   (Changes often)   │                │
│   └─────────────────────┘    └─────────────────────┘                │
│                                                                      │
│   PATTERN:                                                          │
│   1. EXTRACT the changing part                                      │
│   2. Put it into SEPARATE classes                                   │
│   3. Keep the static part isolated                                  │
│                                                                      │
│   Result: Changes affect ONLY the dynamic part —                    │
│           no impact on the static part                              │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Official Count

- **23 official design patterns** exist
- All implement the same idea (separate changing from non-changing) in different ways

---

## 2. Strategy Pattern — Problem Statement

### Application: Robot Simulation

We're building an application that **simulates different robots** walking, talking, and (sometimes) flying.

### Initial (Naive) Design Using Inheritance

```
                    ┌──────────────────────────┐
                    │   <<abstract>>           │
                    │   Robot                  │
                    ├──────────────────────────┤
                    │ + walk(): void           │
                    │ + talk(): void           │
                    │ + projection(): void = 0 │
                    └────────────┬─────────────┘
                                 △
                    ┌────────────┴─────────────┐
                    │                          │
           ┌────────┴────────┐        ┌────────┴────────┐
           │ CompanionRobot  │        │   WorkerRobot   │
           ├─────────────────┤        ├─────────────────┤
           │ + projection()  │        │ + projection()  │
           └─────────────────┘        └─────────────────┘
```

```cpp
class Robot {
public:
    virtual void walk() { cout << "Walking normally\n"; }
    virtual void talk() { cout << "Talking normally\n"; }
    virtual void projection() = 0;  // each robot looks different
    virtual ~Robot() {}
};

class CompanionRobot : public Robot {
public:
    void projection() override { cout << "Companion projection\n"; }
};

class WorkerRobot : public Robot {
public:
    void projection() override { cout << "Worker projection\n"; }
};
```

### New Requirement: Flying Robots

**Sparrow Robot** (flies with wings), **Crow Robot** (flies with wings), **Jet Robot** (flies with jet), **Jet Robot 2, Jet Robot 3**...

### ❌ What Happens If We Keep Using Inheritance?

```
                    Robot
                      │
         ┌────────────┼─────────────┐
         │            │             │
    Companion   Flyable Robot    Worker
                      │
              ┌───────┼────────┐
              │       │        │
          Sparrow   Crow     JetRobot
                                │
                   ┌────────────┼────────────┐
                   │            │            │
              JetRobot2    JetRobot3    JetRobot4
                                            │
                                       (and so on...)
```

### Problems with Inheritance

```
┌─────────────────────────────────────────────────────────────────────┐
│              PROBLEMS WITH INHERITANCE                              │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   1. ❌ CODE DUPLICATION                                            │
│      • Same fly() method copy-pasted across many classes            │
│      • Violates DRY (Don't Repeat Yourself)                         │
│                                                                      │
│   2. ❌ BLOATED HIERARCHY                                           │
│      • Every new behavior (walk/talk/fly) combination explodes     │
│      • Combinatorial explosion of subclasses                        │
│                                                                      │
│   3. ❌ OCP VIOLATION                                               │
│      • Adding a new robot type requires changing structure          │
│                                                                      │
│   4. ❌ LSP VIOLATION RISK                                          │
│      • Robots forced to implement methods they don't need           │
│                                                                      │
│   ⚡ FAMOUS QUOTE:                                                  │
│   "The solution to inheritance is NOT more inheritance."            │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 3. Strategy Pattern — Solution

### Definition

> **Strategy Pattern defines a family of algorithms, puts them into separate classes, so that they can be changed at runtime.**

### The Key Insight

Split the robot's behavior into **separate strategies**:

| Behavior | Strategy Interface |
|----------|---------------------|
| Walk | `Walkable` |
| Talk | `Talkable` |
| Fly | `Flyable` |
| Projection | `Projectable` (optional improvement) |

### Step 1: Extract Behavior Interfaces

```
    ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐
    │  <<abstract>>   │  │  <<abstract>>   │  │  <<abstract>>   │
    │   Walkable      │  │   Talkable      │  │   Flyable       │
    ├─────────────────┤  ├─────────────────┤  ├─────────────────┤
    │ + walk(): void  │  │ + talk(): void  │  │ + fly(): void   │
    └────────┬────────┘  └────────┬────────┘  └────────┬────────┘
             △                    △                    △
      ┌──────┴──────┐      ┌──────┴──────┐      ┌──────┴──────┐
      │             │      │             │      │             │
 ┌────┴────┐   ┌────┴────┐ ┌────┴────┐ ┌────┴────┐ ┌────┴────┐ ┌────┴────┐
 │ Normal  │   │ No Walk │ │ Normal  │ │ No Talk │ │ Normal  │ │ No Fly  │
 │  Walk   │   │         │ │  Talk   │ │         │ │  Fly    │ │         │
 └─────────┘   └─────────┘ └─────────┘ └─────────┘ └─────────┘ └─────────┘
```

**Note:** "Not walking", "Not talking", "Not flying" are ALSO valid behaviors!

### Step 2: Code the Strategy Interfaces

```cpp
// ============ WALKING STRATEGY ============
class Walkable {
public:
    virtual void walk() = 0;
    virtual ~Walkable() {}
};

class NormalWalk : public Walkable {
public:
    void walk() override { cout << "Walking normally...\n"; }
};

class NoWalk : public Walkable {
public:
    void walk() override { cout << "Cannot walk\n"; }
};

// ============ TALKING STRATEGY ============
class Talkable {
public:
    virtual void talk() = 0;
    virtual ~Talkable() {}
};

class NormalTalk : public Talkable {
public:
    void talk() override { cout << "Talking normally...\n"; }
};

class NoTalk : public Talkable {
public:
    void talk() override { cout << "Cannot talk\n"; }
};

// ============ FLYING STRATEGY ============
class Flyable {
public:
    virtual void fly() = 0;
    virtual ~Flyable() {}
};

class NormalFly : public Flyable {
public:
    void fly() override { cout << "Flying normally...\n"; }
};

class NoFly : public Flyable {
public:
    void fly() override { cout << "Cannot fly\n"; }
};

// ============ PROJECTION STRATEGY (Optional improvement) ============
class Projectable {
public:
    virtual void projection() = 0;
    virtual ~Projectable() {}
};

class CompanionProjection : public Projectable {
public:
    void projection() override { cout << "Companion robot projection\n"; }
};

class WorkerProjection : public Projectable {
public:
    void projection() override { cout << "Worker robot projection\n"; }
};
```

### Step 3: Robot Class Uses Composition

```cpp
class Robot {
private:
    // COMPOSITION: Robot HAS-A strategy for each behavior
    Walkable* walkBehavior;
    Talkable* talkBehavior;
    Flyable* flyBehavior;
    Projectable* projectionBehavior;
    
public:
    Robot(Walkable* w, Talkable* t, Flyable* f, Projectable* p)
        : walkBehavior(w), talkBehavior(t),
          flyBehavior(f), projectionBehavior(p) {}
    
    // All methods DELEGATE to the strategies
    void walk() { walkBehavior->walk(); }
    void talk() { talkBehavior->talk(); }
    void fly()  { flyBehavior->fly(); }
    void projection() { projectionBehavior->projection(); }
    
    ~Robot() {
        delete walkBehavior;
        delete talkBehavior;
        delete flyBehavior;
        delete projectionBehavior;
    }
};
```

### Step 4: Client Creates Robots with Any Combination

```cpp
int main() {
    // Companion Robot: walks, talks, doesn't fly
    Robot* companion = new Robot(
        new NormalWalk(),
        new NormalTalk(),
        new NoFly(),
        new CompanionProjection()
    );
    
    // Worker Robot: doesn't walk, doesn't talk, flies
    Robot* worker = new Robot(
        new NoWalk(),
        new NoTalk(),
        new NormalFly(),
        new WorkerProjection()
    );
    
    cout << "--- Companion Robot ---\n";
    companion->walk();       // Walking normally...
    companion->talk();       // Talking normally...
    companion->fly();        // Cannot fly
    companion->projection(); // Companion robot projection
    
    cout << "\n--- Worker Robot ---\n";
    worker->walk();       // Cannot walk
    worker->talk();       // Cannot talk
    worker->fly();        // Flying normally...
    worker->projection(); // Worker robot projection
    
    delete companion;
    delete worker;
}
```

**Output:**
```
--- Companion Robot ---
Walking normally...
Talking normally...
Cannot fly
Companion robot projection

--- Worker Robot ---
Cannot walk
Cannot talk
Flying normally...
Worker robot projection
```

### Runtime Change: A Key Feature

```cpp
// Change behavior at RUNTIME
companion = new Robot(
    new NoWalk(),      // Changed from NormalWalk
    new NormalTalk(),
    new NormalFly(),   // Changed from NoFly
    new CompanionProjection()
);
```

---

## 4. Standard UML for Strategy Pattern

```
                    ┌────────────────────────┐
                    │        Client          │
                    ├────────────────────────┤
                    │ - strategy: Strategy*  │
                    ├────────────────────────┤
                    │ + execute(): void      │
                    │   { strategy->run(); } │
                    └───────────┬────────────┘
                                │ (has-a)
                                ▼
                    ┌────────────────────────┐
                    │   <<abstract>>         │
                    │     Strategy           │
                    ├────────────────────────┤
                    │ + run(): void = 0      │
                    └───────────┬────────────┘
                                △
                                │ (inheritance)
              ┌─────────────────┼─────────────────┐
              │                 │                 │
     ┌────────┴───────┐ ┌───────┴───────┐ ┌───────┴───────┐
     │ConcreteStrategy│ │ConcreteStrategy│ │ConcreteStrategy│
     │      1         │ │      2         │ │      3         │
     ├────────────────┤ ├────────────────┤ ├────────────────┤
     │ + run()        │ │ + run()        │ │ + run()        │
     └────────────────┘ └────────────────┘ └────────────────┘
```

### Key Points
- **Client**: The class that uses the strategies (e.g., `Robot`)
- **Strategy**: The abstract interface (e.g., `Walkable`)
- **Concrete Strategy**: Actual implementations (e.g., `NormalWalk`, `NoWalk`)
- **Client HAS-A Strategy** (composition, not inheritance)

---

## 5. Before vs After Comparison

| Aspect | Inheritance Approach | Strategy Pattern |
|--------|---------------------|------------------|
| **Code Reuse** | Duplicated across classes | Single implementation per strategy |
| **New Behavior** | Modify class hierarchy | Add new strategy class |
| **New Combination** | New subclass needed | Compose with existing strategies |
| **Runtime Change** | Impossible | Possible (change reference) |
| **DRY Principle** | ❌ Violated | ✅ Followed |
| **SRP** | ❌ Multiple behaviors per class | ✅ Each strategy has one job |
| **OCP** | ❌ Violated | ✅ Followed |
| **Class Hierarchy** | Explodes combinatorially | Flat and simple |
| **Client Coupling** | Tight (inherits everything) | Loose (has only what it needs) |

---

## 6. Real-World Examples of Strategy Pattern

### Example 1: Payment System

```
                    ┌─────────────────────┐
                    │  PaymentSystem      │
                    ├─────────────────────┤
                    │ - strategy: Payable │
                    ├─────────────────────┤
                    │ + payNow(): void    │
                    └──────────┬──────────┘
                               │ (has-a)
                               ▼
                    ┌─────────────────────┐
                    │  <<abstract>>       │
                    │     Payable         │
                    ├─────────────────────┤
                    │ + pay(): void = 0   │
                    └──────────┬──────────┘
                               △
              ┌────────────────┼────────────────┐
              │                │                │
        ┌─────┴─────┐    ┌─────┴─────┐    ┌─────┴──────┐
        │  UPI      │    │Credit/Debit│    │Net Banking │
        │ Payment   │    │ Card       │    │            │
        └───────────┘    └────────────┘    └────────────┘
```

```cpp
class Payable {
public:
    virtual void pay() = 0;
    virtual ~Payable() {}
};

class UPIPayment : public Payable {
public:
    void pay() override { cout << "Paying via UPI\n"; }
};

class CardPayment : public Payable {
public:
    void pay() override { cout << "Paying via Credit/Debit Card\n"; }
};

class NetBankingPayment : public Payable {
public:
    void pay() override { cout << "Paying via Net Banking\n"; }
};

class PaymentSystem {
    Payable* strategy;
public:
    PaymentSystem(Payable* p) : strategy(p) {}
    void payNow() { strategy->pay(); }
};
```

### Example 2: Sorting Algorithms

```
                        ┌────────────────────┐
                        │  Sorter (Client)   │
                        ├────────────────────┤
                        │ - strategy: Sort*  │
                        ├────────────────────┤
                        │ + sort()           │
                        └──────────┬─────────┘
                                   │ (has-a)
                                   ▼
                        ┌────────────────────┐
                        │  <<abstract>>      │
                        │     Sort           │
                        ├────────────────────┤
                        │ + sort() = 0       │
                        └──────────┬─────────┘
                                   △
                  ┌────────────────┼─────────────────┐
                  │                │                 │
           ┌──────┴────┐    ┌──────┴─────┐    ┌──────┴────┐
           │Quick Sort │    │Merge Sort  │    │Insertion  │
           │           │    │            │    │Sort       │
           └───────────┘    └────────────┘    └───────────┘
                  │
           ┌──────┼──────┐
           │             │
      ┌────┴────┐   ┌────┴─────┐
      │Normal   │   │Randomized│
      │Quick    │   │Quick     │
      └─────────┘   └──────────┘
```

The client calls `sort()`, and polymorphism decides which algorithm runs.

---

## 7. Benefits Summary

```
┌─────────────────────────────────────────────────────────────────────┐
│              BENEFITS OF STRATEGY PATTERN                           │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   ✅ FAVORS COMPOSITION OVER INHERITANCE                            │
│      "The solution to inheritance is not more inheritance."         │
│                                                                      │
│   ✅ OPEN/CLOSED PRINCIPLE                                          │
│      • New strategies = new classes (no modification)               │
│      • Existing Robot class untouched when adding new behavior      │
│                                                                      │
│   ✅ SINGLE RESPONSIBILITY PRINCIPLE                                │
│      • Each strategy class has one job                              │
│      • Robot class only delegates                                   │
│                                                                      │
│   ✅ RUNTIME FLEXIBILITY                                            │
│      • Behavior can be swapped at runtime                           │
│      • Perfect for dynamic systems                                  │
│                                                                      │
│   ✅ NO COMBINATORIAL EXPLOSION                                     │
│      • N behaviors with M variants = N*M strategies (not N^M)       │
│      • Not a hierarchical tree, just flat interchangeable parts     │
│                                                                      │
│   ✅ FOLLOWS DRY PRINCIPLE                                          │
│      • No code duplication across classes                           │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 8. Key Takeaways

### The One-Line Summary

> **"Favor Composition Over Inheritance."**

### The Big Realization

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                      │
│    ┌─────────────────────┐       ┌─────────────────────┐            │
│    │   INHERITANCE       │       │   COMPOSITION       │            │
│    │   (IS-A)            │       │   (HAS-A)           │            │
│    ├─────────────────────┤       ├─────────────────────┤            │
│    │ • Rigid at compile  │       │ • Flexible at run   │            │
│    │ • Explodes with new │       │ • Scales linearly   │            │
│    │   combinations      │       │   with new features │            │
│    │ • Code duplication  │       │ • No duplication    │            │
│    │ • Tight coupling    │       │ • Loose coupling    │            │
│    └─────────────────────┘       └─────────────────────┘            │
│                                                                      │
│   When in doubt, delegate. Use inheritance only for                 │
│   genuine "is-a" relationships.                                     │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### When to Use Strategy Pattern

- Multiple algorithms exist for the same task
- You want to switch algorithms at runtime
- A class has multiple behaviors that vary independently
- You want to isolate algorithm implementation from client code

### When NOT to Use It

- Only one or two variants exist and won't grow
- The behavior never changes at runtime
- Simplicity is more important than flexibility

### Interview Wisdom

> **Almost every LLD interview will include the Strategy Pattern somewhere.** It's one of the most useful and commonly applied design patterns in real applications.

---

## 9. Final Diagram: Strategy Pattern in Action

```
┌──────────────────────────────────────────────────────────────────────┐
│                     STRATEGY PATTERN — IN ACTION                     │
├──────────────────────────────────────────────────────────────────────┤
│                                                                       │
│   CLIENT (main)                                                      │
│      │                                                                │
│      ├─── Creates Robot with strategies                              │
│      │                                                                │
│      ▼                                                                │
│   ┌────────────────────────────────────────────────┐                 │
│   │              Robot (Client Class)              │                 │
│   ├────────────────────────────────────────────────┤                 │
│   │ - walkBehavior: Walkable*                      │                 │
│   │ - talkBehavior: Talkable*                      │                 │
│   │ - flyBehavior:  Flyable*                       │                 │
│   │ - projection:   Projectable*                   │                 │
│   ├────────────────────────────────────────────────┤                 │
│   │ + walk()      { walkBehavior->walk(); }        │                 │
│   │ + talk()      { talkBehavior->talk(); }        │                 │
│   │ + fly()       { flyBehavior->fly();   }        │                 │
│   │ + projection(){ projection->projection();}     │                 │
│   └──────────┬──────────┬───────────┬──────────────┘                 │
│              │          │           │                                 │
│      ┌───────┘          │           └───────┐                        │
│      │                  │                   │                        │
│      ▼                  ▼                   ▼                        │
│  ┌─────────┐       ┌─────────┐         ┌─────────┐                  │
│  │Walkable │       │Talkable │         │Flyable  │  ...             │
│  │<<iface>>│       │<<iface>>│         │<<iface>>│                  │
│  └────┬────┘       └────┬────┘         └────┬────┘                  │
│       △                 △                   △                        │
│       │                 │                   │                        │
│   ┌───┴───┐         ┌───┴───┐           ┌───┴───┐                    │
│   │Normal │ /NoWalk │Normal │ /NoTalk   │Normal │ /NoFly             │
│   │Walk   │         │Talk   │           │Fly    │                    │
│   └───────┘         └───────┘           └───────┘                    │
│                                                                       │
│   KEY INSIGHT:                                                       │
│   Each strategy is INDEPENDENT. Change one without affecting others.│
│                                                                       │
└──────────────────────────────────────────────────────────────────────┘
```

---

## 10. Conclusion

The **Strategy Design Pattern** is one of the most widely used design patterns. It:

1. **Solves the "inheritance explosion" problem** by replacing inheritance with composition
2. **Enables runtime flexibility** — swap behaviors on the fly
3. **Respects SOLID principles** — especially SRP and OCP
4. **Follows DRY** — no code duplication
5. **Is used everywhere** in real applications (payment systems, sorting, notifications, etc.)

**Golden Rule:** 
> **"Favor composition over inheritance."**

Whenever you find yourself duplicating behavior across sibling classes, reach for the Strategy Pattern.

---

## 09. Factory Design Pattern | Simple, Factory Method & Abstract Factory with Real-Life Examples (32:09)

This lecture covers the **Factory Design Pattern** — one of the most widely used patterns in LLD. It explains why we need object creation to be separated from business logic and covers three variants: **Simple Factory**, **Factory Method**, and **Abstract Factory**.

---

## 1. Introduction — Why Factory Pattern?

### The Core Problem

In **Strategy Pattern**, we assumed objects were already created somewhere else. But in real code, **someone must actually create those objects** using `new`.

```
┌─────────────────────────────────────────────────────────────────────┐
│                    THE CORE PROBLEM                                 │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   Applications have TWO types of logic:                             │
│                                                                      │
│   ┌────────────────────────┐    ┌────────────────────────┐         │
│   │    BUSINESS LOGIC      │    │   OBJECT CREATION      │         │
│   │                        │    │      LOGIC             │         │
│   ├────────────────────────┤    ├────────────────────────┤         │
│   │ • What the app does    │    │ • How objects are made │         │
│   │ • Notification routing │    │ • `new` keyword usage  │         │
│   │ • Payment processing   │    │ • Which class to pick  │         │
│   └────────────────────────┘    └────────────────────────┘         │
│                                                                      │
│   PROBLEM: Mixing these two makes code:                             │
│   • Complex to read                                                 │
│   • Hard to understand                                              │
│   • Tightly coupled                                                 │
│                                                                      │
│   SOLUTION: Factory Design Pattern                                  │
│   → Separate object creation from business logic                    │
│   → Client just asks: "Give me an object"                           │
│   → Factory handles: "Which one, how, and where"                    │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Real-World Analogy

Just like a **real-world factory** produces products (cars, phones, toys), a **software factory class** produces objects.

### The Three Types of Factory

```
┌─────────────────────────────────────────────────────────────────────┐
│                     THREE FACTORY TYPES                             │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   1. SIMPLE FACTORY          (not a true pattern — a principle)    │
│      → One factory class, one method                                │
│      → Decides which concrete class to instantiate                  │
│                                                                      │
│   2. FACTORY METHOD          (official design pattern)              │
│      → Factory itself is abstract                                   │
│      → Subclasses decide which class to instantiate                 │
│                                                                      │
│   3. ABSTRACT FACTORY        (official design pattern)              │
│      → Factory creates FAMILIES of related objects                  │
│      → Multiple products from a single factory                      │
│                                                                      │
│   Note: Simple Factory extends → Factory Method extends →           │
│         Abstract Factory. Each is an extension of the previous.     │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 2. Simple Factory

### Problem: Burger Shop

We need a burger shop that can create different types of burgers based on user choice.

```
                    ┌──────────────────────────┐
                    │   <<abstract>>           │
                    │   Burger                 │
                    ├──────────────────────────┤
                    │ + prepare(): void = 0    │
                    └────────────┬─────────────┘
                                 △
              ┌──────────────────┼──────────────────┐
              │                  │                  │
     ┌────────┴───────┐ ┌────────┴────────┐ ┌──────┴─────────┐
     │ BasicBurger    │ │ StandardBurger  │ │ PremiumBurger  │
     ├────────────────┤ ├─────────────────┤ ├────────────────┤
     │ + prepare()    │ │ + prepare()     │ │ + prepare()    │
     └────────────────┘ └─────────────────┘ └────────────────┘
```

### UML for Simple Factory

```
                ┌──────────────────────┐
                │   BurgerFactory      │
                ├──────────────────────┤
                │ + createBurger(type) │───────┐
                │   : Burger           │       │ (has-a)
                └──────────────────────┘       │
                                               ▼
                                    ┌──────────────────────┐
                                    │   <<abstract>>       │
                                    │      Burger          │
                                    ├──────────────────────┤
                                    │ + prepare() = 0      │
                                    └──────────┬───────────┘
                                               △
                            ┌──────────────────┼──────────────────┐
                            │                  │                  │
                    ┌───────┴────────┐ ┌───────┴────────┐ ┌───────┴────────┐
                    │ BasicBurger    │ │StandardBurger  │ │PremiumBurger   │
                    └────────────────┘ └────────────────┘ └────────────────┘
```

### Code for Simple Factory

```cpp
#include <iostream>
#include <string>
using namespace std;

// Abstract Product
class Burger {
public:
    virtual void prepare() = 0;
    virtual ~Burger() {}
};

// Concrete Products
class BasicBurger : public Burger {
public:
    void prepare() override {
        cout << "Preparing Basic Burger with bun, patty, and ketchup!\n";
    }
};

class StandardBurger : public Burger {
public:
    void prepare() override {
        cout << "Preparing Standard Burger with bun, patty, cheese, and lettuce!\n";
    }
};

class PremiumBurger : public Burger {
public:
    void prepare() override {
        cout << "Preparing Premium Burger with gourmet bun, premium patty, cheese, lettuce, and secret sauce!\n";
    }
};

// THE FACTORY — Decides which concrete class to instantiate
class BurgerFactory {
public:
    Burger* createBurger(string type) {
        if (type == "basic") {
            return new BasicBurger();
        } else if (type == "standard") {
            return new StandardBurger();
        } else if (type == "premium") {
            return new PremiumBurger();
        } else {
            cout << "Invalid burger type!\n";
            return nullptr;
        }
    }
};

// Client
int main() {
    string type = "standard";
    BurgerFactory* myBurgerFactory = new BurgerFactory();
    
    Burger* burger = myBurgerFactory->createBurger(type);
    burger->prepare();  // Output: Preparing Standard Burger with bun, patty, cheese, and lettuce!
    
    delete burger;
    delete myBurgerFactory;
}
```

### Definition

> **"A factory class that decides which concrete class to instantiate."**

### Key Characteristics

| Aspect | Description |
|--------|-------------|
| **Structure** | One factory class, one create method |
| **Decision** | Based on a parameter (string/enum) |
| **Output** | Concrete product returned as abstract type |
| **Pattern?** | Not officially a pattern — more of a principle |

---

## 3. Factory Method

### Problem: Multiple Burger Franchises

Now we have **two franchises**: 
- **SinghBurger** — makes normal burgers (basic, standard, premium)
- **KingBurger** — makes veg-based burgers (basic veg, standard veg, premium veg)

The **factory itself** needs to be abstract!

### UML for Factory Method

```
                ┌────────────────────────────┐
                │    <<abstract>>            │
                │    BurgerFactory           │
                ├────────────────────────────┤
                │ + createBurger(): Burger=0 │
                └─────────────┬──────────────┘
                              △
              ┌───────────────┴───────────────┐
              │                               │
     ┌────────┴────────┐              ┌───────┴─────────┐
     │  SinghBurger    │              │   KingBurger    │
     ├─────────────────┤              ├─────────────────┤
     │ + createBurger()│              │ + createBurger()│
     └────────┬────────┘              └────────┬────────┘
              │                                │
              │ creates                        │ creates
              ▼                                ▼
    ┌─────────────────────┐         ┌─────────────────────────┐
    │ BasicBurger         │         │ BasicVegBurger          │
    │ StandardBurger      │         │ StandardVegBurger       │
    │ PremiumBurger       │         │ PremiumVegBurger        │
    └─────────────────────┘         └─────────────────────────┘
              △                                △
              └───────────────┬────────────────┘
                              │
                    ┌─────────┴──────────┐
                    │   <<abstract>>     │
                    │      Burger        │
                    ├────────────────────┤
                    │ + prepare() = 0    │
                    └────────────────────┘
```

### Code for Factory Method

```cpp
#include <iostream>
#include <string>
using namespace std;

// ============ PRODUCT HIERARCHY ============
class Burger {
public:
    virtual void prepare() = 0;
    virtual ~Burger() {}
};

// Normal Burgers
class BasicBurger : public Burger {
public:
    void prepare() override {
        cout << "Preparing Basic Burger with bun, patty, and ketchup!\n";
    }
};

class StandardBurger : public Burger {
public:
    void prepare() override {
        cout << "Preparing Standard Burger with bun, patty, cheese, and lettuce!\n";
    }
};

class PremiumBurger : public Burger {
public:
    void prepare() override {
        cout << "Preparing Premium Burger with gourmet bun, premium patty, cheese, lettuce, and secret sauce!\n";
    }
};

// Veg Burgers
class BasicVegBurger : public Burger {
public:
    void prepare() override {
        cout << "Preparing Basic Veg Burger with bun, veg patty, and ketchup!\n";
    }
};

class StandardVegBurger : public Burger {
public:
    void prepare() override {
        cout << "Preparing Standard Veg Burger with bun, veg patty, cheese, and lettuce!\n";
    }
};

class PremiumVegBurger : public Burger {
public:
    void prepare() override {
        cout << "Preparing Premium Veg Burger with gourmet bun, premium veg patty, cheese, lettuce, and secret sauce!\n";
    }
};

// ============ FACTORY HIERARCHY ============
class BurgerFactory {
public:
    virtual Burger* createBurger(string type) = 0;
    virtual ~BurgerFactory() {}
};

// Concrete Factory 1: Singh Burger (normal burgers)
class SinghBurger : public BurgerFactory {
public:
    Burger* createBurger(string type) override {
        if (type == "basic")   return new BasicBurger();
        if (type == "standard") return new StandardBurger();
        if (type == "premium")  return new PremiumBurger();
        cout << "Invalid burger type!\n";
        return nullptr;
    }
};

// Concrete Factory 2: King Burger (veg burgers)
class KingBurger : public BurgerFactory {
public:
    Burger* createBurger(string type) override {
        if (type == "basic")   return new BasicVegBurger();
        if (type == "standard") return new StandardVegBurger();
        if (type == "premium")  return new PremiumVegBurger();
        cout << "Invalid burger type!\n";
        return nullptr;
    }
};

// ============ CLIENT ============
int main() {
    string type = "basic";
    
    // Create a King Burger factory (veg burgers)
    BurgerFactory* myBurgerFactory = new KingBurger();
    
    Burger* burger = myBurgerFactory->createBurger(type);
    burger->prepare();
    // Output: Preparing Basic Veg Burger with bun, veg patty, and ketchup!
    
    delete burger;
    delete myBurgerFactory;
}
```

### Definition

> **"Define an interface for creating an object, but allow subclasses to decide which class to instantiate."**

### Key Characteristics

| Aspect | Description |
|--------|-------------|
| **Structure** | Abstract factory + concrete factories |
| **Decision** | Made by concrete factory subclasses |
| **Output** | Concrete product from concrete factory |
| **Extension** | New factory = new subclass (no modification) |

---

## 4. Abstract Factory Method

### Problem: Factories Producing Multiple Product Families

Now **SinghBurger** and **KingBurger** both produce **two products**: Burgers AND Garlic Bread.

### UML for Abstract Factory

```
                       ┌───────────────────────────────┐
                       │      <<abstract>>             │
                       │       MealFactory             │
                       ├───────────────────────────────┤
                       │ + createBurger(): Burger = 0  │
                       │ + createGarlicBread(): GB = 0 │
                       └──────────────┬────────────────┘
                                      △
                       ┌──────────────┴───────────────┐
                       │                              │
              ┌────────┴────────┐             ┌───────┴─────────┐
              │   SinghBurger   │             │   KingBurger    │
              ├─────────────────┤             ├─────────────────┤
              │ + createBurger()│             │ + createBurger()│
              │ + createGarlic()│             │ + createGarlic()│
              └────────┬────────┘             └────────┬────────┘
                       │                               │
         ┌─────────────┴─────────────┐   ┌─────────────┴─────────────┐
         │  Normal products          │   │  Veg products             │
         │                           │   │                           │
         │  BasicBurger              │   │  BasicVegBurger           │
         │  StandardBurger           │   │  StandardVegBurger        │
         │  PremiumBurger            │   │  PremiumVegBurger         │
         │                           │   │                           │
         │  BasicGarlicBread         │   │  BasicVegGarlicBread      │
         │  CheeseGarlicBread        │   │  CheeseVegGarlicBread     │
         └───────────────────────────┘   └───────────────────────────┘
```

### Code for Abstract Factory

```cpp
#include <iostream>
#include <string>
using namespace std;

// ============ PRODUCT FAMILY 1: BURGERS ============
class Burger {
public:
    virtual void prepare() = 0;
    virtual ~Burger() {}
};

class BasicBurger : public Burger {
public:
    void prepare() override { cout << "Preparing Basic Burger\n"; }
};

class StandardBurger : public Burger {
public:
    void prepare() override { cout << "Preparing Standard Burger\n"; }
};

class PremiumBurger : public Burger {
public:
    void prepare() override { cout << "Preparing Premium Burger\n"; }
};

class BasicVegBurger : public Burger {
public:
    void prepare() override { cout << "Preparing Basic Veg Burger\n"; }
};

class StandardVegBurger : public Burger {
public:
    void prepare() override { cout << "Preparing Standard Veg Burger\n"; }
};

class PremiumVegBurger : public Burger {
public:
    void prepare() override { cout << "Preparing Premium Veg Burger\n"; }
};

// ============ PRODUCT FAMILY 2: GARLIC BREAD ============
class GarlicBread {
public:
    virtual void prepare() = 0;
    virtual ~GarlicBread() {}
};

class BasicGarlicBread : public GarlicBread {
public:
    void prepare() override { cout << "Preparing Basic Garlic Bread\n"; }
};

class CheeseGarlicBread : public GarlicBread {
public:
    void prepare() override { cout << "Preparing Cheese Garlic Bread\n"; }
};

class BasicVegGarlicBread : public GarlicBread {
public:
    void prepare() override { cout << "Preparing Basic Veg Garlic Bread\n"; }
};

class CheeseVegGarlicBread : public GarlicBread {
public:
    void prepare() override { cout << "Preparing Cheese Veg Garlic Bread\n"; }
};

// ============ ABSTRACT FACTORY ============
class MealFactory {
public:
    virtual Burger* createBurger(string type) = 0;
    virtual GarlicBread* createGarlicBread(string type) = 0;
    virtual ~MealFactory() {}
};

// Concrete Factory 1: Singh Burger (normal items)
class SinghBurger : public MealFactory {
public:
    Burger* createBurger(string type) override {
        if (type == "basic")    return new BasicBurger();
        if (type == "standard") return new StandardBurger();
        if (type == "premium")  return new PremiumBurger();
        return nullptr;
    }
    
    GarlicBread* createGarlicBread(string type) override {
        if (type == "basic")   return new BasicGarlicBread();
        if (type == "cheese")  return new CheeseGarlicBread();
        return nullptr;
    }
};

// Concrete Factory 2: King Burger (veg items)
class KingBurger : public MealFactory {
public:
    Burger* createBurger(string type) override {
        if (type == "basic")    return new BasicVegBurger();
        if (type == "standard") return new StandardVegBurger();
        if (type == "premium")  return new PremiumVegBurger();
        return nullptr;
    }
    
    GarlicBread* createGarlicBread(string type) override {
        if (type == "basic")   return new BasicVegGarlicBread();
        if (type == "cheese")  return new CheeseVegGarlicBread();
        return nullptr;
    }
};

// ============ CLIENT ============
int main() {
    string burgerType = "basic";
    string garlicBreadType = "cheese";
    
    // Choose a factory
    MealFactory* mealFactory = new KingBurger();
    
    Burger* burger = mealFactory->createBurger(burgerType);
    GarlicBread* garlicBread = mealFactory->createGarlicBread(garlicBreadType);
    
    burger->prepare();       // Output: Preparing Basic Veg Burger
    garlicBread->prepare();  // Output: Preparing Cheese Veg Garlic Bread
    
    delete burger;
    delete garlicBread;
    delete mealFactory;
}
```

### Definition

> **"Provide an interface for creating families of related objects without specifying their concrete classes."**

### Key Characteristics

| Aspect | Description |
|--------|-------------|
| **Structure** | Abstract factory with multiple product methods |
| **Decision** | Factory family + product type |
| **Output** | Multiple related products from one factory |
| **Family Concept** | Products are designed to work together |

---

## 5. Comparison Table

| Feature | Simple Factory | Factory Method | Abstract Factory |
|---------|----------------|----------------|------------------|
| **Official Pattern?** | ❌ (Principle) | ✅ (Pattern) | ✅ (Pattern) |
| **Factory** | Concrete class | Abstract + concrete | Abstract + concrete |
| **Product Count** | One family | One family | Multiple families |
| **Decision By** | Parameter | Subclass | Subclass + parameter |
| **Complexity** | Low | Medium | High |
| **Example** | `BurgerFactory` | `SinghBurger` / `KingBurger` | `MealFactory` (Burger + GarlicBread) |
| **Extensibility** | Modify factory | Add new factory subclass | Add new factory family |

---

## 6. UML Diagrams Summary

### Simple Factory

```
                  ┌─────────────────────┐
                  │   Client            │
                  └──────────┬──────────┘
                             │ uses
                             ▼
                  ┌─────────────────────┐
                  │   ProductFactory    │
                  ├─────────────────────┤
                  │ + create(type)      │
                  └──────────┬──────────┘
                             │ creates
                             ▼
                  ┌─────────────────────┐
                  │   <<abstract>>      │
                  │     Product         │
                  ├─────────────────────┤
                  │ + operation() = 0   │
                  └──────────┬──────────┘
                             △
              ┌──────────────┼──────────────┐
              │              │              │
         Concrete1      Concrete2      Concrete3
```

### Factory Method

```
                  ┌─────────────────────┐
                  │   Client            │
                  └──────────┬──────────┘
                             │ uses
                             ▼
                  ┌─────────────────────┐
                  │  <<abstract>>       │
                  │  ProductFactory     │
                  ├─────────────────────┤
                  │ + create(): Product │
                  └──────────┬──────────┘
                             △
              ┌──────────────┴──────────────┐
              │                             │
        ConcreteFactory1              ConcreteFactory2
              │                             │
              │ creates                     │ creates
              ▼                             ▼
         Product1                      Product2
              △                             △
              └──────────────┬──────────────┘
                             │
                    ┌────────┴────────┐
                    │  <<abstract>>   │
                    │    Product      │
                    └─────────────────┘
```

### Abstract Factory

```
                  ┌─────────────────────┐
                  │   Client            │
                  └──────────┬──────────┘
                             │ uses
                             ▼
                  ┌─────────────────────┐
                  │  <<abstract>>       │
                  │  AbstractFactory    │
                  ├─────────────────────┤
                  │ + createProductA()  │
                  │ + createProductB()  │
                  └──────────┬──────────┘
                             △
              ┌──────────────┴──────────────┐
              │                             │
        Factory1 (Family1)            Factory2 (Family2)
              │                             │
              │ creates both                │ creates both
              ▼                             ▼
    ┌───────────────────┐         ┌───────────────────┐
    │ ProductA1         │         │ ProductA2         │
    │ ProductB1         │         │ ProductB2         │
    └───────────────────┘         └───────────────────┘
              △                             △
              │                             │
    ┌─────────┴─────────┐         ┌─────────┴─────────┐
    │ ProductA abstract │         │ ProductB abstract │
    └───────────────────┘         └───────────────────┘
```

---

## 7. Real-World Applications

### Application 1: Notification System

```
                  ┌─────────────────────┐
                  │   NotificationFactory│
                  ├─────────────────────┤
                  │ + create(type)      │
                  └──────────┬──────────┘
                             │
              ┌──────────────┼──────────────┐
              │              │              │
              ▼              ▼              ▼
         EmailNotif     PushNotif       SMSNotif
```

```cpp
// Abstract product
class Notification {
public:
    virtual void notify() = 0;
    virtual ~Notification() {}
};

class EmailNotification : public Notification {
public:
    void notify() override { cout << "Sending Email Notification\n"; }
};

class PushNotification : public Notification {
public:
    void notify() override { cout << "Sending Push Notification\n"; }
};

class SMSNotification : public Notification {
public:
    void notify() override { cout << "Sending SMS Notification\n"; }
};

// Simple Factory
class NotificationFactory {
public:
    Notification* createNotification(string type) {
        if (type == "email") return new EmailNotification();
        if (type == "push")  return new PushNotification();
        if (type == "sms")   return new SMSNotification();
        return nullptr;
    }
};
```

### Application 2: Database Connections

```
                  ┌─────────────────────┐
                  │ DatabaseFactory     │
                  ├─────────────────────┤
                  │ + create(type)      │
                  └──────────┬──────────┘
                             │
              ┌──────────────┼──────────────┐
              │              │              │
              ▼              ▼              ▼
         MySQL           MongoDB        PostgreSQL
```

---

## 8. Factory vs Strategy — When to Use Which?

```
┌─────────────────────────────────────────────────────────────────────┐
│              FACTORY vs STRATEGY — DECISION GUIDE                   │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   Ask yourself: "What is my intent?"                                │
│                                                                      │
│   ┌────────────────────────────────────────────────────────────┐   │
│   │  "I want to VARY the algorithm at runtime"                 │   │
│   │  → Use STRATEGY PATTERN                                     │   │
│   │  → Assumes objects already exist                            │   │
│   │  → Example: Switch payment methods at checkout             │   │
│   └────────────────────────────────────────────────────────────┘   │
│                                                                      │
│   ┌────────────────────────────────────────────────────────────┐   │
│   │  "I want to SEPARATE object creation from business logic"  │   │
│   │  → Use FACTORY PATTERN                                      │   │
│   │  → Creates objects, then hands them over                    │   │
│   │  → Example: Create a notification based on user preference │   │
│   └────────────────────────────────────────────────────────────┘   │
│                                                                      │
│   NOTE: Both can be used TOGETHER!                                  │
│   Factory creates the strategy objects, then client uses Strategy   │
│   pattern to swap them at runtime.                                  │
│                                                                      │
│   KEY INSIGHT: There's no single right answer. It depends on        │
│   your intent, application context, and design goals.               │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 9. Key Takeaways

### The Golden Rule

> **Factory Pattern separates object creation logic from business logic, making the client code cleaner and more decoupled.**

### Benefits

```
┌─────────────────────────────────────────────────────────────────────┐
│                    BENEFITS OF FACTORY PATTERN                      │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   ✅ DECOUPLING                                                     │
│      Client doesn't know HOW objects are created                    │
│                                                                      │
│   ✅ SINGLE RESPONSIBILITY                                          │
│      Factory handles creation; client handles business logic        │
│                                                                      │
│   ✅ OPEN/CLOSED PRINCIPLE                                          │
│      Add new product types without modifying client                 │
│                                                                      │
│   ✅ ENCAPSULATION                                                  │
│      Complex creation logic is hidden inside factory                │
│                                                                      │
│   ✅ MAINTAINABILITY                                                │
│      Changes to creation logic stay in one place                    │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### When to Use Factory Pattern

| Situation | Use |
|-----------|-----|
| Object creation logic is complex | ✅ Factory |
| Multiple related products exist | ✅ Abstract Factory |
| Need to decide type at runtime | ✅ Factory Method |
| Simple object creation | ❌ Just use `new` |
| Only one type of product | ❌ Overkill |

### Interview Wisdom

> **Almost every LLD interview will require the Factory Pattern.** It's ubiquitous in real-world applications — whenever you need flexible, decoupled object creation, the Factory Pattern is the answer.

### The Three Patterns in One Line

```
┌─────────────────────────────────────────────────────────────────────┐
│                                                                      │
│   Simple Factory:    One class creates objects based on parameter   │
│   Factory Method:    Subclasses decide which class to create        │
│   Abstract Factory:  Families of related objects from one factory   │
│                                                                      │
│   "Simple Factory → Factory Method → Abstract Factory"              │
│   Each extends the previous one for more flexibility.               │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 10. Complete Summary Diagram

```
┌───────────────────────────────────────────────────────────────────────┐
│                     FACTORY PATTERN OVERVIEW                           │
├───────────────────────────────────────────────────────────────────────┤
│                                                                        │
│   CLIENT (Business Logic)                                              │
│      │                                                                 │
│      │ "Give me an object"                                             │
│      ▼                                                                 │
│   ┌──────────────────────┐    ┌──────────────────────────┐            │
│   │    FACTORY           │───▶│   PRODUCT HIERARCHY      │            │
│   │  (Object Creation)   │    │   (Abstract Product)     │            │
│   └──────────────────────┘    └──────────────────────────┘            │
│              │                            △                            │
│              │ decides                    │ extends                    │
│              │ which                       │                            │
│              ▼                            │                            │
│   ┌──────────────────────┐    ┌───────────┴──────────────┐            │
│   │  CONCRETE FACTORY    │    │  CONCRETE PRODUCTS       │            │
│   │  (Specific types)    │    │  (Basic, Standard, ...)  │            │
│   └──────────────────────┘    └──────────────────────────┘            │
│                                                                        │
│   Three variations:                                                    │
│   1. Simple Factory    — Concrete factory class                       │
│   2. Factory Method    — Abstract factory + concrete factories        │
│   3. Abstract Factory  — Creates families of related products         │
│                                                                        │
│   Key benefit: Decouples client from object creation logic.           │
│                                                                        │
└───────────────────────────────────────────────────────────────────────┘
```

---

## 11. Code Files Summary

The lecture produces three complete code files:

1. **Simple Factory** — `BurgerFactory` with one method handling 3 types
2. **Factory Method** — `BurgerFactory` abstract + `SinghBurger` + `KingBurger` concrete factories
3. **Abstract Factory** — `MealFactory` abstract + concrete factories creating Burger AND GarlicBread

All examples follow the pattern:
- **Abstract Product** (interface/abstract class)
- **Concrete Products** (specific implementations)
- **Abstract Factory** (interface for creation)
- **Concrete Factories** (specific creators)
- **Client** (uses factories to get products)

---

## 10. Singleton Design Pattern | Thread-Safe, Lazy & Eager Initialization + Real Use Cases (32:34)

summaries system design tutorial transcript in details along with useful code examples and diagrams