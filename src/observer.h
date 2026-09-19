#ifndef OBSERVER_H
#define OBSERVER_H

class Observer {
	public:
		virtual ~Observer() = default;
		virtual void Notify() = 0;
};

#endif 