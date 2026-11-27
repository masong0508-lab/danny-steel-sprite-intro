# Danny Steele: continuous city expansion

Intent: make the town read as a real connected urban environment rather than a compact story stage. Preserve the existing protagonist, narrative, save coordinates and two camera modes. Do not replace gameplay with a rendered illustration or separate menu.

Approaches considered: scale only the old tile art (does not increase traversal); relocate all existing NPCs (breaks authored cinematic and save assumptions); extend the old district into physically continuous new streets (chosen, with original anchor locations maintained). The existing campaign remains centered on the Old Quarter during this stage. The new districts are explorable immediately; no new story gate.

The 40x30 plan connects the original 24x18 district to eastern avenues, a civic row, a southern park, canal crossings and river plaza. Seven eastern and seven southern gateway cells join the routes. Existing actor coordinates are unchanged. New elevations, facade identities, paving, water, park planting and fixed street furniture use the same perspective and isometric geometry. The original tile bank remains untouched; extended terrain selects bounded repeated material textures. A reverse flood cached per goal reduces walking-path recomputation; no new save format or framebuffer.

Acceptance: all 759 non-sealed walkable cells reachable; live movement between five new destinations and return to the campaign; original historical render fixtures preserved separately; expanded-map reference renderer comparisons, bounds and memory checks; GBA build and mGBA capture. The two pre-existing sealed rooms are not claimed accessible. The district extension adds walking and environmental observations, not yet new interior or mission geometry.
