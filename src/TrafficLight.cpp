#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
  	std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this] { return !_queue.empty(); });
    T msg = std::move(_queue.back());
    _queue.pop_back();
    
  	return msg;
  
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
  	std::lock_guard<std::mutex> uLock(_mutex);
    _queue.push_back(std::move(msg));
    _cond.notify_one();
  
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
  
    while (true) {
      
      TrafficLightPhase phase = _messages->receive();
     
      if (phase == TrafficLightPhase::green) {
        return;
      }
      
    }
  
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
  
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
  	threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
  
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
	// start stopwatch. Helped: https://github.com/MattB17/CppND-Program-a-Concurrent-Traffic-Simulation/blob/master/src/TrafficLight.cpp
  auto prevUpdate = std::chrono::system_clock::now();

  // generate a random number to determine when the traffic light should change.
  std::random_device rd;
  std::mt19937 eng(rd());
  std::uniform_int_distribution<> distr(4000.0, 6000.0);
 
  double ms = distr(eng);

  while(true) {
    // put thread to sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // compute the time since last change and update if it is above threshold.
    double elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now() - prevUpdate).count();
    
    if (elapsedTime >= ms) {
      
      if (_currentPhase == TrafficLightPhase::red) {
        _currentPhase = TrafficLightPhase::green;
      } 
      else {
        _currentPhase = TrafficLightPhase::red;
      }

      // move the phase into the queue and restart the clock.
      TrafficLightPhase phase = _currentPhase;
      _messages->send(std::move(phase));
      prevUpdate = std::chrono::system_clock::now();

      std::random_device rd;
      std::mt19937 eng(rd());
      ms = distr(eng);
    
    }
  
  }  

}

