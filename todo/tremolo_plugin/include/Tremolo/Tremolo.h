#pragma once

namespace tremolo {
class Tremolo {
public:
  enum class LfoWaveform : size_t {
    sine = 0,
    triangle = 1,
  };

  Tremolo() {
    lfo.setFrequency(5.0f, true);
  }
  void prepare(double sampleRate, int expectedMaxFramesPerBlock) {
    const juce::dsp::ProcessSpec processSpec {
      .sampleRate = sampleRate,
      .maximumBlockSize = static_cast<juce::uint32>(expectedMaxFramesPerBlock),
      .numChannels = 1u,
    };
    lfo.prepare(processSpec);
  }

  // Sets the Lfo waveform shape.
  void setLfoWaveform(LfoWaveform waveform) noexcept {
    jassert(waveform == LfoWaveform::sine || waveform == LfoWaveform::triangle);
    lfoToSet = waveform;
  }

  void process(juce::AudioBuffer<float>& buffer) noexcept {

    // Apply any LFO waveform changes prior to processing this buffer.
    updateLfoWaveform();

    // for each frame
    for (const auto frameIndex : std::views::iota(0, buffer.getNumSamples())) {

      // TODO: generate the LFO value
      const auto lfoPhase = lfo.processSample(0.f);
      const auto lfoSine = -std::cos(lfoPhase);
      const auto lfoTriangle = std::abs(2.f * lfoPhase / juce::MathConstants<float>::pi) - 1.f;

      // TODO: calculate the modulation value
      constexpr auto modulationDepth = 0.4f;
      // const auto modulationValue = modulationDepth * lfoValue + 1.0f;
      // const auto modulationValue = modulationDepth * (0.5f * lfoValue - 0.5f) + 1.0f;
      const auto modulationValue = modulationDepth * (0.5f * lfoSine - 0.5f) + 1.0f;

      // for each channel sample in the frame
      for (const auto channelIndex :
           std::views::iota(0, buffer.getNumChannels())) {
        // get the input sample
        const auto inputSample = buffer.getSample(channelIndex, frameIndex);

        // TODO: modulate the sample
        const auto outputSample = inputSample * modulationValue;

        // set the output sample
        buffer.setSample(channelIndex, frameIndex, outputSample);
      }
    }
  }

  void reset() noexcept {
    lfo.reset();
  }

private:
  // You should put class members and private functions here

  // Used for deferring Lfo waveform updates.
  void updateLfoWaveform() noexcept {
    if (lfoToSet != currentLfo)
      currentLfo = lfoToSet;
  }

  // The Lfo itself.
  juce::dsp::Oscillator<float> lfo{[](auto phase){ return phase; }};

  // Lfo waveform initialized to sine.  Second variable used for deferring updates.
  LfoWaveform currentLfo = LfoWaveform::sine;
  LfoWaveform lfoToSet = currentLfo;
};
}  // namespace tremolo
