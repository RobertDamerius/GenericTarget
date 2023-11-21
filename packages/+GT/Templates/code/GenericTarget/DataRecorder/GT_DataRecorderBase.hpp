#pragma once


#include <GenericTarget/GT_Common.hpp>


namespace gt {


/**
 * @brief This abstract class represents the basics of a data recorder.
 */
class DataRecorderBase {
    public:
        /**
         * @brief A virtual destructor to destroy the data recorder object.
         */
        virtual ~DataRecorderBase(){}

        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Implement start and stop member functions for derived classes
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        /**
         * @brief Start the data recorder.
         * @param [in] filename The absolute filename of the log file.
         * @return True if success, false otherwise.
         */
        virtual bool Start(std::string filename) = 0;

        /**
         * @brief Stop the data recorder.
         */
        virtual void Stop(void) = 0;


        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Overwritable member functions for derived classes
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        virtual void Write(double simulationTime, double* values, uint32_t numValues){ (void)simulationTime; (void)values; (void)numValues; }
        virtual void Write(double simulationTime, uint8_t* bytes, uint32_t numBytes){ (void)simulationTime; (void)bytes; (void)numBytes; }
        virtual inline void SetNumSamplesPerFile(uint32_t numSamplesPerFile){ (void)numSamplesPerFile; }
        virtual inline void SetNumSignals(uint32_t numSignals){ (void)numSignals; }
        virtual inline void SetNumBytesPerSample(uint32_t numBytesPerSample){ (void)numBytesPerSample; }
        virtual inline void SetLabels(std::string labels){ (void)labels; }
        virtual inline void SetDimensions(std::string dimensions){ (void)dimensions; }
        virtual inline void SetDataTypes(std::string dataTypes){ (void)dataTypes; }
};


} /* namespace: gt */

