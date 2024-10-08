#include <iostream>
#include <netdb.h>
#include <sys/epoll.h>
#include <signal.h>
#include <sys/socket.h>
#include <malloc.h>

#include "common/file_utility.h"
#include "common/runtime_receiver.h"
#include "common/exception/errno_exception.h"


int main() {
    std::cout << "RuntimeReceiver starting.\n";
    forward::common::RuntimeReceiver& runtime(forward::common::RuntimeReceiver::get_instance());
    try{
        runtime.initialize();
        runtime.run();
        runtime.wait_until_termination();
    } catch(const forward::common::exception::ErrnoException &e){
        std::cout <<  "signal handler was registered failed." << std::endl;
    } catch(const std::exception& e){
        std::cout <<  e.what() << std::endl;
    } catch(...){
        std::cout <<  "caught unknown exception" << std::endl;
    }
    runtime.shutdown();
    runtime.un_initialize();
    std::cout <<  "RuntimeForward exiting." << std::endl;
    return 0;
}
