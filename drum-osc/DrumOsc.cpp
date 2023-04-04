#include "daisy_seed.h"
#include "daisysp.h"





// Use the daisy namespace to prevent having to type
// daisy:: before all libdaisy functions
using namespace daisy;
using namespace daisysp;

// Declare a DaisySeed object called hardware
DaisySeed hardware;

Oscillator osc;
Oscillator osc2;
WhiteNoise noise;

AdEnv kickVolEnv, kickPitchEnv, oscEnv, snareEnv;

Switch kick, snare, oscbutton;

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size) {
    float osc_out, noise_out, snr_env_out, kck_env_out, sig;
    // Get rid of any bouncing
    snare.Debounce();
    kick.Debounce();
    oscbutton.Debounce()

    // If you press the kick button...
    if (kick.RisingEdge()) {
        // Trigger both envelopes!
        kickVolEnv.Trigger();
        kickPitchEnv.Trigger();
    }

    // Press the snare button trigger its envelope
    if (snare.RisingEdge()) {
        snareEnv.Trigger();
    }

// Press the osc button trigger its envelope
    if (oscbutton.RisingEdge()) {
        oscEnv.Trigger();
    }

    // Convert floating point knob to midi (0-127)
    // Then convert midi to freq. in Hz
    osc.SetFreq(mtof(hardware.adc.GetFloat(0) * 127))


    // Prepare the audio block
    for (size_t i = 0; i < size; i += 2) {
        // Get the next volume samples
        snr_env_out = snareEnv.Process();
        kck_env_out = kickVolEnv.Process();
        osc_env_out = oscEnv.Process();

        // Apply the pitch envelope to the kick
        osc.SetFreq(kickPitchEnv.Process());
        // Set the kick volume to the envelope's output
        osc.SetAmp(kck_env_out);
        // Process the next oscillator sample
        osc_out = osc.Process();

        // Process the oscillator for the third button
        osc2.SetAmp(osc_env_out);
        osc2_out = osc2.Process();

        // Get the next snare sample
        noise_out = noise.Process();
        // Set the sample to the correct volume
        noise_out *= snr_env_out;

        // Mix the two signals at half volume
        sig = .5 * noise_out + .5 * osc_out;

        // Set the left and right outputs to the mixed signals
        out[i]     = sig;
        out[i + 1] = sig;
    }
}

int main(void) {
    // Configure and Initialize the Daisy Seed
    // These are separate to allow reconfiguration of any of the internal
    // components before initialization.
    hardware.Configure();
    hardware.Init();
    hardware.SetAudioBlockSize(4);

    // Samples outputted per second
    float samplerate = hardware.AudioSampleRate();

    AdcChannelConfig adcConfig;

     // Add pin 21 as an analog input in this config. Use this to read the knob
    adcConfig.InitSingle(hardware.GetPin(21));

    // Knob for the oscillator
    oscbutton.Init(hardware.GetPin(26));

     // Set the ADC to use our configuration
    hardware.adc.Init(&adcConfig, 1);

    // Initialize oscillator for kickdrum
    osc.Init(samplerate);
    osc.SetWaveform(Oscillator::WAVE_TRI);
    osc.SetAmp(1);

    // Initialize noise
    noise.Init();

    // Osc 2
    osc2.Init(samplerate);
    osc2.SetWaveform(Oscillator::WAVE_SIN);
    osc2.SetAmp(1);
    osc2.SetFreq(1000);

    // Volume enveolope
    oscEnv.Init(samplerate)

    // Attack and decay times
    oscEnv.SetTime(ADENV_SEG_ATTACK, .01);
    oscEnv.SetTime(ADENV_SEG_DECAY, .4)

    // minimum and maximum envelope values
    oscEnv.SetMin(0.0);
    oscEnv.SetMax(1.f);
    oscEnv.SetCurve(0); // linear

    // Initialize envelopes, this one's for the snare amplitude
    snareEnv.Init(samplerate);
    snareEnv.SetTime(ADENV_SEG_ATTACK, .01);
    snareEnv.SetTime(ADENV_SEG_DECAY, .2);
    snareEnv.SetMax(1);
    snareEnv.SetMin(0);

    // This envelope will control the kick oscillator's pitch
    // Note that this envelope is much faster than the volume
    kickPitchEnv.Init(samplerate);
    kickPitchEnv.SetTime(ADENV_SEG_ATTACK, .01);
    kickPitchEnv.SetTime(ADENV_SEG_DECAY, .05);
    kickPitchEnv.SetMax(400);
    kickPitchEnv.SetMin(50);

    // This one will control the kick's volume
    kickVolEnv.Init(samplerate);
    kickVolEnv.SetTime(ADENV_SEG_ATTACK, .01);
    kickVolEnv.SetTime(ADENV_SEG_DECAY, 1);
    kickVolEnv.SetMax(1);
    kickVolEnv.SetMin(0);

    // Initialize the osc, kick, and snare buttons on pins 26,27 and 28
    // The callback rate is samplerate / blocksize (48)
    osc2.Init(hardware.GetPin(26), samplerate / 48.f);
    snare.Init(hardware.GetPin(27), samplerate / 48.f);
    kick.Init(hardware.GetPin(28), samplerate / 48.f);

     // Start the adc
    hardware.adc.Start();

    // Start calling the callback function
    hardware.StartAudio(AudioCallback);

    // Loop forever
    for(;;) {}
}
