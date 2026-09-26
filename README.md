
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

summaries system design tutorial transcript in details along with useful code examples and diagrams