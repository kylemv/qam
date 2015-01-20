#include <iostream>
#include "ossie/ossieSupport.h"

#include "QAM_Mod.h"
int main(int argc, char* argv[])
{
    QAM_Mod_i* QAM_Mod_servant;
    Resource_impl::start_component(QAM_Mod_servant, argc, argv);
    return 0;
}

