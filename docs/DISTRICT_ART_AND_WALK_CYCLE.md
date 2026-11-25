# District artwork and grounded character animation

This pass addresses the repeated roof and gold-window grid visible in the
last actual Old Quarter screenshots. Eight distinct 16×40 facade families
are repainted in their existing 10,240-byte ROM allocation: brick tenement,
stone arch, timber waterfront, artist workshop, commercial shop, boarded
shop, civic archive and Potheads. The actual central archive and east shop
row have coherent roof heights. Buildings are selected by district, not
independent random tiles; facade windows come from the authored wall.

Twelve new 32×16 diamond cels (6,144 bytes ROM) provide subdued asphalt
patches, staggered stone, slate/metal and flat roof materials. The original
fourteen cels and their public modulo behavior remain available unchanged.
No per-pixel procedural texture generation is added to the cartridge.

Danny receives four dedicated contact/passing cels per facing (11,968
bytes ROM). The old idle and two legacy cels remain byte-identical.
Walking now follows the actual interpolated movement phase and alternates
the leading leg by destination parity, rather than unrelated global frame
time. The original head, coat, bag, foot and shadow anchors are preserved.
Cinematic walking has eight contact/passing beats; nonwalking prop-contact
poses are not changed.

The new arrays are read-only cartridge ROM; the same pixel copiers and
framebuffer are reused. No new EWRAM/VRAM surfaces, collision, SRAM format,
raycasting geometry, palette or first-person normal-state changes.
Tests retain original golden art and scaling/clip contracts, add fixed
hashes for the new artwork, and verify all 16 walk cels, alternating phases,
planted soles and existing player visibility. Actual renderer captures
show the district artwork and each movement phase. GBA hardware FPS
still needs physical measurement; host and mGBA checks are not a claim
of full-rate rendering everywhere.

Foreground tree crowns and the extending Potheads sign now use the actual
moving-character overlap check, rather than hiding the player behind their
larger-than-tile silhouettes. They keep their physical anchors and collision.
