# Music Studio live-stage identity pass

The six-tab Advanced Studio remains an editable, cartridge-native PSG sequencer. It is not a recorded vocal or audio workstation: the LYRIC page provides timed textual cues and the other voices are Game Boy Advance sound generators.

## Changes

The LIVE/PERFORM screen now draws a separate miniature, instrument-specific stage for each of its four voices: a microphone with stand for LEAD, a miniature keyboard for HARM, a bass-shaped instrument for BASS, and a drum/cymbal rig for BEAT. A restrained paired lighting accent responds to the existing PSG gate bits, so the stage reads as a performance rather than four interchangeable meters. The transport/playhead, lane order, section names, lyric footer and controls are preserved.

## Safety and testing

No change to the patch data, arrangement, saved SRAM format, audio clocks, rendered 240×160 palette, global data buffers, number of voice lanes or event logic. When STOPPED, the stage is static and the meters remain dark. The LIVE gate pixels still occupy exactly the same positions as the previous test-proven renderer; CI tests additionally check three distinct instrument landmarks and stopped-stage determinism. The full CI build and emulator capture must pass before merge.

This is a native-pixel graphics refinement, not new PCM playback, a new voice synth, full game completion, or a verified physical GBA FPS measurement.