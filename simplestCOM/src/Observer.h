#ifndef SIMPLEPUBSUB_H_
#define SIMPLEPUBSUB_H_

#include <list>
#include <algorithm>


namespace perri {

template <typename TDATA>
class ISubscriber;



template <typename TDATA>
class IPublisher {
public:
	IPublisher() = default;
	virtual ~IPublisher() = default;
	void Publish(TDATA data) const {
		for (auto &s : this->subs) {
			s->Notify(data);
		}
	}

private:
	std::list < ISubscriber <TDATA> * > subs;

	friend class ISubscriber <TDATA>;
	void Add(ISubscriber <TDATA> *sub) {
		if (std::find(this->subs.begin(), this->subs.end(), sub) == this->subs.end())
			this->subs.push_back(sub);
	}
	void Remove(ISubscriber <TDATA> *sub) {
		if (std::find(this->subs.begin(), this->subs.end(), sub) != this->subs.end())
			this->subs.remove(sub);
	}
};



template <typename TDATA>
class ISubscriber {
public:
	virtual void Notify(TDATA data) = 0;
	virtual ~ISubscriber() {}

protected:
	void Subscribe(IPublisher <TDATA> &pub) {
		pub.Add(this);
	}
	void Unsubscribe(IPublisher <TDATA> &pub) {
		pub.Remove(this);
	}
};

}

#endif /* SIMPLEPUBSUB_H_ */
