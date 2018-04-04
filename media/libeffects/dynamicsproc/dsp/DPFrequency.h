/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef DPFREQUENCY_H_
#define DPFREQUENCY_H_

#include <Eigen/Dense>
#include <unsupported/Eigen/FFT>

#include "RDsp.h"
#include "SHCircularBuffer.h"

#include "DPBase.h"


namespace dp_fx {

using FXBuffer = SHCircularBuffer<float>;

class ChannelBuffer {
public:
    FXBuffer cBInput;   // Circular Buffer input
    FXBuffer cBOutput;  // Circular Buffer output
    FloatVec input;     // time domain temp vector for input
    FloatVec output;    // time domain temp vector for output
    FloatVec outTail;   // time domain temp vector for output tail (for overlap-add method)

    //Current parameters
    float inputGainDb;
    struct BandParams {
        bool enabled;
        float freqCutoffHz;
        size_t binStart;
        size_t binStop;
    };
    struct EqBandParams : public BandParams {
        float gainDb;
    };
    struct MbcBandParams : public BandParams {
        float gainPreDb;
        float gainPostDb;
        float attackTimeMs;
        float releaseTimeMs;
        float ratio;
        float thresholdDb;
        float kneeWidthDb;
        float noiseGateThresholdDb;
        float expanderRatio;

        //Historic values
        float previousEnvelope;
    };

    bool mPreEqInUse;
    bool mPreEqEnabled;
    std::vector<EqBandParams> mPreEqBands;

    bool mMbcInUse;
    bool mMbcEnabled;
    std::vector<MbcBandParams> mMbcBands;

    bool mPostEqInUse;
    bool mPostEqEnabled;
    std::vector<EqBandParams> mPostEqBands;

    bool mLimiterInUse;
    bool mLimiterEnabled;
    FloatVec mPreEqFactorVector; // temp pre-computed vector to shape spectrum at preEQ stage
    FloatVec mPostEqFactorVector; // temp pre-computed vector to shape spectrum at postEQ stage

    void initBuffers(unsigned int blockSize, unsigned int overlapSize, unsigned int halfFftSize,
            unsigned int samplingRate, DPBase &dpBase);
    void computeBinStartStop(BandParams &bp, size_t binStart);
private:
    unsigned int mSamplingRate;
    unsigned int mBlockSize;

};

class DPFrequency : public DPBase {
public:
    virtual size_t processSamples(const float *in, float *out, size_t samples);
    virtual void reset();
    void configure(size_t blockSize, size_t overlapSize, size_t samplingRate);
    static size_t getMinBockSize();
    static size_t getMaxBockSize();

private:
    void updateParameters(ChannelBuffer &cb, int channelIndex);
    size_t processMono(ChannelBuffer &cb);
    size_t processOneVector(FloatVec &output, FloatVec &input, ChannelBuffer &cb);

    size_t mBlockSize;
    size_t mHalfFFTSize;
    size_t mOverlapSize;
    size_t mSamplingRate;

    std::vector<ChannelBuffer> mChannelBuffers;

    //dsp
    FloatVec mVWindow;  //window class.
    Eigen::VectorXcf mComplexTemp;
    Eigen::FFT<float> mFftServer;
};

} //namespace dp_fx

#endif  // DPFREQUENCY_H_
