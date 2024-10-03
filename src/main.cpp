#include <iostream>
#include "common/runtime_forward.h"
#include "common/exception/errno_exception.h"

int main() {
    std::cout << "RuntimeForward starting.\n";
    forward::common::RuntimeForward& runtime(forward::common::RuntimeForward::get_instance());
    try{
        runtime.initialize();
        runtime.run();
        runtime.wait_until_termination();
    } catch(const forward::common::exception::ErrnoException &e){
        //MEGA_LOG_ERROR << "signal handler was registered failed.";
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
