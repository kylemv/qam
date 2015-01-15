#ifndef QAM_IMPL_H
#define QAM_IMPL_H

#include "QAM_base.h"
#include <liquid/liquid.h>
#include <complex>

class QAM_i : public QAM_base
{
    ENABLE_LOGGING
    public:
        QAM_i(const char *uuid, const char *label);
        ~QAM_i();
        int serviceFunction();
    private:
        void createModem(void);
        void bitRateChanged(const unsigned short *oldVal, const unsigned short *newVal);
		modem qam_modem;
		BULKIO::StreamSRI m_sriOut;
		double m_delta;

		boost::mutex propLock_;
};

#endif // QAM_IMPL_H
