#ifndef QAM_MOD_IMPL_H
#define QAM_MOD_IMPL_H

#include "QAM_Mod_base.h"
#include <liquid/liquid.h>

class QAM_Mod_i : public QAM_Mod_base
{
    ENABLE_LOGGING
    public:
        QAM_Mod_i(const char *uuid, const char *label);
        ~QAM_Mod_i();
        int serviceFunction();
    private:
		void createModem(void);
		void bitRateChanged(const unsigned short *oldVal, const unsigned short *newVal);
		modem qam_mod;
		BULKIO::StreamSRI m_sriOut;
		double m_delta;

		boost::mutex propLock_;
};

#endif // QAM_MOD_IMPL_H
