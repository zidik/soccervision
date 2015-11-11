#include <signal.h>
#include <errno.h>

#include "SignalHandler.h"

bool SignalHandler::exitRequested = false;

void SignalHandler::handleSignal(int _ignored) {
    exitRequested = true;
}

void SignalHandler::setup() {
    signal((int)SIGINT, SignalHandler::handleSignal);
}
