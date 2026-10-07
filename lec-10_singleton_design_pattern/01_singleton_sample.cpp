#include<iostream>

using namespace std;

class Singleton {
public:

    Singleton() {
        cout << "Singleton Constructor called. New Object created" << endl;
    }
};

int main() {
    Singleton* s1 = new Singleton();
    Singleton* s2 = new Singleton();

    cout << (s1 == s2) << endl;
}