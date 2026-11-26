# Potheads source rotoscope refinement

Verified the recovered original 307530.mp4 (55,498,598 bytes, 1280x720,
548 decoded frames) against SHA256
9181173802c83b52e945ea2f7043eb3367ecbf78401c0f76bdf9911fca8971b8.

Reviewed actual decoded 640x360 analysis views at frames 5, 15, 25, 35,
45, 55, 65, 75, 85 and 95 with a coordinate grid. These ten additional
manual landmark estimates capture intermediate arm, foot, body, bag and
sign positions and more rapid sign changes between the original standing
keyframes. Mark concealed or uncertain joints as inferred. Frame 95
specifically has a lifted right foot; it must not receive a planted sole.
The track now contains 120 measured estimates (57 standing/sign,
63 floor-work); the first 100 source frames are sampled every five frames
instead of ten. The original 110 poses, source EOF, camera, dialogue clock,
sign-release and later floor-work annotations are preserved. These are
manual approximations, not a claim of frame-exact anatomy.

Smooth the authored costume's gravity response through horizontal torso
orientations instead of abruptly switching between upright and inverted
cloth. Test reproduced the old discontinuity at zero and passed with
the continuous Q8 blend. Adds 680 bytes of packed immutable ROM pose data,
no new framebuffer or save fields. Existing face likeness is preserved.

Independent rotoscope CI regenerates the full packed track byte-for-byte,
checks the intermediate source coverage and visibility, tests the drape
blend, and exercises every cinema tick using the native host painter.
The previous likeness branch had failed whole-game build and capture
workflows; their archived job logs were unavailable when queried. Do not
declare a native ROM or film verified without the fresh results. Original
video and private grid images remain outside the repository.
