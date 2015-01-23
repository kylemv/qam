#ifndef QAM_IMPL_BASE_H
#define QAM_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Resource_impl.h>
#include <ossie/ThreadedComponent.h>

#include <bulkio/bulkio.h>

class QAM_base : public Resource_impl, protected ThreadedComponent
{
    public:
        QAM_base(const char *uuid, const char *label);
        ~QAM_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:
        // Member variables exposed as properties
        unsigned short bitRate;

        // Ports
        bulkio::InFloatPort *dataFloat_in;
        bulkio::OutULongPort *dataLong_out;

    private:
};
#endif // QAM_IMPL_BASE_H
