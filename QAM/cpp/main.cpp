#include <iostream>
#include "ossie/ossieSupport.h"

#include "QAM.h"
int main(int argc, char* argv[])
{
    QAM_i* QAM_servant;
    Resource_impl::start_component(QAM_servant, argc, argv);
    return 0;
}

