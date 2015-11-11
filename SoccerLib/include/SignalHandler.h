#ifndef SIGNALHANDLER_H
#define SIGNALHANDLER_H

#include <stdexcept>

using std::runtime_error;

class SignalException : public runtime_error {
    public:
       SignalException(const std::string& _message) : std::runtime_error(_message) {}
};

class SignalHandler {
   
public:
    static void setup();
	static void handleSignal(int _ignored);
    static bool exitRequested;

};

#endif // SIGNALHANDLER_H
