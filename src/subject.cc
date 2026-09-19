#include "subject.h"
#include "observer.h"

void Subject::Attach(Observer* observer) {
	observers.emplace_back(observer);
}

void Subject::Detach(Observer* observer) {
	for (auto ob = observers.begin(); ob != observers.end(); ++ob) {
		if (*ob == observer) {
			observers.erase(ob);
			break;
		}
	}
}

void Subject::NotifyObservers() {
	for (auto observer: observers) observer->Notify();
}