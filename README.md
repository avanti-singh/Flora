# Flora

Flora is a granular and glitch effect plugin (AU, VST3 and standalone), written in C++
with the JUCE framework. It slices incoming audio into short grains and layers them back
together, then runs the result through bitcrushing glitch, delay and reverb effects. The
interface is drawn in a pixel-art garden style.

<img src="docs/flora-ui.png" alt="The Flora interface: a pixel-art garden with a Grain panel (Size, Density, Spray and Dry/Wet knobs, Forward/Backward/Both direction buttons and an Envelope menu) and an FX panel (Glitch Amount, Rate and Resolution, Delay and Reverb knobs and a Freeze button)" width="720">

## About

I made Flora in 2025 as my capstone project for the Electronic Production and Design
(EPD) major at [Berklee College of Music](https://www.berklee.edu/), and presented it at
the EPD showcase.

Rather than starting from a granular engine, I began by building a simple delay and worked
up from there, making sure I understood each component of a granulator before writing
my own.

## Controls

### Grain
- **Size**: grain length, 1 to 1000 ms.
- **Density**: how often new grains start.
- **Spray**: random offset of where each grain reads from in the recent audio.
- **Direction**: play grains forward, backward or a random mix of both.
- **Envelope**: the fade shape of each grain (Hann, Hamming, Percussive, Exponential
  Decay, Reverse Exponential Decay, Cosine).
- **Dry / Wet**: balance between the input and the granulated signal.

### FX
- **Glitch Amount**: mix of the glitch effect, a bitcrusher and sample-rate reducer.
- **Glitch Rate**: how fast the glitch switches on and off, from 1 to 100 times a second.
- **Glitch Res**: low settings crush the bit depth (down to 4-bit); high settings keep 16-bit but downsample harder.
- **Delay**: level of a 500 ms stereo feedback delay.
- **Reverb**: reverb level.
- **Freeze**: stops new audio entering the delay, so the current echoes keep looping.

## Building

Requires CMake 3.15 or later and Xcode on macOS. JUCE 8.0.6 is included as a git
submodule.

```bash
git clone --recurse-submodules https://github.com/avanti-singh/Flora.git
cd Flora
cmake -S . -B build -G Xcode -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build build --config Release --target Flora_VST3 Flora_AU Flora_Standalone
```

The build produces a universal (Apple Silicon and Intel) plugin and copies the AU and
VST3 into `~/Library/Audio/Plug-Ins/`.

## Project layout

```
Source/PluginProcessor.*   audio processing: grain engine, glitch, delay, reverb
Source/PluginEditor.*      interface
Source/RotaryKnob.*        image-based rotary knob
Images/                    background and knob artwork (compiled into the plugin)
JUCE/                      JUCE framework (submodule)
```

## Acknowledgements

- Professor [Jean-Luc Sinclair](https://www.linkedin.com/in/jean-luc-sinclair-32a253/),
  my capstone advisor, for his guidance and for believing in the project from the start.
- [Nikhil Deshpande](https://www.linkedin.com/in/nikhil-deshpande-97a7b21a4/), for
  mentoring me through the process and encouraging me to build a granulator from its
  parts up.
- My peers in EPD at Berklee, whose work inspired me and pushed me to grow.
