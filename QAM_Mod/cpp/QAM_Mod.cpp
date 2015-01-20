/**************************************************************************

    This is the component code. This file contains the child class where
    custom functionality can be added to the component. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "QAM_Mod.h"

PREPARE_LOGGING(QAM_Mod_i)

QAM_Mod_i::QAM_Mod_i(const char *uuid, const char *label) :
    QAM_Mod_base(uuid, label)
{
	addPropertyChangeListener("bitRate",this,&QAM_Mod_i::bitRateChanged);
	qam_mod = NULL;
	m_delta = 0;
	m_sriOut = bulkio::sri::create("QAM_OUT");
}

QAM_Mod_i::~QAM_Mod_i()
{
	if(qam_mod){
		modem_destroy(qam_mod);
		qam_mod = NULL;
	}
}

void QAM_Mod_i::bitRateChanged(const unsigned short *oldVal, const unsigned short *newVal)
{
	boost::mutex::scoped_lock lock(propLock_);

	if(	*newVal == 4 ||
		*newVal == 8 ||
		*newVal == 16 ||
		*newVal == 32 ||
		*newVal == 64 ||
		*newVal == 128 ||
		*newVal == 256)
		bitRate = *newVal;
	else{
		std::cerr<<"Invalid QAM bit rate entered.  Rate must be 4, 8, 16, 32, 64, 128,or 256."<<std::endl;
		std::cerr<<"--- Staying at QAM-"<<*oldVal<<std::endl;
		bitRate = *oldVal;
	}
	if(m_delta != 0)
		createModem();
}
/***********************************************************************************************

    Basic functionality:

        The service function is called by the serviceThread object (of type ProcessThread).
        This call happens immediately after the previous call if the return value for
        the previous call was NORMAL.
        If the return value for the previous call was NOOP, then the serviceThread waits
        an amount of time defined in the serviceThread's constructor.
        
    SRI:
        To create a StreamSRI object, use the following code:
                std::string stream_id = "testStream";
                BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);

	Time:
	    To create a PrecisionUTCTime object, use the following code:
                BULKIO::PrecisionUTCTime tstamp = bulkio::time::utils::now();

        
    Ports:

        Data is passed to the serviceFunction through the getPacket call (BULKIO only).
        The dataTransfer class is a port-specific class, so each port implementing the
        BULKIO interface will have its own type-specific dataTransfer.

        The argument to the getPacket function is a floating point number that specifies
        the time to wait in seconds. A zero value is non-blocking. A negative value
        is blocking.  Constants have been defined for these values, bulkio::Const::BLOCKING and
        bulkio::Const::NON_BLOCKING.

        Each received dataTransfer is owned by serviceFunction and *MUST* be
        explicitly deallocated.

        To send data using a BULKIO interface, a convenience interface has been added 
        that takes a std::vector as the data input

        NOTE: If you have a BULKIO dataSDDS or dataVITA49  port, you must manually call 
              "port->updateStats()" to update the port statistics when appropriate.

        Example:
            // this example assumes that the component has two ports:
            //  A provides (input) port of type bulkio::InShortPort called short_in
            //  A uses (output) port of type bulkio::OutFloatPort called float_out
            // The mapping between the port and the class is found
            // in the component base class header file

            bulkio::InShortPort::dataTransfer *tmp = short_in->getPacket(bulkio::Const::BLOCKING);
            if (not tmp) { // No data is available
                return NOOP;
            }

            std::vector<float> outputData;
            outputData.resize(tmp->dataBuffer.size());
            for (unsigned int i=0; i<tmp->dataBuffer.size(); i++) {
                outputData[i] = (float)tmp->dataBuffer[i];
            }

            // NOTE: You must make at least one valid pushSRI call
            if (tmp->sriChanged) {
                float_out->pushSRI(tmp->SRI);
            }
            float_out->pushPacket(outputData, tmp->T, tmp->EOS, tmp->streamID);

            delete tmp; // IMPORTANT: MUST RELEASE THE RECEIVED DATA BLOCK
            return NORMAL;

        If working with complex data (i.e., the "mode" on the SRI is set to
        true), the std::vector passed from/to BulkIO can be typecast to/from
        std::vector< std::complex<dataType> >.  For example, for short data:

            bulkio::InShortPort::dataTransfer *tmp = myInput->getPacket(bulkio::Const::BLOCKING);
            std::vector<std::complex<short> >* intermediate = (std::vector<std::complex<short> >*) &(tmp->dataBuffer);
            // do work here
            std::vector<short>* output = (std::vector<short>*) intermediate;
            myOutput->pushPacket(*output, tmp->T, tmp->EOS, tmp->streamID);

        Interactions with non-BULKIO ports are left up to the component developer's discretion

    Properties:
        
        Properties are accessed directly as member variables. For example, if the
        property name is "baudRate", it may be accessed within member functions as
        "baudRate". Unnamed properties are given the property id as its name.
        Property types are mapped to the nearest C++ type, (e.g. "string" becomes
        "std::string"). All generated properties are declared in the base class
        (QAM_Mod_base).
    
        Simple sequence properties are mapped to "std::vector" of the simple type.
        Struct properties, if used, are mapped to C++ structs defined in the
        generated file "struct_props.h". Field names are taken from the name in
        the properties file; if no name is given, a generated name of the form
        "field_n" is used, where "n" is the ordinal number of the field.
        
        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            //  - A boolean called scaleInput
              
            if (scaleInput) {
                dataOut[i] = dataIn[i] * scaleValue;
            } else {
                dataOut[i] = dataIn[i];
            }
            
        Callback methods can be associated with a property so that the methods are
        called each time the property value changes.  This is done by calling 
        addPropertyChangeListener(<property name>, this, &QAM_Mod_i::<callback method>)
        in the constructor.

        Callback methods should take two arguments, both const pointers to the value
        type (e.g., "const float *"), and return void.

        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            
        //Add to QAM_Mod.cpp
        QAM_Mod_i::QAM_Mod_i(const char *uuid, const char *label) :
            QAM_Mod_base(uuid, label)
        {
            addPropertyChangeListener("scaleValue", this, &QAM_Mod_i::scaleChanged);
        }

        void QAM_Mod_i::scaleChanged(const float *oldValue, const float *newValue)
        {
            std::cout << "scaleValue changed from" << *oldValue << " to " << *newValue
                      << std::endl;
        }
            
        //Add to QAM_Mod.h
        void scaleChanged(const float* oldValue, const float* newValue);
        

************************************************************************************************/
int QAM_Mod_i::serviceFunction()
{
	//get input data from dataFloat_in
	bulkio::InULongPort::dataTransfer *input = dataUlong_in->getPacket(bulkio::Const::BLOCKING);
	//check that the input stream is ok
	if(not input)
		return NOOP;
	std::vector<unsigned long>* preMod = (std::vector<unsigned long>*) &(input->dataBuffer);
	std::vector<std::complex<float> > output;

	unsigned int size = preMod->size();
	output.resize(size);

	{
		boost::mutex::scoped_lock lock(propLock_);

		if(input->sriChanged){
			//Change and push new SRI if necessary
			m_delta = input->SRI.xdelta;
			m_sriOut = input->SRI;
			m_sriOut.mode = 0;
			createModem();
			dataFloat_out->pushSRI(m_sriOut);
		}

		//Demodulate the input signal and store it in an output vector using [liquid] BPSK demod
		for(unsigned int i = 0; i < size; i++){
			modem_modulate(qam_mod, preMod->at(i), &output[i]);
		}
	}
	std::vector<float>* tmpOut = (std::vector<float>*) &output;
	dataFloat_out->pushPacket(*tmpOut, input->T, input->EOS, input->streamID);
	delete input;
	return NORMAL;
}

void QAM_Mod_i::createModem()
{
	if(qam_mod){
		modem_destroy(qam_mod);
		qam_mod = NULL;
	}
	switch(bitRate){
		case 4: qam_mod = modem_create(LIQUID_MODEM_QAM4); break;
		case 8: qam_mod = modem_create(LIQUID_MODEM_QAM8); break;
		case 16: qam_mod = modem_create(LIQUID_MODEM_QAM16); break;
		case 32: qam_mod = modem_create(LIQUID_MODEM_QAM32); break;
		case 64: qam_mod = modem_create(LIQUID_MODEM_QAM64); break;
		case 128: qam_mod = modem_create(LIQUID_MODEM_QAM128); break;
		case 256: qam_mod = modem_create(LIQUID_MODEM_QAM256); break;
	}
}
