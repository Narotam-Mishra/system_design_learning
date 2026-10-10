
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

## 11. Build Zomato Food Delivery App (1:07:19)

This lecture is a **complete LLD interview walkthrough** for designing a Swiggy/Zomato clone called **Tomato**. It covers requirements gathering, UML design, design patterns, code implementation, and further extensions.

---

## 1. The LLD Interview Approach

```
┌─────────────────────────────────────────────────────────────────────┐
│                  LLD INTERVIEW FLOW                                 │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   1. PROBLEM STATEMENT                                              │
│      "Design a food delivery app"                                   │
│                                                                      │
│   2. REQUIREMENTS GATHERING                                         │
│      • Ask counter-questions                                        │
│      • Narrow down scope                                            │
│      • Functional + Non-functional requirements                     │
│                                                                      │
│   3. HAPPY FLOW DISCUSSION                                          │
│      • Walk through the main user journey                          │
│      • Confirm both on same page                                    │
│                                                                      │
│   4. UML DIAGRAM                                                    │
│      • Identify classes/objects                                     │
│      • Define relationships                                         │
│      • Discuss with interviewer                                     │
│                                                                      │
│   5. CODE IMPLEMENTATION                                            │
│      • Working code (structure at minimum)                          │
│      • Clean OOP + SOLID + Design Patterns                          │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

> **Key Insight:** The interviewer is not your enemy — they're your **friend/manager** helping you build the application. Always discuss, ask questions, and iterate.

---

## 2. Requirements Gathering

### Functional Requirements (Happy Flow)

```
┌─────────────────────────────────────────────────────────────────────┐
│                    HAPPY FLOW                                       │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   User opens app                                                    │
│      │                                                              │
│      ▼                                                              │
│   Search restaurants by location                                    │
│      │                                                              │
│      ▼                                                              │
│   Select a restaurant → View menu items                             │
│      │                                                              │
│      ▼                                                              │
│   Add items to cart                                                 │
│      │                                                              │
│      ▼                                                              │
│   Checkout → Choose order type (Delivery / Pickup)                  │
│      │                                                              │
│      ▼                                                              │
│   Make payment (UPI / Card / NetBanking)                            │
│      │                                                              │
│      ▼                                                              │
│   Receive notification (order confirmed)                            │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Counter-Questions Asked

| Question | Answer | Decision |
|----------|--------|----------|
| Do we build payment service? | No, it's 3rd party | Just integrate |
| User-centric or delivery-agent-centric? | User-centric | Focus on user flow |
| Notification service? | Assume it exists | Just call it |
| Order types? | Delivery + Pickup | Two order types |

### Functional vs Non-Functional

| Type | Description | Example |
|------|-------------|---------|
| **Functional** | Product/business logic | Entities, interactions, search, cart, order |
| **Non-Functional** | Quality attributes | Scalability, multithreading, performance |

---

## 3. Design Approach: Bottom-Up vs Top-Down

```
┌─────────────────────────────────────────────────────────────────────┐
│              BOTTOM-UP vs TOP-DOWN                                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   BOTTOM-UP (Used here)                                            │
│   ─────────────────────                                            │
│   1. Build SMALLER objects first                                    │
│   2. Establish relationships between them                           │
│   3. Build LARGER objects that contain them                         │
│   4. Move up the hierarchy                                          │
│                                                                      │
│   TOP-DOWN                                                          │
│   ────────                                                          │
│   1. Build LARGER objects first                                     │
│   2. Then build SMALLER objects inside                              │
│   3. Connect dependencies                                           │
│                                                                      │
│   ✅ Bottom-Up is preferred in most LLD interviews                  │
│      because once smaller objects exist, it's easier to             │
│      connect them to larger objects.                                │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 4. UML Class Diagram (Bottom-Up Construction)

### Step 1: MenuItem (Smallest Object)

```
┌──────────────────────────────────┐
│          <<model>>               │
│          MenuItem                │
├──────────────────────────────────┤
│ - code: String                   │
│ - name: String                   │
│ - price: double                  │
├──────────────────────────────────┤
│ + getters() / setters()          │
└──────────────────────────────────┘
```

### Step 2: Restaurant (Contains MenuItem)

```
┌──────────────────────────────────┐
│          <<model>>               │
│          Restaurant              │
├──────────────────────────────────┤
│ - id: int (auto-increment)       │
│ - name: String                   │
│ - address: String                │
│ - menuItems: List<MenuItem>      │
├──────────────────────────────────┤
│ + getters() / setters()          │
└──────────────────────────────────┘
       ◆ (composition) 1 ──── * MenuItem
```

> **Relationship:** Restaurant HAS-A MenuItem (Composition) — MenuItem cannot exist without Restaurant.

### Step 3: RestaurantManager (Singleton)

```
┌──────────────────────────────────┐
│       RestaurantManager          │
│         <<singleton>>            │
├──────────────────────────────────┤
│ - instance: RestaurantManager    │
│ - restaurants: List<Restaurant>  │
├──────────────────────────────────┤
│ - RestaurantManager() (private)  │
│ + getInstance(): RestaurantManager│
│ + addRestaurant(Restaurant)      │
│ + searchByLocation(String)       │
│   : List<Restaurant>             │
└──────────────────────────────────┘
       ◇ (aggregation) 1 ──── * Restaurant
```

> **Why Singleton?** Single source of truth for all restaurants. Multiple instances would mean inconsistent lists.

### Step 4: User + Cart + Order (Chain)

```
┌─────────────────────┐       ┌─────────────────────┐
│       User          │       │       Cart          │
│   <<model>>         │       │   <<model>>         │
├─────────────────────┤       ├─────────────────────┤
│ - id: int           │       │ - restaurant: Rest. │
│ - name: String      │       │ - items: List<MI>   │
│ - address: String   │       │ - total: double     │
│ - cart: Cart        │       ├─────────────────────┤
├─────────────────────┤       │ + addItem(MenuItem) │
│ + getters/setters   │       │ + getTotalCost()    │
└─────────────────────┘       │ + isEmpty()         │
      ◆ 1 ──── 1 Cart         │ + clear()           │
                              └─────────────────────┘
                                    ▲
                                    │ has-a (1)
                                    │
                              ┌─────┴─────┐
                              │Restaurant │
                              └───────────┘
```

> **Relationships:**
> - User ◇◆ Cart (Composition, 1:1)
> - Cart ◇── Restaurant (Simple Association, 1:1)
> - Cart ◇── MenuItem (Aggregation, 1:*)

### Step 5: Order Hierarchy

```
                 ┌────────────────────────┐
                 │   <<abstract>>         │
                 │       Order            │
                 ├────────────────────────┤
                 │ - id: int              │
                 │ - user: User           │
                 │ - restaurant: Rest.    │
                 │ - items: List<MI>      │
                 │ - paymentStrategy      │
                 │ - total: double        │
                 │ - scheduled: String    │
                 ├────────────────────────┤
                 │ + processPayment()     │
                 │ + getType(): String = 0│
                 └───────────┬────────────┘
                             △
              ┌──────────────┴──────────────┐
              │                              │
     ┌────────┴────────┐             ┌───────┴─────────┐
     │  DeliveryOrder  │             │   PickupOrder   │
     ├─────────────────┤             ├─────────────────┤
     │ - userAddress   │             │ - restAddress   │
     ├─────────────────┤             ├─────────────────┤
     │ + getType():    │             │ + getType():    │
     │   "DELIVERY"    │             │   "PICKUP"      │
     └─────────────────┘             └─────────────────┘
```

### Step 6: Order Factory (Factory Method Pattern)

```
              ┌────────────────────────────┐
              │   <<interface>>            │
              │     OrderFactory           │
              ├────────────────────────────┤
              │ + createOrder(...): Order  │
              └─────────────┬──────────────┘
                            △
              ┌─────────────┴───────────────┐
              │                              │
     ┌────────┴────────┐             ┌───────┴──────────┐
     │   NowOrder      │             │  ScheduledOrder  │
     │   Factory       │             │    Factory       │
     ├─────────────────┤             ├──────────────────┤
     │ + createOrder() │             │ + createOrder()  │
     └─────────────────┘             └──────────────────┘
              │                              │
              │ creates                      │ creates
              ▼                              ▼
         DeliveryOrder                  DeliveryOrder
         PickupOrder                    PickupOrder
```

### Step 7: Payment Strategy (Strategy Pattern)

```
              ┌────────────────────────────┐
              │   <<abstract>>             │
              │    PaymentStrategy         │
              ├────────────────────────────┤
              │ + pay(double): void = 0    │
              └─────────────┬──────────────┘
                            △
              ┌─────────────┼─────────────┐
              │             │             │
     ┌────────┴────┐ ┌──────┴─────┐ ┌─────┴──────┐
     │ UPIPayment  │ │CreditCard  │ │NetBanking  │
     │             │ │ Payment    │ │ Payment    │
     └─────────────┘ └────────────┘ └────────────┘
```

### Step 8: Order Manager (Singleton)

```
┌──────────────────────────────────┐
│        OrderManager              │
│         <<singleton>>            │
├──────────────────────────────────┤
│ - instance: OrderManager         │
│ - orders: List<Order>            │
├──────────────────────────────────┤
│ - OrderManager() (private)       │
│ + getInstance(): OrderManager    │
│ + addOrder(Order)                │
│ + listOrders()                   │
└──────────────────────────────────┘
       ◇ (aggregation) 1 ──── * Order
```

### Step 9: Notification Service

```
┌──────────────────────────────────┐
│      NotificationService         │
├──────────────────────────────────┤
│ + notify(Order): void            │
└──────────────────────────────────┘
       ◇ (has-a) ──── Order
```

### Step 10: Tomato Orchestrator (Single Point of Contact)

```
┌──────────────────────────────────────────────────┐
│                TomatoApp                          │
│            (Orchestrator Class)                   │
├──────────────────────────────────────────────────┤
│ - restaurantManager: RestaurantManager            │
│ - orderManager: OrderManager                      │
├──────────────────────────────────────────────────┤
│ + TomatoApp()                                     │
│ + searchRestaurant(location): List<Restaurant>    │
│ + selectRestaurant(user, restaurant)              │
│ + addToCart(user, itemCode)                       │
│ + checkoutNow(user, paymentStrategy, orderType)   │
│ + checkoutScheduled(...)                          │
│ + payForOrder(user, order)                        │
│ + printUserCart(user)                             │
└──────────────────────────────────────────────────┘
```

> **Why TomatoApp?** Single point of contact for the client (front-end). It orchestrates all other objects.

---

## 5. Complete UML Diagram (Clean View)

```
┌──────────────────────────────────────────────────────────────────────┐
│                         TOMATO APP (Orchestrator)                    │
│                                                                       │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────────┐          │
│  │   User      │    │   Cart      │    │   Order         │          │
│  │             │───◆│             │    │  <<abstract>>   │          │
│  │             │    │             │    │                 │          │
│  └─────────────┘    └──────┬──────┘    └────────┬────────┘          │
│                            │                    △                    │
│                            │                    │                    │
│                     ┌──────┴──────┐    ┌────────┴────────┐          │
│                     │ Restaurant  │    │                 │          │
│                     │             │    │  DeliveryOrder  │          │
│                     └──────┬──────┘    │  PickupOrder    │          │
│                            │           └─────────────────┘          │
│                            │                                         │
│                     ┌──────┴──────┐    ┌─────────────────┐          │
│                     │  MenuItem   │    │ PaymentStrategy │          │
│                     │             │    │  <<abstract>>   │          │
│                     └─────────────┘    └────────┬────────┘          │
│                                                 △                    │
│                                    ┌────────────┼────────────┐       │
│                                    │            │            │       │
│                              ┌─────┴────┐ ┌─────┴─────┐ ┌────┴─────┐│
│                              │  UPI     │ │CreditCard │ │NetBanking││
│                              └──────────┘ └───────────┘ └──────────┘│
│                                                                       │
│  ┌──────────────────────┐   ┌──────────────────────┐                │
│  │ RestaurantManager    │   │  OrderManager         │                │
│  │ <<singleton>>        │   │  <<singleton>>        │                │
│  └──────────────────────┘   └──────────────────────┘                │
│                                                                       │
│  ┌──────────────────────┐   ┌──────────────────────┐                │
│  │ NotificationService  │   │   OrderFactory        │                │
│  │                      │   │  <<interface>>        │                │
│  └──────────────────────┘   └──────────────────────┘                │
│                                                                       │
└──────────────────────────────────────────────────────────────────────┘
```

---

## 6. Design Patterns Applied

| Pattern | Where Used | Why |
|---------|-----------|-----|
| **Singleton** | RestaurantManager, OrderManager | Single source of truth for managers |
| **Strategy** | PaymentStrategy | Interchangeable payment algorithms |
| **Factory Method** | OrderFactory → NowOrderFactory, ScheduledOrderFactory | Separate object creation logic |
| **Composition** | User-Cart, Restaurant-MenuItem | Strong ownership |
| **Aggregation** | RestaurantManager-Restaurant, OrderManager-Order | Container-object relationship |

---

## 7. Key Code Examples

### 7.1 Restaurant Model

```cpp
class Restaurant {
private:
    static int nextRestaurantId;
    int restaurantId;
    string name;
    string location;
    vector<MenuItem> menuItems;

public:
    Restaurant(string name, string location) 
        : name(name), location(location) {
        restaurantId = ++nextRestaurantId;
    }
    
    string getName() { return name; }
    string getLocation() { return location; }
    vector<MenuItem>& getMenuItems() { return menuItems; }
    
    void addMenuItem(MenuItem item) {
        menuItems.push_back(item);
    }
};

int Restaurant::nextRestaurantId = 0;
```

### 7.2 RestaurantManager (Singleton)

```cpp
class RestaurantManager {
private:
    static RestaurantManager* instance;
    vector<Restaurant*> restaurants;
    
    RestaurantManager() {}
    
public:
    RestaurantManager(const RestaurantManager&) = delete;
    RestaurantManager& operator=(const RestaurantManager&) = delete;
    
    static RestaurantManager* getInstance() {
        if (instance == nullptr) {
            instance = new RestaurantManager();
        }
        return instance;
    }
    
    void addRestaurant(Restaurant* r) {
        restaurants.push_back(r);
    }
    
    vector<Restaurant*> searchByLocation(string loc) {
        vector<Restaurant*> result;
        for (auto r : restaurants) {
            if (r->getLocation() == loc) {
                result.push_back(r);
            }
        }
        return result;
    }
};

RestaurantManager* RestaurantManager::instance = nullptr;
```

### 7.3 Cart

```cpp
class Cart {
private:
    Restaurant* restaurant;
    vector<MenuItem> items;

public:
    Cart() : restaurant(nullptr) {}
    
    void addItem(MenuItem item) {
        if (restaurant == nullptr) {
            cout << "Select a restaurant first!\n";
            return;
        }
        items.push_back(item);
    }
    
    void setRestaurant(Restaurant* r) { restaurant = r; }
    Restaurant* getRestaurant() { return restaurant; }
    vector<MenuItem>& getItems() { return items; }
    
    bool isEmpty() { return items.empty(); }
    
    double getTotalCost() {
        double total = 0;
        for (auto& item : items) total += item.getPrice();
        return total;
    }
    
    void clear() {
        items.clear();
        restaurant = nullptr;
    }
};
```

### 7.4 Order (Abstract)

```cpp
class Order {
protected:
    static int nextOrderId;
    int orderId;
    User* user;
    Restaurant* restaurant;
    vector<MenuItem> items;
    PaymentStrategy* paymentStrategy;
    double total;
    string scheduled;

public:
    Order(User* u, Restaurant* r, vector<MenuItem> items,
          PaymentStrategy* ps, double total, string scheduled)
        : user(u), restaurant(r), items(items),
          paymentStrategy(ps), total(total), scheduled(scheduled) {
        orderId = ++nextOrderId;
    }
    
    virtual ~Order() {}
    
    bool processPayment() {
        if (paymentStrategy) {
            paymentStrategy->pay(total);
            return true;
        }
        cout << "Please choose payment mode first!\n";
        return false;
    }
    
    virtual string getType() = 0;  // Pure virtual
    
    // getters/setters...
};

int Order::nextOrderId = 0;

// Concrete Orders
class DeliveryOrder : public Order {
    string userAddress;
public:
    DeliveryOrder(User* u, Restaurant* r, vector<MenuItem> items,
                  PaymentStrategy* ps, double total, string scheduled)
        : Order(u, r, items, ps, total, scheduled), userAddress("") {}
    
    string getType() override { return "DELIVERY"; }
    void setUserAddress(string addr) { userAddress = addr; }
};

class PickupOrder : public Order {
    string restaurantAddress;
public:
    PickupOrder(...) : Order(...) {}
    
    string getType() override { return "PICKUP"; }
    void setRestaurantAddress(string addr) { restaurantAddress = addr; }
};
```

### 7.5 Order Factory (Factory Method)

```cpp
class OrderFactory {
public:
    virtual Order* createOrder(User* user, Cart* cart,
                               Restaurant* restaurant, vector<MenuItem> menuItems,
                               PaymentStrategy* paymentStrategy,
                               double totalCost, string orderType) = 0;
    virtual ~OrderFactory() {}
};

class NowOrderFactory : public OrderFactory {
public:
    Order* createOrder(User* user, Cart* cart, Restaurant* restaurant,
                       vector<MenuItem> menuItems,
                       PaymentStrategy* paymentStrategy,
                       double totalCost, string orderType) override {
        Order* order = nullptr;
        if (orderType == "DELIVERY") {
            auto* dOrder = new DeliveryOrder(user, restaurant, menuItems,
                                            paymentStrategy, totalCost,
                                            TimeUtils::getCurrentTime());
            dOrder->setUserAddress(user->getAddress());
            order = dOrder;
        } else {
            auto* pOrder = new PickupOrder(user, restaurant, menuItems,
                                          paymentStrategy, totalCost,
                                          TimeUtils::getCurrentTime());
            pOrder->setRestaurantAddress(restaurant->getLocation());
            order = pOrder;
        }
        return order;
    }
};

class ScheduledOrderFactory : public OrderFactory {
    string scheduleTime;
public:
    ScheduledOrderFactory(string time) : scheduleTime(time) {}
    
    Order* createOrder(...) override {
        // Similar to above but uses scheduleTime
    }
};
```

### 7.6 Payment Strategy

```cpp
class PaymentStrategy {
public:
    virtual void pay(double amount) = 0;
    virtual ~PaymentStrategy() {}
};

class UPIPayment : public PaymentStrategy {
    string mobileNumber;
public:
    UPIPayment(string num) : mobileNumber(num) {}
    
    void pay(double amount) override {
        cout << "Paid " << amount << " using UPI (" << mobileNumber << ")\n";
    }
};

class CreditCardPayment : public PaymentStrategy {
    string cardNumber;
public:
    CreditCardPayment(string num) : cardNumber(num) {}
    
    void pay(double amount) override {
        cout << "Paid " << amount << " using Credit Card\n";
    }
};
```

### 7.7 TomatoApp (Orchestrator)

```cpp
class TomatoApp {
private:
    RestaurantManager* restaurantManager;
    OrderManager* orderManager;
    
    void initializeRestaurants() {
        // Create sample restaurants
        auto* r1 = new Restaurant("Bikaner", "Delhi");
        r1->addMenuItem(MenuItem("CH1", "Chole Bhature", 120));
        r1->addMenuItem(MenuItem("SM1", "Samosa", 15));
        
        auto* r2 = new Restaurant("Haldiram", "Kolkata");
        // ... add menu items
        
        restaurantManager->addRestaurant(r1);
        restaurantManager->addRestaurant(r2);
        restaurantManager->addRestaurant(r3);
    }
    
public:
    TomatoApp() {
        restaurantManager = RestaurantManager::getInstance();
        orderManager = OrderManager::getInstance();
        initializeRestaurants();
    }
    
    vector<Restaurant*> searchRestaurant(string location) {
        return restaurantManager->searchByLocation(location);
    }
    
    void selectRestaurant(User* user, Restaurant* restaurant) {
        Cart* cart = user->getCart();
        cart->setRestaurant(restaurant);
    }
    
    void addToCart(User* user, string itemCode) {
        Restaurant* restaurant = user->getCart()->getRestaurant();
        if (!restaurant) {
            cout << "Please select a restaurant first.\n";
            return;
        }
        for (auto& item : restaurant->getMenuItems()) {
            if (item.getCode() == itemCode) {
                user->getCart()->addItem(item);
                break;
            }
        }
    }
    
    Order* checkoutNow(User* user, string orderType, PaymentStrategy* ps) {
        return checkout(user, orderType, ps,
                       new NowOrderFactory());
    }
    
    Order* checkoutScheduled(User* user, string orderType, PaymentStrategy* ps,
                            string scheduleTime) {
        return checkout(user, orderType, ps,
                       new ScheduledOrderFactory(scheduleTime));
    }
    
    Order* checkout(User* user, string orderType, PaymentStrategy* ps,
                   OrderFactory* factory) {
        Cart* cart = user->getCart();
        if (cart->isEmpty()) {
            cout << "Cart is empty. Cannot checkout.\n";
            return nullptr;
        }
        
        Restaurant* orderedRestaurant = cart->getRestaurant();
        vector<MenuItem> itemsOrdered = cart->getItems();
        double totalCost = cart->getTotalCost();
        
        Order* order = factory->createOrder(user, cart, orderedRestaurant,
                                           itemsOrdered, ps, totalCost,
                                           orderType);
        orderManager->addOrder(order);
        return order;
    }
    
    bool payForOrder(User* user, Order* order) {
        bool success = order->processPayment();
        if (success) {
            NotificationService::notify(order);
            user->getCart()->clear();
        }
        return success;
    }
};
```

---

## 8. SOLID Principles Applied

| Principle | How Applied |
|-----------|-------------|
| **SRP** | Each class has one job (Cart manages items, Order manages order, Manager manages lists) |
| **OCP** | New order types = new Order subclass; new payment = new PaymentStrategy |
| **LSP** | DeliveryOrder and PickupOrder substitutable for Order |
| **ISP** | OrderFactory interface only has createOrder(); PaymentStrategy only has pay() |
| **DIP** | TomatoApp depends on abstract OrderFactory, not concrete factories |

### Trade-off: Principle of Least Knowledge

The **TomatoApp orchestrator** deliberately breaks SRP and Law of Demeter because:
- It acts as a **single point of contact** for the client
- The client (front-end) shouldn't know about all internal objects

> **Key Lesson:** SOLID principles are **guidelines, not laws**. Sometimes a trade-off is necessary.

---

## 9. Main Flow Code

```cpp
int main() {
    // 1. Initialize the app
    TomatoApp* tomato = new TomatoApp();
    
    // 2. Create a user
    User* user = new User(1001, "Aditya", "Delhi");
    cout << "User: " << user->getName() << " is active.\n";
    
    // 3. Search restaurants by location
    auto restaurants = tomato->searchRestaurant("Delhi");
    if (restaurants.empty()) {
        cout << "No restaurants found.\n";
        return 0;
    }
    cout << "Restaurants found:\n";
    for (auto r : restaurants)
        cout << r->getName() << " (" << r->getLocation() << ")\n";
    
    // 4. Select first restaurant (simulating front-end choice)
    Restaurant* selected = restaurants[0];
    tomato->selectRestaurant(user, selected);
    cout << "Selected: " << selected->getName() << "\n";
    
    // 5. Add items to cart
    tomato->addToCart(user, "CH1");  // Chole Bhature
    tomato->addToCart(user, "SM1");  // Samosa
    tomato->printUserCart(user);
    
    // 6. Checkout with UPI payment
    PaymentStrategy* payment = new UPIPayment("9876543210");
    Order* order = tomato->checkoutNow(user, "DELIVERY", payment);
    
    // 7. Pay for order
    if (order) {
        tomato->payForOrder(user, order);
    }
    
    // 8. Cleanup
    delete user;
    delete payment;
    delete tomato;
    return 0;
}
```

**Sample Output:**
```
User: Aditya is active.
Restaurants found:
Bikaner (Delhi)

Selected: Bikaner
Cart for Aditya:
Chole Bhature - 120
Samosa - 15
Total: 135
Paid 135 using UPI (9876543210)
Notification sent for Order ID: 1
Order Type: DELIVERY
Restaurant: Bikaner
Total: 135
Scheduled: 2024-01-15 14:30:00
```

---

## 10. Further Extensions

### Extension 1: Payment Strategy Factory

Instead of deciding payment strategy in `main()`, use a factory:

```cpp
class PaymentStrategyFactory {
public:
    static PaymentStrategy* createStrategy(string type) {
        if (type == "UPI") return new UPIPayment("...");
        if (type == "CARD") return new CreditCardPayment("...");
        return nullptr;
    }
};
```

### Extension 2: Notification Service Hierarchy

```
              ┌────────────────────────────┐
              │  <<abstract>>              │
              │  NotificationService       │
              ├────────────────────────────┤
              │ + notify(Order) = 0        │
              └─────────────┬──────────────┘
                            △
              ┌─────────────┼──────────────┐
              │             │              │
     ┌────────┴────┐ ┌──────┴─────┐ ┌─────┴──────┐
     │PushNotif    │ │EmailNotif  │ │WhatsApp    │
     │Service      │ │Service     │ │Notif       │
     └─────────────┘ └────────────┘ └────────────┘
```

### Extension 3: Decentralized Architecture (Modern Frameworks)

Instead of one `TomatoApp` orchestrator:

```
┌─────────────────────────────────────────────────────────────┐
│                        API LAYER                            │
│  /search-restaurant  /add-to-cart  /checkout  /pay          │
└──────────────────────┬──────────────────────────────────────┘
                       │
┌──────────────────────┴──────────────────────────────────────┐
│                     SERVICE LAYER                            │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────┐         │
│  │RestaurantSvc │ │  CartSvc     │ │  OrderSvc    │         │
│  └──────────────┘ └──────────────┘ └──────────────┘         │
│  ┌──────────────┐ ┌──────────────┐                          │
│  │ PaymentSvc   │ │ Notification │                          │
│  └──────────────┘ └──────────────┘                          │
└──────────────────────────────────────────────────────────────┘
```

This is how **Spring Boot / Django** structure applications — separate controllers, services, and repositories.

---

## 11. Key Takeaways

```
┌─────────────────────────────────────────────────────────────────────┐
│                    LLD CASE STUDY — TAKEAWAYS                       │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   ✅ ALWAYS gather requirements with counter-questions              │
│   ✅ Discuss happy flow before designing                            │
│   ✅ Use Bottom-Up approach for LLD problems                        │
│   ✅ Draw UML before code                                           │
│   ✅ Apply design patterns appropriately (don't force them)         │
│   ✅ Managers are Singletons (single source of truth)               │
│   ✅ Use composition over inheritance wherever possible             │
│   ✅ Single point of contact (orchestrator) for client              │
│   ✅ Know when to break principles (trade-offs)                     │
│   ✅ Design patterns used:                                          │
│      • Singleton (Managers)                                         │
│      • Strategy (Payment)                                           │
│      • Factory Method (Order creation)                              │
│      • Composition/Aggregation (Relationships)                      │
│                                                                      │
│   Project Structure (like real apps):                               │
│   ├── models/      (User, Cart, Order, Restaurant, MenuItem)        │
│   ├── managers/    (RestaurantManager, OrderManager)                │
│   ├── factories/   (OrderFactory, NowOrderFactory, ...)             │
│   ├── strategies/  (PaymentStrategy, UPIPayment, ...)               │
│   ├── services/    (NotificationService)                            │
│   └── utils/       (TimeUtils)                                      │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Interview Wisdom

> **"The interviewer is not your interviewer — they are your friend/manager who wants to build the application WITH you."**

- LLD questions are **subjective** — discuss and iterate
- There's no single "right answer" — trade-offs matter
- **Working code** (even structural) is expected at the end
- 1-hour interview can only cover so much — pick the most important parts

---

## 12. Observer Design Pattern Explained (28:30)

This lecture covers the **Observer Design Pattern** — a behavioral pattern that defines a **one-to-many relationship** between objects, so when one object changes state, all its dependents are notified automatically. The classic real-world analogy is **YouTube subscriptions**.

---

## 1. What is Observer Pattern?

> **"Define a one-to-many relationship between objects so that when one object changes state, all of its dependents are notified and updated automatically."**

### Real-World Analogy: YouTube

```
┌─────────────────────────────────────────────────────────────────────┐
│                    YOUTUBE SUBSCRIPTION MODEL                       │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   YouTube Channel              Subscribers                          │
│   (Observable)                 (Observers)                          │
│                                                                      │
│   ┌──────────────┐     ┌──────────────────┐                         │
│   │   Channel    │────▶│  Subscriber 1    │                         │
│   │              │     └──────────────────┘                         │
│   │  Uploads new │────▶│  Subscriber 2    │                         │
│   │    video     │     └──────────────────┘                         │
│   │              │────▶│  Subscriber 3    │                         │
│   │              │     └──────────────────┘                         │
│   │              │────▶│  Subscriber N    │                         │
│   └──────────────┘     └──────────────────┘                         │
│                                                                      │
│   When channel uploads → ALL subscribers get notified               │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Core Terminology

| Term | Meaning | YouTube Example |
|------|---------|-----------------|
| **Observable** (Subject) | Object being watched | YouTube Channel |
| **Observer** | Object doing the watching | Subscriber |
| **One-to-Many** | One observable → many observers | One channel → many subscribers |

---

## 2. The Problem: Polling vs Pushing

### ❌ Polling Technique (Bad)

```
┌─────────────────────────────────────────────────────────────────────┐
│                    POLLING (BAD APPROACH)                           │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   Observer repeatedly asks Observable:                             │
│                                                                      │
│   Observer: "Did your value change?"  ────▶                        │
│   Observable: "No."                    ◀────                        │
│   Observer: "Did your value change?"  ────▶                        │
│   Observable: "No."                    ◀────                        │
│   Observer: "Did your value change?"  ────▶                        │
│   Observable: "No."                    ◀────                        │
│   Observer: "Did your value change?"  ────▶                        │
│   Observable: "Yes!"                   ◀────                        │
│                                                                      │
│   Problems:                                                         │
│   • Wasteful: constant requests                                     │
│   • Time-consuming                                                  │
│   • Difficult to pick polling frequency                             │
│   • Never know the exact right moment                               │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### ✅ Pushing Technique (Good)

```
┌─────────────────────────────────────────────────────────────────────┐
│                    PUSHING (GOOD APPROACH)                          │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   Observable actively notifies Observers:                          │
│                                                                      │
│   Observable state changes →                                         │
│   Observable pushes notification to ALL observers                    │
│                                                                      │
│   ┌──────────────┐                                                  │
│   │  Observable  │──▶ "My value changed!"                          │
│   └──────────────┘     ├──▶ Observer 1                              │
│                        ├──▶ Observer 2                              │
│                        ├──▶ Observer 3                              │
│                        └──▶ Observer N                              │
│                                                                      │
│   Benefits:                                                         │
│   • Efficient — no wasted requests                                 │
│   • Real-time — immediate notification                             │
│   • Clean — no polling logic needed                                │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 3. UML Design

### 3.1 Naming Convention: `I` Prefix for Interfaces

The lecture introduces a **standard naming convention**:
- Pure abstract classes (all virtual methods) → prefix with `I` (e.g., `IObservable`, `IObserver`)
- This distinguishes interfaces from concrete classes

### 3.2 Generic UML Diagram

```
                    ┌──────────────────────────────┐
                    │     <<interface>>            │
                    │      IObservable             │
                    ├──────────────────────────────┤
                    │ + add(IObserver): void       │
                    │ + remove(IObserver): void    │
                    │ + notify(): void             │
                    └──────────────┬───────────────┘
                                   △
                                   │ (implements)
                    ┌──────────────┴───────────────┐
                    │    ConcreteObservable        │
                    ├──────────────────────────────┤
                    │ - observers: List<IObserver> │
                    ├──────────────────────────────┤
                    │ + add(IObserver)             │
                    │ + remove(IObserver)          │
                    │ + notify()                   │
                    │ + getValue(): ...            │
                    └──────────────────────────────┘

                    ┌──────────────────────────────┐
                    │     <<interface>>            │
                    │      IObserver               │
                    ├──────────────────────────────┤
                    │ + update(): void             │
                    └──────────────┬───────────────┘
                                   △
                                   │ (implements)
                    ┌──────────────┴───────────────┐
                    │    ConcreteObserver          │
                    ├──────────────────────────────┤
                    │ - observable: IObservable*   │
                    ├──────────────────────────────┤
                    │ + update()                   │
                    └──────────────────────────────┘
                                    │ (has-a)
                                    ▼
                              IObservable
```

### 3.3 Key Methods

| Interface | Method | Purpose |
|-----------|--------|---------|
| **IObservable** | `add(IObserver)` | Subscribe an observer |
| **IObservable** | `remove(IObserver)` | Unsubscribe an observer |
| **IObservable** | `notify()` | Notify all observers of state change |
| **IObserver** | `update()` | Called by observable when state changes |

### 3.4 The "Concrete-to-Concrete" Relationship

**Unique to Observer Pattern:** Unlike most patterns where relationships are between abstractions, Observer Pattern uses **concrete observer → concrete observable** relationship. This enables the observer to read the actual value from the observable.

---

## 4. Code Implementation: YouTube Example

### 4.1 Interfaces

```cpp
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

// ============ INTERFACES ============
class ISubscriber {
public:
    virtual void update() = 0;
    virtual ~ISubscriber() {}
};

class IChannel {
public:
    virtual void subscribe(ISubscriber* sub) = 0;
    virtual void unsubscribe(ISubscriber* sub) = 0;
    virtual void notifySubscribers() = 0;
    virtual ~IChannel() {}
};
```

### 4.2 Concrete Channel (Observable)

```cpp
class Channel : public IChannel {
private:
    vector<ISubscriber*> subscribers;
    string name;
    string latestVideo;

public:
    Channel(string name) : name(name) {}

    void subscribe(ISubscriber* sub) override {
        // Avoid duplicate subscriptions
        if (find(subscribers.begin(), subscribers.end(), sub) 
            == subscribers.end()) {
            subscribers.push_back(sub);
        }
    }

    void unsubscribe(ISubscriber* sub) override {
        subscribers.erase(
            remove(subscribers.begin(), subscribers.end(), sub),
            subscribers.end()
        );
    }

    void notifySubscribers() override {
        for (auto* sub : subscribers) {
            sub->update();
        }
    }

    void uploadVideo(string title) {
        latestVideo = title;
        cout << name << " uploaded: " << title << "\n";
        notifySubscribers();
    }

    string getVideoData() {
        return "Check out our new video: " + latestVideo;
    }
};
```

### 4.3 Concrete Subscriber (Observer)

```cpp
class Subscriber : public ISubscriber {
private:
    string name;
    Channel* channel;  // Has-a concrete observable

public:
    Subscriber(string name, Channel* ch) : name(name), channel(ch) {}

    void update() override {
        cout << "Hey " << name << ", "
             << channel->getVideoData() << "\n";
    }
};
```

### 4.4 Main (Client)

```cpp
int main() {
    // Create channel (Observable)
    Channel* channel = new Channel("CodeArmy");

    // Create subscribers (Observers)
    Subscriber* varun = new Subscriber("Varun", channel);
    Subscriber* tarun = new Subscriber("Tarun", channel);

    // Subscribe
    channel->subscribe(varun);
    channel->subscribe(tarun);

    // Upload video → both get notified
    channel->uploadVideo("Observer Pattern Tutorial");

    // Unsubscribe Varun
    channel->unsubscribe(varun);

    // Upload another video → only Tarun gets notified
    channel->uploadVideo("Decorator Pattern Tutorial");

    // Cleanup
    delete varun;
    delete tarun;
    delete channel;
    return 0;
}
```

### 4.5 Output

```
CodeArmy uploaded: Observer Pattern Tutorial
Hey Varun, Check out our new video: Observer Pattern Tutorial
Hey Tarun, Check out our new video: Observer Pattern Tutorial

CodeArmy uploaded: Decorator Pattern Tutorial
Hey Tarun, Check out our new video: Decorator Pattern Tutorial
```

**Observation:** After Varun unsubscribes, only Tarun receives the notification.

---

## 5. Complete UML for YouTube Example

```
                    ┌──────────────────────────────┐
                    │      <<interface>>           │
                    │       IChannel               │
                    ├──────────────────────────────┤
                    │ + subscribe(ISubscriber*)    │
                    │ + unsubscribe(ISubscriber*)  │
                    │ + notifySubscribers()        │
                    └──────────────┬───────────────┘
                                   △
                                   │ implements
                    ┌──────────────┴───────────────┐
                    │         Channel              │
                    ├──────────────────────────────┤
                    │ - subscribers: List<ISub*>   │
                    │ - name: string               │
                    │ - latestVideo: string        │
                    ├──────────────────────────────┤
                    │ + subscribe()                │
                    │ + unsubscribe()              │
                    │ + notifySubscribers()        │
                    │ + uploadVideo(title)         │
                    │ + getVideoData(): string     │
                    └──────────────────────────────┘
                                    △
                                    │ (has-a)
                                    │ 1..*
                    ┌───────────────┴──────────────┐
                    │                              │
                    │  ┌──────────────────────┐   │
                    │  │  <<interface>>       │   │
                    │  │   ISubscriber        │   │
                    │  ├──────────────────────┤   │
                    │  │ + update()           │   │
                    │  └──────────┬───────────┘   │
                    │             △                │
                    │             │ implements     │
                    │  ┌──────────┴───────────┐   │
                    │  │    Subscriber        │   │
                    │  ├──────────────────────┤   │
                    │  │ - name: string       │   │
                    │  │ - channel: Channel*  │   │
                    │  ├──────────────────────┤   │
                    │  │ + update()           │   │
                    │  └──────────────────────┘   │
                    │             │                │
                    │             │ has-a          │
                    │             ▼                │
                    │         Channel              │
                    └──────────────────────────────┘
```

---

## 6. Observer Pattern Breaks SRP (Trade-off)

### Why It Breaks SRP

```
┌─────────────────────────────────────────────────────────────────────┐
│           CONCRETE CHANNEL — TWO RESPONSIBILITIES                   │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   Responsibility 1: OBSERVER PATTERN LOGIC                          │
│   • subscribe()                                                     │
│   • unsubscribe()                                                   │
│   • notifySubscribers()                                             │
│                                                                      │
│   Responsibility 2: BUSINESS LOGIC                                  │
│   • uploadVideo()                                                   │
│   • getVideoData()                                                  │
│                                                                      │
│   Two reasons to change → Violates Single Responsibility            │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Why This Trade-off Is Acceptable

The lecture explains:

> **"The Observer pattern logic NEVER changes. Only the business logic changes."**

- `subscribe()`, `unsubscribe()`, `notifySubscribers()` → **Static, never change**
- `uploadVideo()`, `getVideoData()` → **Business logic, can change**

This aligns with the **fundamental design pattern idea**: separate what changes from what doesn't.

### How to Fix It (If You Want to Be Strict)

Extract the observer pattern logic into a base abstract class:

```cpp
class Observable {
protected:
    vector<ISubscriber*> subscribers;
public:
    void subscribe(ISubscriber* sub) { /* ... */ }
    void unsubscribe(ISubscriber* sub) { /* ... */ }
    void notifySubscribers() { /* ... */ }
};

class Channel : public Observable {
    string name;
    string latestVideo;
public:
    void uploadVideo(string title) { /* ... */ }
    string getVideoData() { /* ... */ }
};
```

**But:** The lecture recommends keeping it simple — standard Observer pattern UML diagrams also break SRP.

---

## 7. Real-World Applications

### Application 1: Notification Service

```
┌─────────────────────────────────────────────────────────────────────┐
│                    NOTIFICATION SERVICE                             │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   ┌──────────────────┐                                              │
│   │ Notification     │                                              │
│   │ Server           │────▶ Subscriber A (Email)                    │
│   │ (Observable)     │────▶ Subscriber B (Push)                     │
│   │                  │────▶ Subscriber C (SMS)                      │
│   │ New notification │────▶ Subscriber D (WhatsApp)                 │
│   └──────────────────┘                                              │
│                                                                      │
│   All subscribers get notified when a new notification arrives      │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Application 2: News Feed (Facebook/Instagram)

```
┌─────────────────────────────────────────────────────────────────────┐
│                    SOCIAL MEDIA FEED                                │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   User posts something → All followers' feeds get updated           │
│                                                                      │
│   ┌──────────────┐       ┌────────────┐                             │
│   │ User A       │──────▶│ Follower 1 │                             │
│   │ (posts)      │──────▶│ Follower 2 │                             │
│   │              │──────▶│ Follower 3 │                             │
│   └──────────────┘       └────────────┘                             │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Application 3: Event Handling (Frontend)

```
┌─────────────────────────────────────────────────────────────────────┐
│                    DOM EVENT LISTENERS                              │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   button.addEventListener('click', handler1);                       │
│   button.addEventListener('click', handler2);                       │
│   button.addEventListener('click', handler3);                       │
│                                                                      │
│   When button clicked → ALL handlers execute                        │
│                                                                      │
│   Internally, this is Observer Pattern!                             │
│   button = Observable                                               │
│   handler1/2/3 = Observers                                          │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 8. Benefits and Drawbacks

### ✅ Benefits

| Benefit | Description |
|---------|-------------|
| **Loose Coupling** | Observable doesn't need to know concrete observer classes |
| **Dynamic Relationships** | Subscribe/unsubscribe at runtime |
| **Broadcast Communication** | One change → many reactions |
| **Open/Closed** | Add new observers without modifying observable |
| **Efficient** | No polling, only push-based updates |

### ❌ Drawbacks

| Drawback | Description |
|----------|-------------|
| **SRP Violation** | Concrete observable handles both pattern + business logic |
| **Memory Leaks** | If observers aren't removed, they stay referenced |
| **Unexpected Updates** | Observers may be notified in unpredictable order |
| **Debugging Complexity** | Tracing notification chains can be hard |

---

## 9. Key Takeaways

```
┌─────────────────────────────────────────────────────────────────────┐
│                  OBSERVER PATTERN — SUMMARY                         │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   🎯 PURPOSE                                                        │
│   Define a one-to-many relationship so that when one object        │
│   changes state, all dependents are notified automatically         │
│                                                                      │
│   🧩 KEY COMPONENTS                                                 │
│   • IObservable (Subject) — interface for being observed            │
│   • IObserver — interface for observing                             │
│   • ConcreteObservable — actual subject                             │
│   • ConcreteObserver — actual observer                              │
│                                                                      │
│   🔑 KEY METHODS                                                    │
│   • subscribe() / add()                                             │
│   • unsubscribe() / remove()                                        │
│   • notify() / notifySubscribers()                                  │
│   • update()                                                        │
│                                                                      │
│   💡 CORE INSIGHT                                                   │
│   Push-based notification (not pull-based polling)                  │
│                                                                      │
│   ⚠️ TRADE-OFF                                                      │
│   Breaks SRP — but it's an accepted compromise                      │
│   because pattern logic never changes                               │
│                                                                      │
│   🌍 REAL-WORLD USES                                                │
│   • YouTube / Social media subscriptions                            │
│   • Notification services                                           │
│   • Event handling (DOM, GUI)                                       │
│   • News feeds                                                      │
│   • Stock market tickers                                            │
│   • Message queues                                                  │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### The Golden Rule

> **"When you need one object's changes to trigger reactions in many others, reach for the Observer Pattern."**

### Interview Wisdom

Observer Pattern is one of the **most useful behavioral patterns** and frequently appears in LLD interviews — especially in:
- Notification system designs
- Chat/messaging applications
- Real-time feed designs
- Event-driven architectures

---

## 13. Decorator Pattern Explained | Real-world use case + Code (29:19)

This lecture covers the **Decorator Design Pattern** — a structural pattern that lets you attach additional responsibilities to an object **dynamically at runtime**, providing a flexible alternative to subclassing (inheritance).

---

## 1. What is Decorator Pattern?

> **"Attach additional responsibilities to an object dynamically. Decorators provide a flexible alternative to subclassing for extending functionality."**

### Core Idea

```
┌─────────────────────────────────────────────────────────────────────┐
│                    DECORATOR PATTERN — CORE IDEA                    │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   Instead of creating subclasses for every combination,             │
│   WRAP the object with decorators at runtime.                       │
│                                                                      │
│   ┌──────────────┐                                                  │
│   │   Client     │                                                  │
│   └──────┬───────┘                                                  │
│          │ calls doSomething()                                      │
│          ▼                                                          │
│   ┌──────────────┐     wraps      ┌──────────────┐                  │
│   │  Decorator2  │───────────────▶│  Decorator1  │                  │
│   └──────────────┘                └──────┬───────┘                  │
│                                          │ wraps                     │
│                                          ▼                          │
│                                   ┌──────────────┐                  │
│                                   │ Base Object  │                  │
│                                   └──────────────┘                  │
│                                                                      │
│   Each decorator adds its own behavior and delegates to the         │
│   next object in the chain.                                         │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Simple Example: `doSomething()` Enhancement

```
WITHOUT DECORATOR:
────────────────────
Client calls: obj1.doSomething()
obj1 returns: "I did something"

WITH DECORATOR:
────────────────────
Client calls: decorator2.doSomething()
decorator2 → decorator1 → obj1 returns "I did something"
decorator1 enhances: "I did something amazingly"
decorator2 enhances: "I did something amazingly today"
Client receives: "I did something amazingly today"
```

**Key Point:** The original object is never modified. Its behavior is **extended** by wrapping it with decorators.

---

## 2. The Problem: Inheritance Class Explosion

### Mario Game Example (Using Inheritance)

**Scenario:** Design a Mario character that can gain power-ups:
- Height Up
- Gun Shooting
- Star Ability (fast speed, destroys enemies)
- (Later: Flying ability)

### ❌ Inheritance Approach — Class Explosion

```
                    ┌─────────────────────┐
                    │       Mario         │
                    │ + getAbilities()    │
                    └──────────┬──────────┘
                               △
              ┌────────────────┼────────────────┐
              │                │                │
     ┌────────┴────────┐ ┌─────┴──────┐ ┌──────┴──────────┐
     │ MarioWithHeight │ │MarioWithGun│ │MarioWithStar    │
     └────────┬────────┘ └─────┬──────┘ └──────┬──────────┘
              │                │                │
              └────────────────┼────────────────┘
                               │
              COMBINATIONS EXPLODE:
              ┌────────────────┼────────────────────────┐
              │                │                        │
     ┌────────┴───────┐ ┌─────┴────────┐ ┌──────────────┴─────────┐
     │MarioWithHeight │ │MarioWithGun  │ │MarioWithHeightAndGun   │
     │AndGun          │ │AndStar       │ │AndStarAndFly           │
     └────────────────┘ └──────────────┘ └────────────────────────┘
              └────────────────┼────────────────────────┘
                               │
                  AND SO ON... (2^n combinations!)
```

### Why This Explodes

If Mario has 4 power-ups: `Height`, `Gun`, `Star`, `Fly`:

| # Power-ups | Combinations | Subclasses Needed |
|-------------|--------------|-------------------|
| 1 | 4 | 4 |
| 2 | 6 | 6 |
| 3 | 4 | 4 |
| 4 | 1 | 1 |
| **Total** | | **15 subclasses!** |

Add a 5th power-up → **31 subclasses**. This is **class explosion**.

### The Two Big Problems

```
┌─────────────────────────────────────────────────────────────────────┐
│              PROBLEMS WITH INHERITANCE APPROACH                     │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   ❌ 1. CLASS EXPLOSION                                             │
│      • 2^n subclasses for n features                                │
│      • Every new feature → many new subclasses                      │
│                                                                      │
│   ❌ 2. NO RUNTIME FLEXIBILITY                                      │
│      • Power-ups come and go (Star is limited time)                 │
│      • Subclass is a compile-time decision                          │
│      • Can't add/remove abilities dynamically                       │
│                                                                      │
│   🎯 REMEMBER: "Favor composition over inheritance"                 │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 3. The Solution: Decorator Pattern

### Core Concept

**Instead of extending behavior via inheritance, wrap the object with decorators.**

```
MARIO WITH DECORATORS:

┌────────────────────┐
│  StarPowerDecorator│  ← Outermost wrapper
└─────────┬──────────┘
          │ wraps
          ▼
┌────────────────────┐
│  GunPowerDecorator │
└─────────┬──────────┘
          │ wraps
          ▼
┌────────────────────┐
│  HeightUpDecorator │
└─────────┬──────────┘
          │ wraps
          ▼
┌────────────────────┐
│   MarioCharacter   │  ← Base object (unchanged!)
└────────────────────┘
```

### Benefits of Decorator Approach

| Benefit | Description |
|---------|-------------|
| **No class explosion** | N decorators instead of 2^N subclasses |
| **Runtime flexibility** | Add/remove decorators dynamically |
| **Open/Closed** | New decorator = new class, no modification |
| **Single Responsibility** | Each decorator handles one enhancement |
| **Composability** | Any combination, any order |

---

## 4. UML Design

### 4.1 Naming Convention

- Pure abstract classes (all virtual methods) → prefix with `I` (e.g., `ICharacter`)
- This is a standard convention to distinguish interfaces from concrete classes

### 4.2 Generic UML Diagram

```
                    ┌──────────────────────────────┐
                    │      <<interface>>           │
                    │      IComponent              │
                    ├──────────────────────────────┤
                    │ + operation(): string = 0    │
                    └──────────────┬───────────────┘
                                   △
                    ┌──────────────┴───────────────┐
                    │                              │
                    │ (implements)                 │ (implements + has-a)
                    ▼                              ▼
       ┌────────────────────────┐    ┌─────────────────────────────┐
       │   ConcreteComponent    │    │    <<abstract>>             │
       ├────────────────────────┤    │    Decorator                │
       │ + operation()          │    ├─────────────────────────────┤
       └────────────────────────┘    │ - component: IComponent*    │
                                     ├─────────────────────────────┤
                                     │ + operation()               │
                                     └──────────────┬──────────────┘
                                                    △
                                     ┌──────────────┼──────────────┐
                                     │              │              │
                              ┌──────┴──────┐ ┌─────┴─────┐ ┌──────┴──────┐
                              │ConcreteDecA │ │ConcreteDecB│ │ConcreteDecC│
                              └─────────────┘ └───────────┘ └────────────┘
```

### 4.3 Key Relationships

```
┌─────────────────────────────────────────────────────────────────────┐
│                 DECORATOR'S TWO RELATIONSHIPS                       │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   ┌─────────────────┐         ┌─────────────────┐                  │
│   │   Decorator     │         │   IComponent    │                  │
│   └────────┬────────┘         └────────┬────────┘                  │
│            │                           │                            │
│            │ IS-A (inheritance)        │                            │
│            └───────────────────────────┘                            │
│                                                                      │
│   Decorator IS-A IComponent                                         │
│   → So it can be used where IComponent is expected                 │
│   → Enables stacking/wrapping                                       │
│                                                                      │
│   ┌─────────────────┐         ┌─────────────────┐                  │
│   │   Decorator     │         │   IComponent    │                  │
│   └────────┬────────┘         └────────┬────────┘                  │
│            │                           │                            │
│            │ HAS-A (composition)       │                            │
│            └───────────────────────────┘                            │
│                                                                      │
│   Decorator HAS-A IComponent                                        │
│   → Holds a reference to the wrapped object                        │
│   → Delegates calls to wrapped object                              │
│   → Then adds its own behavior                                     │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

**Key Insight:** Decorator uses **BOTH** `IS-A` (inheritance) and `HAS-A` (composition):
- **IS-A** → To behave like the base class (so it can be stacked)
- **HAS-A** → To delegate behavior and add new behavior dynamically

---

## 5. Code Implementation: Mario Example

### 5.1 Interfaces and Base Character

```cpp
#include <iostream>
#include <string>
using namespace std;

// ============ INTERFACE ============
class ICharacter {
public:
    virtual string getAbilities() = 0;
    virtual ~ICharacter() {}
};

// ============ CONCRETE COMPONENT ============
class Mario : public ICharacter {
public:
    string getAbilities() override {
        return "Mario";
    }
};
```

### 5.2 Abstract Decorator

```cpp
// ============ ABSTRACT DECORATOR ============
class CharacterDecorator : public ICharacter {
protected:
    ICharacter* character;  // HAS-A (composition)

public:
    CharacterDecorator(ICharacter* c) : character(c) {}

    string getAbilities() override {
        return character->getAbilities();
    }

    ~CharacterDecorator() {
        delete character;
    }
};
```

### 5.3 Concrete Decorators

```cpp
// ============ CONCRETE DECORATOR 1: Height Up ============
class HeightUpDecorator : public CharacterDecorator {
public:
    HeightUpDecorator(ICharacter* c) : CharacterDecorator(c) {}

    string getAbilities() override {
        return character->getAbilities() + " with Height Up";
    }
};

// ============ CONCRETE DECORATOR 2: Gun Power ============
class GunPowerDecorator : public CharacterDecorator {
public:
    GunPowerDecorator(ICharacter* c) : CharacterDecorator(c) {}

    string getAbilities() override {
        return character->getAbilities() + " with Gun";
    }
};

// ============ CONCRETE DECORATOR 3: Star Power ============
class StarPowerDecorator : public CharacterDecorator {
public:
    StarPowerDecorator(ICharacter* c) : CharacterDecorator(c) {}

    string getAbilities() override {
        return character->getAbilities() + " with Star Power (Limited Time)";
    }
};

// ============ NEW DECORATOR — No existing code changed! ============
class FlyPowerDecorator : public CharacterDecorator {
public:
    FlyPowerDecorator(ICharacter* c) : CharacterDecorator(c) {}

    string getAbilities() override {
        return character->getAbilities() + " with Fly Power";
    }
};
```

### 5.4 Main (Client)

```cpp
int main() {
    // Start with a base Mario
    ICharacter* mario = new Mario();
    cout << "Base: " << mario->getAbilities() << "\n";

    // Wrap with HeightUpDecorator
    mario = new HeightUpDecorator(mario);
    cout << "After HeightUp: " << mario->getAbilities() << "\n";

    // Wrap with GunPowerDecorator
    mario = new GunPowerDecorator(mario);
    cout << "After GunPower: " << mario->getAbilities() << "\n";

    // Wrap with StarPowerDecorator
    mario = new StarPowerDecorator(mario);
    cout << "After StarPower: " << mario->getAbilities() << "\n";

    // Clean up (deletes entire chain)
    delete mario;
    return 0;
}
```

### 5.5 Output

```
Base: Mario
After HeightUp: Mario with Height Up
After GunPower: Mario with Height Up with Gun
After StarPower: Mario with Height Up with Gun with Star Power (Limited Time)
```

### 5.6 Call Chain Visualization

```
mario->getAbilities()
    │
    ▼
┌──────────────────────┐
│ StarPowerDecorator   │ → calls character->getAbilities()
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│ GunPowerDecorator    │ → calls character->getAbilities()
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│ HeightUpDecorator    │ → calls character->getAbilities()
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│       Mario          │ → returns "Mario"
└──────────────────────┘

Return path (adding enhancements):
"Mario"
    ▲ + " with Height Up"
"Mario with Height Up"
    ▲ + " with Gun"
"Mario with Height Up with Gun"
    ▲ + " with Star Power (Limited Time)"
"Mario with Height Up with Gun with Star Power (Limited Time)"
```

**Note:** This is essentially **recursion** — walking down the chain, hitting the base case, then walking back up adding enhancements.

---

## 6. How Decorator Solves Inheritance Problem (Simple Example)

### The Problem Setup

Suppose you want to add features **F1, F2, F3** to an object.

### ❌ Inheritance Solution → Class Explosion

```
┌─────────────────────────────────────────────────────────────────────┐
│         INHERITANCE — CLASS EXPLOSION FOR 3 FEATURES                │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│                    ┌─────────────┐                                  │
│                    │  Base       │                                  │
│                    └──────┬──────┘                                  │
│           ┌───────────────┼───────────────┐                         │
│           │               │               │                         │
│      ┌────┴────┐     ┌────┴────┐     ┌────┴────┐                    │
│      │Base+F1  │     │Base+F2  │     │Base+F3  │                    │
│      └────┬────┘     └────┬────┘     └────┬────┘                    │
│           │               │               │                         │
│      ┌────┴────┐     ┌────┴────┐     ┌────┴────┐                    │
│      │Base+F1  │     │Base+F2  │     │Base+F3  │                    │
│      │  +F2    │     │  +F3    │     │  +F1    │                    │
│      └────┬────┘     └────┬────┘     └────┬────┘                    │
│           │               │               │                         │
│      ┌────┴───────────────┴───────────────┴────┐                   │
│      │        Base+F1+F2+F3                    │                   │
│      └─────────────────────────────────────────┘                    │
│                                                                      │
│   Total subclasses for N features = 2^N - 1                        │
│   For N=3: 7 subclasses                                            │
│   For N=5: 31 subclasses                                           │
│   For N=10: 1023 subclasses! 😱                                    │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### ✅ Decorator Solution → Just N Decorators

```
┌─────────────────────────────────────────────────────────────────────┐
│         DECORATOR — JUST N DECORATORS FOR N FEATURES                │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│                    ┌─────────────┐                                  │
│                    │  Base       │                                  │
│                    └──────┬──────┘                                  │
│                           △                                          │
│                    ┌──────┴──────┐                                  │
│                    │  Decorator  │                                  │
│                    └──────┬──────┘                                  │
│           ┌───────────────┼───────────────┐                         │
│           │               │               │                         │
│      ┌────┴────┐     ┌────┴────┐     ┌────┴────┐                    │
│      │  F1 Dec │     │  F2 Dec │     │  F3 Dec │                    │
│      └─────────┘     └─────────┘     └─────────┘                    │
│                                                                      │
│   For N features → N decorator classes                              │
│   For N=3: 3 classes                                               │
│   For N=5: 5 classes                                               │
│   For N=10: 10 classes ✅                                          │
│                                                                      │
│   And ANY combination works at runtime:                             │
│   new F1(new F2(new F3(new Base())))   ✅                          │
│   new F2(new F1(new Base()))           ✅                          │
│   new F3(new F3(new F1(new Base())))   ✅                          │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Side-by-Side Comparison

| Aspect | Inheritance | Decorator |
|--------|-------------|-----------|
| **Classes for N=3** | 7 | 3 |
| **Classes for N=5** | 31 | 5 |
| **Classes for N=10** | 1023 | 10 |
| **Runtime change** | ❌ Impossible | ✅ Possible |
| **Order flexibility** | ❌ Fixed at compile time | ✅ Any order at runtime |
| **Modify existing code for new feature** | ❌ Yes (new subclasses) | ✅ No (just new decorator) |
| **Duplicate code** | ❌ Yes (common logic repeated) | ✅ No (reuse via composition) |

### Simple Concrete Example

**Problem:** A coffee shop where you can add toppings (Milk, Sugar, Whip) to a base coffee.

**❌ Inheritance approach:** You'd need:
- `CoffeeWithMilk`
- `CoffeeWithSugar`
- `CoffeeWithWhip`
- `CoffeeWithMilkAndSugar`
- `CoffeeWithMilkAndWhip`
- `CoffeeWithSugarAndWhip`
- `CoffeeWithMilkAndSugarAndWhip`
- ... and so on. **7 classes for 3 toppings!**

**✅ Decorator approach:** Just 3 decorators:
- `MilkDecorator`
- `SugarDecorator`
- `WhipDecorator`

And compose them at runtime:

```cpp
// Just milk
ICoffee* c1 = new MilkDecorator(new Coffee());

// Milk + Sugar
ICoffee* c2 = new SugarDecorator(new MilkDecorator(new Coffee()));

// Milk + Sugar + Whip
ICoffee* c3 = new WhipDecorator(
                  new SugarDecorator(
                      new MilkDecorator(new Coffee())));
```

**Same 3 classes handle ALL combinations!**

---

## 7. The Recursive Nature of Decorators

```
┌─────────────────────────────────────────────────────────────────────┐
│                    RECURSIVE CALL STRUCTURE                         │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   Calling getAbilities() on the outermost decorator:                │
│                                                                      │
│   ┌────────────────────────────────────────────────────────────┐   │
│   │  StarPowerDecorator.getAbilities()                         │   │
│   │     └── GunPowerDecorator.getAbilities()                   │   │
│   │            └── HeightUpDecorator.getAbilities()            │   │
│   │                   └── Mario.getAbilities()                 │   │
│   │                       returns "Mario"                     │   │
│   │                   appends " with Height Up"               │   │
│   │            appends " with Gun"                            │   │
│   │     appends " with Star Power"                            │   │
│   │  returns full string                                      │   │
│   └────────────────────────────────────────────────────────────┘   │
│                                                                      │
│   This is essentially RECURSION with different classes.             │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 8. Real-World Use Cases

### Use Case 1: Text Editor (like Google Docs)

```
┌─────────────────────────────────────────────────────────────────────┐
│                    TEXT EDITOR — DECORATOR PATTERN                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   ┌──────────────┐                                                  │
│   │   IText      │                                                  │
│   │  (abstract)  │                                                  │
│   ├──────────────┤                                                  │
│   │ + render()   │                                                  │
│   └──────┬───────┘                                                  │
│          △                                                          │
│          │                                                          │
│   ┌──────┴──────────┬──────────────┬──────────────┐                 │
│   │                 │              │              │                 │
│   ▼                 ▼              ▼              ▼                 │
│ SimpleText    BoldDecorator  ItalicDecorator  UnderlineDecorator   │
│                                                                      │
│   Example: Bold + Italic text on underline                          │
│   new UnderlineDecorator(                                           │
│       new ItalicDecorator(                                          │
│           new BoldDecorator(                                        │
│               new SimpleText("Hello"))))                            │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Use Case 2: Form Validation (Backend)

```
┌─────────────────────────────────────────────────────────────────────┐
│                    FORM VALIDATION — DECORATOR                      │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   Base Form                                                         │
│      │                                                              │
│      ├─▶ EmailValidator      (checks valid email)                   │
│      ├─▶ SQLInjectionChecker (checks SQL attacks)                   │
│      ├─▶ XSSChecker          (checks XSS attacks)                   │
│      └─▶ LengthValidator     (checks length constraints)            │
│                                                                      │
│   Each validator is a decorator that can be chained.                │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Use Case 3: Java I/O Streams

```
┌─────────────────────────────────────────────────────────────────────┐
│                    JAVA I/O — CLASSIC DECORATOR                     │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   InputStream                    (abstract component)               │
│      ├── FileInputStream         (concrete component)               │
│      ├── BufferedInputStream     (decorator)                        │
│      ├── DataInputStream         (decorator)                        │
│      └── GZIPInputStream         (decorator)                        │
│                                                                      │
│   Used in real Java code:                                           │
│   new DataInputStream(                                              │
│       new BufferedInputStream(                                      │
│           new FileInputStream("file.txt")))                         │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 9. Decorator vs Inheritance — Summary

```
┌─────────────────────────────────────────────────────────────────────┐
│              INHERITANCE vs DECORATOR — DECISION GUIDE              │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   USE INHERITANCE WHEN:                                             │
│   • Features are fixed at compile time                              │
│   • Number of combinations is small                                 │
│   • "Is-a" relationship truly applies                               │
│                                                                      │
│   USE DECORATOR WHEN:                                               │
│   • Features should be added/removed at RUNTIME                     │
│   • Many combinations exist (combinatorial explosion)               │
│   • You want to keep "single responsibility" per feature            │
│   • You want to avoid class explosion                               │
│   • Composition is more natural than inheritance                    │
│                                                                      │
│   🎯 GOLDEN RULE: "Favor composition over inheritance"              │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 10. Key Takeaways

```
┌─────────────────────────────────────────────────────────────────────┐
│                    DECORATOR PATTERN — SUMMARY                      │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   🎯 PURPOSE                                                        │
│   Attach additional responsibilities to an object DYNAMICALLY.      │
│   Provides flexible alternative to subclassing.                     │
│                                                                      │
│   🧩 KEY COMPONENTS                                                 │
│   • IComponent     — interface/abstract base                        │
│   • ConcreteComponent — actual base object                          │
│   • Decorator      — abstract wrapper (IS-A + HAS-A)                │
│   • ConcreteDecorator — specific enhancement                        │
│                                                                      │
│   🔑 KEY INSIGHT                                                    │
│   Decorator uses BOTH inheritance AND composition:                  │
│   • IS-A → to be substitutable for the base                         │
│   • HAS-A → to delegate and wrap behavior                           │
│                                                                      │
│   ✅ BENEFITS                                                       │
│   • No class explosion                                              │
│   • Runtime flexibility                                             │
│   • Open/Closed Principle                                           │
│   • Single Responsibility per decorator                             │
│   • Unlimited combinations with few classes                         │
│                                                                      │
│   ⚠️ TRADE-OFFS                                                     │
│   • Many small classes                                              │
│   • Debugging a long decorator chain is harder                      │
│   • Order of decorators matters                                     │
│                                                                      │
│   🌍 REAL-WORLD USES                                                │
│   • Java I/O Streams (BufferedInputStream, DataInputStream)         │
│   • Text formatting (Bold, Italic, Underline)                       │
│   • Form validation chains                                          │
│   • HTTP middleware/authentication chains                           │
│   • Pizza/Coffee topping customization                              │
│                                                                      │
│   💡 THE ONE-LINE SUMMARY                                           │
│   "Wrap objects dynamically to add features without subclassing."   │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Interview Wisdom

The Decorator Pattern is:
- A **structural pattern** (deals with class/object composition)
- **Frequently asked** in LLD interviews, especially for text editors, form validation, and I/O streams
- Excellent answer to the question: **"How do you avoid class explosion?"**

---

## 14. Build Your Own Notification Engine (42:26)

This lecture is a **complete LLD interview walkthrough** for designing a **Notification System** — a classic interview problem. It combines **three design patterns**: Observer, Decorator, and Strategy, plus the Singleton pattern.

---

## 1. Requirements Gathering

### Functional & Non-Functional Requirements

```
┌─────────────────────────────────────────────────────────────────────┐
│                    NOTIFICATION SYSTEM REQUIREMENTS                 │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   ✅ PLUG AND PLAY MODEL                                            │
│      Integration into any application with minimal code changes     │
│                                                                      │
│   ✅ HIGHLY EXTENDABLE                                              │
│      Support SMS, Email, Popup now — WhatsApp tomorrow             │
│                                                                      │
│   ✅ DYNAMIC NOTIFICATION ENHANCEMENT                               │
│      Add headers, footers, signatures, timestamps at runtime       │
│                                                                      │
│   ✅ STORE ALL NOTIFICATIONS                                        │
│      Maintain history of all notifications sent                     │
│                                                                      │
│   ✅ LOGGING                                                        │
│      Log every notification (console for now, file later)          │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 2. Design Patterns Used

```
┌─────────────────────────────────────────────────────────────────────┐
│                    PATTERNS IN NOTIFICATION SYSTEM                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   ┌──────────────────────┐                                          │
│   │  DECORATOR PATTERN   │  → Dynamically enhance notification      │
│   │                      │    (add timestamp, signature, etc.)      │
│   └──────────────────────┘                                          │
│                                                                      │
│   ┌──────────────────────┐                                          │
│   │  OBSERVER PATTERN    │  → Notify all subscribers when a         │
│   │                      │    new notification is pushed            │
│   └──────────────────────┘                                          │
│                                                                      │
│   ┌──────────────────────┐                                          │
│   │  STRATEGY PATTERN    │  → Different delivery methods            │
│   │                      │    (Email, SMS, Popup)                   │
│   └──────────────────────┘                                          │
│                                                                      │
│   ┌──────────────────────┐                                          │
│   │  SINGLETON PATTERN   │  → NotificationService has one instance  │
│   │                      │    (single source of truth for history)  │
│   └──────────────────────┘                                          │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 3. UML Design — Building Bottom-Up

### 3.1 Notification Hierarchy (Decorator Pattern)

```
                    ┌──────────────────────────────┐
                    │      <<interface>>           │
                    │      INotification           │
                    ├──────────────────────────────┤
                    │ + getContent(): string = 0   │
                    └──────────────┬───────────────┘
                                   △
                    ┌──────────────┴───────────────┐
                    │                              │
                    ▼                              ▼
       ┌────────────────────────┐    ┌─────────────────────────────┐
       │  SimpleNotification    │    │  <<abstract>>               │
       ├────────────────────────┤    │  INotificationDecorator     │
       │ - text: string         │    ├─────────────────────────────┤
       ├────────────────────────┤    │ - notification: INotif*     │
       │ + getContent()         │    ├─────────────────────────────┤
       └────────────────────────┘    │ + getContent()              │
                                     └──────────────┬──────────────┘
                                                    △
                                     ┌──────────────┼──────────────┐
                                     │              │              │
                              ┌──────┴──────┐ ┌─────┴─────┐ ┌─────┴─────┐
                              │ Timestamp   │ │ Signature │ │ (Future   │
                              │ Decorator   │ │ Decorator │ │  Decors)  │
                              └─────────────┘ └───────────┘ └───────────┘
```

**Key Points:**
- `INotificationDecorator` uses **IS-A** (inherits `INotification`) + **HAS-A** (holds reference to `INotification`)
- Enables runtime stacking of decorators

### 3.2 Observer Pattern Components

```
                    ┌──────────────────────────────┐
                    │      <<interface>>           │
                    │      IObserver               │
                    ├──────────────────────────────┤
                    │ + update(): void = 0         │
                    └──────────────┬───────────────┘
                                   △
                    ┌──────────────┴───────────────┐
                    │                              │
                    ▼                              ▼
       ┌────────────────────────┐    ┌─────────────────────────────┐
       │      Logger            │    │   NotificationEngine        │
       ├────────────────────────┤    ├─────────────────────────────┤
       │ - observable: INotif*  │    │ - observable: IObservable*  │
       ├────────────────────────┤    │ - strategies: List<IStrat*> │
       │ + update()             │    ├─────────────────────────────┤
       └────────────────────────┘    │ + update()                  │
                                     │ + addStrategy(IStrategy*)   │
                                     └─────────────────────────────┘

                    ┌──────────────────────────────┐
                    │      <<interface>>           │
                    │      IObservable             │
                    ├──────────────────────────────┤
                    │ + add(IObserver*): void      │
                    │ + remove(IObserver*): void   │
                    │ + notify(): void             │
                    └──────────────┬───────────────┘
                                   △
                    ┌──────────────┴───────────────┐
                    │   NotificationObservable     │
                    ├──────────────────────────────┤
                    │ - observers: List<IObserver*>│
                    │ - notification: INotification│
                    ├──────────────────────────────┤
                    │ + add() / remove() / notify()│
                    │ + setNotification(INotif*)   │
                    │ + getNotification(): INotif* │
                    │ + getNotificationContent()   │
                    └──────────────────────────────┘
```

### 3.3 Strategy Pattern Components

```
                    ┌──────────────────────────────┐
                    │      <<interface>>           │
                    │   INotificationStrategy      │
                    ├──────────────────────────────┤
                    │ + sendNotification(str) = 0  │
                    └──────────────┬───────────────┘
                                   △
                    ┌──────────────┼──────────────┐
                    │              │              │
              ┌─────┴─────┐  ┌─────┴─────┐  ┌─────┴─────┐
              │  Email    │  │   SMS     │  │  Popup    │
              │ Strategy  │  │ Strategy  │  │ Strategy  │
              └───────────┘  └───────────┘  └───────────┘
```

### 3.4 Complete UML (Clean View)

```
┌─────────────────────────────────────────────────────────────────────┐
│                    NOTIFICATION SYSTEM — COMPLETE UML               │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   [NOTIFICATION HIERARCHY]                                          │
│   INotification ◀── SimpleNotification                              │
│        △                                                            │
│   INotificationDecorator                                            │
│        ├── TimestampDecorator                                       │
│        └── SignatureDecorator                                       │
│                                                                      │
│   [OBSERVER PATTERN]                                                │
│   IObserver ◀── Logger                                              │
│             ◀── NotificationEngine                                  │
│                                                                      │
│   IObservable ◀── NotificationObservable (holds INotification*)     │
│                                                                      │
│   [STRATEGY PATTERN]                                                │
│   INotificationStrategy ◀── EmailStrategy                           │
│                         ◀── SMSStrategy                             │
│                         ◀── PopupStrategy                           │
│                                                                      │
│   NotificationEngine HAS-A List<INotificationStrategy>              │
│                                                                      │
│   [SINGLETON]                                                       │
│   NotificationService (Singleton)                                   │
│   • Holds IObservable*                                              │
│   • Holds List<INotification> (history)                             │
│   • sendNotification(INotification*)                                │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 4. Data Flow

```
┌─────────────────────────────────────────────────────────────────────┐
│                    NOTIFICATION FLOW                                │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   Client                                                            │
│      │                                                              │
│      │ 1. Create INotification                                      │
│      │    new SignatureDecorator(                                   │
│      │      new TimestampDecorator(                                 │
│      │        new SimpleNotification("Your order shipped")))        │
│      │                                                              │
│      ▼                                                              │
│   NotificationService.sendNotification(notification)                │
│      │                                                              │
│      │ 2. Store in history list                                     │
│      │ 3. Call observable->setNotification(notification)            │
│      ▼                                                              │
│   NotificationObservable.setNotification()                          │
│      │                                                              │
│      │ 4. Internally calls notify()                                 │
│      ▼                                                              │
│   NotificationObservable.notify()                                   │
│      │                                                              │
│      │ 5. Loop through all observers, call update()                 │
│      ▼                                                              │
│   ┌───────────────────────────┬──────────────────────────────────┐  │
│   ▼                           ▼                                  ▼  │
│ Logger.update()      NotificationEngine.update()                     │
│   │                           │                                      │
│   │ Reads content             │ Reads content                       │
│   │ Logs to console           │ Loops through strategies            │
│   │                           ▼                                      │
│   │              ┌────────────┼────────────┐                        │
│   │              ▼            ▼            ▼                        │
│   │         Email.send()  SMS.send()  Popup.send()                  │
│   │              │            │            │                        │
│   │              ▼            ▼            ▼                        │
│   │         Email user    SMS user    Popup user                    │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 5. Code Implementation

### 5.1 Notification Hierarchy (Decorator Pattern)

```cpp
#include <iostream>
#include <vector>
#include <string>
#include <mutex>
using namespace std;

// ============ NOTIFICATION INTERFACE ============
class INotification {
public:
    virtual string getContent() = 0;
    virtual ~INotification() {}
};

// ============ CONCRETE NOTIFICATION ============
class SimpleNotification : public INotification {
    string text;
public:
    SimpleNotification(string msg) : text(msg) {}
    string getContent() override { return text; }
};

// ============ ABSTRACT DECORATOR ============
class INotificationDecorator : public INotification {
protected:
    INotification* notification;
public:
    INotificationDecorator(INotification* n) : notification(n) {}
    ~INotificationDecorator() { delete notification; }
    virtual string getContent() = 0;
};

// ============ CONCRETE DECORATORS ============
class TimestampDecorator : public INotificationDecorator {
public:
    TimestampDecorator(INotification* n) : INotificationDecorator(n) {}
    string getContent() override {
        return "[2024-01-15 10:30:00] " + notification->getContent();
    }
};

class SignatureDecorator : public INotificationDecorator {
    string signature;
public:
    SignatureDecorator(INotification* n, string sig)
        : INotificationDecorator(n), signature(sig) {}
    string getContent() override {
        return notification->getContent() + "\n-- " + signature;
    }
};
```

### 5.2 Observer Pattern Components

```cpp
// ============ OBSERVER INTERFACE ============
class IObserver {
public:
    virtual void update() = 0;
    virtual ~IObserver() {}
};

// ============ OBSERVABLE INTERFACE ============
class IObservable {
public:
    virtual void add(IObserver* obs) = 0;
    virtual void remove(IObserver* obs) = 0;
    virtual void notify() = 0;
    virtual ~IObservable() {}
};

// ============ CONCRETE OBSERVABLE ============
class NotificationObservable : public IObservable {
    vector<IObserver*> observers;
    INotification* currentNotification;
public:
    NotificationObservable() : currentNotification(nullptr) {}
    
    void add(IObserver* obs) override {
        observers.push_back(obs);
    }
    
    void remove(IObserver* obs) override {
        observers.erase(remove(observers.begin(), observers.end(), obs),
                       observers.end());
    }
    
    void notify() override {
        for (auto* obs : observers) obs->update();
    }
    
    void setNotification(INotification* n) {
        if (currentNotification) delete currentNotification;
        currentNotification = n;
        notify();
    }
    
    INotification* getNotification() { return currentNotification; }
    string getNotificationContent() {
        return currentNotification ? currentNotification->getContent() : "";
    }
    
    ~NotificationObservable() {
        if (currentNotification) delete currentNotification;
    }
};
```

### 5.3 Strategy Pattern (Notification Delivery)

```cpp
// ============ STRATEGY INTERFACE ============
class INotificationStrategy {
public:
    virtual void sendNotification(const string& content) = 0;
    virtual ~INotificationStrategy() {}
};

// ============ CONCRETE STRATEGIES ============
class EmailStrategy : public INotificationStrategy {
    string email;
public:
    EmailStrategy(string e) : email(e) {}
    void sendNotification(const string& content) override {
        cout << "Sending Email to " << email << ": " << content << "\n";
    }
};

class SMSStrategy : public INotificationStrategy {
    string mobile;
public:
    SMSStrategy(string m) : mobile(m) {}
    void sendNotification(const string& content) override {
        cout << "Sending SMS to " << mobile << ": " << content << "\n";
    }
};

class PopupStrategy : public INotificationStrategy {
public:
    void sendNotification(const string& content) override {
        cout << "Popup Notification: " << content << "\n";
    }
};
```

### 5.4 Concrete Observers

```cpp
// ============ LOGGER OBSERVER ============
class Logger : public IObserver {
    NotificationObservable* observable;
public:
    // Default constructor — gets observable from service
    Logger() {
        observable = NotificationService::getInstance()->getObservable();
        observable->add(this);  // Auto-register
    }
    
    Logger(NotificationObservable* obs) : observable(obs) {}
    
    void update() override {
        cout << "[LOG] New notification: "
             << observable->getNotificationContent() << "\n";
    }
};

// ============ NOTIFICATION ENGINE OBSERVER ============
class NotificationEngine : public IObserver {
    NotificationObservable* observable;
    vector<INotificationStrategy*> strategies;
public:
    NotificationEngine() {
        observable = NotificationService::getInstance()->getObservable();
        observable->add(this);  // Auto-register
    }
    
    void addStrategy(INotificationStrategy* s) {
        strategies.push_back(s);
    }
    
    void update() override {
        string content = observable->getNotificationContent();
        for (auto* s : strategies) {
            s->sendNotification(content);
        }
    }
};
```

### 5.5 Notification Service (Singleton)

```cpp
class NotificationService {
    static NotificationService* instance;
    NotificationObservable* observable;
    vector<INotification*> history;  // Store all notifications
    
    NotificationService() {
        observable = new NotificationObservable();
    }
    
public:
    NotificationService(const NotificationService&) = delete;
    NotificationService& operator=(const NotificationService&) = delete;
    
    static NotificationService* getInstance() {
        if (instance == nullptr) {
            instance = new NotificationService();
        }
        return instance;
    }
    
    NotificationObservable* getObservable() { return observable; }
    
    void sendNotification(INotification* n) {
        history.push_back(n);              // Store history
        observable->setNotification(n);    // Trigger observers
    }
    
    ~NotificationService() {
        delete observable;
        for (auto* n : history) delete n;
    }
};

NotificationService* NotificationService::instance = nullptr;
```

### 5.6 Client (Main)

```cpp
int main() {
    // 1. Get singleton service instance
    NotificationService* service = NotificationService::getInstance();
    
    // 2. Create observers (auto-register via default constructor)
    Logger* logger = new Logger();
    NotificationEngine* engine = new NotificationEngine();
    
    // 3. Configure engine with strategies
    engine->addStrategy(new EmailStrategy("user@example.com"));
    engine->addStrategy(new SMSStrategy("+91-9876543210"));
    engine->addStrategy(new PopupStrategy());
    
    // 4. Create notification with decorators
    INotification* notification = new SignatureDecorator(
        new TimestampDecorator(
            new SimpleNotification("Your order has been shipped!")),
        "Customer Care");
    
    // 5. Send notification
    service->sendNotification(notification);
    
    // 6. Cleanup
    delete logger;
    delete engine;
    // (service stays alive as singleton)
    return 0;
}
```

### 5.7 Output

```
[LOG] New notification: [2024-01-15 10:30:00] Your order has been shipped!
-- Customer Care

Sending Email to user@example.com: [2024-01-15 10:30:00] Your order has been shipped!
-- Customer Care

Sending SMS to +91-9876543210: [2024-01-15 10:30:00] Your order has been shipped!
-- Customer Care

Popup Notification: [2024-01-15 10:30:00] Your order has been shipped!
-- Customer Care
```

---

## 6. Key Design Insight: Plug-and-Play

### Before: Tightly Coupled Client

```
┌─────────────────────────────────────────────────────────────────────┐
│                    BEFORE — CLIENT DOES TOO MUCH                    │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   Client must:                                                      │
│   • Create the observable                                           │
│   • Create observers                                                │
│   • Attach observers to observable                                  │
│   • Pass observable to observers                                    │
│   • Then finally send notification                                  │
│                                                                      │
│   ❌ Client knows too much about internal structure                 │
│   ❌ Not plug-and-play                                              │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### After: Plug-and-Play with `this` Auto-Registration

```
┌─────────────────────────────────────────────────────────────────────┐
│                    AFTER — CLIENT DOES MINIMUM                      │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   In Logger Constructor:                                            │
│   ────────────────────────                                          │
│   Logger() {                                                        │
│       observable = NotificationService::getInstance()->getObservable│
│       observable->add(this);  // ← Auto-register                    │
│   }                                                                 │
│                                                                      │
│   In NotificationEngine Constructor:                                │
│   ────────────────────────────────────                              │
│   NotificationEngine() {                                            │
│       observable = NotificationService::getInstance()->getObservable│
│       observable->add(this);  // ← Auto-register                    │
│   }                                                                 │
│                                                                      │
│   Client code:                                                      │
│   ────────────                                                      │
│   NotificationService* service = NotificationService::getInstance();│
│   Logger* logger = new Logger();          // Auto-registers!        │
│   NotificationEngine* engine = new NotificationEngine(); // Auto   │
│   engine->addStrategy(...);                                         │
│   service->sendNotification(notification);                          │
│                                                                      │
│   ✅ Truly plug-and-play!                                           │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 7. Design Principles Applied

| Principle | How Applied |
|-----------|-------------|
| **SRP** | Each class has one job (Notification = content, Decorator = enhancement, Observer = reaction, Strategy = delivery) |
| **OCP** | New notification type → new subclass; new delivery method → new strategy |
| **LSP** | All concrete decorators substitutable for `INotification` |
| **ISP** | Small, focused interfaces (`INotification`, `IObserver`, `IStrategy`) |
| **DIP** | `NotificationEngine` depends on `INotificationStrategy`, not concrete strategies |

---

## 8. Extension Points

### How to Add a New Notification Type

```cpp
// Just create a new concrete notification — no existing code changes!
class HTMLNotification : public INotification {
    string htmlContent;
public:
    HTMLNotification(string html) : htmlContent(html) {}
    string getContent() override { return htmlContent; }
};
```

### How to Add a New Decorator

```cpp
class BoldDecorator : public INotificationDecorator {
public:
    BoldDecorator(INotification* n) : INotificationDecorator(n) {}
    string getContent() override {
        return "**" + notification->getContent() + "**";
    }
};
```

### How to Add a New Delivery Strategy (e.g., WhatsApp)

```cpp
class WhatsAppStrategy : public INotificationStrategy {
    string number;
public:
    WhatsAppStrategy(string n) : number(n) {}
    void sendNotification(const string& content) override {
        cout << "WhatsApp to " << number << ": " << content << "\n";
    }
};

// In main:
engine->addStrategy(new WhatsAppStrategy("+91-9876543210"));
```

**No changes needed to:** `NotificationService`, `NotificationEngine`, `NotificationObservable`, or any existing class!

---

## 9. Key Takeaways

```
┌─────────────────────────────────────────────────────────────────────┐
│                    NOTIFICATION SYSTEM — SUMMARY                    │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   🎯 THE THREE PATTERNS WORKING TOGETHER                            │
│                                                                      │
│   1. DECORATOR → Enhances the notification content dynamically     │
│      (add timestamp, signature, bold, etc.)                         │
│                                                                      │
│   2. OBSERVER → Broadcasts notification to multiple consumers      │
│      (Logger, NotificationEngine)                                   │
│                                                                      │
│   3. STRATEGY → Selects delivery channel at runtime                │
│      (Email, SMS, Popup, WhatsApp)                                  │
│                                                                      │
│   4. SINGLETON → Single source of truth for the service            │
│      (history, observable)                                          │
│                                                                      │
│   💡 KEY INSIGHT                                                    │
│   Design patterns don't work in isolation — they compose!           │
│   The best LLD solutions combine multiple patterns.                 │
│                                                                      │
│   ✅ ACHIEVEMENTS                                                   │
│   • Plug-and-play model with auto-registration                      │
│   • Extensible — add features without modifying existing code       │
│   • Dynamically enhanced notifications                              │
│   • History maintained by singleton                                 │
│   • Logging integrated through observer                             │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Interview Wisdom

> **"As you practice more LLD problems, you'll start seeing patterns everywhere — 'Ah, Observer fits here', 'Decorator fits there'. That's the real skill."**

The lecture emphasizes:
- **Start with requirements gathering** — ask counter-questions
- **Build bottom-up** — smaller objects first, then compose
- **Use UML before code** — keeps you and interviewer on same page
- **Apply design patterns naturally** — don't force them
- **Refactor for plug-and-play** — client should know minimum

### The Meta-Lesson

This problem beautifully demonstrates that **real-world LLD problems rarely use just one pattern**. The Notification System combines:
- **Decorator** for content enhancement
- **Observer** for broadcasting
- **Strategy** for delivery mechanisms
- **Singleton** for service management

The **NotificationService** acts as a **bridge** connecting:
- The **notification creation flow** (Decorator pattern)
- The **notification delivery flow** (Observer + Strategy patterns)

---

## 15. Command Design Pattern | Real-world use case + Code (29:54)

This lecture covers the **Command Design Pattern** — a behavioral pattern that encapsulates a request as an object, thereby allowing you to parameterize clients with different requests, queue or log requests, and support undoable operations. The classic example used is a **Smart Home Automation System**.

---

## 1. What is Command Pattern?

> **"Encapsulate a request as an object, thereby letting you parameterize clients with different requests, queue or log requests, and support undoable operations."**

### Core Idea

```
┌─────────────────────────────────────────────────────────────────────┐
│                    COMMAND PATTERN — CORE IDEA                      │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   Instead of directly calling a method on a receiver,               │
│   wrap the request in a Command object.                             │
│                                                                      │
│   Source (Invoker) ──▶ Command ──▶ Receiver                         │
│                                                                      │
│   • Source doesn't know Receiver                                    │
│   • Command knows Receiver and what to do                           │
│   • Receiver does the actual work                                   │
│                                                                      │
│   Benefits: Loose coupling, dynamic assignment, undo support        │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 2. The Problem: Smart Home Automation

### Scenario

Design a **remote control** that can control various smart home devices:
- Light (On/Off)
- Fan (On/Off)
- AC (On/Off)

### ❌ Naive Approach (Without Command Pattern)

```
┌─────────────────────────────────────────────────────────────────────┐
│                    NAIVE APPROACH — TIGHT COUPLING                  │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   ┌─────────────────────┐                                           │
│   │      Remote         │                                           │
│   ├─────────────────────┤                                           │
│   │ - light: Light      │                                           │
│   │ - fan: Fan          │                                           │
│   │ - ac: AC            │                                           │
│   ├─────────────────────┤                                           │
│   │ + pressLightButton()│                                           │
│   │ + pressFanButton()  │                                           │
│   │ + pressACButton()   │                                           │
│   └─────────────────────┘                                           │
│                                                                      │
│   Problems:                                                         │
│   ❌ Remote is tightly coupled to concrete devices                  │
│   ❌ Adding new device → modify Remote class                        │
│   ❌ Violates Open/Closed Principle                                 │
│   ❌ Cannot dynamically reassign buttons at runtime                 │
│   ❌ No support for undo                                            │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

**Code Example (Bad Design):**

```cpp
class Light {
public:
    void on()  { cout << "Light is ON\n"; }
    void off() { cout << "Light is OFF\n"; }
};

class Fan {
public:
    void on()  { cout << "Fan is ON\n"; }
    void off() { cout << "Fan is OFF\n"; }
};

class Remote {
    Light* light;
    Fan* fan;
public:
    Remote(Light* l, Fan* f) : light(l), fan(f) {}
    
    void pressLightButton() { light->on(); }
    void pressFanButton()   { fan->on(); }
};
```

**Problem:** If you want to change a button to control a different device, you must modify the `Remote` class.

---

## 3. The Solution: Command Pattern

### Key Insight

Introduce a **Command** object between the **Invoker** (Remote) and the **Receiver** (Light/Fan). The invoker calls `execute()` on the command, which then calls the appropriate method on the receiver.

```
┌─────────────────────────────────────────────────────────────────────┐
│                    COMMAND PATTERN SOLUTION                         │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   ┌─────────────┐      ┌─────────────┐      ┌─────────────┐         │
│   │   Remote    │─────▶│   Command   │─────▶│  Receiver   │         │
│   │ (Invoker)   │      │  (Object)   │      │ (Light/Fan) │         │
│   └─────────────┘      └─────────────┘      └─────────────┘         │
│                                                                      │
│   Remote only knows Command interface                                │
│   Command knows which Receiver to call and how                      │
│   Receiver does the actual work                                     │
│                                                                      │
│   ✅ Loose coupling                                                 │
│   ✅ Dynamic assignment                                             │
│   ✅ Open/Closed Principle                                          │
│   ✅ Undo support                                                   │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 4. UML Design

### 4.1 Components

```
                    ┌──────────────────────────────┐
                    │      <<interface>>           │
                    │       ICommand               │
                    ├──────────────────────────────┤
                    │ + execute(): void = 0        │
                    │ + undo(): void = 0           │
                    └──────────────┬───────────────┘
                                   △
                    ┌──────────────┴───────────────┐
                    │                              │
                    ▼                              ▼
       ┌────────────────────────┐    ┌─────────────────────────────┐
       │    LightCommand        │    │      FanCommand             │
       ├────────────────────────┤    ├─────────────────────────────┤
       │ - light: Light*        │    │ - fan: Fan*                 │
       ├────────────────────────┤    ├─────────────────────────────┤
       │ + execute()            │    │ + execute()                 │
       │ + undo()               │    │ + undo()                    │
       └────────────────────────┘    └─────────────────────────────┘
                    │                              │
                    │ has-a                        │ has-a
                    ▼                              ▼
       ┌────────────────────────┐    ┌─────────────────────────────┐
       │        Light           │    │           Fan               │
       ├────────────────────────┤    ├─────────────────────────────┤
       │ + on()                 │    │ + on()                      │
       │ + off()                │    │ + off()                     │
       └────────────────────────┘    └─────────────────────────────┘

                    ┌──────────────────────────────┐
                    │          Remote              │
                    ├──────────────────────────────┤
                    │ - commands: ICommand*[]      │
                    │ - pressed: bool[]            │
                    ├──────────────────────────────┤
                    │ + setCommand(idx, ICommand*) │
                    │ + pressButton(idx)           │
                    └──────────────────────────────┘
                                   │ has-a (1..*)
                                   ▼
                              ICommand
```

### 4.2 Key Relationships

- **Remote** HAS-A `ICommand` (array of commands)
- **LightCommand** HAS-A `Light` (concrete receiver)
- **LightCommand** IS-A `ICommand` (inheritance)
- **ICommand** defines `execute()` and `undo()`

---

## 5. Code Implementation

### 5.1 Interfaces and Receivers

```cpp
#include <iostream>
#include <vector>
#include <string>
using namespace std;

// ============ COMMAND INTERFACE ============
class ICommand {
public:
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual ~ICommand() {}
};

// ============ RECEIVERS ============
class Light {
    string name;
public:
    Light(string n) : name(n) {}
    void on()  { cout << name << " Light is ON\n"; }
    void off() { cout << name << " Light is OFF\n"; }
};

class Fan {
    string name;
public:
    Fan(string n) : name(n) {}
    void on()  { cout << name << " Fan is ON\n"; }
    void off() { cout << name << " Fan is OFF\n"; }
};
```

### 5.2 Concrete Commands

```cpp
// ============ LIGHT COMMAND ============
class LightCommand : public ICommand {
    Light* light;
public:
    LightCommand(Light* l) : light(l) {}
    
    void execute() override { light->on(); }
    void undo() override    { light->off(); }
};

// ============ FAN COMMAND ============
class FanCommand : public ICommand {
    Fan* fan;
public:
    FanCommand(Fan* f) : fan(f) {}
    
    void execute() override { fan->on(); }
    void undo() override    { fan->off(); }
};
```

### 5.3 Invoker (Remote Control)

```cpp
// ============ REMOTE CONTROL ============
class Remote {
    static const int NUM_BUTTONS = 4;
    ICommand* commands[NUM_BUTTONS];
    bool pressed[NUM_BUTTONS];
    
public:
    Remote() {
        for (int i = 0; i < NUM_BUTTONS; i++) {
            commands[i] = nullptr;
            pressed[i] = false;
        }
    }
    
    void setCommand(int idx, ICommand* cmd) {
        if (idx < 0 || idx >= NUM_BUTTONS) return;
        if (commands[idx]) delete commands[idx];
        commands[idx] = cmd;
        pressed[idx] = false;
    }
    
    void pressButton(int idx) {
        if (idx < 0 || idx >= NUM_BUTTONS || commands[idx] == nullptr) {
            cout << "No command assigned at button " << idx << "\n";
            return;
        }
        
        if (!pressed[idx]) {
            commands[idx]->execute();
            pressed[idx] = true;
        } else {
            commands[idx]->undo();
            pressed[idx] = false;
        }
    }
    
    ~Remote() {
        for (int i = 0; i < NUM_BUTTONS; i++) {
            delete commands[i];
        }
    }
};
```

### 5.4 Main (Client)

```cpp
int main() {
    // Create receivers
    Light* livingRoomLight = new Light("Living Room");
    Fan* ceilingFan = new Fan("Ceiling");
    
    // Create remote
    Remote* remote = new Remote();
    
    // Assign commands to buttons
    remote->setCommand(0, new LightCommand(livingRoomLight));
    remote->setCommand(1, new FanCommand(ceilingFan));
    
    // Simulate pressing buttons
    cout << "--- Pressing Button 0 (Light) ---\n";
    remote->pressButton(0);  // Light ON
    remote->pressButton(0);  // Light OFF (undo)
    
    cout << "\n--- Pressing Button 1 (Fan) ---\n";
    remote->pressButton(1);  // Fan ON
    remote->pressButton(1);  // Fan OFF (undo)
    
    cout << "\n--- Pressing Button 2 (Unassigned) ---\n";
    remote->pressButton(2);  // No command assigned
    
    // Cleanup
    delete remote;
    delete livingRoomLight;
    delete ceilingFan;
    return 0;
}
```

### 5.5 Output

```
--- Pressing Button 0 (Light) ---
Living Room Light is ON
Living Room Light is OFF

--- Pressing Button 1 (Fan) ---
Ceiling Fan is ON
Ceiling Fan is OFF

--- Pressing Button 2 (Unassigned) ---
No command assigned at button 2
```

---

## 6. Undo Functionality

The pattern naturally supports undo by having each command implement both `execute()` and `undo()`. The invoker can track the state (e.g., `pressed` array) to decide whether to execute or undo.

```
┌─────────────────────────────────────────────────────────────────────┐
│                    UNDO MECHANISM                                   │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   Button pressed?    → Action                                       │
│   ────────────────    ────────                                      │
│   false               → execute()                                   │
│   true                → undo()                                      │
│                                                                      │
│   Each command knows:                                               │
│   • execute() → light.on(), fan.on(), etc.                          │
│   • undo()    → light.off(), fan.off(), etc.                        │
│                                                                      │
│   For more complex operations (e.g., text editor), you can store    │
│   previous state or use a stack of commands to undo multiple steps. │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 7. Why Concrete Command Holds Concrete Receiver (LSP Discussion)

A common question: **Why doesn't `ICommand` have a `has-a` relationship with a general `Appliance` interface? Why do concrete commands hold concrete receivers?**

**Answer:**

```
┌─────────────────────────────────────────────────────────────────────┐
│                    LSP AND RECEIVER DESIGN                          │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   If we tried to use a general Appliance interface:                 │
│                                                                      │
│   class Appliance {                                                 │
│       virtual void on() = 0;                                        │
│       virtual void off() = 0;                                       │
│   };                                                                │
│                                                                      │
│   class Light : public Appliance { ... };                           │
│   class Fan : public Appliance { ... };                             │
│   class AC : public Appliance {                                     │
│       void on() override;                                           │
│       void off() override;                                          │
│       void setTemperature(int) ;  // extra methods!                 │
│       void setTimer(int);                                           │
│   };                                                                │
│                                                                      │
│   ❌ Problem: AC has many features beyond on/off.                   │
│   ❌ The Appliance interface cannot capture all of them.            │
│   ❌ LSP violation: AC is not fully substitutable for Appliance     │
│      because it has extra behavior and constraints.                 │
│                                                                      │
│   ✅ Solution: Concrete commands hold concrete receivers.           │
│   • LightCommand knows Light and uses its on/off.                   │
│   • FanCommand knows Fan and uses its on/off.                       │
│   • ACCommand would know AC and use all its specific methods.       │
│                                                                      │
│   This keeps the Command interface simple (execute/undo)            │
│   and avoids forcing a one-size-fits-all Appliance interface.       │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

This design choice avoids LSP violations and keeps the system flexible.

---

## 8. Real-World Use Cases

| Use Case | How Command Pattern Helps |
|----------|---------------------------|
| **Text Editors / IDEs** | Undo/Redo operations (Ctrl+Z, Ctrl+Y) |
| **Photoshop / Image Editors** | Undo/Redo of filters, transformations |
| **Keyboard Shortcuts** | Assign commands to keys dynamically |
| **Smart Home Automation** | Remote controls with configurable buttons |
| **Transaction Systems** | Rollback on failure |
| **GUI Buttons** | Each button triggers a command object |
| **Job Queues / Task Schedulers** | Queue commands for later execution |
| **Macro Recording** | Record sequence of commands and replay |

---

## 9. How Command Pattern Solves Inheritance Problems (Simple Example)

### The Inheritance Problem: Class Explosion

Suppose you want to support multiple devices and multiple actions. Using inheritance, you might create:

```
Remote
├── LightRemote
├── FanRemote
├── ACRemote
├── LightAndFanRemote
├── LightAndACRemote
└── ... (combinatorial explosion)
```

**This is the same class explosion problem seen in the Decorator pattern.**

### Command Pattern Solution: Composition Over Inheritance

Instead of creating subclasses for every combination, the `Remote` **has-a** collection of `Command` objects. Each command **has-a** receiver.

```
Remote HAS-A ICommand[]
LightCommand HAS-A Light
FanCommand HAS-A Fan
```

**Benefits:**
- **No class explosion** — N commands instead of 2^N subclasses
- **Runtime flexibility** — Assign commands dynamically
- **Open/Closed** — New commands don't modify Remote
- **Single Responsibility** — Each command handles one action

### Simple Example: Coffee Machine

**❌ Inheritance Approach:**
```
CoffeeMachine
├── CoffeeWithMilk
├── CoffeeWithSugar
├── CoffeeWithMilkAndSugar
└── ... (explosion)
```

**✅ Command Pattern Approach:**
```
ICommand
├── AddMilkCommand
├── AddSugarCommand
└── ...

CoffeeMachine (Invoker) has-a list of commands
```
You can execute any combination at runtime without new classes.

---

## 10. Definition and Key Takeaways

### Official Definition

> **"Encapsulate a request as an object, thereby letting you parameterize clients with different requests, queue or log requests, and support undoable operations."**

### Key Takeaways

```
┌─────────────────────────────────────────────────────────────────────┐
│                    COMMAND PATTERN — SUMMARY                        │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   🎯 PURPOSE                                                        │
│   Encapsulate a request as an object, decoupling the invoker        │
│   from the receiver.                                                │
│                                                                      │
│   🧩 KEY COMPONENTS                                                 │
│   • Command (interface) — declares execute() and undo()             │
│   • ConcreteCommand — binds a Receiver to an action                 │
│   • Receiver — knows how to perform the work                        │
│   • Invoker — asks the command to carry out the request             │
│   • Client — creates commands and sets their receivers              │
│                                                                      │
│   ✅ BENEFITS                                                       │
│   • Loose coupling between invoker and receiver                     │
│   • Dynamic assignment of commands                                  │
│   • Supports undo/redo                                              │
│   • Open/Closed Principle                                           │
│   • No class explosion (composition over inheritance)               │
│                                                                      │
│   ⚠️ TRADE-OFFS                                                     │
│   • More classes (one per command)                                  │
│   • Slightly more complex to set up                                 │
│                                                                      │
│   🌍 REAL-WORLD USES                                                │
│   • Undo/Redo in editors                                            │
│   • Keyboard shortcuts                                              │
│   • Smart home remotes                                              │
│   • Transaction rollback                                            │
│   • Task queues                                                     │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### The Golden Rule

> **"When you need to parameterize objects with operations, queue operations, or support undo, use the Command Pattern."**

### Interview Wisdom

The Command Pattern is **very common in LLD interviews** and real-world applications. It's the go-to pattern for any system requiring:
- Undo/Redo
- Configurable buttons/shortcuts
- Decoupling request senders from receivers

It elegantly demonstrates **composition over inheritance** and helps avoid class explosion.

---

## 16. Adapter Design Pattern | Real-world use case + Code (21:52)

This lecture covers the **Adapter Design Pattern** — a structural pattern that allows two incompatible interfaces to work together. The classic real-world analogy is a **plug adapter** that lets an Indian charger fit into a US socket.

---

## 1. Real-Life Analogy

```
┌─────────────────────────────────────────────────────────────────────┐
│                    REAL-LIFE ADAPTER                                │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   Indian Charger (Type C)  ──▶  Adapter  ──▶  US Socket (Type A)    │
│                                                                      │
│   • Your charger has a Type C plug                                  │
│   • The wall socket expects a Type A plug                           │
│   • They can't connect directly                                     │
│   • An adapter bridges the two                                      │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

**In programming:** The Adapter pattern lets classes with incompatible interfaces collaborate.

---

## 2. The Problem: Incompatible Interfaces

### Scenario

Your application (`Existing Code`) needs JSON data from a report. However, a third-party library (`XMLDataProvider`) only provides XML data. The two cannot talk directly because their interfaces are different.

```
┌─────────────────────────────────────────────────────────────────────┐
│                    INCOMPATIBLE INTERFACES                          │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   ┌─────────────────────┐         ┌─────────────────────┐           │
│   │   Existing Code     │         │  XMLDataProvider    │           │
│   │   (Client)          │         │  (Third-Party)      │           │
│   ├─────────────────────┤         ├─────────────────────┤           │
│   │ + getJsonData()     │         │ + getXmlData()      │           │
│   └─────────────────────┘         └─────────────────────┘           │
│              │                               │                       │
│              └───────────┬───────────────────┘                       │
│                          │                                           │
│                    ❌ Cannot talk directly                           │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### Why Not Just Modify Existing Code?

- **Tight coupling:** Existing code becomes dependent on the third-party library.
- **Vendor lock-in:** Switching to another library later requires changes throughout the codebase.
- **Violates Open/Closed Principle:** Modifying existing code for every new integration.

---

## 3. The Solution: Adapter Pattern

Introduce an **Adapter** class that sits between the client and the third-party library. The adapter implements the interface the client expects and internally calls the third-party library.

```
┌─────────────────────────────────────────────────────────────────────┐
│                    ADAPTER SOLUTION                                 │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   ┌─────────────┐      ┌─────────────┐      ┌─────────────┐         │
│   │   Client    │─────▶│   Adapter   │─────▶│  Adaptee    │         │
│   │ (Existing)  │      │             │      │ (Third-Party)│        │
│   └─────────────┘      └─────────────┘      └─────────────┘         │
│                                                                      │
│   • Client calls Adapter's method (e.g., getJsonData)               │
│   • Adapter calls Adaptee's method (e.g., getXmlData)               │
│   • Adapter converts the result to what Client expects              │
│                                                                      │
│   ✅ Loose coupling                                                 │
│   ✅ Open/Closed Principle                                          │
│   ✅ Easy to swap third-party libraries                             │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 4. UML Diagram (Object Adapter)

```
                    ┌──────────────────────────────┐
                    │      <<interface>>           │
                    │        IReport               │
                    ├──────────────────────────────┤
                    │ + getJsonData(string): string│
                    └──────────────┬───────────────┘
                                   △
                                   │ implements
                    ┌──────────────┴───────────────┐
                    │  XMLDataProviderAdapter      │
                    ├──────────────────────────────┤
                    │ - xmlProvider: XMLDataProvider│
                    ├──────────────────────────────┤
                    │ + getJsonData(string)        │
                    └──────────────┬───────────────┘
                                   │ has-a (composition)
                                   ▼
                    ┌──────────────────────────────┐
                    │    XMLDataProvider           │
                    ├──────────────────────────────┤
                    │ + getXmlData(string): string │
                    └──────────────────────────────┘

   Client ─── uses ───▶ IReport
```

**Key Relationships:**
- Adapter **implements** the Target interface (`IReport`).
- Adapter **has-a** reference to the Adaptee (`XMLDataProvider`).
- Client only knows about `IReport`.

---

## 5. Code Example: JSON vs XML

### 5.1 Target Interface

```cpp
#include <iostream>
#include <string>
#include <sstream>
using namespace std;

// Target interface – what the client expects
class IReport {
public:
    virtual string getJsonData(const string& rawData) = 0;
    virtual ~IReport() {}
};
```

### 5.2 Adaptee (Third-Party Library)

```cpp
// Third-party library – provides XML data
class XMLDataProvider {
public:
    string getXmlData(const string& rawData) {
        // Simulate conversion from raw string to XML
        // Example: "Alice,42" -> "<name>Alice</name><id>42</id>"
        size_t comma = rawData.find(',');
        string name = rawData.substr(0, comma);
        string id = rawData.substr(comma + 1);
        return "<name>" + name + "</name><id>" + id + "</id>";
    }
};
```

### 5.3 Adapter

```cpp
// Adapter – implements IReport, wraps XMLDataProvider
class XMLDataProviderAdapter : public IReport {
    XMLDataProvider* xmlProvider;
public:
    XMLDataProviderAdapter(XMLDataProvider* provider) 
        : xmlProvider(provider) {}
    
    string getJsonData(const string& rawData) override {
        // 1. Get XML from adaptee
        string xmlData = xmlProvider->getXmlData(rawData);
        
        // 2. Convert XML to JSON (simple example)
        // Extract name and id from XML
        size_t nameStart = xmlData.find("<name>") + 6;
        size_t nameEnd = xmlData.find("</name>");
        string name = xmlData.substr(nameStart, nameEnd - nameStart);
        
        size_t idStart = xmlData.find("<id>") + 4;
        size_t idEnd = xmlData.find("</id>");
        string id = xmlData.substr(idStart, idEnd - idStart);
        
        // 3. Return JSON
        return "{\"name\":\"" + name + "\",\"id\":" + id + "}";
    }
};
```

### 5.4 Client

```cpp
// Client – only knows about IReport
class Client {
public:
    void getReport(IReport* report, const string& rawData) {
        string json = report->getJsonData(rawData);
        cout << "JSON Data: " << json << endl;
    }
};
```

### 5.5 Main

```cpp
int main() {
    // Create adaptee
    XMLDataProvider* xmlProvider = new XMLDataProvider();
    
    // Create adapter with adaptee
    IReport* adapter = new XMLDataProviderAdapter(xmlProvider);
    
    // Client uses adapter
    Client client;
    client.getReport(adapter, "Alice,42");
    
    // Cleanup
    delete adapter;
    delete xmlProvider;
    return 0;
}
```

**Output:**
```
JSON Data: {"name":"Alice","id":42}
```

---

## 6. Object Adapter vs. Class Adapter

### Object Adapter (Used above)

- **Uses composition:** Adapter **has-a** Adaptee.
- **Preferred approach:** Favors composition over inheritance.
- **Works in all languages** (Java, C++, etc.).

### Class Adapter

- **Uses multiple inheritance:** Adapter **inherits** from both Target and Adaptee.
- **Only possible in C++** (Java doesn't support multiple inheritance of classes).
- **Generally discouraged** because it tightly couples the adapter to a specific adaptee class.

```
┌─────────────────────────────────────────────────────────────────────┐
│                    CLASS ADAPTER (MULTIPLE INHERITANCE)             │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│                    ┌─────────────┐   ┌─────────────┐                │
│                    │   Target    │   │   Adaptee   │                │
│                    │ (interface) │   │   (class)   │                │
│                    └──────┬──────┘   └──────┬──────┘                │
│                           │                 │                        │
│                           └────────┬────────┘                        │
│                                    ▼                                 │
│                           ┌─────────────────┐                        │
│                           │  Class Adapter  │                        │
│                           │ (inherits both) │                        │
│                           └─────────────────┘                        │
│                                                                      │
│   ❌ Multiple inheritance (not supported in Java)                    │
│   ❌ Tightly coupled to Adaptee                                      │
│   ✅ Simpler if multiple inheritance is available                    │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

**Recommendation:** Always prefer **Object Adapter** (composition over inheritance).

---

## 7. How It Solves the Inheritance Problem (Simple Example)

### The Problem: Class Explosion with Inheritance

Suppose you have a base class `Report` and you want to support multiple data formats (JSON, XML, CSV). Using inheritance, you might create:

```
Report
├── JSONReport
├── XMLReport
├── CSVReport
└── ... (and combinations)
```

This leads to a **class explosion** when you add new formats or new data sources.

### The Adapter Solution: Composition

Instead of creating subclasses for each combination, the Adapter pattern uses **composition**:

- `XMLDataProviderAdapter` **has-a** `XMLDataProvider` and implements `IReport`.
- Adding a new format (e.g., CSV) means creating a new adapter, not modifying existing classes.
- No need to change the `Client` or the existing `IReport` interface.

**Simple Example: Coffee Machine**

- **Inheritance approach:** You'd need `CoffeeWithMilk`, `CoffeeWithSugar`, `CoffeeWithMilkAndSugar`, etc. – class explosion.
- **Adapter/Composition approach:** You have a `Coffee` base and a `MilkAdapter`, `SugarAdapter`. You compose them at runtime: `new SugarAdapter(new MilkAdapter(new Coffee()))`. No new classes for each combination.

Thus, the Adapter pattern (specifically the Object Adapter) demonstrates **"favor composition over inheritance"** and avoids the pitfalls of deep inheritance hierarchies.

---

## 8. Real-World Use Cases

| Use Case | Description |
|----------|-------------|
| **Third-Party Integration** | When integrating payment gateways, notification services, or any external library with a different interface. |
| **Legacy Code Integration** | When a modern application must communicate with an old legacy system that uses outdated interfaces. |
| **Data Format Conversion** | When you need to convert data from one format (XML) to another (JSON) without modifying the source. |
| **Java I/O Streams** | `InputStreamReader` and `OutputStreamWriter` are adapters that convert byte streams to character streams. |
| **GUI Frameworks** | Adapting different UI components to a common interface. |

---

## 9. Standard Definition

> **"The Adapter pattern converts the interface of a class into another interface that a client expects. Adapter lets classes work together that couldn't otherwise because of incompatible interfaces."**

---

## 10. Key Takeaways

```
┌─────────────────────────────────────────────────────────────────────┐
│                    ADAPTER PATTERN — SUMMARY                        │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│   🎯 PURPOSE                                                        │
│   Allow incompatible interfaces to work together.                   │
│                                                                      │
│   🧩 KEY COMPONENTS                                                 │
│   • Target (interface) – what the client expects                    │
│   • Adaptee – the existing class with incompatible interface        │
│   • Adapter – bridges the two, implements Target, holds Adaptee     │
│   • Client – uses Target                                            │
│                                                                      │
│   ✅ BENEFITS                                                       │
│   • Loose coupling between client and third-party code              │
│   • Open/Closed Principle – new adapters without modifying client   │
│   • Reusability – adapters can be reused across projects            │
│   • Promotes composition over inheritance (Object Adapter)          │
│                                                                      │
│   ⚠️ TRADE-OFFS                                                     │
│   • Adds an extra layer of indirection                              │
│   • Slightly more classes                                           │
│                                                                      │
│   🌍 REAL-WORLD USES                                                │
│   • Third-party library integration                                 │
│   • Legacy system integration                                       │
│   • Data format conversion                                          │
│   • Java I/O (InputStreamReader, OutputStreamWriter)                │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### The Golden Rule

> **"When you need to use an existing class but its interface doesn't match what your code expects, use the Adapter pattern."**

### Interview Wisdom

The Adapter pattern is **very common in LLD interviews** and real-world projects. It is the go-to solution for integrating third-party libraries, legacy code, or any incompatible interfaces. It elegantly demonstrates the principle of **"favor composition over inheritance"** through the Object Adapter variant.

---

## 17. Facade Design Pattern | Real-world use case + Code (18:52)

summaries system design tutorial transcript in details along with useful code examples and diagrams