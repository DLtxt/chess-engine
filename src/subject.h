#ifndef SUBJECT_H
#define SUBJECT_H

#include <vector>

class Observer;

class Subject {
	std::vector<Observer*> observers;
	public:
		void Attach(Observer*);
		void Detach(Observer*);
		void NotifyObservers();
		virtual ~Subject() = default;
};

#endif