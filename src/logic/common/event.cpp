#include "event.h"
#include <iostream>

EventManager::EventManager() {}

EventManager::~EventManager() {}

void EventManager::Register_event(EventType event, std::function<void()> callback)
{
    std::lock_guard<std::mutex> lock(mtx);
    callbacks_no_args[event].push_back(callback);
}

void EventManager::Register_event(EventType event, std::function<void(int)> callback)
{
    std::lock_guard<std::mutex> lock(mtx);
    callbacks_one_arg_int[event].push_back(callback);
}

void EventManager::Register_event(EventType event, std::function<void(const std::string&)> callback)
{
    std::lock_guard<std::mutex> lock(mtx);
    callbacks_one_arg_string[event].push_back(callback);
}

void EventManager::Register_event(EventType event, std::function<void(int, const std::string&)> callback)
{
    std::lock_guard<std::mutex> lock(mtx);
    callbacks_two_args[event].push_back(callback);
}

void EventManager::Unregister_event(EventType event)
{
    std::lock_guard<std::mutex> lock(mtx);
    callbacks_no_args.erase(event);
    callbacks_one_arg_int.erase(event);
    callbacks_one_arg_string.erase(event);
    callbacks_two_args.erase(event);
}

void EventManager::Trigger_event(EventType event)
{
    std::unique_lock<std::mutex> lock(mtx);
    auto it = callbacks_no_args.find(event);
    if(it != callbacks_no_args.end())
    {
        for(auto& callback : it->second)
        {
            lock.unlock();
            callback();
            lock.lock();
        }
    }
    cv.notify_all();
}

void EventManager::Trigger_event(EventType event, int value)
{
    std::unique_lock<std::mutex> lock(mtx);
    auto it = callbacks_one_arg_int.find(event);
    if(it != callbacks_one_arg_int.end())
    {
        for(auto& callback : it->second)
        {
            lock.unlock();
            callback(value);
            lock.lock();
        }
    }
    cv.notify_all();
}

void EventManager::Trigger_event(EventType event, const std::string& message)
{
    std::unique_lock<std::mutex> lock(mtx);
    auto it = callbacks_one_arg_string.find(event);
    if(it != callbacks_one_arg_string.end())
    {
        for(auto& callback : it->second)
        {
            lock.unlock();
            callback(message);
            lock.lock();
        }
    }
    cv.notify_all();
}

void EventManager::Trigger_event(EventType event, int value, const std::string& message)
{
    std::unique_lock<std::mutex> lock(mtx);
    auto it = callbacks_two_args.find(event);
    if(it != callbacks_two_args.end())
    {
        for(auto& callback : it->second)
        {
            lock.unlock();
            callback(value, message);
            lock.lock();
        }
    }
    cv.notify_all();
}
